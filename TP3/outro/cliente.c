#include "msg.h"
#include <sys/shm.h>
#include <string.h>

pid_t pidC1,pidC2;
int id_msg1,id_msg2,shmid;
struct s_shm *ptr; 
struct s_msg msg;

int i=0; //verificar se ele fez connect primeiro

tratarSinal(int numSinal){

    if(numSinal==SIGINT)
    {   
        if(i==0)
        {
            printf("ATENCAO!!! ");
        }else
        {
            char conteudo[1024];
            sprintf(conteudo,"C1 %d C2 %d", pidC1, pidC2);
            printf("\n Cliente %d vai terminar \n",pidC1); 
            
            enviarMSG(conteudo,"shutdown");
            exit(0);
        }   
    }
}

int mygetline(char line[], int max)
{int nch = 0, c;
max = max - 1; /* espaço for '\0' */
while((c = getchar()) != EOF)
 {if(c == '\n')break;
 if(nch < max)
 {line[nch] = c;
 nch = nch + 1;}
 }
if(c == EOF && nch == 0) return EOF;
line[nch] = '\0';
return nch;
}

void enviarMSG(char conteudo[1024],char nome[TAMANHO])
{
    strcpy(msg.nome,nome);
    strcpy(msg.conteudo,conteudo);
    int r=msgsnd(id_msg1, (struct msgbuf *) &msg,sizeof(msg)-sizeof(long), 0);
    exit_on_error(r,"Erro no envio da mensagem para a fila\n");
}

//ler o nome do ficheiro e o conteudo 
void ficheiro(char *nomeFich,char *conteudo)
{
    DIR *d;
    struct dirent *dir;
    d = opendir(".");

    char *p;    

    if (d) {
        while ((dir = readdir(d)) != NULL) {
            
            if (strstr(dir->d_name, ".txt")) {
        
                p=strrchr(dir->d_name, '.'); //encontar a posicao do '.' do arquivo
                
                //ler o conteudo do ficheiro
                FILE *fp;
               
                fp = fopen(dir->d_name, "r");
                long tamanho;
                fseek(fp, 0L, SEEK_END);
                tamanho=ftell(fp);
                rewind(fp);
                fread(conteudo, tamanho, 1, fp);
                conteudo[tamanho]='\0';
            
                fclose(fp);

                if (p != NULL) {
                    strncpy(nomeFich, dir->d_name, p - dir->d_name);
                    nomeFich[p - dir->d_name] = '\0'; //adicionar '\0' para o nome do ficheiro
                }   
            }
        }
    }
    closedir(d);
}

main(int argc, char* argv[])
{
    
if (argc!=3)
{
    printf("$ %s <chave_mg1> <chave_mg2>\n", argv[0]);
    exit (1);
}   

//ver se o shm foi criado no servidor se nao o servidor nao foi criado
key_t key = 20;
int shmid;
shmid=shmget(key, sizeof(struct s_shm), 0666);

if(shmid == -1)
{
    printf("================================\n");
    printf("Servidor nao esta a executar ...\n");
    printf("================================\n");
    exit(0);
}

int chave_mg1=atoi(argv[1]);
//ligar fila de mensagem 1
id_msg1=msgget(chave_mg1, 0);
exit_on_error(id_msg1,"Erro ao tentar obter o id da fila do mg1\n");


int chave_mg2=atoi(argv[2]);
//ligar fila de mensagem 2
id_msg2=msgget(chave_mg2, 0);
exit_on_error(id_msg2,"Erro ao tentar obter o id da fila do mg2\n");

// verificar se pode adicionar mais um clinete ja que o maximo sao 10
ptr= (struct s_shm *) shmat(shmid,NULL,0);
if(ptr->tamanho == 10)
{
    printf("Nao pode adicionar mais clientes o maximo e 10");
    exit(0);
}


/*Criar pipe para enviar o Pid do filho para o pai*/
int pipeFd[2];
pipe(pipeFd); //criar o pipe


int pid=fork();

char texto[TAMANHO];
char conteudo[1024];


    if (pid!=0) /* processo pai C1*/
    {
        pidC1 = getpid();
        printf("Cliente PID=%d \n",pidC1);

        while(1)
        {   
                
            //tratamento de sinal
            signal(SIGINT, tratarSinal);
        
            msg.tipo= 1; /*destino*/
            printf("Escreva comandos:\n");
            
            mygetline(texto,TAMANHO);
            
            //para o "procura" e "quero" dividir o primiro argumento para verificacao de comandos
            char argumento1[256];
            char *delimitador;
            char *copia = malloc(strlen(texto) + 1);
            strcpy(copia,texto);
            delimitador= strchr(copia, ' ');
            if (delimitador != NULL) {
                strncpy(argumento1, copia, delimitador - copia);
                argumento1[delimitador - copia] = '\0'; 
            }else {
                strcpy(argumento1, copia);
            }
            free(copia);

            char PIDs[1024];
            close(pipeFd[1]); // Fecha o descritor de escrita
            read(pipeFd[0], PIDs, 1024);
            pidC2=atoi(PIDs);
            sprintf(conteudo,"C1 %d C2 %s", getpid(), PIDs);
            
            if( strcmp(texto,"connect") == 0 )
            {
            enviarMSG(conteudo,texto);
            i=1; //mudar o valor para 1 ja que ele fez connect
            }else if(i==1 && (strcmp(texto,"shutdown") == 0 )) 
            {
                enviarMSG(conteudo,texto);
                i=0; //voltar a 0 para verificar novamente
            }else if(i==1 && (strcmp(argumento1,"procura") == 0 || strcmp(argumento1,"quero") == 0))
            {
                enviarMSG(conteudo,texto);
            }else if(i==0)
            {
                printf("Cliente nao fez connect primeiro\n");  
            }else
            printf("Comando invalido\n");   

        }
    }else /* processo filho C2*/
    {   
        //enviar o pid do C2 para o pai
        pidC2 = getpid();
        close(pipeFd[0]); // Fecha o descritor de leitura
        sprintf(texto, "%d", pidC2);
        write(pipeFd[1], texto, strlen(texto));

        char nomeFich[256];
        char conteudo[1024];

        ficheiro(nomeFich,conteudo);
        
        msg.tipo= 2; /*destino*/
        strcpy(msg.nome,nomeFich);
        strcpy(msg.conteudo,conteudo);
        int r=msgsnd(id_msg2, (struct msgbuf *) &msg,sizeof(msg)-sizeof(long), 0);
        exit_on_error(r,"Erro no envio da mensagem para a fila\n");

    }

}

