#ifndef C_COROUTINE_H
#define C_COROUTINE_H

#include <ucontext.h>

#define STACK_SIZE (1024*1024)
#define DEFAULT_COROUTINE 16

#define COROUTINE_DEAD 0
#define COROUTINE_READY 1
#define COROUTINE_RUNNING 2
#define COROUTINE_SUSPEND 3

struct schedule;

typedef void (*coroutine_func)(struct schedule* S, void* arg);

struct schedule* coroutine_open(void);

void coroutine_close(struct schedule* S);

int  coroutine_new(struct schedule* S,coroutine_func func,void *arg);

void coroutine_yield(struct schedule* S);

void coroutine_resume(struct schedule* S,int id);

int  coroutine_running(struct schedule *S);
int  schedule_status(struct schedule* S);

#endif
