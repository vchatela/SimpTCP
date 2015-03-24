/*! \file Simptcp_api.h
*  \brief{Define  simptcp service primitives. Simptcp provides the same system calls as those of TCP}
* \author{DGEI-INSAT 2010-2011}
*/

#ifndef _SIMPTCP_API_H_
#define _SIMPTCP_API_H_

#include <netdb.h>              /* for struct sockaddr and socklen_t */

/*! \def IPPROTO_SIMPTCP
 *  \brief{SimpTCP protocol number <in.h>}
 */
#define IPPROTO_SIMPTCP	15

int socket(int domain, int type, int protocol);
int bind (int fd, const struct sockaddr *addr, socklen_t len);
int connect (int fd, const struct sockaddr *addr, socklen_t len);
ssize_t send (int fd, const void *buf, size_t n, int flags);
ssize_t recv (int fd, void *buf, size_t n, int flags);
int listen (int fd, int n);
int accept (int fd, struct sockaddr *addr, socklen_t *addr_len);
int shutdown (int fd, int how);
int close (int fd);
ssize_t read (int fd, void *buf, size_t n);
ssize_t write (int fd, const void *buf, size_t n);
int getsockname (int fd, struct sockaddr *addr, socklen_t *len);
int getpeername (int fd, struct sockaddr *addr, socklen_t *len);
int getsockopt (int fd, int level, int optname, void *optval, 
                socklen_t *optlen);
int setsockopt (int fd, int level, int optname, const void *optval, 
                socklen_t optlen);

#endif /* _SIMPTCP_API_H_ */

/* vim: set expandtab ts=4 sw=4 tw=80: */
