#include <windows.h>
#include <commctrl.h>
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
#define IDC_GROUP_CONFIG      1008
#define IDC_GROUP_PREVIEW     1009

// Estrutura para armazenar arquivos
typedef struct {
    char original[MAX_PATH];
    char novo[MAX_PATH];
} ArquivoRenomeado;

ArquivoRenomeado* arquivos = NULL;
int totalArquivos = 0;
HWND hListPreview;

// Função para ordenar arquivos (callback para qsort)
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

// Callback da janela
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            // Criar grupo de configuração
            HWND hGroupConfig = CreateWindow(WC_BUTTON, "Configuração",
                WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                10, 10, 450, 160, hwnd, (HMENU)IDC_GROUP_CONFIG, NULL, NULL);
            
            // Labels e campos
            CreateWindow(WC_STATIC, "Caminho da pasta:", WS_CHILD | WS_VISIBLE,
                20, 35, 120, 20, hwnd, NULL, NULL, NULL);
            
            CreateWindow(WC_EDIT, "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                20, 55, 340, 25, hwnd, (HMENU)IDC_EDIT_PATH, NULL, NULL);
            
            CreateWindow(WC_BUTTON, "Procurar...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                365, 55, 85, 25, hwnd, (HMENU)IDC_BTN_BROWSE, NULL, NULL);
            
            CreateWindow(WC_STATIC, "Extensão (ex: jpg):", WS_CHILD | WS_VISIBLE,
                20, 90, 120, 20, hwnd, NULL, NULL, NULL);
            
            CreateWindow(WC_EDIT, "jpg", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                20, 110, 200, 25, hwnd, (HMENU)IDC_EDIT_EXT, NULL, NULL);
            
            CreateWindow(WC_STATIC, "Nome base:", WS_CHILD | WS_VISIBLE,
                240, 90, 80, 20, hwnd, NULL, NULL, NULL);
            
            CreateWindow(WC_EDIT, "", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                240, 110, 200, 25, hwnd, (HMENU)IDC_EDIT_NEWNAME, NULL, NULL);
            
            // Botões
            CreateWindow(WC_BUTTON, "Visualizar Alterações", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                20, 145, 200, 30, hwnd, (HMENU)IDC_BTN_PREVIEW, NULL, NULL);
            
            CreateWindow(WC_BUTTON, "Renomear Arquivos", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                240, 145, 200, 30, hwnd, (HMENU)IDC_BTN_RENAME, NULL, NULL);
            
            // Grupo de preview
            CreateWindow(WC_BUTTON, "Preview das Alterações", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                10, 180, 450, 300, hwnd, (HMENU)IDC_GROUP_PREVIEW, NULL, NULL);
            
            // Lista de preview
            hListPreview = CreateWindow(WC_LISTBOX, "", WS_CHILD | WS_VISIBLE | WS_BORDER | 
                WS_VSCROLL | LBS_NOTIFY,
                20, 200, 430, 270, hwnd, (HMENU)IDC_LIST_PREVIEW, NULL, NULL);
            
            // Definir fonte
            HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
            
            SetFont(GetDlgItem(hwnd, IDC_EDIT_PATH), hFont);
            SetFont(GetDlgItem(hwnd, IDC_EDIT_EXT), hFont);
            SetFont(GetDlgItem(hwnd, IDC_EDIT_NEWNAME), hFont);
            SetFont(hListPreview, hFont);
            
            break;
        }
        
        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            
            if (wmId == IDC_BTN_BROWSE) {
                BROWSEINFO bi = {0};
                bi.lpszTitle = "Selecione a pasta com os arquivos";
                bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
                
                LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
                if (pidl != 0) {
                    char path[MAX_PATH];
                    SHGetPathFromIDList(pidl, path);
                    SetDlgItemText(hwnd, IDC_EDIT_PATH, path);
                    IMalloc* imalloc = 0;
                    SHGetMalloc(&imalloc);
                    imalloc->Free(pidl);
                    imalloc->Release();
                }
            }
            else if (wmId == IDC_BTN_PREVIEW) {
                char path[MAX_PATH];
                char extensao[50];
                char novoNome[100];
                
                GetDlgItemText(hwnd, IDC_EDIT_PATH, path, MAX_PATH);
                GetDlgItemText(hwnd, IDC_EDIT_EXT, extensao, 50);
                GetDlgItemText(hwnd, IDC_EDIT_NEWNAME, novoNome, 100);
                
                if (strlen(path) == 0 || strlen(extensao) == 0 || strlen(novoNome) == 0) {
                    MessageBox(hwnd, "Preencha todos os campos!", "Erro", MB_OK | MB_ICONERROR);
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
                
                char msg[100];
                snprintf(msg, sizeof(msg), "%d arquivos encontrados", totalArquivos);
                SetWindowText(hwnd, msg);
            }
            else if (wmId == IDC_BTN_RENAME) {
                if (totalArquivos == 0) {
                    MessageBox(hwnd, "Nenhuma pré-visualização gerada! Clique em 'Visualizar Alterações' primeiro.", 
                              "Aviso", MB_OK | MB_ICONWARNING);
                    return 0;
                }
                
                char path[MAX_PATH];
                GetDlgItemText(hwnd, IDC_EDIT_PATH, path, MAX_PATH);
                
                int result = MessageBox(hwnd, "Tem certeza que deseja renomear os arquivos?", 
                                        "Confirmar", MB_YESNO | MB_ICONQUESTION);
                if (result == IDYES) {
                    RenomearArquivos(path);
                    SendMessage(hListPreview, LB_RESETCONTENT, 0, 0);
                    totalArquivos = 0;
                    
                    // Limpar preview
                    if (arquivos) {
                        free(arquivos);
                        arquivos = NULL;
                    }
                }
            }
            break;
        }
        
        case WM_DESTROY: {
            if (arquivos) free(arquivos);
            PostQuitMessage(0);
            return 0;
        }
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Inicializar controles comuns
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);
    
    // Registrar classe da janela
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "RenomeadorArquivos";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    if (!RegisterClass(&wc)) return 1;
    
    // Criar janela principal
    HWND hwnd = CreateWindowEx(0, "RenomeadorArquivos", "Renomeador de Arquivos em Lote",
        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 480, 530,
        NULL, NULL, hInstance, NULL);
    
    if (!hwnd) return 1;
    
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