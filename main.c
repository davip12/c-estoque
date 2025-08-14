#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <locale.h>

#define MAX_NOME 500
#define ESTOQUE_TAMANHO_INICIAL 5

const char ESTOQUE_ARQUIVO_NOME[] = "estoque.txt";

typedef enum {
    ADICIONAR = 1,
    REMOVER,
    LISTAR,
    SAIR
} menu_opcoes;

typedef struct {
    char* nome;
    size_t quantidade;
} itens_t;

typedef struct {
    itens_t* dados;
    size_t tamanho;
    size_t quantidade_itens;
} estoque_t;

void limpar_buffer(void);
void trim(char* str);

int menu_exibir(void);

estoque_t* estoque_criar(void);
void estoque_destruir(estoque_t* estoque_ptr);
void estoque_item_anexar(estoque_t* estoque_ptr, const char* item_nome, size_t item_quantidade);

void estoque_ler(estoque_t* estoque_ptr);
void estoque_gravar(estoque_t* estoque_ptr);

itens_t* estoque_item_encontrar(estoque_t* estoque_ptr, const char* busca, int* pos_ptr);

void estoque_adicionar(estoque_t* estoque_ptr);
void estoque_remover(estoque_t* estoque_ptr);
void estoque_listar(estoque_t* estoque_ptr);

int main() {
    setlocale(LC_ALL, "pt_BR.UTF-8");

    estoque_t* estoque = estoque_criar();
    estoque_ler(estoque);

    bool sair = false;
    while (!sair) {
        int opcao = menu_exibir();
        switch (opcao)
        {
        case ADICIONAR: estoque_adicionar(estoque); break;
        case REMOVER: estoque_remover(estoque); break;
        case LISTAR: estoque_listar(estoque); break;
        case SAIR: 
            sair = true; 
            printf("Obrigado por usar o Controle de Estoques! Até a próxima.\n"); 
            break;
        default: 
            fprintf(stderr, "Opção inválida! Tente novamente.\n"); 
            break;
        }
    }
    estoque_destruir(estoque);
}

void limpar_buffer(void) {
    char c;
    while ((c = getchar()) != '\n' && c != EOF) { ;}
}
void trim(char *str) {
    if (!str) return;

    // --- Remove espaços do início ---
    char *start = str;
    while (isspace((unsigned char)*start)) {
        start++;
    }

    // Se a string é só espaços, zera
    if (*start == '\0') {
        str[0] = '\0';
        return;
    }

    // --- Remove espaços do fim ---
    char *end = start + strlen(start) - 1;
    while (end > start && isspace((unsigned char)*end)) {
        end--;
    }
    *(end + 1) = '\0'; // coloca terminador no novo fim

    // --- Move o conteúdo para o início ---
    if (start != str) {
        memmove(str, start, strlen(start) + 1);
    }
}

int menu_exibir(void) {
    /*
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
    */

    printf( "=============================\n"
            "    Controle de Estoque      \n"
            "=============================\n"
            "Selecione uma opção:\n"
            "%d. Adicionar Item.\n"
            "%d. Remover Item.\n"
            "%d. Listar estoque.\n"
            "%d. Sair.\n",
            ADICIONAR, REMOVER, LISTAR, SAIR
    );

    int opcao = 0;
    while(1) {
        printf("Opção: ");
        if(scanf("%d", &opcao) != 1) {
            limpar_buffer();
            fprintf(stderr, "Opção inválida. Tente novamente.\n");
            continue;
        }
        limpar_buffer();
        if ( opcao >= ADICIONAR && opcao <= SAIR) {
            break;
        } else {
            printf("Opção inválida! Tente novamente.\n");
            continue;
        }
    }
    return opcao;
}

estoque_t* estoque_criar(void) {

    estoque_t* estoque_ptr = (estoque_t*) malloc(sizeof(estoque_t));

    if (!estoque_ptr) {
        fprintf(stderr, "Não foi possível alocar memória p/ estoque.\n");
        return NULL;
    }
    void* temp = malloc(sizeof(itens_t) * ESTOQUE_TAMANHO_INICIAL);
    if (!temp) {
        fprintf(stderr, "Não foi possível alocar memória p/ estoque.\n");
        return NULL;
    }

    estoque_ptr->dados = (itens_t*) temp;
    estoque_ptr->tamanho = ESTOQUE_TAMANHO_INICIAL;
    estoque_ptr->quantidade_itens = 0;

    return estoque_ptr;
}
void estoque_destruir(estoque_t* estoque_ptr) {
    if (!estoque_ptr) return;
    for (size_t i = 0; i < estoque_ptr->quantidade_itens; i++) {
        free(estoque_ptr->dados[i].nome);
    }
    free(estoque_ptr->dados);
    free(estoque_ptr);
    estoque_ptr = NULL;
}

void estoque_item_anexar(estoque_t* estoque_ptr, const char* item_nome, size_t item_quantidade) {
    if (!estoque_ptr || !item_nome || !item_quantidade) return;

    // redimensiona, se necessário
    if (estoque_ptr->quantidade_itens >= estoque_ptr->tamanho) {
        size_t tamanho_novo = estoque_ptr->tamanho * 2;
        void* temp = realloc(estoque_ptr->dados, sizeof(itens_t) * tamanho_novo);
        if (!temp){
            fprintf(stderr, "em %s:\nERRO: Falha ao alocar memória para redimensionar estoque.\n", __func__);
            return;
        } 
        estoque_ptr->dados = (itens_t*) temp;
        estoque_ptr->tamanho = tamanho_novo;
    }

    char* novo_item_nome = strdup(item_nome);
    if (!novo_item_nome) {
        fprintf(stderr, "em %s\nERRO: Falha ao alocar memória para \"%s\".\n", __func__, item_nome);
        return;
    }

    int pos = estoque_ptr->quantidade_itens;
    estoque_ptr->dados[pos].nome = novo_item_nome;
    estoque_ptr->dados[pos].quantidade = item_quantidade;
    (estoque_ptr->quantidade_itens)++;
}
void estoque_item_remover(estoque_t* estoque_ptr, char* item_nome) {
    if(!estoque_ptr || !item_nome) return;

    int pos;
    itens_t* resultado = estoque_item_encontrar(estoque_ptr, item_nome, &pos);

    if (!resultado) return;

    free(resultado->nome);
    free(resultado);

    // tapa o buraco no estoque; decrementa a quantidade
    for (int i = pos; i < --(estoque_ptr->quantidade_itens); i++) {
        estoque_ptr->dados[i] = estoque_ptr->dados[i+1];
    }
}
itens_t* estoque_item_encontrar(estoque_t* estoque_ptr, const char* busca, int* pos_ptr) {
    if (!estoque_ptr || !busca) return NULL;
    if (!estoque_ptr->quantidade_itens) return NULL;
    for (size_t i = 0; i < estoque_ptr->quantidade_itens; i++) {
        if (!strcasecmp(busca, estoque_ptr->dados[i].nome)) {
            if(pos_ptr != NULL) *pos_ptr = i;
            return &estoque_ptr->dados[i];
        }
    }
    return NULL;
}

void estoque_ler(estoque_t* estoque_ptr) {
    FILE* estoque_arquivo = fopen(ESTOQUE_ARQUIVO_NOME, "r+");
    if (!estoque_ptr || !estoque_arquivo) ;

    char item_nome[MAX_NOME] = {0};
    size_t item_quantidade = 0;

    while (fscanf(estoque_arquivo, "%499s\n%zu\n", item_nome, &item_quantidade) == 2) {
        if (!item_quantidade) continue;
        estoque_item_anexar(estoque_ptr, item_nome, item_quantidade);
    }
}
void estoque_gravar(estoque_t* estoque_ptr) {
    if (!estoque_ptr) return;
    if (!estoque_ptr->quantidade_itens) return;

    FILE* estoque_arquivo = fopen(ESTOQUE_ARQUIVO_NOME, "w");
    for (size_t i = 0; i < estoque_ptr->quantidade_itens; i++) {
        fprintf(
            estoque_arquivo, 
            "%s\n%zu\n", 
            estoque_ptr->dados[i].nome, 
            estoque_ptr->dados[i].quantidade
        );
    }
    fclose(estoque_arquivo);
}

void estoque_adicionar(estoque_t* estoque_ptr) {
    if (!estoque_ptr) return;
    char item_nome[MAX_NOME] = {0};
    size_t item_quantidade = 0;

    printf("Digite o nome do item: ");
    fgets(item_nome, MAX_NOME, stdin);
    trim(item_nome);
    if (item_nome[0] == '\0') {
        printf("Nome inválido. Tente novamente.\n");
        return;
    }

    printf("Digite a quantiade: ");
    if ((scanf("%zu", &item_quantidade) != 1) && item_quantidade <= 0) {
        printf("Quantidade inválida. Tente novamente.\n");
        return;
    }

    estoque_item_anexar(estoque_ptr, item_nome, item_quantidade);
    estoque_gravar(estoque_ptr);
    printf("Item adicionado com sucesso!\n");
    return;
}
void estoque_remover(estoque_t* estoque_ptr) {
    char item_nome[MAX_NOME] = {0};
    size_t item_quantidade = 0;

    printf("Digite o nome do item: ");
    fgets(item_nome, MAX_NOME, stdin);
    trim(item_nome);
    if (item_nome[0] == '\0') {
        printf("Nome inválido. Tente novamente.\n");
        return;
    }

    printf("Digite a quantiade: ");
    if ((scanf("%zu", &item_quantidade) != 1) && item_quantidade <= 0) {
        printf("Quantidade inválida. Tente novamente.\n");
        return;
    }

    itens_t* agulha = estoque_item_encontrar(estoque_ptr, item_nome, NULL);
    if(!agulha) {
        printf("Item não encontrado.\n"); 
        return;
    }
    if (agulha->quantidade >= item_quantidade) {
        agulha->quantidade -= item_quantidade;
        if (agulha->quantidade > 0) printf("Quantidade atualizada com sucesso!\n");
        else printf("Item removido do estoque!\n");
        estoque_gravar(estoque_ptr);
        return;
    } else {
        printf("Estoque insuficiente. Quantidade disponpivel: %zu\n", agulha->quantidade);
        return;
    }
}
void estoque_listar(estoque_t* estoque_ptr) {
    if (!estoque_ptr) {fprintf(stderr, "Não foi possível abrir estoque.\n"); return;}
    if(!estoque_ptr->quantidade_itens) {printf("O estoque está vazio.\n"); return;}

    printf(
        "==========================\n"
        "     Estoque Atual        \n"
        "==========================\n"
    );
    for (size_t i = 0; i < estoque_ptr->quantidade_itens; i++) {
        printf(
            "Nome: %s\n"
            "Quantidade: %zu\n\n",
            estoque_ptr->dados[i].nome, 
            estoque_ptr->dados[i].quantidade
        );
    }   
}
