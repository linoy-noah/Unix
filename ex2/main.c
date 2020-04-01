//server
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "threadManager.h"
#include <dirent.h>
#include <limits.h>

#define PORT 0x0da2 // 3492
#define IP_ADDR 0x7f000001
#define QUEUE_LEN 20
#define LINE_LEN 1024

typedef struct arg
{
   // ThreadPoolManager *t;
    int * found;
    char** root ;
    char* filename;
   // char** filepath;
   //char filepath[PATH_MAX];
} Taskarg;
static char filepath[PATH_MAX];
//void search_file(ThreadPoolManager *t, int* task_num, char* root, char* filename, char* filepath){

/*
void search_file(Taskarg* arg){
    Taskarg* targ=NULL;
    Task* ttask=NULL;
    char* n_root=NULL;
    char* temp_root=NULL;
    if(arg->found>0){
        DIR* d;
        struct dirent * t;
        d=opendir(arg->root);
        for(t=readdir(d);t!=NULL && arg->found>0;t=readdir(d)){
            temp_root= (char*)malloc(strlen(arg->root)+strlen(t->d_name)+2);
            if(temp_root==NULL){
                perror("ALLOC");
                return;
            }
            // temp_root="";
            strcat(temp_root,*arg->root);
            strcat(temp_root,"/");
            strcat(temp_root,t->d_name);
            char* buf=(char*)malloc(PATH_MAX);
            if(buf==NULL){
                perror("ALLOC");
                return;
            }
            n_root = realpath(temp_root, buf);
            free(temp_root);
            if (n_root) {
                printf("This source is at %s.\n", buf);
            } else {
                perror("realpath");
                return;
            }

            if(!(strcmp(t->d_name,arg->filename))){
                arg->found=0;
                free(arg->filepath);

                arg->filepath= (char*)malloc(strlen(n_root)+1);
                if(arg->filepath==NULL){
                    perror("alloc");
                    return;
                }

                strcpy(arg->filepath,n_root);
            }
            else{

                //++(arg->task_num);
                targ=(Taskarg*)malloc(sizeof(Taskarg));
                if(targ==NULL){
                    perror("alloc");
                    return;
                }
                targ->found=arg->found;
                targ->t=arg->t;
                targ->filepath=arg->filepath;
                targ->filename=arg->filename;
                targ->root=n_root;
                ttask=(Task*)malloc(sizeof(Task));
                if(ttask==NULL){
                    perror("alloc");
                    return;
                }
                ttask->args=targ;
                ttask->f=(void*)search_file;
                ThreadPoolInsertTask(arg->t, &ttask);


            }
        }
    }
    //--(arg->task_num);
    free(arg->root);
    return;
}
*/
void searchFile(Taskarg* arg){
    char** curr=arg->root;
    if(arg->found>0){
        while(*curr!=NULL && arg->found>0) {
            DIR *d;
            struct dirent *t;
            d = opendir(*curr);
            if (d != NULL) {


                char *n_root = NULL;
                while (NULL != (t = readdir(d)) && arg->found > 0) {
                    if (strcmp(t->d_name, ".") && strcmp(t->d_name, "..") &&strcmp(t->d_name, "lost+found")) {

                    n_root = (char *) malloc((strlen(*curr)) + (strlen(t->d_name)) + 2);
                    if (n_root == NULL) {
                        perror("alloc");
                        return;
                    }
                    strcat(n_root, *curr);
                    strcat(n_root, "/");
                    strcat(n_root, t->d_name);

                    if (!strcmp(t->d_name, arg->filename)) {
                        *(arg->found) = 0;
                        //free(arg->filepath);
                        strcpy(filepath ,n_root);
                    }

                    else {

                            char **t_root = (char **) malloc(sizeof(char *) * 2);
                            if (t_root == NULL) {
                                perror("alloc");
                                return;
                            }
                            t_root[0] = n_root;
                            t_root[1] = NULL;
                            Taskarg *targ = (Taskarg *) malloc(sizeof(Taskarg));
                            if (targ == NULL) {
                                perror("alloc");
                                return;
                            }
                            //targ->filepath = arg->filepath;
                            targ->found = arg->found;
                            targ->filename = arg->filename;
                            targ->root = t_root;
                            searchFile(targ);
                            //free(targ);
                        }
                    }
                }
            }
            curr++;
        }
    }

    curr=arg->root;
    while(*curr){
        free(*curr);
        curr++;
    }
   // free(arg->root);
    //free(arg);
    return;
}
void handleClient(int fd, int n,ThreadPoolManager *t)
{
    //strcpy(filepath,"");
    //t->found=1;
    int isfound=1;
    int nrecv;
    int size;
    char* filename;
    //char** filepath =(char**)malloc(sizeof(char*));
   // *filepath=NULL;


    if ((nrecv = recv(fd, (char *)&size, sizeof(int), 0)) < 0)
    {
        perror("recv");
        return;
    }//recv lenght of the file name
    filename=(char*)malloc(size);
    if ((nrecv = recv(fd, filename, size, 0)) < 0)
    {
        perror("recv");
        return;
    }//recv the file name

    int count=0;
    char** root_folders;
    DIR* d;
    struct dirent* temp;
    d=opendir("/");
    while(temp=readdir(d)){
        if (strcmp(temp->d_name, ".") && strcmp(temp->d_name, "..")&&strcmp(temp->d_name, "lost+found")){
        count++;}
    }
    root_folders=(char**)malloc((sizeof(char*)*count)+1);
    if(root_folders==NULL){
        perror("alloc");
        return;
    }
    d=opendir("/");
    char* n_root;
    char** curr=root_folders;
    while(NULL!=(temp=readdir(d)) && isfound>0){
        if (strcmp(temp->d_name, ".") && strcmp(temp->d_name, "..")&&strcmp(temp->d_name, "lost+found")){
        n_root=(char*)malloc(strlen(temp->d_name)+2);
        strcat(n_root,"/");
        strcat(n_root,temp->d_name);

        if(!(strcmp(temp->d_name,filename))){
            isfound=0;
            //free(filepath);
            strcpy(filepath,n_root);
        }
        else{
            *curr=n_root;
            curr++;
        }
    }
    }
    curr=NULL;


    if(isfound>0){
        int div,extra,i,b;
        int MorN;// r morn=count,morn=thread;
        Task* tas=NULL;
        Taskarg* arg=NULL;
        char** task_roots;
        if(count<n){
            div=1;
            extra=0;
            MorN=count;
        }
        else{
            div=count/n;
            extra=count%n;
            MorN=n;
        }
        if(extra){
            for(curr=root_folders,i=0;i<n-1;i++){
                task_roots=(char**)malloc((sizeof(char*)*div)+1);
                if(task_roots==NULL){
                    perror("alloc");
                    return;
                }
                for(b=0;b<div;b++){
                    task_roots[b]=*curr;
                    curr++;
                }
                task_roots[b]=NULL;
                 arg=(Taskarg*)malloc(sizeof(Taskarg));
                 if(arg==NULL){
                     perror("alloc");
                 }
                 arg->filename=filename;
                 //arg->filepath=filepath;
                 //arg->t=t;
                 arg->found=&isfound;
                 arg->root=task_roots;
                 tas=(Task*)malloc(sizeof(Task));
                 if(tas==NULL){
                     perror("alloc");
                     return;
                 }
                 tas->args=arg;
                 tas->f=(void*)searchFile;
                ThreadPoolInsertTask(t,&tas);



            }
            task_roots=(char**)malloc((sizeof(char*)*(div+extra))+1);
            if(task_roots==NULL){
                perror("alloc");
                return;
            }
            for(b=0;b<(div+extra);b++){
                task_roots[b]=*curr;
                curr++;
            }
            task_roots[b]=NULL;
            arg=(Taskarg*)malloc(sizeof(Taskarg));
            if(arg==NULL){
                perror("alloc");
            }
            arg->filename=filename;
            //arg->filepath=filepath;
           // arg->t=t;
            arg->found=&isfound;
            arg->root=task_roots;
            tas=(Task*)malloc(sizeof(Task));
            if(tas==NULL){
                perror("alloc");
                return;
            }
            tas->args=arg;
            tas->f=(void*)searchFile;
            ThreadPoolInsertTask(t,&tas);
        }
        else{
            for(curr=root_folders,i=0;i<MorN;i++){
                task_roots=(char**)malloc((sizeof(char*)*div)+1);
                if(task_roots==NULL){
                    perror("alloc");
                    return;
                }
                for(b=0;b<div;b++){
                    task_roots[b]=*curr;
                    curr++;
                }
                task_roots[b]=NULL;
                arg=(Taskarg*)malloc(sizeof(Taskarg));
                if(arg==NULL){
                    perror("alloc");
                }
                arg->filename=filename;
                //arg->filepath=filepath;
               // arg->t=t;
                arg->found=&isfound;
                arg->root=task_roots;
                tas=(Task*)malloc(sizeof(Task));
                if(tas==NULL){
                    perror("alloc");
                    return;
                }
                tas->args=arg;
                tas->f=(void*)searchFile;
                ThreadPoolInsertTask(t,&tas);

            }
        }

    }


    /*
    Task* task=NULL;
    Taskarg *arg=NULL;
    arg=(Taskarg*)malloc(sizeof(Taskarg));
    if(arg==NULL){
        perror("alloc");
        return;
    }
    arg->filename=filename;
    arg->filepath=filepath;
    arg->root=(char*)malloc(strlen("/")+1);
    if(arg->root==NULL){
        perror("alloc");
        return;
    }
    strcpy(arg->root,"/");
    arg->t=t;
    arg->found=&isfound;
    task = (Task*)malloc(sizeof(Task));
    if(task==NULL){
        perror("alloc");
        return;
    }
    task->args=arg;
    task->f=(void*)search_file;
    ThreadPoolInsertTask(t,&task);*/
    //plan the task split and insert tasks  to the tpm for each task we insert ++task_num
    while(isfound>0 && t->queue->task_num>0){} //if file not exist wait until the search over, if its found the loop stops
    if(isfound>0){
        //*filepath =(char*)malloc(sizeof(char)*15);
        strcpy(filepath,"file not found");
    }
    size=strlen(filepath)+1;
    if (send(fd, (char *)&size, sizeof(int), 0) < 0)
    {
        perror("send");
        return;
    }
    if (send(fd, filepath, size, 0) < 0)
    {
        perror("send");
        return;
    }

free(root_folders);
    return;
}



int main(int argc,char* argv[])
{

    if (argc < 2)
    {
        perror("not enough arguments");
        return 1;
    }

    printf("before");//
    int n;
    n=atoi(argv[1]);
    printf("%d",n);//
    ThreadPoolManager* t = NULL;
    t= (ThreadPoolManager*)malloc(sizeof(ThreadPoolManager));
    if(t==NULL){
        perror("alloc");
        return 1;
    }
    //t->found=1;
    t->queue=(TaskQueue*)malloc (sizeof(TaskQueue));
    if(t->queue==NULL){
        perror("alloc");
        return -1;
    }
    t->queue->tail=NULL;
    t->queue->head=NULL;
    t->t=NULL;
    if( ThreadPoolInit(t, n) <0){
        perror("init tpm");
        return 1;
    }
    int listenS = socket(AF_INET, SOCK_STREAM, 0);

    if (listenS < 0)
    {
        perror("socket");
        return 1;
    }
    struct sockaddr_in s = {0};
    s.sin_family = AF_INET;
    s.sin_port = htons(PORT);
    s.sin_addr.s_addr = htonl(IP_ADDR);
    if (bind(listenS, (struct sockaddr *)&s, sizeof(s)) < 0)
    {
        perror("bind");
        return 1;
    }
    if (listen(listenS, QUEUE_LEN) < 0)
    {
        perror("listen");
        return 1;
    }




    while (1)
    {
        struct sockaddr_in clientIn;
        int clientInSize = sizeof clientIn;
        int newfd = accept(listenS, (struct sockaddr *)&clientIn, (socklen_t *)&clientInSize);
        if (newfd < 0)
        {
            perror("accept");
            return 1;
        }
        handleClient(newfd,n,t);
        close(newfd);
    }
    close(listenS);
    ThreadPoolDestroy(t);
    return 0;
}