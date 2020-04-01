

#ifndef UNTITLE_THREADMANAGER_H
#define UNTITLE_THREADMANAGER_H
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

typedef struct Task
{
    void *(*f)(void *);
    void *args;
} Task;
typedef struct TaskNode
{
    Task *task;
    struct TaskNode *next;
} TaskNode;

typedef struct TaskQueue
{
    TaskNode *head;
    TaskNode *tail;
    int task_num;
} TaskQueue;

TaskNode *popTask(TaskQueue *queue);
void pushTask(TaskQueue *queue, TaskNode *node);
///////////////

typedef struct ThreadPoolManager
{
    TaskQueue *queue;
    pthread_t **t;
    // int found;
} ThreadPoolManager;

void* popThreadM(struct ThreadPoolManager *t);

int ThreadPoolInit(struct ThreadPoolManager *t, int n);
int ThreadPoolInsertTask(struct ThreadPoolManager *t, struct Task **task);
void ThreadPoolDestroy(struct ThreadPoolManager *t);

#endif //
