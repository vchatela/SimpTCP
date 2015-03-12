/*
 * libc_socket.c
 */

#include <stdio.h>              /* for printf() */
#include <netdb.h>              /* for struct sockaddr and socklen_t */

#define __USE_GNU
#include <dlfcn.h>              /* for dlsym(), */
#include <term_colors.h>        /* for color macros */
#define __PREFIX__              "[" COLOR("LIBC-SOCKET", BRIGHT_BLUE) " ] "
#include <term_io.h>            /* for printf() and perror() redefinitions */

#ifndef __DEBUG__
#define __DEBUG__               1
#endif


#define INIT_FUNCTION_POINTER(funcname)                         \
    if (!funcname ## _ptr)                                      \
        funcname ## _ptr = dlsym(RTLD_NEXT, #funcname)

#define CHECK_FUNCTION_POINTER(funcname)                        \
    if (!funcname ## _ptr) {                                    \
        printf("Unable to resolve symbol %s\n", #funcname);     \
        return -1;                                              \
    }

/* Some function pointers to the libc */
static int (*socket_ptr) (int domain, int type, int protocol);
static int (*bind_ptr) (int fd, const struct sockaddr *addr, socklen_t len);
static int (*connect_ptr) (int fd, const struct sockaddr *addr, socklen_t len);
static ssize_t (*send_ptr) (int fd, const void *buf, size_t n, int flags);
static ssize_t (*recv_ptr) (int fd, void *buf, size_t n, int flags);
static ssize_t (*sendto_ptr) (int fd, const void *buf, size_t n, int flags, 
                              const struct sockaddr *addr, socklen_t addr_len);
static ssize_t (*recvfrom_ptr) (int fd, void *buf, size_t n, int flags, 
                                struct sockaddr *addr, socklen_t *addr_len);
static ssize_t (*sendmsg_ptr) (int fd, const struct msghdr *message, int flags);
static ssize_t (*recvmsg_ptr) (int fd, struct msghdr *message, int flags);
static int (*listen_ptr) (int fd, int n);
static int (*accept_ptr) (int fd, struct sockaddr *addr, socklen_t *addr_len);
static int (*shutdown_ptr) (int fd, int how);
static int (*close_ptr) (int fildes);
static ssize_t (*read_ptr) (int fd, void *buf, size_t nbytes);
static ssize_t (*write_ptr) (int fd, const void *buf, size_t n);
static int (*getsockname_ptr) (int fd, struct sockaddr *addr, socklen_t *len);
static int (*getpeername_ptr) (int fd, struct sockaddr *addr, socklen_t *len);
static int (*getsockopt_ptr) (int fd, int level, int optname, void *optval,
                             socklen_t *optlen);
static int (*setsockopt_ptr) (int fd, int level, int optname, const void *optval,
                             socklen_t optlen);

/* Functions that wraps the libc. Basically initialize a function pointer the
 * first time a function is called, and then directly call the libc socket api
 */
int libc_socket(int domain, int type, int protocol)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    INIT_FUNCTION_POINTER(socket);
    CHECK_FUNCTION_POINTER(socket);

    return socket_ptr(domain, type, protocol);
}

int libc_bind (int fd, const struct sockaddr *addr, socklen_t len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    INIT_FUNCTION_POINTER(bind);
    CHECK_FUNCTION_POINTER(bind);

    return bind_ptr(fd, addr, len);
}

int libc_connect (int fd, const struct sockaddr *addr, socklen_t len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    INIT_FUNCTION_POINTER(connect);
    CHECK_FUNCTION_POINTER(connect);

    return connect_ptr(fd, addr, len);
}

ssize_t libc_send (int fd, const void *buf, size_t n, int flags)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    INIT_FUNCTION_POINTER(send);
    CHECK_FUNCTION_POINTER(send);

    return send_ptr(fd, buf, n, flags);
}

ssize_t libc_recv (int fd, void *buf, size_t n, int flags)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    INIT_FUNCTION_POINTER(recv);
    CHECK_FUNCTION_POINTER(recv);
    
    return recv_ptr(fd, buf, n, flags);
}

ssize_t libc_sendto(int fd, const void *buf, size_t n, int flags, 
                    const struct sockaddr *addr, socklen_t addr_len) 
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    INIT_FUNCTION_POINTER(sendto);
    CHECK_FUNCTION_POINTER(sendto);
    
    return sendto_ptr(fd, buf, n, flags, addr, addr_len);
}
ssize_t libc_recvfrom(int fd, void *buf, size_t n, int flags, 
                      struct sockaddr *addr, socklen_t *addr_len)
{
  //#if __DEBUG__
  //printf("function %s called\n", __func__);
  //#endif

    INIT_FUNCTION_POINTER(recvfrom);
    CHECK_FUNCTION_POINTER(recvfrom);
    
    return recvfrom_ptr(fd, buf, n, flags, addr, addr_len);
}

ssize_t libc_sendmsg (int fd, const struct msghdr *message, int flags)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    INIT_FUNCTION_POINTER(sendmsg);
    CHECK_FUNCTION_POINTER(sendmsg);

    return sendmsg_ptr(fd, message, flags);
}

ssize_t libc_recvmsg (int fd, struct msghdr *message, int flags)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    INIT_FUNCTION_POINTER(recvmsg);
    CHECK_FUNCTION_POINTER(recvmsg);

    return recvmsg_ptr(fd, message, flags);
}


int libc_listen (int fd, int n)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    INIT_FUNCTION_POINTER(listen);
    CHECK_FUNCTION_POINTER(listen);

    return listen_ptr(fd, n);
}

int libc_accept (int fd, struct sockaddr *addr, socklen_t *addr_len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    INIT_FUNCTION_POINTER(accept);
    CHECK_FUNCTION_POINTER(accept);

    return accept_ptr(fd, addr, addr_len);
}

int libc_shutdown (int fd, int how)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    INIT_FUNCTION_POINTER(shutdown);
    CHECK_FUNCTION_POINTER(shutdown);
    
    return shutdown_ptr(fd, how);
}

int libc_close (int fd)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    INIT_FUNCTION_POINTER(close);
    CHECK_FUNCTION_POINTER(close);

    return close_ptr(fd);
}

ssize_t libc_read (int fd, void *buf, size_t n)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    INIT_FUNCTION_POINTER(read);
    CHECK_FUNCTION_POINTER(read);

    return read_ptr(fd, buf, n);
}

ssize_t libc_write (int fd, const void *buf, size_t n)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    INIT_FUNCTION_POINTER(write);
    CHECK_FUNCTION_POINTER(write);

    return write_ptr(fd, buf, n);
}

int libc_getsockname (int fd, struct sockaddr *addr, socklen_t *len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    INIT_FUNCTION_POINTER(getsockname);
    CHECK_FUNCTION_POINTER(getsockname);

    return getsockname_ptr(fd, addr, len);
}

int libc_getpeername (int fd, struct sockaddr *addr, socklen_t *len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    INIT_FUNCTION_POINTER(getpeername);
    CHECK_FUNCTION_POINTER(getpeername);

    return getpeername_ptr(fd, addr, len);
}


int libc_getsockopt (int fd, int level, int optname, void *optval, 
		     socklen_t *optlen)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    INIT_FUNCTION_POINTER(getsockopt);
    CHECK_FUNCTION_POINTER(getsockopt);

    return getsockopt_ptr(fd, level, optname, optval, optlen);
}

int libc_setsockopt (int fd, int level, int optname, const void *optval,
                     socklen_t optlen)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    INIT_FUNCTION_POINTER(setsockopt);
    CHECK_FUNCTION_POINTER(setsockopt);

    return setsockopt_ptr(fd, level, optname, optval, optlen);
}

/* vim: set expandtab ts=4 sw=4 tw=80: */
