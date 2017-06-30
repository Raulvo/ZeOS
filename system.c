/*
 * system.c -
 */

#include <segment.h>
#include <types.h>
#include <interrupt.h>
#include <hardware.h>
#include <system.h>
#include <sched.h>
#include <mm.h>
#include <io.h>
#include <utils.h>
#include <zeos_mm.h> /* TO BE DELETED WHEN ADDED THE PROCESS MANAGEMENT CODE TO BECOME MULTIPROCESS */

/*
 * PH_USER_START = 0x1 MiB.
 * KERNEL_START, +1, +2 memory positions are written by build.c.
 * With protected mode we can reach addresses > 1 MiB.
 */

int (*usr_main)(void) = (void *) PH_USER_START; 
unsigned int *p_sys_size = (unsigned int *) KERNEL_START;
unsigned int *p_usr_size = (unsigned int *) KERNEL_START+1;
unsigned int *p_rdtr = (unsigned int *) KERNEL_START+2;

/*
 *   Main entry point to ZEOS Operating System kernel
 */

__attribute__((__section__(".text.main.system")))
int main(void) {

    set_eflags();

    /* Define the kernel segment registers */
    set_seg_regs();

    printk("Kernel Loaded!    ");

    /* Initialize hardware data */
    setGdt(); /* Definicio de la taula de segments de memoria */
    setIdt(); /* Definicio del vector de interrupcions */
    setTSS(); /* Definicio de la TSS */


    /* Initialize Memory */
    init_mm();

    /* Initialize an address space to be used for the monoprocess version of ZeOS */

    monoprocess_init_addr_space(); /* TO BE DELETED WHEN ADDED THE PROCESS MANAGEMENT CODE TO BECOME MULTIPROCESS */

    /* Initialize Scheduling */
    init_sched();

    /* Initialize idle task data */
    init_idle();
    /* Initialize task 1 data */
    init_task1();

    /* Move user code/data now (after the page table initialization) */
    copy_data((void *) KERNEL_START + *p_sys_size, usr_main, *p_usr_size);


    printk("Entering user mode...");

    enable_int();

    /*
     * We return from a 'theorical' call to a 'call gate' to reduce our privileges
     * and going to execute 'magically' at 'usr_main'...
     */
    return_gate(__USER_DS, __USER_DS, USER_ESP, __USER_CS, L_USER_START);

    /* The execution never arrives to this point */
    return 0;
}


