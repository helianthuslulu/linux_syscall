/*************************************************************************
	> File Name: ptrace1.c
	> Author: helianthus
	> Mail: helianthus.lu@gmail.com 
	> Created Time: 2013年12月19日 星期四 19时32分21秒
 ************************************************************************/
/*
 */
#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/user.h>/*for constants ORIG_EAX etc...*/

int main(void) 
{
	pid_t child_pid;
	long original_eax;
	struct user * user_space = (struct user *)0;
	child_pid = fork();
	if (child_pid == 0) {
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		execl("/bin/ls", "ls", "/home/helianthus/文档", "-l", NULL);
	} else {
		wait(NULL);
		original_eax = ptrace(PTRACE_PEEKUSER, child_pid, &user_space->regs.orig_eax, NULL);
		printf("the child made a system call %ld \n", original_eax);
		ptrace(PTRACE_CONT, child_pid, NULL, NULL);
	}
	return 0;
}
