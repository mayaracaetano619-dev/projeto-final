#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <unistd.h>

#ifdef _WIN32
    #include <windows.h>
    #define PATH_SEPARATOR '\\'
    #define CLEAR_SCREEN "cls"
#else
    #include <linux/limits.h>
    #define PATH_SEPARATOR '/'
    #define CLEAR_SCREEN "clear"
#endif

#define MAX_PATH 4096
#define MAX_FILES 10000
#define MAX_NAME 512
#define MAX_EXT 50

typedef struct {
    char old_name[MAX_NAME];
    char new_name[MAX_NAME];
    char full_path[MAX_PATH];
    char extension[MAX_EXT];
    int selected;
    long long size;
} FileInfo;

FileInfo files[MAX_FILES];
int total_files = 0;
char current_dir[MAX_PATH];

// Funcao para limpar tela
void clear_screen() {
    system(CLEAR_SCREEN);
}

// Funcao para obter extensao do arquivo
void get_extension(const char *filename, char *ext) {
    const char *dot = strrchr(filename, '.');
    if (dot && dot != filename) {
        strcpy(ext, dot + 1);
        for (int i = 0; ext[i]; i++) {
            ext[i] = tolower(ext[i]);
        }
    } else {
        strcpy(ext, "");
    }
}

// Funcao para remover extensao
void remove_extension(char *filename) {
    char *dot = strrchr(filename, '.');
    if (dot) {
        *dot = '\0';
    }
}

// Funcao para formatar tamanho do arquivo
void format_size(long long size, char *output) {
    if (size < 1024) {
        sprintf(output, "%lld B", size);
    } else if (size < 1024 * 1024) {
        sprintf(output, "%.1f KB", size / 1024.0);
    } else if (size < 1024 * 1024 * 1024) {
        sprintf(output, "%.1f MB", size / (1024.0 * 1024));
    } else {
        sprintf(output, "%.1f GB", size / (1024.0 * 1024 * 1024));
    }
}

// Funcao para obter tipo do arquivo
const char* get_file_type(const char *ext) {
    if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0 || 
        strcmp(ext, "png") == 0 || strcmp(ext, "gif") == 0 || strcmp(ext, "bmp") == 0)
        return "[IMG]";
    if (strcmp(ext, "pdf") == 0) return "[PDF]";
    if (strcmp(ext, "txt") == 0) return "[TXT]";
    if (strcmp(ext, "doc") == 0 || strcmp(ext, "docx") == 0) return "[DOC]";
    if (strcmp(ext, "xls") == 0 || strcmp(ext, "xlsx") == 0) return "[XLS]";
    if (strcmp(ext, "zip") == 0 || strcmp(ext, "rar") == 0 || strcmp(ext, "7z") == 0) return "[ZIP]";
    if (strcmp(ext, "mp3") == 0 || strcmp(ext, "wav") == 0) return "[AUD]";
    if (strcmp(ext, "mp4") == 0 || strcmp(ext, "avi") == 0 || strcmp(ext, "mkv") == 0) return "[VID]";
    if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0) return "[WEB]";
    if (strcmp(ext, "c") == 0 || strcmp(ext, "cpp") == 0 || strcmp(ext, "py") == 0) return "[COD]";
    return "[FIL]";
}

// Funcao para listar arquivos do diretorio
int list_files(const char *directory) {
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    char full_path[MAX_PATH];
    
    dir = opendir(directory);
    if (dir == NULL) {
        printf("[ERRO] Nao foi possivel abrir o diretorio '%s'\n", directory);
        return 0;
    }
    
    total_files = 0;
    
    while ((entry = readdir(dir)) != NULL && total_files < MAX_FILES) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        snprintf(full_path, sizeof(full_path), "%s%c%s", directory, PATH_SEPARATOR, entry->d_name);
        
        if (stat(full_path, &st) == 0 && S_ISREG(st.st_mode)) {
            strcpy(files[total_files].old_name, entry->d_name);
            strcpy(files[total_files].full_path, full_path);
            get_extension(entry->d_name, files[total_files].extension);
            files[total_files].selected = 1;
            files[total_files].size = st.st_size;
            total_files++;
        }
    }
    
    closedir(dir);
    return total_files;
}

// Funcao para exibir lista de arquivos
void display_files() {
    char size_str[20];
    
    printf("\n[ARQUIVOS ENCONTRADOS]\n");
    printf("================================================================================\n");
    printf("  #  | TIPO | NOME DO ARQUIVO                                      | TAMANHO    \n");
    printf("================================================================================\n");
    
    for (int i = 0; i < total_files; i++) {
        format_size(files[i].size, size_str);
        const char *type = get_file_type(files[i].extension);
        printf("  %-3d | %s | %-45s | %10s\n", 
               i + 1, 
               type,
               files[i].old_name,
               size_str);
    }
    printf("================================================================================\n");
    printf("Total: %d arquivo(s)\n", total_files);
}

// Funcao para selecionar arquivos
void select_files() {
    char input[100];
    int choice;
    
    printf("\n[SELECAO DE ARQUIVOS]\n");
    printf("+--------------------------------------------------+\n");
    printf("| 1 - Selecionar todos                             |\n");
    printf("| 2 - Selecionar nenhum                            |\n");
    printf("| 3 - Selecionar por intervalo                     |\n");
    printf("| 4 - Selecionar por extensao                      |\n");
    printf("| 5 - Voltar                                       |\n");
    printf("+--------------------------------------------------+\n");
    printf("Opcao: ");
    
    fgets(input, sizeof(input), stdin);
    choice = atoi(input);
    
    switch(choice) {
        case 1:
            for (int i = 0; i < total_files; i++) {
                files[i].selected = 1;
            }
            printf("[OK] Todos os arquivos selecionados.\n");
            break;
            
        case 2:
            for (int i = 0; i < total_files; i++) {
                files[i].selected = 0;
            }
            printf("[OK] Nenhum arquivo selecionado.\n");
            break;
            
        case 3: {
            printf("Digite os numeros (ex: 1,3,5-8): ");
            fgets(input, sizeof(input), stdin);
            
            char *token = strtok(input, ",\n");
            while (token) {
                if (strchr(token, '-')) {
                    int start, end;
                    sscanf(token, "%d-%d", &start, &end);
                    for (int i = start - 1; i < end && i < total_files; i++) {
                        if (i >= 0) files[i].selected = 1;
                    }
                } else {
                    int num = atoi(token);
                    if (num >= 1 && num <= total_files) {
                        files[num - 1].selected = 1;
                    }
                }
                token = strtok(NULL, ",\n");
            }
            printf("[OK] Selecao atualizada.\n");
            break;
        }
        
        case 4: {
            char ext[MAX_EXT];
            printf("Digite a extensao (sem ponto): ");
            fgets(ext, sizeof(ext), stdin);
            ext[strcspn(ext, "\n")] = 0;
            
            int count = 0;
            for (int i = 0; i < total_files; i++) {
                if (strcmp(files[i].extension, ext) == 0) {
                    files[i].selected = 1;
                    count++;
                }
            }
            printf("[OK] %d arquivo(s) com extensao .%s selecionado(s).\n", count, ext);
            break;
        }
        
        case 5:
            return;
            
        default:
            printf("[ERRO] Opcao invalida!\n");
    }
    
    // Mostrar resumo da selecao
    int selected = 0;
    for (int i = 0; i < total_files; i++) {
        if (files[i].selected) selected++;
    }
    printf("\n[RESUMO] %d/%d arquivos selecionados.\n", selected, total_files);
}

// Funcao para gerar novo nome
void generate_new_name(char *new_name, const char *old_name, int index, 
                       const char *prefix, const char *suffix, 
                       const char *find_text, const char *replace_text,
                       int use_numbering, int start_number, int padding) {
    
    char name_without_ext[MAX_NAME];
    char ext[MAX_EXT];
    
    strcpy(name_without_ext, old_name);
    remove_extension(name_without_ext);
    get_extension(old_name, ext);
    
    // Aplicar substituicao
    if (strlen(find_text) > 0 && strlen(replace_text) > 0) {
        char temp[MAX_NAME];
        char *pos;
        char *dest = temp;
        const char *src = name_without_ext;
        
        while ((pos = strstr(src, find_text)) != NULL) {
            int len = pos - src;
            strncpy(dest, src, len);
            dest += len;
            strcpy(dest, replace_text);
            dest += strlen(replace_text);
            src = pos + strlen(find_text);
        }
        strcpy(dest, src);
        strcpy(name_without_ext, temp);
    }
    
    // Aplicar prefixo e sufixo
    char temp_name[MAX_NAME];
    snprintf(temp_name, sizeof(temp_name), "%s%s%s", prefix, name_without_ext, suffix);
    
    // Aplicar numeracao
    if (use_numbering) {
        int number = start_number + index;
        char num_str[10];
        
        if (padding == 1) sprintf(num_str, "%d", number);
        else if (padding == 2) sprintf(num_str, "%02d", number);
        else if (padding == 3) sprintf(num_str, "%03d", number);
        else sprintf(num_str, "%04d", number);
        
        snprintf(new_name, MAX_NAME, "%s_%s.%s", temp_name, num_str, ext);
    } else {
        snprintf(new_name, MAX_NAME, "%s.%s", temp_name, ext);
    }
}

// Funcao para mostrar preview
void show_preview(const char *prefix, const char *suffix, 
                  const char *find_text, const char *replace_text,
                  int use_numbering, int start_number, int padding) {
    
    printf("\n[PREVIEW DA RENOMEACAO]\n");
    printf("================================================================================\n");
    printf("  #  | NOME ORIGINAL                                        -> NOVO NOME\n");
    printf("================================================================================\n");
    
    int seq = 0;
    for (int i = 0; i < total_files; i++) {
        if (!files[i].selected) continue;
        
        char new_name[MAX_NAME];
        generate_new_name(new_name, files[i].old_name, seq, 
                         prefix, suffix, find_text, replace_text,
                         use_numbering, start_number, padding);
        
        printf("  %-3d | %-45s -> %s\n", seq + 1, files[i].old_name, new_name);
        seq++;
    }
    printf("================================================================================\n");
    printf("Total a renomear: %d arquivo(s)\n", seq);
}

// Funcao para aplicar renomeacao
int apply_renaming(const char *prefix, const char *suffix, 
                   const char *find_text, const char *replace_text,
                   int use_numbering, int start_number, int padding) {
    
    char old_path[MAX_PATH];
    char new_path[MAX_PATH];
    int renamed = 0;
    int seq = 0;
    
    printf("\n[RENOMEANDO ARQUIVOS...]\n");
    printf("--------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < total_files; i++) {
        if (!files[i].selected) continue;
        
        char new_name[MAX_NAME];
        generate_new_name(new_name, files[i].old_name, seq, 
                         prefix, suffix, find_text, replace_text,
                         use_numbering, start_number, padding);
        
        snprintf(old_path, sizeof(old_path), "%s", files[i].full_path);
        snprintf(new_path, sizeof(new_path), "%s%c%s", current_dir, PATH_SEPARATOR, new_name);
        
        if (rename(old_path, new_path) == 0) {
            printf("  [OK] %s\n", files[i].old_name);
            printf("       -> %s\n", new_name);
            renamed++;
        } else {
            printf("  [ERRO] %s\n", files[i].old_name);
        }
        
        seq++;
    }
    
    printf("--------------------------------------------------------------------------------\n");
    printf("[OK] Renomeacao concluida! %d arquivo(s) renomeado(s).\n", renamed);
    
    return renamed;
}

// Funcao principal
int main() {
    char prefix[100] = "";
    char suffix[100] = "";
    char find_text[100] = "";
    char replace_text[100] = "";
    int start_number = 1;
    int padding = 1;
    int use_numbering = 0;
    int option;
    
    clear_screen();
    
    printf("+================================================================================+\n");
    printf("|                      RENOMEADOR EM MASSA DE ARQUIVOS                           |\n");
    printf("|                             Versao 2.0 em C                                    |\n");
    printf("+================================================================================+\n\n");
    
    // Obter diretorio atual
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        printf("[ERRO] Ao obter diretorio atual!\n");
        return 1;
    }
    
    printf("[DIRETORIO ATUAL] %s\n", current_dir);
    printf("Deseja usar este diretorio? (S/n): ");
    
    char use_current[10];
    fgets(use_current, sizeof(use_current), stdin);
    
    if (use_current[0] == 'n' || use_current[0] == 'N') {
        printf("Digite o caminho da pasta: ");
        fgets(current_dir, sizeof(current_dir), stdin);
        current_dir[strcspn(current_dir, "\n")] = 0;
    }
    
    // Listar arquivos
    if (!list_files(current_dir)) {
        printf("[ERRO] Nenhum arquivo encontrado no diretorio.\n");
        printf("Pressione Enter para sair...");
        getchar();
        return 1;
    }
    
    // Menu principal
    do {
        clear_screen();
        
        printf("+================================================================================+\n");
        printf("|                           MENU PRINCIPAL                                       |\n");
        printf("+================================================================================+\n");
        
        display_files();
        
        printf("\n[CONFIGURACOES ATUAIS]\n");
        printf("+--------------------------------------------------------------------------------+\n");
        printf("| Prefixo: '%s'                                                                  \n", strlen(prefix) ? prefix : "(nenhum)");
        printf("| Sufixo:  '%s'                                                                  \n", strlen(suffix) ? suffix : "(nenhum)");
        printf("| Substituir: '%s' -> '%s'                                                        \n", 
               strlen(find_text) ? find_text : "(nenhum)", 
               strlen(replace_text) ? replace_text : "(nenhum)");
        printf("| Numeracao: %s                                                                  \n", use_numbering ? "SIM" : "NAO");
        if (use_numbering) {
            printf("|   * Inicio: %d | Padding: %d                                                  \n", start_number, padding);
        }
        printf("+--------------------------------------------------------------------------------+\n");
        
        printf("\n[OPCOES]\n");
        printf("+--------------------------------------------------------------------------------+\n");
        printf("| 1 - Selecionar arquivos                                                        |\n");
        printf("| 2 - Configurar prefixo                                                         |\n");
        printf("| 3 - Configurar sufixo                                                          |\n");
        printf("| 4 - Configurar substituicao de texto                                           |\n");
        printf("| 5 - Configurar numeracao                                                       |\n");
        printf("| 6 - Preview e renomear                                                         |\n");
        printf("| 0 - Sair                                                                       |\n");
        printf("+--------------------------------------------------------------------------------+\n");
        printf("Escolha uma opcao: ");
        
        fgets(use_current, sizeof(use_current), stdin);
        option = atoi(use_current);
        
        switch(option) {
            case 1:
                select_files();
                printf("\nPressione Enter para continuar...");
                getchar();
                break;
                
            case 2:
                printf("Digite o prefixo: ");
                fgets(prefix, sizeof(prefix), stdin);
                prefix[strcspn(prefix, "\n")] = 0;
                printf("[OK] Prefixo definido: '%s'\n", prefix);
                printf("Pressione Enter para continuar...");
                getchar();
                break;
                
            case 3:
                printf("Digite o sufixo: ");
                fgets(suffix, sizeof(suffix), stdin);
                suffix[strcspn(suffix, "\n")] = 0;
                printf("[OK] Sufixo definido: '%s'\n", suffix);
                printf("Pressione Enter para continuar...");
                getchar();
                break;
                
            case 4:
                printf("Texto a ser substituido: ");
                fgets(find_text, sizeof(find_text), stdin);
                find_text[strcspn(find_text, "\n")] = 0;
                printf("Substituir por: ");
                fgets(replace_text, sizeof(replace_text), stdin);
                replace_text[strcspn(replace_text, "\n")] = 0;
                printf("[OK] Substituicao configurada: '%s' -> '%s'\n", find_text, replace_text);
                printf("Pressione Enter para continuar...");
                getchar();
                break;
                
            case 5:
                printf("Adicionar numeracao? (1-SIM / 0-NAO): ");
                fgets(use_current, sizeof(use_current), stdin);
                use_numbering = atoi(use_current);
                
                if (use_numbering) {
                    printf("Numero inicial (padrao 1): ");
                    fgets(use_current, sizeof(use_current), stdin);
                    start_number = atoi(use_current);
                    if (start_number < 1) start_number = 1;
                    
                    printf("Padding (1=1,2,3 | 2=01,02,03 | 3=001,002): ");
                    fgets(use_current, sizeof(use_current), stdin);
                    padding = atoi(use_current);
                    if (padding < 1) padding = 1;
                    if (padding > 3) padding = 3;
                }
                printf("[OK] Numeracao %s\n", use_numbering ? "ativada" : "desativada");
                printf("Pressione Enter para continuar...");
                getchar();
                break;
                
            case 6: {
                clear_screen();
                show_preview(prefix, suffix, find_text, replace_text, 
                           use_numbering, start_number, padding);
                
                printf("\n[ATENCAO] Esta acao ira renomear os arquivos permanentemente!\n");
                printf("Confirmar renomeacao? (s/N): ");
                
                char confirm[10];
                fgets(confirm, sizeof(confirm), stdin);
                
                if (confirm[0] == 's' || confirm[0] == 'S') {
                    apply_renaming(prefix, suffix, find_text, replace_text,
                                 use_numbering, start_number, padding);
                    
                    // Recarregar lista de arquivos
                    list_files(current_dir);
                } else {
                    printf("[CANCELADO] Operacao cancelada.\n");
                }
                
                printf("\nPressione Enter para continuar...");
                getchar();
                break;
            }
            
            case 0:
                printf("[SAIR] Saindo...\n");
                break;
                
            default:
                printf("[ERRO] Opcao invalida!\n");
                printf("Pressione Enter para continuar...");
                getchar();
        }
        
    } while (option != 0);
    
    return 0;
}