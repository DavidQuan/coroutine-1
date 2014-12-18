#include "coroutine.h"
#include <stdio.h>

struct args {
	int n;
};

static void
foo(struct schedule * S, void *ud) {
	struct args * arg = ud;
	int start = arg->n;
	int i;
	for (i=0;i<3;i++) {
		printf("coroutine %d : %d\n", coroutine_running(S), start + i);
		coroutine_yield(S);
	}
}

static void
foo2(struct schedule * S, void *ud) {
	struct args * arg = ud;
	int start = arg->n;
	int i;
	for (i=0;i<2;i++) {
		printf("coroutine %d : %d\n", coroutine_running(S), start + i);
		coroutine_yield(S);
	}
}

static void
test(struct schedule *S) {
	struct args arg1 = { 0 };
	struct args arg2 = { 100 };

    // 协程co1 ，对应的协程函数是 foo
	int co1 = coroutine_new(S, foo, &arg1);
    // 协程co2 ，对应协程函数是 foo2
	int co2 = coroutine_new(S, foo2, &arg2);

	printf("main start\n");
	while (schedule_status(S)) {
		coroutine_resume(S,co1);
		coroutine_resume(S,co2);
	} 
	printf("main end\n");
}

int 
main() {
    // 初始化协程调度器
	struct schedule * S = coroutine_open();
	test(S);
    // 关闭协程调度器
	coroutine_close(S);
	
	return 0;
}

