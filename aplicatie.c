#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <errno.h>

void citesteDirector(DIR* director, char* cale,int fd){
    struct dirent* aux;
    struct stat infFisier;

    while((aux = readdir(director))){
        if(strcmp(aux->d_name,".") && strcmp(aux->d_name,"..")){
            if(aux->d_type == DT_DIR){
                DIR* d;
                char* nume = malloc(sizeof(char)*(strlen(cale)+1+strlen(aux->d_name)));
                strcpy(nume,cale);
                strcat(nume,"/");
                strcat(nume,aux->d_name);

                if((lstat(nume,&infFisier))==-1){
                    perror("Eroare de a lua informatii despre fisier!!!\n");
                    exit(-1);
                }

                d = opendir(nume);
                if(!d){
                    perror("Nu am putut deschide un director interior.\n");
                    exit(-1);
                }
                citesteDirector(d,nume,fd);

                dprintf(fd,"%s : %s : %ld\n",cale,aux->d_name,infFisier.st_mtime);

                free(nume);
            }
            else if(aux->d_type == DT_REG){
                char* nume = malloc(sizeof(char)*(strlen(cale)+1+strlen(aux->d_name)));
                strcpy(nume,cale);
                strcat(nume,"/");
                strcat(nume,aux->d_name);

                if((lstat(nume,&infFisier))==-1){
                    perror("Eroare de a lua informatii despre fisier!!!\n");
                    exit(-1);
                }

                dprintf(fd,"%s : %s : %ld\n",cale,aux->d_name,infFisier.st_mtime);

                free(nume);
            }
        }
    }
}

int main(int argc, char* argv[]){

    int fd =0;
    struct dirent* aux;
    struct stat infFisier;

    for(int i=1;i<argc;i++){
        fd = open("temporary.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) { 
            perror("Eroare deschidere fisier temporar!!!");
            exit(-1);
        }
        printf("Verificare director : %s\n",argv[i]);
        DIR* director;
        if(!(director = opendir(argv[i]))){
            perror("Calea catre director nu este corecta/directorul nu s-a putut deschide.\n");
            exit(-1);
        }

        citesteDirector(director,argv[i],fd);

        close(fd);

    }

    return 0;
}
