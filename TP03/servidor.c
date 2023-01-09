#include "msg.h"
#include <sys/shm.h>
#define PERMISSOES 0600



int id_msg1,id_msg2,id_shm;
struct s_msg msg;
struct s_shm *ptr; 


//ordenar vetor para por os pid todos para a esquerda se remover algum do meio 
void ordenarVetor(int vetor[], int tamanho) {
  int i, j, temp;
  for (i = 0; i < tamanho - 1; i++) {
    for (j = i + 1; j < tamanho; j++) {
      if (vetor[i] < vetor[j]) {
        temp = vetor[i];
        vetor[i] = vetor[j];
        vetor[j] = temp;
      }
    }
  }
}

int obeterPIDC2(const char* str) {
  char* p = strstr(str, "C2");

  p += strlen("C2");

  int value;

  sscanf(p, " %d", &value);

  return value;
}

tratarSinal(int numSinal){

    if(numSinal==SIGINT)
    {
        printf("\nDESLIGAR SERVIDOR\n");
        msgctl(id_msg1, IPC_RMID, NULL);
        msgctl(id_msg2, IPC_RMID, NULL);
        shmctl(id_shm, IPC_RMID, NULL);
        exit(0);
    }else if(numSinal == SIGTERM)
    {
        printf("OLA ESTOU A SER CHAMDA SIGTERM\n");
    }else if(numSinal == SIGCHLD)
    {
        printf("OLA ESTOU A SER CHAMDA SIGCHLD\n");
    }         
}

main(int argc, char* argv[])
{   
    
    
if (argc!=3)
{
    printf("$ %s <chave_mg1> <chave_mg2>\n", argv[0]);
    exit (1);
}


int chave_mg1=atoi(argv[1]);
//criar fila de mensagem 1
id_msg1=msgget(chave_mg1,0666 | IPC_CREAT |IPC_EXCL);
exit_on_error(id_msg1,"Erro ao tentar obter o id da fila\n");


int chave_mg2=atoi(argv[2]);
//criar fila de mensagem 2
id_msg2=msgget(chave_mg2,0666 | IPC_CREAT |IPC_EXCL);
exit_on_error(id_msg2,"Erro ao tentar obter o id da fila\n");


//criar shm
key_t key = 20;
id_shm=shmget(key, sizeof(struct s_shm), IPC_CREAT | PERMISSOES);
exit_on_error(id_shm,"Erro ao tentar criar a mem. partilhada\n");


//tratamento de sinal
signal(SIGINT, tratarSinal);

pid_t pid = getpid();
printf("Servidor PID=%d\n",pid);


while(1)
{
    printf("a receber pedidos de clientes...\n");
    
    //ler a lista de mensagem
    long tipo=1;
    int r=msgrcv(id_msg1, (struct msgbuf *) &msg, sizeof(msg)-sizeof(long),tipo,0);
    exit_on_error(r,"Erro na leitura de mensagem 1");
    
  
    char PIDs[1024];
    strcpy(PIDs,msg.conteudo);

    char comando[TAMANHO];
    strcpy(comando,msg.nome);
   
    
    //para o "procura" e "quero" dividir o primiro argumento para verificacao de comandos
    char argumento1[256];
    char *delimitador;
    char *copia = malloc(strlen(comando) + 1);
    strcpy(copia,comando);
    delimitador= strchr(copia, ' ');
    if (delimitador != NULL) {
        strncpy(argumento1, copia, delimitador - copia);
        argumento1[delimitador - copia] = '\0'; 
    }else {
        strcpy(argumento1, copia);
    }
    free(copia);



    ptr= (struct s_shm *) shmat(id_shm,NULL,0);

    if(ptr->tamanho>10)
    {
        printf("O servidor so pode ter 10 clientes");
    }

    if(strcmp(comando,"connect") == 0)
    {  
        int pidC2=obeterPIDC2(PIDs);
        ptr->pid[ptr->tamanho]=pidC2;
        printf("%s %s\n",msg.conteudo,msg.nome);
        printf("Ligar C2 %d com o pid %d\n",ptr->tamanho+1,pidC2);
        
       /*
        for(int i=0;i<TAMANHO_SHM;i++)
            printf("numero=%d:%d ",i+1, ptr->pid[i]);
       */
        
        ptr->tamanho++;
        shmdt(ptr);
    }else if (strcmp(comando,"shutdown") == 0 )
    {
        printf("%s %s\n",msg.conteudo,comando);
        int pidC2=obeterPIDC2(PIDs);
        int n=0;
        for (int i = 0; i < 10; i++) {
            if (ptr->pid[i] == pidC2) {
                ptr->pid[i] = 0;
                
                n=i+1;
            }       
        }
        if(n==0)
            printf("esse numero nao esta no array\n");
        
        printf("Desligar C2 %d PID=%d\n",n,pidC2);
        
        //ordenar o vetor para por todos os valores para a esquera para nao ter problemas a adicionar um novo cliente
        ordenarVetor(ptr->pid,10);
        ptr->tamanho--;
          
       /*
        for(int i=0;i<TAMANHO_SHM;i++)
            printf("numero=%d:%d ",i+1, ptr->pid[i]);
       */
       
    }else if (strcmp(argumento1,"procura") == 0 )
    {
        printf("%s %s\n",msg.conteudo,comando);
    }else if (strcmp(argumento1,"quero") == 0 )
    {
        printf("%s %s\n",msg.conteudo,comando);
    }
      
}
    
}
