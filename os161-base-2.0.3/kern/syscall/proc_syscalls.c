/*
#include <addrspace.h>
#include <proc.h>
#include <thread.h>
#include <syscall.h> */
#include <types.h>
#include <kern/unistd.h>
#include <clock.h>
#include <copyinout.h>
#include <syscall.h>
#include <lib.h>
#include <proc.h>
#include <thread.h>
#include <addrspace.h>
#include <current.h>

int
sys__exit(int status)
{
    /* status handling */
    #if OPT_WAITPID
    struct proc* p = curproc;

    /* set status in proc structure */
    spinlock_acquire(&p->p_lock);
    p->p_status_set = true;
    p->p_status = status;
    spinlock_release(&p->p_lock);
    //proc_remthread(curthread);    // TODO: modify thread_exit to accept detached threads
    lock_acquire(p->p_cv_lock);
    cv_signal(p->p_cv, p->p_cv_lock);
    lock_release(p->p_cv_lock);
    #else
    struct addrspace* as = proc_getas();
    as_destroy(as);
    #endif
    thread_exit();
    return status;
}

pid_t
sys_waitpid(pid_t pid, userptr_t status)
{
    #if OPT_WAITPID
    /* find the proc structure with the pid */
    struct proc* p = proc_find_by_pid(pid);
    if (p == NULL) {
        return -1;
    }
    int p_status = proc_wait(p);
    *((int*) status) = p_status;
    return p->p_pid;
    #else
    (void) pid;
    (void) status;
    return pid;
    #endif
}

pid_t
sys_getpid(void)
{
    #if OPT_WAITPID
    KASSERT(curproc != NULL);
    struct proc* p = curproc;
    pid_t pid = p->p_pid;
    return pid;
    #else
    return -1;
    #endif
}

pid_t
sys_fork(void)
{
    #if OPT_WAITPID
    struct proc *child, *parent;

    KASSERT(curproc != NULL);
    parent = curproc;
    child = proc_create_runprogram(parent->p_name);
    if (child == NULL) {
        return -1;
    }
    /* duplicate the address space */
    as_copy(parent->p_addrspace, &(child->p_addrspace));
    if (child->p_addrspace == NULL) {
        proc_destroy(child);
        return -1;
    }

    

    #else
    return -1
    #endif
    return -1;
}