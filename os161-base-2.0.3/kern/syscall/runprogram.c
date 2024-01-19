/*
 * Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2008, 2009
 *	The President and Fellows of Harvard College.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
 * Sample/test code for running a user program.  You can use this for
 * reference when implementing the execv() system call. Remember though
 * that execv() needs to do more than runprogram() does.
 */

#include <types.h>
#include <kern/errno.h>
#include <kern/fcntl.h>
#include <lib.h>
#include <proc.h>
#include <current.h>
#include <addrspace.h>
#include <vm.h>
#include <vfs.h>
#include <syscall.h>
#include <test.h>
#include <copyinout.h>
#include "opt-args.h"

#if OPT_ARGS
/* copy argc into the process stack */
static
userptr_t
load_args(userptr_t stackptr, unsigned int argc, char **args)
{
	int i, res;
	size_t offset = 0;
	vaddr_t stack_base = (vaddr_t) stackptr;
	vaddr_t stack_top = stack_base;
	char** arg_addrs = NULL;

	arg_addrs = kmalloc(sizeof(char*) * argc);
	/* load string args into the stack */
	for (i = 0; i < (int) argc; i++) {
		offset = strlen(args[i])+1;
		res = copyout(args[i], (userptr_t) (stack_top-offset), offset);
		if (res) {
			return NULL;
		}
		stack_top -= offset;
		arg_addrs[i] = (char*) stack_top;
	}
	stack_top = (((stack_top - stack_base) / 4) + 1) * 4;
	KASSERT(stack_top % 4 == 0);
	/* load pointers to string args into the stack */
	for (i = 0; i < (int) argc; i++) {
		res = copyout(&arg_addrs[i], (userptr_t) (stack_top-4), 4);
		if (res) {
			return NULL;
		}
		stack_top -= 4;
	}
	stack_top = (((stack_top - stack_base) / 8) + 1) * 8;
	KASSERT(stack_top % 8 == 0);

	return (userptr_t) stack_top;
}
#endif


/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.
 *
 * Calls vfs_open on progname and thus may destroy it.
 */
int
runprogram(char *progname, unsigned int argc, char **args) 
{
	struct addrspace *as;
	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	userptr_t user_args;
	int result;
	(void) argc; // to suppress warnings
	(void) args;
	(void) user_args;

	/* Open the file. */
	result = vfs_open(progname, O_RDONLY, 0, &v);
	if (result) {
		return result;
	}

	/* We should be a new process. */
	KASSERT(proc_getas() == NULL);

	/* Create a new address space. */
	as = as_create();
	if (as == NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Switch to it and activate it. */
	proc_setas(as);
	as_activate();

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	#if !OPT_PAGING
	vfs_close(v);
	#endif

	/* Define the user stack in the address space */
	result = as_define_stack(as, &stackptr);
	if (result) {
		/* p_addrspace will go away when curproc is destroyed */
		return result;
	}

	#if OPT_ARGS
	/* Program arguments */
	user_args = load_args((userptr_t) stackptr, argc, args);
	stackptr = (vaddr_t) user_args;
	#endif

	/* Warp to user mode. */
	# if !OPT_ARGS
	enter_new_process(0 /*argc*/, NULL /*userspace addr of argv*/,
			  NULL /*userspace addr of environment*/,
			  stackptr, entrypoint);
	# else 
	enter_new_process(argc, user_args, NULL, stackptr, entrypoint);
	#endif
	/* enter_new_process does not return. */
	panic("enter_new_process returned\n");
	return EINVAL;
}



