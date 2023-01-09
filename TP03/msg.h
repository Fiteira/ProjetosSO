#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#define exit_on_error(s,m) if (s < 0) { perror(m); exit(1); }
#define TAMANHO 256

#define PERMISSOES 0600 
#define TAMANHO_SHM 10



/*estrutura das mensagem*/
struct s_msg {
    long tipo;
    char nome[256];
    char conteudo[1024];
};



/*Organização da memoria partilhada*/
struct s_shm{
 int pid[TAMANHO_SHM];
 int tamanho;
};

