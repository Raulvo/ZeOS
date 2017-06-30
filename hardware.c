/*
 * hardware.c
 */


#include <types.h>
#include <segment.h>
#include <sched.h>

extern unsigned int *p_rdtr; // KERNEL_START + 2
DWord get_eflags(void) {
    register DWord flags;
    __asm__ __volatile__(
        "pushfl\n\t"
        "popl %0"
        : "=q" (flags) );

    return flags;
}

void set_eflags(void) {
    __asm__ __volatile__(
        "pushl $0\n\t"
        "popfl"
        :
        :
        : "memory");
}

void set_idt_reg(Register * idt) {
    __asm__ __volatile__(
        "lidtl (%0)"
        : /*no output*/
        : "r" (idt)
        : "memory");
}

void set_gdt_reg(Register * gdt) {
    __asm__ __volatile__(
        "lgdtl (%0)"
        : /*no output*/
        : "r" (gdt)
        : "memory");
}

void set_ldt_reg(Selector ldt) {
    __asm__ __volatile__(
        "lldtw %0"
        : /*no output*/
        : "r" (ldt)
        : "memory");
}

void set_task_reg(Selector tr) {
    __asm__ __volatile__(
        "ltrw %0"
        : /*no output*/
        : "r" (tr)
        : "memory");
}

/**************************
 ** setSegmentRegisters ***
 **************************
 * Set properly all the registers, used
 * at initialization code.
 *   DS, ES, FS, GS, SS <- DS
 *   SS:ESP <- DS:DATA_SEGMENT_SIZE
 *         (the stacks grows towards 0)
 *
 * cld -> gcc2 wants DF (Direction Flag (eFlags.df))
 *        always clear.
 */

/*
 * KERNEL_ESP is an immediate value. If it was put into memory,
 * after changing ss register a segment violation would occur,
 * when trying to load KERNEL_ESP from a parameter passed into
 * the stack.
 * [ss_base_address+@logical(KERNEL_ESP)] maps now to a different address.
 * 
 * __KERNEL_DS is put into register. Writes from memory or immediate
 * to segment registers are not allowed.
 */
void set_seg_regs(void) {
    register Word data_segment = __KERNEL_DS;
    __asm__ __volatile__(
        "cld\n\t"
        "mov (%%esp),%%eax\n\t"
        "mov %0,%%ds\n\t"
        "mov %0,%%es\n\t"
        "mov %0,%%fs\n\t"
        "mov %0,%%gs\n\t"
        "mov %0,%%ss\n\t"
        "mov %1,%%esp\n\t"
        "mov %%eax,(%%esp)"
        : /* no output */
        : "r" (data_segment), "i" (KERNEL_ESP)
        : "%eax","memory");
}

/*
 * Perform a far return (lret has CB instruction opcode)
 * lret pops eip and then cs from the stack. Because it is
 * an inter-privilege-level return (we are in protected mode), 
 * esp and ss are also popped to perform the stack switch 
 * involved in an inter-privilege-level return.
 * Processor continues execution from the new code segment 
 * and instruction pointer.
 * Segment registers ds, es, fs, and gs are cleared if their RPL
 * is not allowed under the new privilege level.
 * 
 * See https://c9x.me/x86/html/file_module_x86_id_280.html
 */ 


__attribute__((noreturn))
void return_gate(Word ds, Word ss, DWord esp, Word cs, DWord eip) {
    __asm__ __volatile__ (
        "mov %0,%%es\n\t"
        "mov %0,%%ds\n\t"
        "movl %2, %%eax\n\t"
        "addl $12, %%eax\n\t"
        "movl %5,(%%eax)\n\t"
        "pushl %1\n\t"       /* user ss */
        "pushl %2\n\t"       /* user esp */
        "pushl %3\n\t"       /* user cs */
        "pushl %4\n\t"       /* user eip */
        "lret"
        : /*no output*/
        : "r" (ds), "r" (ss), "r" (esp), "r" (cs), "r" (eip), "d" (*p_rdtr)
        : "%eax","memory");
    while(1); /* Remove warning of returning in a noreturn function */
}

/*
 * enable_int: Set interruput mask
 *
 *    register 0x21:
 *    7 6 5 4 3 2 1 0
 *    x x x x x x x x
 *
 *    bit 0 : Timer
 *    bit 1 : Keyboard
 *    bit 2 : PIC cascading
 *    bit 3 : 2nd Serial Port
 *    bit 4 : 1st Serial Port
 *    bit 5 : Reserved
 *    bit 6 : Floppy disk
 *    bit 7 : Reserved
 *
 *
 *   x = 0 -> enabled
 *   x = 1 -> disabled
 */

void enable_int(void) {
    __asm__ __volatile__(
        "movb %0,%%al\n\t"
        "outb %%al,$0x21\n\t"
        "call delay\n\t"
        "sti"
        : /*no output*/
        : "i" (0xbf)       /* 0xFF = 11111111 -> all bits disabled */
        : "%al" );
}

void delay(void) {
    __asm__ __volatile__(
        "jmp a\na:\t"
        : : );
}


