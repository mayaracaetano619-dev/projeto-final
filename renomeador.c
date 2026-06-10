#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>

// Declarações globais
HWND hEditCaminho, hEditNomeBase, hListBox, hStatus;
char pastaAtual[MAX_PATH];
int arquivosEncontrados = 0;

// Função para listar arquivos na pasta
void ListarArquivos(const char* caminho) {
    DIR *dir;
    struct dirent *entry;
    char caminhoCompleto[MAX_PATH];
    
    // Limpar ListBox
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    arquivosEncontrados = 0;
    
    dir = opendir(caminho);
    if (dir == NULL) {
        SetWindowText(hStatus, "Erro: Não foi possível abrir a pasta!");
        return;
    }
    
    while ((entry = readdir(dir)) != NULL) {
        // Pular . e ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        sprintf(caminhoCompleto, "%s\\%s", caminho, entry->d_name);
        
        // Verificar se é arquivo (não diretório)
        DWORD atributos = GetFileAttributes(caminhoCompleto);
        if (atributos != INVALID_FILE_ATTRIBUTES && !(atributos & FILE_ATTRIBUTE_DIRECTORY)) {
            SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)entry->d_name);
            arquivosEncontrados++;
        }
    }
    
    closedir(dir);
    
    char statusMsg[200];
    sprintf(statusMsg, "Encontrados %d arquivos na pasta", arquivosEncontrados);
    SetWindowText(hStatus, statusMsg);
}

// Função para renomear arquivos
void RenomearArquivos() {
    char caminho[MAX_PATH];
    char nomeBase[100];
    char extensao[50];
    char novoNome[MAX_PATH];
    char caminhoCompleto[MAX_PATH];
    char novoCaminho[MAX_PATH];
    int contador = 1;
    int sucesso = 0;
    int falha = 0;
    
    // Obter texto dos campos
    GetWindowText(hEditCaminho, caminho, MAX_PATH);
    GetWindowText(hEditNomeBase, nomeBase, 100);
    
    if (strlen(caminho) == 0 || strlen(nomeBase) == 0) {
        MessageBox(NULL, "Por favor, preencha o caminho e o nome base!", "Erro", MB_OK | MB_ICONERROR);
        return;
    }
    
    DIR *dir;
    struct dirent *entry;
    
    dir = opendir(caminho);
    if (dir == NULL) {
        MessageBox(NULL, "Caminho inválido!", "Erro", MB_OK | MB_ICONERROR);
        return;
    }
    
    char statusMsg[500];
    sprintf(statusMsg, "Renomeando arquivos...\nNome base: %s\n", nomeBase);
    SetWindowText(hStatus, statusMsg);
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        sprintf(caminhoCompleto, "%s\\%s", caminho, entry->d_name);
        
        // Verificar se é arquivo
        DWORD atributos = GetFileAttributes(caminhoCompleto);
        if (atributos != INVALID_FILE_ATTRIBUTES && !(atributos & FILE_ATTRIBUTE_DIRECTORY)) {
            // Extrair extensão
            char* ponto = strrchr(entry->d_name, '.');
            if (ponto != NULL) {
                strcpy(extensao, ponto);
            } else {
                strcpy(extensao, "");
            }
            
            // Criar novo nome
            sprintf(novoNome, "%s_%03d%s", nomeBase, contador, extensao);
            sprintf(novoCaminho, "%s\\%s", caminho, novoNome);
            
            // Renomear arquivo
            if (rename(caminhoCompleto, novoCaminho) == 0) {
                sucesso++;
            } else {
                falha++;
            }
            
            contador++;
        }
    }
    
    closedir(dir);
    
    // Mostrar resultado
    char resultado[300];
    sprintf(resultado, "Operação concluída!\nSucesso: %d arquivos\nFalha: %d arquivos", sucesso, falha);
    MessageBox(NULL, resultado, "Resultado", MB_OK | MB_ICONINFORMATION);
    
    // Atualizar lista
    ListarArquivos(caminho);
}

// Procedimento da janela principal
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_CREATE: {
            // Criar labels e campos de texto
            CreateWindow("STATIC", "Seja bem-vindo(a) ao Organizador de Arquivos!",
                        WS_VISIBLE | WS_CHILD, 20, 20, 400, 25, hwnd, NULL, NULL, NULL);
            
            CreateWindow("STATIC", "Este programa irá ajudar você a renomear vários arquivos de uma vez.",
                        WS_VISIBLE | WS_CHILD, 20, 45, 400, 20, hwnd, NULL, NULL, NULL);
            
            CreateWindow("STATIC", "Caminho da pasta:",
                        WS_VISIBLE | WS_CHILD, 20, 80, 120, 20, hwnd, NULL, NULL, NULL);
            
            hEditCaminho = CreateWindow("EDIT", "",
                        WS_VISIBLE | WS_CHILD | WS_BORDER, 140, 78, 300, 25, hwnd, NULL, NULL, NULL);
            
            CreateWindow("BUTTON", "Listar Arquivos",
                        WS_VISIBLE | WS_CHILD, 450, 78, 120, 25, hwnd, (HMENU)1, NULL, NULL);
            
            CreateWindow("STATIC", "Nome base para renomear:",
                        WS_VISIBLE | WS_CHILD, 20, 120, 160, 20, hwnd, NULL, NULL, NULL);
            
            hEditNomeBase = CreateWindow("EDIT", "",
                        WS_VISIBLE | WS_CHILD | WS_BORDER, 180, 118, 260, 25, hwnd, NULL, NULL, NULL);
            
            CreateWindow("BUTTON", "Renomear Arquivos",
                        WS_VISIBLE | WS_CHILD, 450, 118, 120, 25, hwnd, (HMENU)2, NULL, NULL);
            
            // ListBox para mostrar arquivos
            CreateWindow("STATIC", "Arquivos na pasta:",
                        WS_VISIBLE | WS_CHILD, 20, 160, 150, 20, hwnd, NULL, NULL, NULL);
            
            hListBox = CreateWindow("LISTBOX", "",
                        WS_VISIBLE | WS_CHILD | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
                        20, 180, 550, 200, hwnd, NULL, NULL, NULL);
            
            // Status bar
            hStatus = CreateWindow("STATIC", "Pronto para começar!",
                        WS_VISIBLE | WS_CHILD | WS_BORDER,
                        20, 400, 550, 25, hwnd, NULL, NULL, NULL);
            
            break;
        }
        
        case WM_COMMAND: {
            if (LOWORD(wParam) == 1) { // Botão Listar
                char caminho[MAX_PATH];
                GetWindowText(hEditCaminho, caminho, MAX_PATH);
                if (strlen(caminho) > 0) {
                    ListarArquivos(caminho);
                } else {
                    MessageBox(hwnd, "Por favor, insira o caminho da pasta!", "Aviso", MB_OK);
                }
            }
            else if (LOWORD(wParam) == 2) { // Botão Renomear
                RenomearArquivos();
            }
            break;
        }
        
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
        
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = {0};
    HWND hwnd;
    MSG msg;
    
    // Registrar classe da janela
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "OrganizadorArquivos";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    
    if (!RegisterClass(&wc)) {
        MessageBox(NULL, "Falha ao registrar classe!", "Erro", MB_OK);
        return 1;
    }
    
    // Criar janela
    hwnd = CreateWindow("OrganizadorArquivos",
                        "Organizador de Arquivos - Renomeador em Massa",
                        WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
                        CW_USEDEFAULT, CW_USEDEFAULT, 620, 480,
                        NULL, NULL, hInstance, NULL);
    
    if (!hwnd) {
        MessageBox(NULL, "Falha ao criar janela!", "Erro", MB_OK);
        return 1;
    }
    
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
    
    // Loop de mensagens
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return msg.wParam;
}