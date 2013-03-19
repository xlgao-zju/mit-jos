// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	pte_t pte;
	envid_t id;

	if ((err & FEC_WR) != FEC_WR)
		panic("pgfautl:bad err!");

	pte = uvpt[PGNUM(addr)];

	if ((pte & PTE_COW) != PTE_COW)
		panic("pgfault:bad pte!");

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	//   No need to explicitly delete the old page's mapping.

	// LAB 4: Your code here.
	r = sys_page_alloc(0, PFTEMP, PTE_P|PTE_W|PTE_U);
	if (r != 0)
		panic("pgfault:sys_page_alloc %e", r);
	
	//I really don't that i need to do the ROUNDDOWN!
	//要对addr所在的页进行COW，所以要找到那页的首地址才能进行从定向！
	addr = (void *)ROUNDDOWN(addr, PGSIZE);
	memmove(PFTEMP, addr, PGSIZE);

	r = sys_page_map(0, PFTEMP, 0, addr, PTE_P|PTE_W|PTE_U);
	if (r != 0)
		panic("pgfault:sys_page_map %e", r);

	//panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	void * va = (void *)(pn * PGSIZE);
	pte_t pte;
	pte = uvpt[pn];

	if ((pte & PTE_W) || (pte & PTE_COW)){
		r = sys_page_map(0, va, envid, va, PTE_P | PTE_U | PTE_COW);
	if (r != 0)
		panic("duppage:sys_page_map %e", r);
	r = sys_page_map(0, va, 0, va, PTE_P | PTE_U | PTE_COW);
	if (r != 0)
		panic("duppage:sys_page_map %e", r);
	} else {
		r = sys_page_map(0, va, envid, va, PTE_P | PTE_U);
		if (r != 0)
			panic("duppage:sys_page_map %e", r);
	}
	//panic("duppage not implemented");
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 4: Your code here.
	envid_t id;
	int r, i, j, pn;


	set_pgfault_handler(pgfault);
	id = sys_exofork();	

	if (id == 0){
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}
	
	for (i = 0; i < PDX(UTOP); i++) {
		if (uvpd[i] & PTE_P) {
			for (j = 0; j < NPTENTRIES; j++) {
				pn = i* NPTENTRIES + j;
				if (pn == PGNUM(UXSTACKTOP - PGSIZE)) 
					continue;
				if (uvpt[pn] & PTE_P) 
					duppage(id, pn);
			}
		}
	}

	r = sys_page_alloc(id, (void *)(UXSTACKTOP - PGSIZE), PTE_W | PTE_P | PTE_U);
	if (r != 0)
		panic("fork:alloc failed %e",r);
	r = sys_env_set_pgfault_upcall(id, thisenv->env_pgfault_upcall);
	if (r != 0)
		panic("fork:set pgfault failed %e", r);

	r = sys_env_set_status(id, ENV_RUNNABLE);
	if (r != 0)
		panic("fork:set status failed %e", r);
	
	return id;	
	//panic("fork not implemented");
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
