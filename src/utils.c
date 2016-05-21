#include "utils.h"

int is_valid_port(uint16_t port) {
	return 0 < port && port < MAX_PORT;
}

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
	int clientfd;
	do {
		clientfd = TEMP_FAILURE_RETRY(accept(socket_fd, NULL, NULL));
	} while (clientfd == -1 && (EAGAIN == errno || EWOULDBLOCK == errno));
	if (clientfd == -1)
		FORCE_EXIT("accept");
	return clientfd;
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

void msleep(time_t seconds, long nanoseconds) {
    struct timespec tt, t;
    t.tv_sec = seconds; 
    t.tv_nsec = nanoseconds;
    for (tt=t; nanosleep(&tt,&tt); ) {
        if (EINTR != errno) 
            FORCE_EXIT("nanosleep");
    }
}