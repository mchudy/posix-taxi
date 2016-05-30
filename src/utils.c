#include "utils.h"

int safe_open(char* filename, int flags) {
    int fd;
    if ((fd = TEMP_FAILURE_RETRY(open(filename, flags))) == -1)
        FORCE_EXIT("open");
    return fd;
}

void safe_close(int fd) {
    if (TEMP_FAILURE_RETRY(close(fd)) == -1)
        FORCE_EXIT("Cannot close");
}

void safe_fflush(FILE* stream) {
    if (TEMP_FAILURE_RETRY(fflush(stream)) == EOF) {
        FORCE_EXIT("fflush");
    }
}

void set_nonblock(int fd) {
    int new_flags = fcntl(fd, F_GETFL) | O_NONBLOCK;
	if(fcntl(fd, F_SETFL, new_flags) == -1) {
        FORCE_EXIT("fcntl");
    }
}

int is_valid_port(uint16_t port) {
	return 0 < port && port <= MAX_PORT;
}

int make_socket(int domain, int type) {
	int sock;
	sock = socket(domain,type,0);
	if(sock < 0) {
        FORCE_EXIT("socket");
    }
	return sock;
}

int bind_inet_socket(uint16_t port, int type) {
	struct sockaddr_in addr;
	int socketfd,t=1;
	socketfd = make_socket(PF_INET, type);
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR,&t, sizeof(t))) FORCE_EXIT("setsockopt");
	if(bind(socketfd,(struct sockaddr*) &addr,sizeof(addr)) < 0)  FORCE_EXIT("bind");
	if(SOCK_STREAM==type)
		if(listen(socketfd, BACKLOG) < 0) FORCE_EXIT("listen");
	return socketfd;
}


int accept_client(int socket_fd) {
	int client_fd;
	if ((client_fd = TEMP_FAILURE_RETRY(accept(socket_fd, NULL, NULL))) < 0)
	{
		if (EAGAIN == errno || EWOULDBLOCK == errno) {
			return -1;
        }
        FORCE_EXIT("accept");
	}
	return client_fd;
}

ssize_t bulk_read(int fd, char *buf, size_t count) {
	int c;
	size_t len = 0;
	do {
		c = TEMP_FAILURE_RETRY(read(fd,buf,count));
		if(c < 0) return c;
		if (0 == c) return len;
		buf += c;
		len += c;
		count -= c;
	} while(count > 0);
	return len ;
}

ssize_t bulk_write(int fd, char *buf, size_t count) {
	int c;
	size_t len = 0;
	do {
		c = TEMP_FAILURE_RETRY(write(fd, buf, count));
		if(c < 0) return c;
		buf += c;
		len += c;
		count -= c;
	} while(count > 0);
	return len ;
}

void set_handler(void (*f)(int), int signo) {
	struct sigaction act;
	memset(&act, 0, sizeof(struct sigaction));
	act.sa_handler = f;
	if (-1 == sigaction(signo, &act, NULL)) {
		FORCE_EXIT("sigaction");
    }
}

pthread_t create_detached_thread(void* data, void*(*handler)(void*)) {
    pthread_t client_tid;
    pthread_attr_t attr;
    if(pthread_attr_init(&attr) != 0) {
        FORCE_EXIT("pthread_attr_init");
    }
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(pthread_create(&client_tid, &attr, handler, data) != 0) {
        FORCE_EXIT("pthread_create");
    }
    pthread_detach(client_tid);
    pthread_attr_destroy(&attr);
    return client_tid;
}

void msleep(time_t seconds, long nanoseconds) {
    struct timespec tt, t;
    t.tv_sec = seconds; 
    t.tv_nsec = nanoseconds;
    for (tt=t; nanosleep(&tt,&tt); ) {
        if (EINTR != errno) 
            FORCE_EXIT("nanosleep");
    }
}

void* safe_malloc (size_t size) {
    register void *value = malloc (size);
    if (value == NULL){
        FORCE_EXIT("malloc");
    }
    return value;
}

/* Returns 1 if the result is negative */
int timespec_subtract (struct timespec *x, struct timespec *y, struct timespec *result) {
    if (x->tv_nsec < y->tv_nsec) {
        int nsec = (y->tv_nsec - x->tv_nsec) / 1000000000 + 1;
        y->tv_nsec -= 1000000000 * nsec;
        y->tv_sec += nsec;
    }
    if (x->tv_nsec - y->tv_nsec > 1000000000) {
        int nsec = (x->tv_nsec - y->tv_nsec) / 1000000000;
        y->tv_nsec += 1000000000 * nsec;
        y->tv_sec -= nsec;  
    }
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_nsec = x->tv_nsec - y->tv_nsec;
    return x->tv_sec < y->tv_sec;
}