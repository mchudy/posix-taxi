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

#ifdef DEBUG
#define LOG_DEBUG(message, args...) do { \
    fprintf(stderr, "[%s] %s:%d: " message "\n", "DEBUG", __FUNCTION__,__LINE__, ##args); \
    fflush(stderr); \
} while (0)
#else
#define LOG_DEBUG(message) do { } while(0)
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

#define MAX_PORT 65535
#define BACKLOG 3

int is_valid_port(uint16_t port);
int safe_open(char* filename, int flags);
void safe_close(int fd);
void safe_fflush(FILE* stream);

int make_socket(int domain, int type);
int bind_inet_socket(uint16_t port,int type);