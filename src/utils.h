#ifndef UTILS_H
#define UTILS_H

#define DEBUG

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(expression) \
  (                                                                           \
    ({ long int __result;                                                     \
       do __result = (long int) (expression);                                 \
       while (__result == -1L && errno == EINTR);                             \
       __result; }))
#endif

#ifdef DEBUG
#define LOG_DEBUG(message, args...) do { \
    fprintf(stderr, "[%s] " message "\n", "DEBUG", ##args); \
    safe_fflush(stderr); \
} while (0)
#else
#define LOG_DEBUG(message, args...) do { } while(0)
#endif

#define LOG_ERROR(message) do { \
    fprintf(stderr, "[%s] %s:%d: ", "ERROR", __FUNCTION__,__LINE__); \
    perror(message); \
} while (0)

#define FORCE_EXIT(source) do { \
    LOG_ERROR(source); \
    kill(0, SIGKILL); \
    exit(EXIT_FAILURE); \
} while (0)

#define MAX(a, b) (((a) > (b)) ? (a) : (b)) 

#define MAX_PORT 65535
#define BACKLOG 3

int is_valid_port(uint16_t port);

/* Files */
int safe_open(char* filename, int flags);
void safe_close(int fd);
void safe_fflush(FILE* stream);
void set_nonblock(int fd);

/* Sockets */
int make_socket(int domain, int type);
int bind_inet_socket(uint16_t port,int type);
int accept_client(int socket_fd);

/* I/O */
ssize_t bulk_read(int fd, char *buf, size_t count);
ssize_t bulk_write(int fd, char *buf, size_t count);

void msleep(time_t seconds, long nanoseconds);
void* safe_malloc (size_t size);

/* Signals */
void set_handler(void (*f)(int), int signo);
 
 /* Threads */
 pthread_t create_detached_thread(void* data, void*(*handler)(void*));
 
#endif