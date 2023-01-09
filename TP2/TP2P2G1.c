#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h> 
#include <dirent.h>
#define MAX 500

int aleatorio(int de, int ate,pid_t pid1)
{
    srand(time(NULL)*pid1);
    return de+(int)(((double)(ate-de+1))*rand()/(RAND_MAX+1.0));
}


void criarFicheiro()
{
    pid_t pid1,pid1_1,pid1_2;
    pid1 = fork();

    if (pid1 == 0) 
    {
        pid1_1 = fork();

        if (pid1_1 == 0) /* processo pid1_1_1*/
        {
            printf("[Pai, Filho] = [%d, %d]\n",getppid(),getpid());
            gravarFicheiro(getppid(),getpid());
			exit(0);
        }
        else 
        {
            wait(NULL);
            pid1_1 = fork();

            if (pid1_1 == 0) /* processo pid1_1_2*/
            {
                printf("[Pai, Filho] = [%d, %d]\n",getppid(),getpid());
                gravarFicheiro(getppid(),getpid());
				exit(0);
            }
            else /* processo pid1_1*/
            {
                wait(NULL);
                printf("[Pai, Filho] = [%d, %d]\n",getppid(),getpid());
                gravarFicheiro(getppid(),getpid());
				exit(0);
            }
        }
    }
    else 
    {
        wait(NULL);
        pid1_2 = fork();

        if (pid1_2 == 0) //* processo pid1_2*/
        {
            printf("[Pai, Filho] = [%d, %d]\n",getppid(),getpid());
            gravarFicheiro(getppid(),getpid()); 
			exit(0);
        }
        else /* processo pid1*/
        {
            wait(NULL);
            printf("[Pai, Filho] = [%d, %d]\n",getppid(),getpid());
            gravarFicheiro(getppid(),getpid());
        }
    }
}

void gravarFicheiro(pid_t pidPai,pid_t pidFilho)
{
    char texto[MAX]="A cidadania na Grecia Antiga estava ligada a nocao de cidade-estado e era um conceito utilizado para designar os direitos dos cidadaos. Na Grecia de Platao e Aristoteles, eram considerados cidadãos os homens livres. Era uma cidadania limitada, pois excluiam-se as mulheres, os metecos (estrangeiros residentes) e os escravos (Morgado & Rosas, 2010)";

    int num=aleatorio(1,sizeof(texto),pidFilho);

    char str[MAX];
    char str1[MAX];
    strncpy(str1, texto, num);
  
    sprintf(str, "%d.pso",pidFilho);
    
    FILE *f;
    f=fopen(str, "w");

    fprintf(f, "[PAI %d FILHO %d]\n",pidPai,pidFilho);
    fprintf(f, "P2G1\n");
    fprintf(f, "%s",str1);

    fclose(f);
}

void mostrarValores()
{
    
    DIR *dirp;
    struct dirent *dp;
    struct stat f;
    
    dirp = opendir("./");
    int errno;
    int i=0;
    do {
        if ((dp = readdir(dirp)) != NULL) {
            if (strstr(dp->d_name, ".pso") == NULL)
                continue;
            i++;
            stat(dp->d_name, &f);
            printf("Ficheiro: %s %ld Bytes\n",dp->d_name,f.st_size); 
        }else
        {
            if(i==0)
            printf("Nao existem ficheiros .pso na pasta\n");
        }       
    } while (dp != NULL);

    closedir(dirp);   
}

void eliminarFicheiros()
{
    DIR *dirp;
    struct dirent *dp;
    struct stat f;
    
    dirp = opendir("./");
    int errno;
    int i=0;
    do {
        if ((dp = readdir(dirp)) != NULL) {
            if (strstr(dp->d_name, ".pso") == NULL)
                continue;
            i++;
            stat(dp->d_name, &f);
            remove(dp->d_name);
        }else
        {
            if(i==0)
            printf("Nao existem ficheiros .pso na pasta\n");
        }       
    } while (dp != NULL);

    closedir(dirp);   
}

void filhoTrata(int sinal)
{ 
    exit(0);
}

void terminar()
{
    pid_t pid;
    int estado;
    pid = fork();
    if (pid == 0) /* filho */
    {
        signal(SIGCHLD, filhoTrata);
    }
    else { /* pai */
    kill(pid, SIGINT);

    pid = wait(&estado);
    kill(pid, SIGKILL);
    } 
}

int main(int argc, char *argv[]) {

    int n=0;

    printf("\n\n1. Criar ficheiros\n"); 
    printf("2. Mostrar valores\n"); 
    printf("3. Eliminar ficheiros\n"); 
    printf("4. Terminar\n");
        
 
    scanf("%d",&n);
    switch(n)
    {
        case 1:
            criarFicheiro();
            break;
        case 2:
            mostrarValores();
            break;
        case 3:
            eliminarFicheiros();
            break;
        case 4:
            terminar();
            break;
        default:
            printf("Digite uma opcao valida\n");
    }
   
    
    return 0;
}