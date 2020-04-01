#include "threadManager.h"

pthread_mutex_t qlock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t qcond = PTHREAD_COND_INITIALIZER;

void* popThreadM(struct ThreadPoolManager *tpm)
{
    for (;;)
    {
        struct TaskNode *temp = popTask(tpm->queue);
        Task* t=temp->task;
        /*if(tpm->found!=0){
            tpm->found++;
            t->f(t->args);
        }*/
        t->f(t->args);
        --(tpm->queue->task_num);
        free(t->args);
        free(t);
        free(temp);
    }
    return NULL;
}

TaskNode *popTask(TaskQueue *queue)
{
    TaskNode *item;

    pthread_mutex_lock(&qlock);
    while (queue->head == NULL)
        pthread_cond_wait(&qcond, &qlock);

    item = queue->head;
    queue->head = queue->head->next;
    pthread_mutex_unlock(&qlock);
    return item;
}

void pushTask(TaskQueue *queue, TaskNode *node)
{
    pthread_mutex_lock(&qlock);
    ++(queue->task_num);
   if(queue->tail){
       queue->tail->next=node;
   }
    queue->tail = node;
    if(queue->head==NULL){
        queue->head=queue->tail;
    }
    pthread_cond_signal(&qcond);
    pthread_mutex_unlock(&qlock);
}

int ThreadPoolInit(struct ThreadPoolManager * t, int n)
{

    printf("im in init tpm");
    int i;
    t->t = (pthread_t **)malloc(n*(sizeof(pthread_t*)));
    if (t->t == NULL)
    {
        perror("allocation error");
        return -2;
    }
    for (i = 0; i < n; i++)
    {
        t->t[i] = (pthread_t *)malloc(n*(sizeof(pthread_t)));
        if (pthread_create(t->t[i], NULL, (void*)popThreadM, (void*)t))
        {
            perror("failed to create thread");
            return -1;
        }
    }
}
int ThreadPoolInsertTask(struct ThreadPoolManager *t, struct Task **task)
{

    TaskNode *tas = (TaskNode *)malloc(sizeof(TaskNode));
    if (task == NULL)
    {
        perror("allocation error");
        return -2;
    }
    tas->task = *task;
    tas->next = NULL;
    pushTask(t->queue, tas);
    return 0;
}

void ThreadPoolDestroy(struct ThreadPoolManager *t)
{
    int n= sizeof(t->t)/sizeof(pthread_t);
    int i=0;
    for(i=0;i<n;i++){
        pthread_exit((pthread_t *)t->t[i]);
    }
    free(t->queue);
    free(t);

}
