#include <sys/mman.h> // mmap, munmap, shm_open, shm_unlink
#include <sys/stat.h> // constantes de modo
#include <fcntl.h> // constantes O_*
#include <unistd.h> // ftruncate, close
#include <stdio.h> // printf, perror
#include <stdlib.h> // exit, malloc
#include <string.h> // strcpy, memset
#include <time.h> // time, timestamp
#include <sys/wait.h> // wait, waitpid


#define SHM_BUSCA "/busca_space"    
#define SHM_RETORNO "/retorno_space"    
#define true (unsigned char) 1
#define false (unsigned char) 0


typedef struct{
    char faixa[200];
    char artista[200];
} tBusca;

typedef struct {
    tBusca musica;
    char letra[5000];
} tRetorno;

void linkFormat(char* buffer, char* input){
    for(int i = 0; input[i] != '\0'; i++){
        if(input[i] != ' '){
            char temp[2];
            temp[0] = input[i];
            temp[1] = '\0';
            strcat(buffer, temp);
        }else
            strcat(buffer, "%20");
    }
    return;
}

int main() {
    // Teremos 2 processos filhos: 1 para interagir com o usuário, outro para fazer a busca das letras
    pid_t filhos[2]; // [0] cliente, [1] busca
    int pipe1[2], pipe2[2];
    pipe(pipe1);
    pipe(pipe2);

    // criação da memória compartilhada da busca
    shm_unlink(SHM_BUSCA); // remove o objeto anterior, caso exista
    int shm_busca = shm_open(SHM_BUSCA, O_CREAT | O_RDWR, 0666); // cria uma memoria compartilhada

    if(shm_busca == -1){
        perror("Erro ao criar memoria de busca compartilhada!");
        exit(1);
    }

    // criação da memória compartilhada da retorno
    shm_unlink(SHM_RETORNO); // remove o objeto anterior, caso exista
    int shm_retorno = shm_open(SHM_RETORNO, O_CREAT | O_RDWR, 0666); // cria uma memoria compartilhada

    if(shm_retorno == -1){
        perror("Erro ao criar memoria de retorno compartilhada!");
        exit(1);
    }

    // filho cliente =============================
    if((filhos[0] = fork()) == 0){
        
        
        // Define o tamanho do objeto
        if (ftruncate(shm_busca, sizeof(tBusca)) == -1) {
            perror("Erro ao definir tamanho da memoria de busca!");
            exit(1);
        }

        // mapeando o espaço de escrita
        tBusca* output = mmap(NULL, sizeof(tBusca),
                             PROT_READ | PROT_WRITE, MAP_SHARED, shm_busca, 0);

        if(output == MAP_FAILED){
            perror("Erro ao mapear memoria de busca!");
            exit(1);
        }
        
        // mapeando o espaço de leitura
        tRetorno* input = mmap(NULL, sizeof(tRetorno),
                             PROT_READ, MAP_SHARED, shm_retorno, 0);

        if(input == MAP_FAILED){
            perror("Erro ao mapear memoria de busca!");
            exit(1);
        }

        int choose;
        close(pipe1[0]);
        close(pipe2[1]);

        while(true){
            // system("clear");
            printf("================================================\n"
                   "|                                              |\n"
                   "|                 SEARCH SONG                  |\n"
                   "|                                              |\n"
                   "================================================\n");
            printf("Procurar nova música?\n\t1. Sim\n\t0. Não\n");
            printf("Opcao: ");
            scanf("%d", &choose);
            getchar();
            if(!choose) exit(0);
            
            printf("\nNome do artista: ");
            fgets(output->artista, sizeof(output->artista), stdin);
            output->artista[strcspn(output->artista, "\n")] = '\0';
            
            printf("Nome da musica: ");
            fgets(output->faixa, sizeof(output->faixa), stdin);
            output->faixa[strcspn(output->faixa, "\n")] = '\0';
            // system("clear");
            // guardar na shared memory
            
            // sinaliza pro servico de busca:
            write(pipe1[1], "processar", 9);
            
            // espera o servico de busca terminar (reaproveita um buffer ali de cima):
            while(true){
                char buffer[11];
                read(pipe2[0], buffer, sizeof(buffer));
                if(!strcmp("processado", buffer)){
                    printf("\n====================================\n"
                           "|    ~>%s\n"
                           "|    ~~~>%s\n"
                           "====================================\n\n", input->musica.faixa, input->musica.artista);
                    printf("%s\n"
                           "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n", input->letra);
                    break;
                }
            }
        }
        
        // fecha os descritores de memória dos espaços compartilhados de busca e retorno
        close(shm_busca);
        close(shm_retorno);

        exit(0);
    }

    // filho retorno =============================
    if((filhos[1] == fork()) == 0){
        
        // Define o tamanho do objeto
        if (ftruncate(shm_retorno, sizeof(tRetorno)) == -1) {
            perror("Erro ao definir tamanho da retorno!");
            exit(1);
        }

        // mapeando o espaco de escrita
        tRetorno* output = mmap(NULL, sizeof(tRetorno),
                             PROT_READ | PROT_WRITE, MAP_SHARED, shm_retorno, 0);

        if(output == MAP_FAILED){
            perror("Erro ao mapear memoria de retorno!");
            exit(1);
        }

        // mapeando o espaço de leitura
        tBusca* input = mmap(NULL, sizeof(tBusca),
                             PROT_READ, MAP_SHARED, shm_busca, 0);

        if(input == MAP_FAILED){
            perror("Erro ao mapear memoria de busca!");
            exit(1);
        }

        close(pipe1[1]);
        close(pipe2[0]);

        char buffer[15];
        while(true){
            // Espera receber dados do usuario:
            while(true){
                read(pipe1[0], buffer, 15);
                if(!strcmp("processar", buffer)) break;
                
            }
            
            // printf("\n%s", input->artista);
            // printf("\n%s", input->faixa);
            // Agora, capta a letra da música usando curl para o link http://lyricmatch.com.br:4000/api-requests/lyrics?track={input->faixa}&artist={input->artista}
            // guarda em output->letra

            char link[400], linkInput[200];
            strcpy(link, "http://lyricmatch.com.br:4000/api-requests/lyrics?track=");
            linkInput[0] = '\0';
            linkFormat(linkInput, input->faixa);
            // printf("%s\n", linkInput);
            strcat(link, linkInput);
            
            strcat(link, "&artist=");
            linkInput[0] = '\0';
            linkFormat(linkInput, input->artista);
            // printf("%s\n", linkInput);
            strcat(link, linkInput);

            char comando[500];
            sprintf(comando, "curl -s -X GET \"%s\" -H \"accept: */*\"", link);
            // printf("%s\n", comando);

            FILE *res = popen(comando, "r");
            if(!res){
                perror("Erro ao executar curl!");
                return;
            }

            char buffer_linha[1000], buffer[100000];
            buffer[0] = '\0';

            // Lê a resposta da "api"
            while(fgets(buffer_linha, sizeof(buffer_linha), res) != NULL){
                strcat(buffer, buffer_linha);
            }

            pclose(res);

            strcpy(output->musica.artista, input->artista);
            strcpy(output->musica.faixa, input->faixa);
            strcpy(output->letra, buffer);

            write(pipe2[1], "processado", 10);
        }
        
        // fecha os descritores de memória dos espaços compartilhados de busca e retorno
        close(shm_busca);
        close(shm_retorno);

        exit(0);
    }
    //=============================
    // pai
    
    // Quando o usuário sair do serviço, desativa o serviço de busca das letras
    // waitpid(filhos[0], NULL, 0);
    // kill(filhos[1], SIGKILL);

    return 0;
}
