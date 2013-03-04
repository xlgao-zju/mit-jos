/* See COPYRIGHT for copyright information. */

#include <inc/mmu.h>
#include <inc/memlayout.h>
#include <inc/trap.h>



###################################################################
# exceptions/interrupts
###################################################################

/* TRAPHANDLER defines a globally-visible function for handling a trap.
 * It pushes a trap number onto the stack, then jumps to _alltraps.
 * Use TRAPHANDLER for traps where the CPU automatically pushes an error code.
 *
 * You shouldn't call a TRAPHANDLER function from C, but you may
 * need to _declare_ one in C (for instance, to get a function pointer
 * during IDT setup).  You can declare the function with
 *   void NAME();
 * where NAME is the argument passed to TRAPHANDLER.
 */
#define TRAPHANDLER(name, num)						\
	.globl name;		/* define global symbol for 'name' */	\
	.type name, @function;	/* symbol type is function */		\
	.align 2;		/* align function definition */		\
	name:			/* function starts here */		\
	pushl $(num);							\
	jmp _alltraps

/* Use TRAPHANDLER_NOEC for traps where the CPU doesn't push an error code.
 * It pushes a 0 in place of the error code, so the trap frame has the same
 * format in either case.
 */
#define TRAPHANDLER_NOEC(name, num)					\
	.globl name;							\
	.type name, @function;						\
	.align 2;							\
	name:								\
	pushl $0;							\
	pushl $(num);							\
	jmp _alltraps

.text

/*
 * Lab 3: Your code here for generating entry points for the different traps.
 */
TRAPHANDLER_NOEC(idivide,T_DIVIDE);	
TRAPHANDLER_NOEC(idebug,T_DEBUG);	
TRAPHANDLER_NOEC(inmi,T_NMI);	
TRAPHANDLER_NOEC(ibrkpt,T_BRKPT);	
TRAPHANDLER_NOEC(ioflow,T_OFLOW);
TRAPHANDLER_NOEC(ibound,T_BOUND);	
TRAPHANDLER_NOEC(iillop,T_ILLOP);	
TRAPHANDLER_NOEC(idevice,T_DEVICE);	
TRAPHANDLER(idblflt,T_DBLFLT);
TRAPHANDLER(itss,T_TSS);		
TRAPHANDLER(isegnp,T_SEGNP);		
TRAPHANDLER(istack,T_STACK);	
TRAPHANDLER(igpflt,T_GPFLT);	
TRAPHANDLER(ipgflt,T_PGFLT);	
TRAPHANDLER_NOEC(ifperr,T_FPERR);		
TRAPHANDLER(ialing,T_ALIGN);	
TRAPHANDLER_NOEC(imchk,T_MCHK);		
TRAPHANDLER_NOEC(isimderr,T_SIMDERR);		
TRAPHANDLER_NOEC(isyscall,T_SYSCALL);		
TRAPHANDLER_NOEC(idefault,T_DEFAULT);	

/*
 * Lab 3: Your code here for _alltraps
 */
_alltraps:
  pushl %ds;
  pushl %es;
  pushal;
  movl $GD_KD, %eax;
  movw %ax, %ds;
  movw %ax, %es;
  pushl %esp
  call trap
