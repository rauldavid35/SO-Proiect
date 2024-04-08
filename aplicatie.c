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

    int fd =0,mfd=0;
    char *string=(char*)malloc(sizeof(char));
    char numePtTextFile[30];
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

        strcpy(string,argv[i]);

        char *p=strtok(string,"/");
        while(p){
            strcpy(numePtTextFile,p);
            strcat(numePtTextFile,".txt\0");
            p=strtok(NULL,"/");
        }

        mfd = open(numePtTextFile,  O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 0644);
        if (mfd == -1) { 
            perror("Eroare deschidere fisier temporar!!!");
            exit(-1);
        }

        if (fstat(mfd, &infFisier) == -1) {
            perror("Unable to get file information");
            exit(-1);
        }

        close(mfd);
        close(fd);

        if (infFisier.st_size == 0) {
            // If true, write information from fd to mfd
            mfd = open(numePtTextFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (mfd == -1) {
                perror("Eroare deschidere fisier temporar!!!");
                exit(-1);
            }

            fd = open("temporary.txt", O_RDONLY);
            if (fd == -1) {
                perror("Eroare deschidere fisier temporar!!!");
                exit(-1);
            }

            char buffer[4096];
            ssize_t bytes_read;
            while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
                write(mfd, buffer, bytes_read);
            }

            printf("Informatia a fost copiata din temporary.txt in %s\n", numePtTextFile);

            close(fd);
            close(mfd);
        } else {
            mfd = open(numePtTextFile, O_RDONLY);
            if (mfd == -1) {
                perror("Eroare deschidere fisier temporar!!!");
                exit(-1);
            }

            fd = open("temporary.txt", O_RDONLY);
            if (fd == -1) {
                perror("Eroare deschidere fisier temporar!!!");
                exit(-1);
            }

            char buffer1[4096], buffer2[4096];
            int diferit=0;
            ssize_t bytesRead1, bytesRead2;
            while (((bytesRead1 = read(mfd, buffer1, 4096)) > 0) && ((bytesRead2 = read(fd, buffer2, 4096)) > 0)) {
                if (bytesRead1 != bytesRead2 || memcmp(buffer1, buffer2, bytesRead1) != 0) {
                    diferit=1;
                }
            }

            close(mfd);
            close(fd);

            if(diferit){
                printf("Fisierul s-a schimbat!!!\n");
                mfd = open(numePtTextFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (mfd == -1) {
                    perror("Eroare deschidere fisier temporar!!!");
                    exit(-1);
                }

                fd = open("temporary.txt", O_RDONLY);
                if (fd == -1) {
                    perror("Eroare deschidere fisier temporar!!!");
                    exit(-1);
                }

                char buffer[4096];
                ssize_t bytes_read;
                while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
                    write(mfd, buffer, bytes_read);
                }

                printf("Informatia a fost copiata din temporary.txt in %s\n", numePtTextFile);
            }
            else{
                printf("Fisierul nu s-a schimbat!!!\n");
            }
        }

    }

    return 0;
}
