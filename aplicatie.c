////////////////////////////////////////////////////////////
//Programul primeste foldere , se parcurg recursiv pentru a creea snapshot-uri pentru fiecare in parte
//Se verifica daca acesta a fost schimbat sau nu , si se salveaza/creeaza in caz pozitiv
////////////////////////////////////////////////////////////
//Se compileaza cu gcc -Wall -o rez aplicatia1.c
////////////////////////////////////////////////////////////
//Se poate rula in 3 moduri:
//1. ./rez dir1 dir2 dir3...
//2. ./rez -o outFolder dir1 dir2 dir3...
//3. ./rez -o outFolder -s malitiousFolder dir1 dir2 dir3...
////////////////////////////////////////////////////////////
//Cum rulez eu cu fisierele ce sunt in folder:
// ./rez -o /mnt/c/Users/rauld/OneDrive/Desktop/VisualStudio/LimbajulC/Anul2/SO_Proiect/outFolder -s /mnt/c/Users/rauld/OneDrive/Desktop/VisualStudio/LimbajulC/Anul2/SO_Proiect/malitiousFolder /mnt/c/Users/rauld/OneDrive/Desktop/VisualStudio/LimbajulC/Anul2/SO_Proiect/Folder1 /mnt/c/Users/rauld/OneDrive/Desktop/VisualStudio/LimbajulC/Anul2/SO_Proiect/Folder2 /mnt/c/Users/rauld/OneDrive/Desktop/VisualStudio/LimbajulC/Anul2/SO_Proiect/Folder3

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

int counterFisiereMalitioase=0; //variabila globala de numarare fisiere malitioase

void citesteDirector(DIR* director, char* cale,int fd,const char* malitiousFolder){ //subprogram de citire fisiere din director intr-un temporaryFile, recursiv
    struct dirent* aux;
    struct stat infFisier;
    errno=0;

    while((aux = readdir(director))!=NULL && errno==0){ //citirea directorului
        if(strcmp(aux->d_name,".") && strcmp(aux->d_name,"..")){
            if(aux->d_type == DT_DIR){ //cazul inca care calea este director
                DIR* d;
                char nume[1000];
                strcpy(nume,cale);
                strcat(nume,"/");
                strcat(nume,aux->d_name);

                if((lstat(nume,&infFisier))==-1){ //cu functia lstat luam informatii despre director
                    perror("Eroare de a lua informatii despre fisier!!!\n");
                    exit(-1);
                }

                d = opendir(nume); //deschidem calea director

                if(!d){
                    perror("Nu am putut deschide un director interior.\n");
                    exit(-1);
                }

                citesteDirector(d,nume,fd,malitiousFolder); //apelam recursiv pentru a cauta alte foldere/fisiere in folderul actual

                char bufferDir[4096];

                if((write(fd,bufferDir,strlen(bufferDir)))==-1){ //scrierea cu ajutorul unui buffer in temporaryFile
                    perror("Eroare scriere date director!!!\n");
                    exit(-1);
                }
            }
            else if(aux->d_type == DT_REG || aux->d_type == DT_LNK){ // cazul in care calea este fisier sau legatura simbolica
                int isMalitious=0;  //variabila pentru identificarea cazului de fisier malitios
                char nume[1000];
                strcpy(nume,cale);
                strcat(nume,"/");
                strcat(nume,aux->d_name);

                //printf("%s\n",nume);

                if((lstat(nume,&infFisier))==-1){ //cu lstat luam informatii despre fisier
                    perror("Eroare de a lua informatii despre fisier!!!\n");
                    exit(-1);
                }

                if(strcmp(malitiousFolder,"")){ //daca avem malitious folder , incepem verificarea de fisier malitios
                    int nr=0;
                    char nouaLocatie[1000];
                    //printf("Avem malitious folder, incepem cautarea!!!\n");
                    //printf("%d\n",infFisier.st_mode);
                    int status;
                    if(infFisier.st_mode!=33279){ // la mine verificarea de acces merge doar asa , in schimb se poate si asa : (!infFisier.st_mode & S_IRWXU) && (!infFisier.st_mode & S_IRWXG) && (!infFisier.st_mode & S_IRWXO) 

                        //printf("Fisierul este suspect!!!\n");
                        int pipefd[2]; //crearea pipe intre fiu si parinte , deoarece procesul copil va trebui sa apeleze un shellscript pentru a verifica daca este malitios

                        if(pipe(pipefd)<0){
                            perror("Eroare la creare pipe pentru comunicare intre parinte si fiu!!!\n");
                            exit(-1);
                        }

                        int pid=fork();

                        if(pid==0){ //suntem in copil
                            close(pipefd[0]); //inchidem capatul de citire

                            dup2(pipefd[1],1); //redirectam stdout al outputului in pipe
                            dup2(pipefd[1],2); //redirectam stderror al erorilor in pipe

                            close(pipefd[1]); //inchidem capatul de scriere


                            //apelam execlp pentru a verifica daca fisierul are caractere nepermise
                            execlp("/mnt/c/Users/rauld/OneDrive/Desktop/VisualStudio/LimbajulC/Anul2/SO_Proiect/verificareMalitios.sh","/mnt/c/Users/rauld/OneDrive/Desktop/VisualStudio/LimbajulC/Anul2/SO_Proiect/verificareMalitios.sh",nume,NULL);

                            printf("Eroare la execlp!!!\n");
                        }
                        else{

                            char buffer[strlen(nume)]; //cream buffer pentru a retine outputul de la script din procesul fiu

                            close(pipefd[1]); //inchidere capat ce scriere
                        
                            if((nr=read(pipefd[0],buffer,sizeof(buffer)))!=0){ //citirea in buffer al outputului fisierului copil prin pipe
                                buffer[strlen(nume)]='\0';
                                strcpy(nouaLocatie,malitiousFolder);
                                strcat(nouaLocatie,"/");
                                strcat(nouaLocatie,aux->d_name);

                                //printf("%s\n",nouaLocatie);

                                if(strcmp(buffer,nume)==0){ //daca fisierul a iesit ca fiind unsafe din shell script , atunci mutam
                                    isMalitious=1; //setam valoarea pe 1 , sa stim ca este malitios
                                    //printf("%s ||| %s\n",buffer,nume);
                                    if(rename(nume,nouaLocatie)!=0){ //muta fisierul din folderul sau in folderul pentru fisiere malitioase
                                        perror("Eroare mutare fisier malitios!!\n");
                                        exit(-1);
                                    }
                                }
                                else{
                                    buffer[strlen(nume)-2]='\0';
                                    if(strcmp(buffer,"SAFE\n")==0){
                                        isMalitious=0;
                                        //printf("%s\n",buffer);
                                    }
                                }
                            }

                            close(pipefd[0]); //inchidere capat de citire

                        }
                        wait(&status);
                    }
                }

                //printf("%s\n",malitiousFolder);

                //printf("%d\n",isMalitious);

                if(!isMalitious){ //in cazul in care nu este malitios , cream salvare in temporaryFile pentru datele fisierului

                    char bufferReg[4096];

                    sprintf(bufferReg,"%s : %s : %ld\n",cale,aux->d_name,infFisier.st_mtime);

                    if((write(fd,bufferReg,strlen(bufferReg)))==-1){ //scriem cu ajutorul unui buffer in temporaryFile
                        perror("Eroare scriere date fisier text!!!\n");
                        exit(-1);
                    }
                }
                else{
                    counterFisiereMalitioase++; //daca avem fisier malitios, crestem valoarea fisierelor malitioase gasite
                }
            }
        }
    }
    if(errno!=0){
        perror("Eroare citire director!!!\n");
        exit(-1);
    }
}

void verificareFisiere(const char* numeFileSimplu,const char* numePtTemporaryTextFile,const char* outputFolder,const char *malitiousFolder){ //verificare intre temporaryFile si posibilul fisierSnapshot creat
    int fd =0,mfd=0;
    struct stat infFisier;
    char numePtTextFile[1000];

    if(strcmp(outputFolder,"")!=0){ //daca avem output folder , cream calea noua pentru output al snapshotului
        snprintf(numePtTextFile, sizeof(numePtTextFile), "%s/%s.txt", outputFolder, numeFileSimplu);
    }
    else{
        strcpy(numePtTextFile,numeFileSimplu); //in caz contrar facem snapshot in folderul "mama"
        strcat(numePtTextFile,".txt");
    }

    printf("%s\n",numePtTextFile);

    mfd = open(numePtTextFile,  O_RDWR | O_CREAT, S_IRUSR | S_IWUSR, 0644); //deschide numePtTextFile pentru citire si scriere, creeaza fisierul daca nu exista si stabileste permisiunile pentru utilizator
    if (mfd == -1) { 
        perror("Eroare deschidere fisier temporar2!!!");
        exit(-1);
    }

    if (fstat(mfd, &infFisier) == -1) { //iau informatiile despre fisier cu ajutorul fstat
        perror("Eroare de a lua informatii despre fisier!!!");
        exit(-1);
    }

    close(mfd);//inchid ambele fisiere
    close(fd);

    if (infFisier.st_size == 0) { //verific daca fisierul pe care trebuie sa il deschid are scris ceva in el sau nu, in caz pozitiv scriu direct in el ca are temporaryFile

        mfd = open(numePtTextFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);//deschide numePtTextFile pentru citire si scriere, creeaza fisierul daca nu exista si stabileste permisiunile pentru utilizator
        if (mfd == -1) {
            perror("Eroare deschidere fisier temporar!!!");
            exit(-1);
        }

        fd = open(numePtTemporaryTextFile, O_RDONLY); //deschide numePtTemporaryTextFile pentru citire
        if (fd == -1) {
            perror("Eroare deschidere fisier temporar!!!");
            exit(-1);
        }

        char buffer[4096];
        ssize_t bytes_read;
        while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) { //cu ajutorul unui buffer citim bitii din temporaryFile
            if(bytes_read==-1){
                perror("Eroare citire din fisier temporar!!!\n");
                exit(-1);
            }
            if((write(mfd, buffer, bytes_read))==-1){//scriem in acelasi timp scriindu-i in file-ul nostru actual
                perror("Eroare scriere fisier main!!!\n");
            }
        }

        //printf("Informatia a fost copiata din fisierul temporar in %s\n", numePtTextFile);

        close(fd);//inchidem cele doua fisiere
        close(mfd);
    } else { //in cazul in care avem ceva in fisierul actual, se compara cu temporaryFile
        mfd = open(numePtTextFile, O_RDONLY); //se deschide numePtTextFile pentru citire
        if (mfd == -1) {
            perror("Eroare deschidere fisier temporar!!!");
            exit(-1);
        }

        fd = open(numePtTemporaryTextFile, O_RDONLY); //se deschide numePtTemporaryTextFile pentru citire
        if (fd == -1) {
            perror("Eroare deschidere fisier temporar!!!");
            exit(-1);
        }

        char buffer1[4096], buffer2[4096];
        int diferit=0; //variabila diferit setata pe 0 , insemnand ca nu sunt diferite
        ssize_t bytesRead1, bytesRead2;
        while (((bytesRead1 = read(mfd, buffer1, 4096)) > 0) && ((bytesRead2 = read(fd, buffer2, 4096)) > 0)) { //cu ajutorul a doi bufferi citim in acelasi timp si din fisier actual si din temporaryFile
            if (bytesRead1 != bytesRead2 || memcmp(buffer1, buffer2, bytesRead1) != 0) { //verificam daca numarul de biti cititi sunt asemanatori
                diferit=1; //setam variabila diferit pe 1 , insemnand ca fisierele sunt diferite
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

        close(mfd);//inchidem cele 2 fisiere
        close(fd);

        if(diferit){ //in cazul in care sunt diferite , scriem informatiile din temporaryFile in fisierul actual iar in ca contrar nu schimbam nimic
            printf("%s\n",numePtTextFile);
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

    int isOutput=0;
    int isMalitious=0;

    int pid;

    int j=1;

    for(int i=1;i<argc;i++){ //verificare daca avem ca argumente mai multe "-s" si "-o"
        if(!strcmp(argv[i],"-o")){
            isOutput++;
        }
        if(!strcmp(argv[i],"-s")){
            isMalitious++;
        }
    }

    if(isOutput>1 || isMalitious>1){
        //printf("> || >\n");
        printf("A-ti introdus date necorespunzatoare!!!\n");
        return 0;
    }

    if(isOutput==1 && isMalitious==0){
        //printf("1 || 0\n");
        if(strcmp(argv[1],"-o") || argc<2 || argc>13){
            printf("A-ti introdus date necorespunzatoare!!!\n");
            return 0;
        }
    }

    if(isOutput==0 && isMalitious==1){
        //printf("0 || 1\n");
        printf("A-ti introdus date necorespunzatoare!!!\n");
        return 0;
    }

    if(isOutput==1 && isMalitious==1){
        //printf("1 || 1\n");
        if(strcmp(argv[1],"-o") || strcmp(argv[3],"-s")){
            printf("A-ti introdus date necorespunzatoare!!!\n");
            return 0;
        }

        if(argc>15){
            printf("A-ti introdus date necorespunzatoare!!!\n");
            return 0;
        }
    }

    if(isOutput==0 && isMalitious==0){
        //printf("0 || 0\n");
        if(argc>11){
            printf("A-ti introdus date necorespunzatoare1!!!\n");
            return 0;
        }
    }

    int fisiereMalitioaseFolder[100]; //variabila in care tinem minte cate fisiere malitioase avem pentru fiecare folder
    for(int i=0;i<100;i++){
        fisiereMalitioaseFolder[i]=0;
    }

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
            //printf("%s\n",malitiousFolder);

            int pipefd[2];

            if(pipe(pipefd)<0){
                    perror("Eroare la creare pipe pentru comunicare intre parinte si fiu!!!\n");
                    exit(-1);
            }

            if((pid=fork())==-1){
                printf("Eroare creare proces!!!\n");
                exit(-1);
            }

            if(pid==0){

                int var=0; // ca sa stim daca verificam ca este malitios
                    
                close(pipefd[0]);
            
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
                citesteDirector(director,argv[i],fd,malitiousFolder);

                var=1;

                //printf("Counter Malitioase : %d\n", counterFisiereMalitioase);

                write(pipefd[1], &var, sizeof(var));
                write(pipefd[1], &counterFisiereMalitioase, counterFisiereMalitioase);

                verificareFisiere(numeFileSimplu,numePtTemporaryTextFile,outputFolder,malitiousFolder);

                close(pipefd[1]);

                exit(0);
            }
            else{
                close(pipefd[1]);

                int var;

                read(pipefd[0],&var,sizeof(var));

                if(var==1){
                    read(pipefd[0],&fisiereMalitioaseFolder[i],sizeof(fisiereMalitioaseFolder[i]));
                }

                close(pipefd[0]);
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
        printf("Child process %d terminated with PID %d , %d possible malitious files and exit code %d. \n",j,waitVariable,fisiereMalitioaseFolder[i],status);
        j++;
    }
    return 0;
}
