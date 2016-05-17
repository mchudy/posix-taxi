#define _GNU_SOURCE
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

#define ERR(source) (fprintf(stderr,"%s:%d\n",__FILE__,__LINE__),\
        perror(source),kill(0,SIGKILL),\
        exit(EXIT_FAILURE))

/* Logging */
#ifdef DEBUG
#define LOG_DEBUG(message, arg...) do { \
    fprintf(stderr, "[%s] %s:%d: " message "\n", "DEBUG", __FUNCTION__,__LINE__, ##arg); \
    fflush(stderr); \
} while (0)
#else
#define LOG_DEBUG(message) do { } while(0)
#endif

#define MAX_PORT 65535
#define BACKLOG 3

int is_valid_port(uint16_t port);
int safe_open(char* filename, int flags);
void safe_close(int fd);
void safe_fflush(FILE* stream);

int make_socket(int domain, int type);
int bind_inet_socket(uint16_t port,int type);