#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>

void verificareAcelasiContinut(const char *director,DIR *pdir,FILE* fisier){
    struct dirent *citireDirector=NULL;
    struct stat informatiiFisier;
    char timpModificare[30];
    char *caleDirector=NULL;

    while((citireDirector=readdir(pdir))!=NULL){

        if((caleDirector=(char*)malloc(strlen(director)+strlen(citireDirector->d_name)+2))==NULL){
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

        if(citireDirector->d_type==DT_DIR){
            DIR *p=NULL;
            if((p=opendir(citireDirector->d_name))){
                perror("Eroare deschidere director!!!\n");
                exit(-1);
            }

            fprintf(fisier, "%s (%s)\n", caleDirector, timpModificare);

            verificareAcelasiContinut(caleDirector,p,fisier);
            closedir(p);
        }
        else if(citireDirector->d_type==DT_REG){
            fprintf(fisier, "%s (%s)\n", caleDirector, timpModificare);  
        }

        free(caleDirector);
    }
}

int main(int argc , char **argv){
    char *director=argv[1];

    DIR *p=NULL;
    if((p=opendir(director))){
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
