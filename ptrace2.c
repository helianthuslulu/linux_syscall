/*************************************************************************
	> File Name: ptrace2.c
	> Author: helianthus
	> Mail: helianthus.lu@gmail.com 
	> Created Time: 2013年12月19日 星期四 20时24分45秒
 ************************************************************************/
/*
 * 注释1：
 *eax “累加器”，它是很多加法乘法指令的缺省寄存器
 *ebx “基地址”寄存器，在内存寻址时存放基地址
 *ecx “计数器”，是重复(REP)前缀指令和LOOP指令的内定计数器
 *edx 总是被用来放整数除法产生的余数
 * 在i386体系中，系统调用号将放入%eax,它的参数则依次放在%ebx,%ecx,%edx,%esi和%edi中。如：
 * write(2, "Hi", 3);
 *的汇编形式大概是这样：
 *movl $4, %eax
 *movl $2, %ebx
 *movl $Hi, %ecx
 *movl $5, %edx
 *int $0x80
 *
 *注释2：ptrace会在什么时候出现呢？
 *在执行系统调用之前，内核会先检查当前进程是否处于“被跟踪”的状态；如果是的话，
 *内核会暂停当前进程并将控制权交给
 *跟踪进程，使得跟踪进程得以查看或者修改被跟踪进程的寄存器
*/
#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/user.h>

int main(void)
{
	pid_t child;
	long original_eax, eax;
	struct user * user_space = (struct user *)0;
	long params[3];
	int status;
	int insyscall = 0;
	child = fork();
	if (child == 0) {
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		execl("/bin/ls", "ls", NULL);
	} else {
		while (1) {
			wait(&status);//检查子进程是否已经退出，这是检查子进程是被ptrace暂停还是退出的典型用法
			if (WIFEXITED(status))
				break;
			//ptrace的第一个参数设置为PTRACE_PEEKUSER来查看write系统调用的参数
			//当系统调用返回时，返回值存放在%eax寄存器中
			original_eax = ptrace(PTRACE_PEEKUSER, child, &user_space->regs.orig_eax, NULL);
			if (original_eax == SYS_write) {
				if (insyscall == 0) {//syscall entry
					insyscall = 1;
					params[0] = ptrace(PTRACE_PEEKUSER, child, &user_space->regs.ebx, NULL);
					params[1] = ptrace(PTRACE_PEEKUSER, child, &user_space->regs.ecx, NULL);
					params[2] = ptrace(PTRACE_PEEKUSER, child, &user_space->regs.edx, NULL);			
					printf("write called with %ld, %ld, %ld", params[0], params[1], params[2]);
				} else {
					eax = ptrace(PTRACE_PEEKUSER, child, &user_space->regs.eax, NULL);
					printf("write returned with %ld", (long int)&user_space->regs.eax);
					insyscall = 0;
				}
			}
			//将ptrace的第一个参数设置为PTRACE_SYSCALL,内核会在系统调用的入口或者出口处暂停子进程；这等同于使用PTRACE_CONT
			//并在下一个系统调用的入口/出口处暂停子进程
			ptrace(PTRACE_SYSCALL, child, NULL, NULL);
		}
	}
	return 0;
}
