#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

// Definições de controles
#define IDC_EDIT_PATH         1001
#define IDC_EDIT_EXT          1002
#define IDC_EDIT_NEWNAME      1003
#define IDC_BTN_BROWSE        1004
#define IDC_BTN_PREVIEW       1005
#define IDC_BTN_RENAME        1006
#define IDC_LIST_PREVIEW      1007
#define IDC_STATIC_PATH       1008
#define IDC_STATIC_EXT        1009
#define IDC_STATIC_NEWNAME    1010

// Estrutura para armazenar arquivos
typedef struct {
    char original[MAX_PATH];
    char novo[MAX_PATH];
} ArquivoRenomeado;

ArquivoRenomeado* arquivos = NULL;
int totalArquivos = 0;
HWND hListPreview;
HWND hEditPath, hEditExt, hEditNewName;

// Função para ordenar arquivos
int CompareStrings(const void* a, const void* b) {
    return strcmp(((ArquivoRenomeado*)a)->original, ((ArquivoRenomeado*)b)->original);
}

// Função para carregar arquivos da pasta
int CarregarArquivos(const char* path, const char* extensao) {
    DIR* dir;
    struct dirent* entry;
    struct stat st;
    char fullPath[MAX_PATH];
    int count = 0;
    
    if (arquivos) {
        free(arquivos);
        arquivos = NULL;
    }
    
    dir = opendir(path);
    if (!dir) return 0;
    
    // Primeira passagem: contar arquivos
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        
        snprintf(fullPath, sizeof(fullPath), "%s\\%s", path, entry->d_name);
        if (stat(fullPath, &st) == 0 && S_ISREG(st.st_mode)) {
            char* ext = strrchr(entry->d_name, '.');
            if (ext && stricmp(ext + 1, extensao) == 0) {
                count++;
            }
        }
    }
    
    if (count == 0) {
        closedir(dir);
        return 0;
    }
    
    // Alocar memória
    arquivos = (ArquivoRenomeado*)malloc(count * sizeof(ArquivoRenomeado));
    if (!arquivos) {
        closedir(dir);
        return 0;
    }
    
    // Segunda passagem: armazenar nomes
    rewinddir(dir);
    int index = 0;
    while ((entry = readdir(dir)) != NULL && index < count) {
        if (entry->d_name[0] == '.') continue;
        
        snprintf(fullPath, sizeof(fullPath), "%s\\%s", path, entry->d_name);
        if (stat(fullPath, &st) == 0 && S_ISREG(st.st_mode)) {
            char* ext = strrchr(entry->d_name, '.');
            if (ext && stricmp(ext + 1, extensao) == 0) {
                strcpy(arquivos[index].original, entry->d_name);
                index++;
            }
        }
    }
    
    closedir(dir);
    
    // Ordenar arquivos
    qsort(arquivos, count, sizeof(ArquivoRenomeado), CompareStrings);
    
    return count;
}

// Função para gerar novos nomes
void GerarNovosNomes(const char* baseName, int startNumber) {
    char novoNome[MAX_PATH];
    
    for (int i = 0; i < totalArquivos; i++) {
        char* ext = strrchr(arquivos[i].original, '.');
        snprintf(novoNome, sizeof(novoNome), "%s_%03d%s", baseName, startNumber + i, ext);
        strcpy(arquivos[i].novo, novoNome);
    }
}

// Função para atualizar a lista de preview
void AtualizarPreview(HWND hList) {
    SendMessage(hList, LB_RESETCONTENT, 0, 0);
    
    char previewText[512];
    for (int i = 0; i < totalArquivos; i++) {
        snprintf(previewText, sizeof(previewText), "%s  →  %s", 
                 arquivos[i].original, arquivos[i].novo);
        SendMessage(hList, LB_ADDSTRING, 0, (LPARAM)previewText);
    }
}

// Função para renomear arquivos
void RenomearArquivos(const char* path) {
    char oldPath[MAX_PATH];
    char newPath[MAX_PATH];
    int successCount = 0;
    
    for (int i = 0; i < totalArquivos; i++) {
        snprintf(oldPath, sizeof(oldPath), "%s\\%s", path, arquivos[i].original);
        snprintf(newPath, sizeof(newPath), "%s\\%s", path, arquivos[i].novo);
        
        if (rename(oldPath, newPath) == 0) {
            successCount++;
        }
    }
    
    char msg[256];
    snprintf(msg, sizeof(msg), "%d de %d arquivos renomeados com sucesso!", 
             successCount, totalArquivos);
    MessageBox(NULL, msg, "Resultado", MB_OK | MB_ICONINFORMATION);
}

// Procedimento da janela principal
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            // Criar fonte
            HFONT hFont = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            
            // Labels
            CreateWindow("STATIC", "Caminho da pasta:", WS_CHILD | WS_VISIBLE,
                20, 20, 120, 20, hwnd, (HMENU)IDC_STATIC_PATH, NULL, NULL);
            
            CreateWindow("STATIC", "Extensão (sem ponto):", WS_CHILD | WS_VISIBLE,
                20, 70, 120, 20, hwnd, (HMENU)IDC_STATIC_EXT, NULL, NULL);
            
            CreateWindow("STATIC", "Nome base:", WS_CHILD | WS_VISIBLE,
                20, 120, 80, 20, hwnd, (HMENU)IDC_STATIC_NEWNAME, NULL, NULL);
            
            // Edit boxes
            hEditPath = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                20, 40, 300, 25, hwnd, (HMENU)IDC_EDIT_PATH, NULL, NULL);
            
            hEditExt = CreateWindow("EDIT", "jpg", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                20, 90, 150, 25, hwnd, (HMENU)IDC_EDIT_EXT, NULL, NULL);
            
            hEditNewName = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                20, 140, 300, 25, hwnd, (HMENU)IDC_EDIT_NEWNAME, NULL, NULL);
            
            // Botão Procurar
            CreateWindow("BUTTON", "Procurar...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                330, 40, 80, 25, hwnd, (HMENU)IDC_BTN_BROWSE, NULL, NULL);
            
            // Botões de ação
            CreateWindow("BUTTON", "Visualizar Alterações", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 180, 180, 35, hwnd, (HMENU)IDC_BTN_PREVIEW, NULL, NULL);
            
            CreateWindow("BUTTON", "Renomear Arquivos", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                210, 180, 180, 35, hwnd, (HMENU)IDC_BTN_RENAME, NULL, NULL);
            
            // Lista de preview
            hListPreview = CreateWindow("LISTBOX", "", WS_CHILD | WS_VISIBLE | WS_BORDER | 
                WS_VSCROLL | LBS_NOTIFY,
                20, 230, 400, 300, hwnd, (HMENU)IDC_LIST_PREVIEW, NULL, NULL);
            
            // Aplicar fonte aos controles
            SendMessage(hEditPath, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hEditExt, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hEditNewName, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessage(hListPreview, WM_SETFONT, (WPARAM)hFont, TRUE);
            
            break;
        }
        
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            
            if (wmId == IDC_BTN_BROWSE) {
                char folderPath[MAX_PATH] = "";
                BROWSEINFO bi = {0};
                bi.hwndOwner = hwnd;
                bi.lpszTitle = "Selecione a pasta com os arquivos";
                bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
                
                LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
                if (pidl != NULL) {
                    if (SHGetPathFromIDList(pidl, folderPath)) {
                        SetWindowText(hEditPath, folderPath);
                    }
                    CoTaskMemFree(pidl);
                }
            }
            else if (wmId == IDC_BTN_PREVIEW) {
                char path[MAX_PATH];
                char extensao[50];
                char novoNome[100];
                
                GetWindowText(hEditPath, path, MAX_PATH);
                GetWindowText(hEditExt, extensao, 50);
                GetWindowText(hEditNewName, novoNome, 100);
                
                if (strlen(path) == 0) {
                    MessageBox(hwnd, "Selecione uma pasta!", "Erro", MB_OK | MB_ICONERROR);
                    return 0;
                }
                if (strlen(extensao) == 0) {
                    MessageBox(hwnd, "Informe a extensão dos arquivos!", "Erro", MB_OK | MB_ICONERROR);
                    return 0;
                }
                if (strlen(novoNome) == 0) {
                    MessageBox(hwnd, "Informe o nome base para os arquivos!", "Erro", MB_OK | MB_ICONERROR);
                    return 0;
                }
                
                totalArquivos = CarregarArquivos(path, extensao);
                if (totalArquivos == 0) {
                    MessageBox(hwnd, "Nenhum arquivo encontrado com a extensão especificada!", 
                              "Aviso", MB_OK | MB_ICONWARNING);
                    SendMessage(hListPreview, LB_RESETCONTENT, 0, 0);
                    return 0;
                }
                
                GerarNovosNomes(novoNome, 1);
                AtualizarPreview(hListPreview);
            }
            else if (wmId == IDC_BTN_RENAME) {
                if (totalArquivos == 0) {
                    MessageBox(hwnd, "Gere uma pré-visualização primeiro!", 
                              "Aviso", MB_OK | MB_ICONWARNING);
                    return 0;
                }
                
                char path[MAX_PATH];
                GetWindowText(hEditPath, path, MAX_PATH);
                
                if (MessageBox(hwnd, "Confirmar renomeação dos arquivos?", 
                              "Confirmar", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    RenomearArquivos(path);
                    
                    // Limpar após renomear
                    SendMessage(hListPreview, LB_RESETCONTENT, 0, 0);
                    totalArquivos = 0;
                    if (arquivos) {
                        free(arquivos);
                        arquivos = NULL;
                    }
                }
            }
            break;
        }
        
        case WM_DESTROY:
            if (arquivos) free(arquivos);
            PostQuitMessage(0);
            break;
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

// Ponto de entrada principal
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Registrar a classe da janela
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "RenomeadorArquivosClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Falha ao registrar a classe da janela!", "Erro", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    // Criar a janela principal
    HWND hwnd = CreateWindow(
        "RenomeadorArquivosClass",
        "Renomeador de Arquivos em Lote",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 460, 580,
        NULL, NULL, hInstance, NULL);
    
    if (!hwnd) {
        MessageBox(NULL, "Falha ao criar a janela!", "Erro", MB_OK | MB_ICONERROR);
        return 1;
    }
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    // Loop de mensagens
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return msg.wParam;
}