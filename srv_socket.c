#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>
#include <fcntl.h>

char inputbuffer[1024];
pthread_t tids[100];
int thds, h_thds;

int pid;
static void * handle(void *);
void*  mydaemon(void *);

char buffer[1024];
char r_buffer[1024];
/*
int socket_recv(int socket, char* buffer, int size)
{
	int total;
	int received;

	assert(buffer);
	assert(size >0 );

	total =0;

	while
*/
int main(int argc, char *argv[])
{

	pthread_create(&tids[thds], NULL, mydaemon, NULL);
	thds++;
	//int nonblflags = fcntl(fileno(stdin), F_GETFL,0);
	//fcntl(fileno(stdin), F_SETFL, nonblflags | O_NONBLOCK);
	int fd_sock, cli_sock;
	int port_num, ret;
	struct sockaddr_in addr;
	int len;
	size_t getline_len;

	// arg parsing
	if (argc != 3) {
		printf("usage: cli srv_ip_addr port\n");
		return 0;
	}
	port_num = atoi(argv[2]);

	// socket creation
	fd_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_sock == -1) {
		perror("socket");
		return 0;
	}

	// addr binding, and connect
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons (port_num);
	inet_pton(AF_INET, argv[1], &addr.sin_addr);
	ret=-1;
	printf("try connect to %s:%s\n",argv[1],argv[2]);
	while(ret==-1)
	{
	ret = connect(fd_sock, (struct sockaddr *)&addr, sizeof(addr));
	}
	printf("connection established\n");
	if (ret == -1) {
		perror("connect");
		close(fd_sock);
		return 0;
	}

	while (1) {
		//buffer = NULL;
		printf("send$ ");
		//ret = getline(&buffer, &getline_len, stdin);
		ret = read(fileno(stdin),&buffer,1024);
		if (ret == -1) { // EOF
			perror("getline");
			close(fd_sock);
			break;
		}
		len = strlen(buffer);
		if (len == 0) {
			memset(buffer, 0, sizeof(buffer));
			continue;
		}
		send(fd_sock,&len,sizeof(len),0);
		send(fd_sock, buffer, len, 0);
		memset(buffer, 0, sizeof(buffer));
		memset(r_buffer, 0, sizeof(r_buffer));
		len = recv(fd_sock, r_buffer, sizeof(r_buffer), 0);
		if (len < 0) break;
		printf("server says $ %s\n", r_buffer);
		fflush(NULL);
	}
	// bye-bye
	close(fd_sock);
	return 0;
}

void * mydaemon(void * args)
{
	int srv_sock, cli_sock;
	int port_num, ret;
	struct sockaddr_in addr;
	int len;

	// socket creation
	port_num = 8179;

	srv_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (srv_sock == -1) {
		perror("Server socket CREATE fail!!");
		return 0;
	}

	// addr binding
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htons (INADDR_ANY); // 32bit IPV4 addr that not use static IP addr
	addr.sin_port = htons (port_num); // using port num

	ret = bind (srv_sock, (struct sockaddr *)&addr, sizeof(addr));

	if (ret == -1) {
		perror("BIND error!!");
		close(srv_sock);
		return 0;
	}

	for (;;) 
	{
		// Listen part
		ret = listen(srv_sock, 0);

		if (ret == -1) 
		{
			perror("LISTEN stanby mode fail");
			close(srv_sock);
			return 0;
		}

		// Accept part ( create new client socket for communicate to client ! )
		cli_sock = accept(srv_sock, (struct sockaddr *)NULL, NULL); // client socket
		if (cli_sock == -1) 
		{
			perror("cli_sock connect ACCEPT fail");
			close(srv_sock);
		}
		thds++;
		// cli handler
		pthread_create(&tids[h_thds], NULL, handle, &cli_sock);
	} // end for
	ret = 0;
	pthread_exit(&ret);
}

static void * handle(void * arg)
{
	int cli_sockfd = *(int *)arg;
	int ret = -1;
	char *recv_buffer = (char *)malloc(1024);
	char *send_buffer = (char *)malloc(1024);
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

	/* get peer addr */
	struct sockaddr peer_addr;
	socklen_t peer_addr_len;
	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr_len = sizeof(peer_addr);
	ret = getpeername(cli_sockfd, &peer_addr, &peer_addr_len);
	ret = getnameinfo(&peer_addr, peer_addr_len, 
			hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), 
			NI_NUMERICHOST | NI_NUMERICSERV); 

	if (ret != 0) {
		ret = -1;
		pthread_exit(&ret);
	}
	/* read from client host:port */

	while (1) {
		int len = 0;
		int len2 = 0;
		memset(recv_buffer, 0, sizeof(recv_buffer));
		recv(cli_sockfd, &len, sizeof(len),0);
		printf("%d\n",len);
		while(len!=len2)
		{
			len2 += recv(cli_sockfd, recv_buffer+len2, sizeof(recv_buffer), 0);
		}
		fsync(cli_sockfd);
		printf("from client (%d) ----\n", len);
		if (len == 0) continue;
		printf("%s\n len:%d\n", recv_buffer, len);
		memset(send_buffer, 0, sizeof(send_buffer));
		sprintf(send_buffer, "[%s:%s]%s len:%d\n", 
				hbuf, sbuf, recv_buffer, len);
		len = strlen(send_buffer);

		ret = send(cli_sockfd, send_buffer, len, 0);
		if (ret == -1) break;
		printf("----\n");
		fflush(NULL);
		fsync(cli_sockfd);

	}
	printf("in handle while loop end\n");
	close(cli_sockfd);
	ret = 0;
	pthread_exit(&ret);
}

