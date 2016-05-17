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