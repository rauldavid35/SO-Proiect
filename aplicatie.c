#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
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
    errno=0;

    while((aux = readdir(director))!=NULL && errno==0){
        if(strcmp(aux->d_name,".") && strcmp(aux->d_name,"..")){
            if(aux->d_type == DT_DIR){
                DIR* d;
                char nume[1000];
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

                char bufferDir[4096];
                sprintf(bufferDir,"%s : %s : %ld\n",cale,aux->d_name,infFisier.st_mtime);

                if((write(fd,bufferDir,strlen(bufferDir)))==-1){
                    perror("Eroare scriere date director!!!\n");
                    exit(-1);
                }
            }
            else if(aux->d_type == DT_REG || aux->d_type == DT_LNK){
                char nume[1000];
                strcpy(nume,cale);
                strcat(nume,"/");
                strcat(nume,aux->d_name);

                if((lstat(nume,&infFisier))==-1){
                    perror("Eroare de a lua informatii despre fisier!!!\n");
                    exit(-1);
                }

                char bufferReg[4096];
                sprintf(bufferReg,"%s : %s : %ld\n",cale,aux->d_name,infFisier.st_mtime);

                if((write(fd,bufferReg,strlen(bufferReg)))==-1){
                    perror("Eroare scriere date fisier text!!!\n");
                    exit(-1);
                }
            }
        }
    }
    if(errno!=0){
        perror("Eroare citire director!!!\n");
        exit(-1);
    }
}

int main(int argc, char* argv[]){

    int fd =0,mfd=0;
    char string[100];
    char numePtTextFile[30];
    char numePtTemporaryTextFile[40];
    struct stat infFisier;

    int n;

    for(int i=1;i<argc;i++){
        if((n=fork())==-1){
            printf("Eroare creare proces!!!\n");
            exit(-1);
        }

        if(n==0){
            
            strcpy(string,argv[i]);

            char *p=strtok(string,"/");
            while(p){
                strcpy(numePtTextFile,p);
                strcpy(numePtTemporaryTextFile,p);
                strcat(numePtTextFile,".txt");
                p=strtok(NULL,"/");
            }

            strcat(numePtTemporaryTextFile,"TEMPORARY.txt");

            printf("Sunt in fisierul %d\n",i);
            fd = open(numePtTemporaryTextFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
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

            mfd = open(numePtTextFile,  O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 0644);
            if (mfd == -1) { 
                perror("Eroare deschidere fisier temporar!!!");
                exit(-1);
            }

            if (fstat(mfd, &infFisier) == -1) {
                perror("Eroare de a lua informatii despre fisier!!!");
                exit(-1);
            }

            close(mfd);
            close(fd);

            if (infFisier.st_size == 0) {
                mfd = open(numePtTextFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (mfd == -1) {
                    perror("Eroare deschidere fisier temporar!!!");
                    exit(-1);
                }

                fd = open(numePtTemporaryTextFile, O_RDONLY);
                if (fd == -1) {
                    perror("Eroare deschidere fisier temporar!!!");
                    exit(-1);
                }

                char buffer[4096];
                ssize_t bytes_read;
                while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
                    if(bytes_read==-1){
                        perror("Eroare citire din fisier temporar!!!\n");
                        exit(-1);
                    }
                    if((write(mfd, buffer, bytes_read))==-1){
                        perror("Eroare scriere fisier main!!!\n");
                    }
                }

                printf("Informatia a fost copiata din fisierul temporar in %s\n", numePtTextFile);

                close(fd);
                close(mfd);
            } else {
                mfd = open(numePtTextFile, O_RDONLY);
                if (mfd == -1) {
                    perror("Eroare deschidere fisier temporar!!!");
                    exit(-1);
                }

                fd = open(numePtTemporaryTextFile, O_RDONLY);
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
                    if(bytesRead1==-1){
                        perror("Eroare citire fisier main!!!\n");
                        exit(-1);
                    }
                    if(bytesRead2==-1){
                        perror("Eroare citire fisier temporar!!!\n");
                        exit(-1);
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

                    fd = open(numePtTemporaryTextFile, O_RDONLY);
                    if (fd == -1) {
                        perror("Eroare deschidere fisier temporar!!!");
                        exit(-1);
                    }

                    char buffer[4096];
                    ssize_t bytes_read;
                    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
                        if(bytes_read==-1){
                            perror("Eroare citire din fisier temporar!!!\n");
                            exit(-1);
                        }
                        if((write(mfd, buffer, bytes_read))==-1){
                            perror("Eroare scriere fisier main!!!\n");
                            exit(-1);
                        }
                    }

                    printf("Informatia a fost copiata din fisierul temporar in %s\n", numePtTextFile);
                }
                else{
                    printf("Fisierul nu s-a schimbat!!!\n");
                }
            }

            exit(0);
        }
    }
    int waitVariable;
    int status;
    for(int i=1;i<argc;i++){
        waitVariable=wait(&status);
        printf("Procesul %d s-a terminat cu statusul : %d\n",waitVariable,status);
    }
    return 0;
}
