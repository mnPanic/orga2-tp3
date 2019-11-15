/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================
*/

#include "syscall.h" 

void handler(void);

void task() {
	breakpoint();
    char* message = "Tarea A1";
    syscall_talk(message);
    syscall_setHandler(handler);

    while(1) { __asm __volatile("mov $1, %%eax":::"eax"); }
}

void handler() {
    syscall_informAction(Center);
}
