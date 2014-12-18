#include "coroutine.h"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

struct coroutine
{
    ucontext_t ctx;
    coroutine_func func;
    void *arg;
    int status;
    char stack[STACK_SIZE];
};

struct schedule
{
    ucontext_t main;
    int running;

    int nco;
    struct coroutine * C[DEFAULT_COROUTINE];
};

static void
mainfunc(struct schedule *S)
{
    int id = S->running;
    if(id != -1){
        struct coroutine *C = S->C[id];
        C->func(S, C->arg);
        C->status = COROUTINE_DEAD;
        S->running = -1;
    }
}


void 
coroutine_resume(struct schedule *S , int id)
{
    assert(id >=0 && id < DEFAULT_COROUTINE);

    struct coroutine *C = S->C[id];

    switch(C->status){
        case COROUTINE_READY:
            getcontext(&C->ctx);
    
            C->ctx.uc_stack.ss_sp = C->stack;
            C->ctx.uc_stack.ss_size = STACK_SIZE;
            C->ctx.uc_link = &S->main;
            S->running = id;
            C->status = COROUTINE_RUNNING;
            makecontext(&C->ctx,(void (*)(void))(mainfunc),1,S);
            
            swapcontext(&S->main, &C->ctx);
            break;
        case COROUTINE_SUSPEND:
            S->running = id;
            C->status = COROUTINE_RUNNING;
            swapcontext(&S->main, &C->ctx);
            break;
        default: ;
    }
}

void
coroutine_yield(struct schedule *S)
{
    int id = S->running;
    if(id != -1 ){
        struct coroutine *C = S->C[id];
        C->status = COROUTINE_SUSPEND;
        S->running = -1;

        swapcontext(&C->ctx, &S->main);
    }
}

struct schedule *
coroutine_open(void) {
    struct schedule * S = (struct schedule *)malloc(sizeof(*S));
    S->running = -1;
    S->nco = 0;
    memset(S->C, 0, sizeof(S->C));
    return S;
}

void
coroutine_close(struct schedule* S) {
    int id = 0;
    for(id = 0; id < S->nco; id++) {
        free(S->C[id]);
    }
    free(S);
}

int 
coroutine_new(struct schedule *S,coroutine_func func,void *arg)
{
    int id;
    for(id = S->nco; id < DEFAULT_COROUTINE; id++) {
        if(NULL == S->C[id]) {
            S->C[id] = malloc(sizeof(struct coroutine));
            struct coroutine *C = S->C[id];
            C->status = COROUTINE_READY;
            C->func = func;
            C->arg = arg;
            S->nco++;
            return id;
        }
    }
    return -1;
}

int 
schedule_status(struct schedule *S)
{
    int i;
    if (S->running != -1){
        return 1;
    }else{
        for(i = 0; i < S->nco; i++){
            if(S->C[i]->status != COROUTINE_DEAD){
                return 1;
            }
        }
    }
    return 0;
}

int 
coroutine_running(struct schedule * S) {
	return S->running;
}
