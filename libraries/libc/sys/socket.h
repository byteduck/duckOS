/* Workaround for GCC compilation */
#ifdef CODY_NETWORKING
#define CODY_NETWORKING 0
#endif

#include <kernel/api/socket.h>
#include "../netinet/in.h"
#include "types.h"
#include "un.h"

__DECL_BEGIN
int socket(int domain, int type, int protocol);
int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
int setsockopt(int sockfd, int level, int option, const void* option_value, socklen_t option_len);
int getsockopt(int sockfd, int level, int option, void* option_value, socklen_t* option_len);
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);
ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);
int listen(int sockfd, int backlog);
int shutdown(int sockfd, int how);
int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen);
__DECL_END