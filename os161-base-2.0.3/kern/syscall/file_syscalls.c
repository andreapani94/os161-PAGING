/*
#include <types.h>
#include <kern/unistd.h>
#include <lib.h>
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
// LAB 5
#include <copyinout.h>
#include <vnode.h>
#include <vfs.h>
#include <limits.h>
#include <uio.h>
#include <current.h>
#include <kern/errno.h>


#define SYS_OPEN_MAX 10*OPEN_MAX

struct openfile {
    struct vnode *v;
    off_t offset;
    unsigned int proc_cnt;
};

struct openfile sys_open_ftable[SYS_OPEN_MAX]; 

#if OPT_FILESYS
static
int
file_read(int fd, userptr_t buf, size_t size)
{
    struct proc *p = curproc;
    struct openfile *of = NULL;
    struct iovec iov;
    struct uio u;
    int res;

    if (fd < STDERR_FILENO || fd > OPEN_MAX) {
        return EBADF;
    }
    of = p->p_open_ftable[fd];
    if (of == NULL || of->v == NULL) {
        return EFAULT;
    }

    /* setup IO structures */
    iov.iov_ubase = buf;
    iov.iov_len = size;
    u.uio_iov = &iov;
    u.uio_iovcnt = 1;
    u.uio_offset = of->offset;
    u.uio_resid = size;
    u.uio_segflg = UIO_USERISPACE;
    u.uio_rw = UIO_READ;
    u.uio_space = p->p_addrspace;

    res = VOP_READ(of->v, &u);
    if (res) {
        return res;
    }
    of->offset = u.uio_offset;
    return (size - u.uio_resid);
}

static
int 
file_write(int fd, userptr_t buf, size_t size)
{
    struct proc *p = curproc;
    struct openfile *of = NULL;
    struct iovec iov;
    struct uio u;
    int res;

    if (fd < STDERR_FILENO || fd > OPEN_MAX) {
        return EBADF;
    }
    of = p->p_open_ftable[fd];
    if (of == NULL || of->v == NULL) {
        return EFAULT;
    }

    /* setup IO structures */
    iov.iov_ubase = buf;
    iov.iov_len = size;
    u.uio_iov = &iov;
    u.uio_iovcnt = 1;
    u.uio_offset = of->offset;
    u.uio_resid = size;
    u.uio_segflg = UIO_USERISPACE;
    u.uio_rw = UIO_WRITE;
    u.uio_space = p->p_addrspace;

    res = VOP_WRITE(of->v, &u);
    if (res) {
        return res;
    }
    of->offset = u.uio_offset;
    return (size - u.uio_resid);
}

#endif

#if OPT_FILESYS

int 
sys_open(userptr_t pathname, int flags, mode_t mode)
{
    struct vnode* v = NULL;
    struct openfile* of = NULL;
    int rv, i;
    struct proc* p = NULL;
    int fd = -1;

    /* open the file */
    rv = vfs_open((char *) pathname, flags, mode, &v);
    if (rv) {
        // handle errors
        vfs_close(v);
        return -1;
    }

    /* find an empty spot in the sys open file table */
    for (i = 0; i < SYS_OPEN_MAX; i++) {
        if (sys_open_ftable[i].v == NULL) {
            of = &sys_open_ftable[i];       // to use later in the per process table
            sys_open_ftable[i].v = v; 
            sys_open_ftable[i].offset = 0;
            sys_open_ftable[i].proc_cnt = 1;
            break;
        }
    }
    if (of == NULL) {
        vfs_close(v);
        return -1;
    }
    p = curproc;
    for (i = STDERR_FILENO + 1; i < OPEN_MAX; i++) {
        if (p->p_open_ftable[i] == NULL) {
            p->p_open_ftable[i] = of;
            fd = i;
            break;
        }
    }
    if (fd < 0) {
        vfs_close(v);
        return -1;
    }
    return fd;
}

#endif

#if OPT_FILESYS

int 
sys_close(int fd)
{
    struct openfile *of = NULL;
    struct proc *p = curproc;

    if (fd < STDERR_FILENO || fd > OPEN_MAX) {
        return EBADF;
    }
    of = p->p_open_ftable[fd];
    if (of == NULL || of->v == NULL) {
        return EFAULT;
    }
    p->p_open_ftable[fd] = NULL;    // free the entry in the process open file table
    of->proc_cnt--;                 // decrement process counter in the system open file table
    if (of->proc_cnt == 0) {
        /* close the file */
        vfs_close(of->v);
        of->v = NULL;              // free the entry
    }
    return 0;
}

#endif



int
sys_write(int fd, userptr_t buf, size_t size)
{
    int i;
    char* buf_ptr = (char*) buf;
    
    if (fd != STDOUT_FILENO && fd != STDERR_FILENO) {
        #if OPT_FILESYS
        return file_write(fd, buf, size);
        #else
        kprintf("SYS_write is currently supported for STDIN and STDERR only\n");
        return -1;
        #endif
    }

    for (i = 0; i < (int) size; i++) {
        putch(buf_ptr[i]);
    }

    return (int) size;

}


int
sys_read(int fd, userptr_t buf, size_t size)
{
    int i;
    char c;
    char* buf_ptr = (char*) buf;

    if (fd != STDIN_FILENO && fd != STDERR_FILENO) {
        #if OPT_FILESYS
        return file_read(fd, buf, size);
        #else
        kprintf("SYS_read is currently supported for STDIN and STDERR only\n");
        return -1;
        #endif
    }

    for (i = 0; i < (int) size; i++) {
        c = getch();    // get character from the console
        buf_ptr[i] = c;
    }
    

    return (int) size;
}