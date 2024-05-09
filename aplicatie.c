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

typedef struct {
    char nume[1000];
    long int mtime;
    int Dir;
    mode_t mode;

}FisiereFolder;

FisiereFolder fisiereFolder[1000];
int counterFisiere=0;

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

                strcpy(fisiereFolder[counterFisiere].nume,cale);
                fisiereFolder[counterFisiere].Dir=1;
                sprintf(bufferDir,"%s : %s : %ld\n",cale,aux->d_name,infFisier.st_mtime);

                if((write(fd,bufferDir,strlen(bufferDir)))==-1){
                    perror("Eroare scriere date director!!!\n");
                    exit(-1);
                }
                counterFisiere++;
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

                strcpy(fisiereFolder[counterFisiere].nume,nume);
                fisiereFolder[counterFisiere].Dir=0;
                fisiereFolder[counterFisiere].mtime=infFisier.st_mtime;
                fisiereFolder[counterFisiere].mode=infFisier.st_mode;
                sprintf(bufferReg,"%s : %s : %ld\n",cale,aux->d_name,infFisier.st_mtime);

                if((write(fd,bufferReg,strlen(bufferReg)))==-1){
                    perror("Eroare scriere date fisier text!!!\n");
                    exit(-1);
                }
                counterFisiere++;
            }
        }
    }
    if(errno!=0){
        perror("Eroare citire director!!!\n");
        exit(-1);
    }
}

void verificareFisiere(const char* numeFileSimplu,const char* numePtTemporaryTextFile,const char* outputFolder,const char *malitiousFolder){
    int fd =0,mfd=0;
    struct stat infFisier;
    char numePtTextFile[1000];

    if(strcmp(outputFolder,"")!=0){
        snprintf(numePtTextFile, sizeof(numePtTextFile), "%s/%s.txt", outputFolder, numeFileSimplu);
    }
    else{
        strcpy(numePtTextFile,numeFileSimplu);
        strcat(numePtTextFile,".txt");
    }

    mfd = open(numePtTextFile,  O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 0644);
    if (mfd == -1) { 
        perror("Eroare deschidere fisier temporar2!!!");
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
        }
    }
}

int main(int argc, char* argv[]){
    int fd=0;
    char string[100];
    char numePtTextFile[1000];
    char numePtTemporaryTextFile[120];
    char numeFileSimplu[100];

    const char* outputFolder ="";
    const char* malitiousFolder="";

    int pid;

    int j=1;

    for(int i=1;i<argc;i++){
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            outputFolder = argv[i + 1];
            i++;
            if(strcmp(argv[i+1], "-s")==0 && i + 2 < argc){
                malitiousFolder=argv[i + 2];
                i+=2;
            }
        }
        else{
            if((pid=fork())==-1){
                printf("Eroare creare proces!!!\n");
                exit(-1);
            }

            if(pid==0){
            
                strcpy(string,argv[i]);

                char *p=strtok(string,"/");
                while(p){
                    strcpy(numeFileSimplu,p);
                    strcpy(numePtTextFile,p);
                    strcpy(numePtTemporaryTextFile,p);
                    p=strtok(NULL,"/");
                }

                strcat(numePtTemporaryTextFile,"TEMPORARY.txt");

                printf("Snapshot for Directory %d created successfully.\n", j);
                fd = open(numePtTemporaryTextFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1) {
                    perror("Eroare deschidere fisier temporar1!!!");
                    exit(-1);
                }
                //printf("Verificare director : %s\n",argv[i]);
                DIR* director;
                if(!(director = opendir(argv[i]))){
                    perror("Calea catre director nu este corecta/directorul nu s-a putut deschide.\n");
                    exit(-1);
                }
                citesteDirector(director,argv[i],fd);

                printf("%s\n",fisiereFolder[2].nume);

                verificareFisiere(numeFileSimplu,numePtTemporaryTextFile,outputFolder,malitiousFolder);

                exit(0);
            }
            j++;
        }
    }

    int waitVariable;
    int status;
    int i=1;
    if(strcmp(outputFolder,"")!=0)
        i=3;
    if(strcmp(malitiousFolder,"")!=0){
        i=5;
    }
    j=1;
    for(;i<argc;i++){
        waitVariable=wait(&status);
        printf("Child process %d terminated with PID %d and exit code %d. \n",j,waitVariable,status);
        j++;
    }
    return 0;
}
