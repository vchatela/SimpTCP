/*! \file simptcp_api.c
 * \brief{define simptcp service primitives simptcp provides the same system calls as those of TCP}
 * 
*/

#include <stdio.h>              /* for printf() */
#include <stdlib.h>             /* for malloc() */
#include <string.h>             /* for memset() */
#include <unistd.h>             /* for usleep() */
#include <errno.h>              /* for errno macros */
#include <simptcp_api.h>        /* for simptcp related functions */
#include <simptcp_lib.h>       /* for simptcp_core related functions */
#include <simptcp_entity.h> 
#include <libc_socket.h>        /* for libc_related functions */
#include <term_colors.h>        /* for color macros */
#define __PREFIX__              "[" COLOR("SIMPTCP_API", BRIGHT_YELLOW) " ] "
#include <term_io.h>

#ifndef __DEBUG__
#define __DEBUG__               1
#endif



/* determines if the descriptor is a simptcp descriptor  by looking at the
 * simptcp_descriptors table. if it is a simptcp descriptor the function 
 * returns 1. Else, 0 is returned.
 */
int is_simptcp_descriptor(int fd) 
{
	int res = 0;

#if __DEBUG__
	printf("function %s called\n", __func__);
#endif

	if ((fd >= 0) && (fd <= UINT16_MAX)) {
		res = (simptcp_entity.simptcp_socket_descriptors[fd] != NULL);
	}
#if __DEBUG__
	printf("descriptor %d %s a simptcp descriptor\n", fd, res ? "IS" : "IS NOT");
#endif

	return res;
}

/* determines if the socket we want to create is a simptcp socket or not. if the
 * parameters correspond to the creation of a simptcp socket, the function 
 * returns 1. Else, 0 is returned.
 */
int is_simptcp_socket(int domain, int type, int protocol)
{
	int res = 0;

#if __DEBUG__
	printf("function %s called\n", __func__);
#endif

	res = ((domain == AF_INET) && (type == SOCK_STREAM) &&  
		   (protocol == IPPROTO_SIMPTCP));

#if __DEBUG__
	printf("socket (%d,%d,%d) %s a simptcp socket\n",
		   domain, type, protocol, res ? "CREATES" : "DOES NOT CREATE");
#endif

	return res;
}



int socket(int domain, int type, int protocol)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    if (!is_simptcp_socket(domain, type, protocol))
      return libc_socket(domain, type, protocol);
    
    /* create a simptcp socket */    
    return create_simptcp_socket();
}

int bind (int fd, const struct sockaddr *addr, socklen_t len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    
    if (!is_simptcp_descriptor(fd)) {
        return libc_bind(fd, addr, len);
    }
    if (addr == NULL)
      return -EINVAL;
    /* Set the simptcp local socket with the binded one */
    memcpy(&(simptcp_entity.simptcp_socket_descriptors[fd]->local_simptcp), addr, len);
    
    return 0;
}

int connect (int fd, const struct sockaddr *addr, socklen_t len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    struct simptcp_socket* sock;
 

    if (!is_simptcp_descriptor(fd)) {
        return libc_connect(fd, addr, len);
    }
    /* Here comes the code for the connect related to simptcp */
    sock=simptcp_entity.simptcp_socket_descriptors[fd];
    return sock->socket_state->active_open(sock,(struct sockaddr *)addr,len);
}

ssize_t send (int fd, const void *buf, size_t n, int flags)
{
    struct simptcp_socket* sock;

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    if (!is_simptcp_descriptor(fd)) {
        return libc_send(fd, buf, n, flags);
    }

    /* Here comes the code for the send related to simptcp */
        sock=simptcp_entity.simptcp_socket_descriptors[fd];
	return sock->socket_state->send(sock,buf,n,flags);

}

ssize_t recv (int fd, void *buf, size_t n, int flags)
{
    struct simptcp_socket* sock;

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    if (!is_simptcp_descriptor(fd)) {
        return libc_recv(fd, buf, n, flags);
    }

    /* Here comes the code for the recv related to simptcp */
    

    sock=simptcp_entity.simptcp_socket_descriptors[fd];
    return sock->socket_state->recv(sock,buf,n,flags);
}



int listen (int fd, int n)
{
  struct simptcp_socket* sock;
    
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    
    if (!is_simptcp_descriptor(fd)) {
        return libc_listen(fd, n);
    }

    /* Here comes the code for the listen related to simtcp */
    if (n >= SOMAXCONN)
        return -EINVAL;

    sock=simptcp_entity.simptcp_socket_descriptors[fd];
    return sock->socket_state->passive_open(sock,n);

    return 0;
}

int accept (int fd, struct sockaddr *addr, socklen_t *addr_len)
{
  struct simptcp_socket* sock;
    
#if __DEBUG__
  printf("function %s called\n", __func__);
#endif
  
  if (!is_simptcp_descriptor(fd)) {
    return libc_accept(fd, addr, addr_len);
  }
  
  /* Here comes the code for the accept related to simtcp */
  sock=simptcp_entity.simptcp_socket_descriptors[fd];
  return sock->socket_state->accept(sock,addr,addr_len);
}

int shutdown (int fd, int how)
{
  struct simptcp_socket* sock;

#if __DEBUG__
  printf("function %s called\n", __func__);
#endif
	
  if (!is_simptcp_descriptor(fd))
    return libc_shutdown(fd, how);
  
  /* Here comes the code for the shutdown related to simtcp */
  sock=simptcp_entity.simptcp_socket_descriptors[fd];
  return sock->socket_state->shutdown (sock,how);  
}

int close (int fd)
{
#if __DEBUG__
  printf("function %s called\n", __func__);
#endif
	
  if (!is_simptcp_descriptor(fd)) {
    return libc_close(fd);
  }

  /* Here comes the code for the close related to simtcp */

  return shutdown(fd, SHUT_RDWR);
}

ssize_t read (int fd, void *buf, size_t n)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    if (!is_simptcp_descriptor(fd)) {
        return libc_read(fd, buf, n);
    }

    /* Here comes the code for the read related to simtcp */
    return recv(fd, buf, n, MSG_WAITALL);
}

ssize_t write (int fd, const void *buf, size_t n)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
	
    if (!is_simptcp_descriptor(fd)) {
        return libc_write(fd, buf, n);
    }

    /* Here comes the code for the xxxx related to simtcp */
    return send(fd, buf, n, 0);
}

int getsockname (int fd, struct sockaddr *addr, socklen_t *len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    return libc_getsockname(fd, addr, len);
}

int getpeername (int fd, struct sockaddr *addr, socklen_t *len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
	
    return libc_getpeername(fd, addr, len);
}

// TODO : Hide the fact that an udp socket is used in reality.
int getsockopt (int fd, int level, int optname, void *optval, 
                socklen_t *optlen)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
   
    return libc_getsockopt(fd, level, optname, optval, optlen);
}

// TODO: Prevent some setsockopt() parameters values.
int setsockopt (int fd, int level, int optname, const void *optval, 
                socklen_t optlen)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
 
    return libc_setsockopt(fd, level,optname, optval, optlen);
}


/* vim: set expandtab ts=4 sw=4 tw=80: */
