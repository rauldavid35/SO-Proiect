#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

void verificareAcelasiContinut(DIR *pdir){
    struct dirent *citireDirector=NULL;
    while((citireDirector=readdir(pdir))!=NULL){
        if(citireDirector->d_type=="DT_DIR"){
            DIR *p=NULL;
            if((p=opendir(citireDirector->d_name))){
                perror("Eroare deschidere director!!!\n");
                exit(-1);
            }

            verificareAcelasiContinut(p);
            close(p);
        }
        if(citireDirector->d_type=="DT_LINK"){

        }
        if(citireDirector->d_type=="DT_REG"){
            
        }
    }
}

int main(int argc , char **argv){
    char *director=argv[1];
    DIR *p=NULL;
    if((p=opendir(director))){
        perror("Eroare deschidere director!!!\n");
        exit(-1);
    }
    char buffer[4096];

    close(director);
    return 0;
}
