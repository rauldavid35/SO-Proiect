/*#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <errno.h>

void verificareAcelasiContinut(const char *director,DIR *pdir,FILE* fisier){
    struct dirent *citireDirector=NULL;
    struct stat informatiiFisier;
    char timpModificare[30];
    char *caleDirector=NULL;

    while((citireDirector=readdir(pdir))!=NULL){

        if((caleDirector=(char*)malloc(sizeof(char)*(strlen(director)+strlen(citireDirector->d_name)+2)))==NULL){
            printf("Eroare alocare memorie pentru cale!!!\n");
            exit(-1);
        }
        strcpy(caleDirector,director);
        strcat(caleDirector,"/");
        strcat(caleDirector,citireDirector->d_name);

        if((lstat(caleDirector,&informatiiFisier))==-1){
            perror("Eroare de a lua informatii despre fisier!!!\n");
            exit(-1);
        }

        strftime(timpModificare, sizeof(timpModificare), "%Y-%m-%d %H:%M:%S", localtime(&informatiiFisier.st_mtime));

        if(S_ISDIR(informatiiFisier.st_mode)){
            DIR *p=NULL;
            if((p=opendir(citireDirector->d_name))==NULL){
                perror("Eroare deschidere director2!!!\n");
                exit(-1);
            }

            fprintf(fisier, "%s (%s)\n", caleDirector, timpModificare);

            verificareAcelasiContinut(caleDirector,p,fisier);
            closedir(p);
        }
        else if(S_ISREG(informatiiFisier.st_mode)){
            fprintf(fisier, "%s (%s)\n", caleDirector, timpModificare);  
        }

        free(caleDirector);
    }
}

int main(int argc , char **argv){
    char *director=argv[1];

    printf("%s\n",argv[1]);

    DIR *p=opendir(director);
    if(p==NULL){
        perror("Eroare deschidere director!!!\n");
        exit(-1);
    }
    FILE *fisierNou = fopen("fisierNou.txt","w");
    if (fisierNou == NULL) {
        perror("Error opening new output file");
        exit(-1);
    }

    // Traverse directories and files and write to new file
    verificareAcelasiContinut(director, p, fisierNou);

    // Close the directory and new file
    closedir(p);
    fclose(fisierNou);
    return 0;
}
*/


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

void citesteDirector(DIR* director, char* cale){
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
                    fprintf(stderr,"Nu am putut deschide un director interior.\n");
                    exit(-1);
                }
                citesteDirector(d,nume);
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
            }
            printf("%s : %s : %ld\n",cale,aux->d_name,infFisier.st_mtime);
        }
    }
    if(errno != 0){
        fprintf(stderr,"Eroare la citirea din director.\n");
        exit(-1);
    }
}

int main(int argc, char* argv[]){

    for(int i=1;i<argc;i++){
        printf("Verificare director nr. : %d\n",i);
        DIR* director;
        if(!(director = opendir(argv[i]))){
            fprintf(stderr,"Calea catre director nu este corecta/directorul nu s-a putut deschide.\n");
            exit(-1);
        }

        citesteDirector(director,argv[i]);
    }

    return 0;
}
