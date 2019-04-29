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
#define myself "220.149.244.144"

char inputbuffer[1024];
pthread_t tids[100];
int thds, h_thds;
int pid;
static void * handle(void *);

char buffer[1024];
char r_buffer[1024];

FILE* input;
char* line = NULL;
char* temp_line = NULL;
int line_num;
size_t len_new = 0;
ssize_t my_read;
size_t getline_len;
char** dst;
char** next_hop;

int main(int argc, char *argv[]){

		int cli_sock;
		int port_num, ret;
		struct sockaddr_in addr;
		size_t getline_len;
		int srv_sock;
		int i = 0;

		dst = (char**)malloc(sizeof(char*)*5);
		next_hop = (char**)malloc(sizeof(char*)*5);

		port_num = 6627;

		/*file read & forwarding part*/
		input = fopen("table.txt", "r");
		if(input == NULL){
				printf("main : fopen failed\n");
				return 0;
		}
		while((my_read = getline(&line, &len_new,input) != -1)){
				dst[i] = (char*)malloc(15);
				next_hop[i] = (char*)malloc(15);
				line = strtok_r(line,":",&temp_line);
				strcpy(dst[i],line);
				strcpy(next_hop[i], temp_line);
				printf("%s:%s\n", dst[i], next_hop[i]);
				i++; line_num++;
		}//put dst & next_hop in the seperate array at every each line

		/*mydaemon*/
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

		for(;1;){
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
						continue;
				}
				thds++;
				// cli handler
				int tclisock=cli_sock;
				pthread_create(&tids[h_thds], NULL, handle, &tclisock);
				cli_sock=-1;
		} // end for
		ret = 0;
		return 0;
}

static void * handle(void * arg)
{
		int fd_sock;
		int cli_sockfd = *(int *)arg;
		int ret = -1;
		int ret2= -1;
		struct sockaddr_in addr;
		struct sockaddr_in addr2;
		int len, len2;

		int port_num = 6627;
		int my_port = 6628;
		char* src_ip = (char *)malloc(16);
		char* dst_ip = (char *)malloc(16);
		char* data = (char *)malloc(1024);

		char* next_ip = (char*)malloc(15);
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
		printf("connected %s\n",myself);

		memset(recv_buffer, 0,1024);
		recv(cli_sockfd, recv_buffer, 1024, 0);
		len = strlen(recv_buffer);
		printf("from client (%d) %s\n", len, recv_buffer);
		printf("----\n");

		for(int k=0; k<line_num; k++){
				if(strncmp(dst[k], recv_buffer, 15)==0){
						strncpy(next_ip, next_hop[k],15);
						strncpy(dst_ip, recv_buffer,15);
						printf("next_hop : %s\n", next_ip);
				}
		}

		//init for target socket
		fd_sock = socket(AF_INET, SOCK_STREAM, 0);
		if(fd_sock == -1){
				perror("socket");
				return 0;
		}

		memset(&addr2, 0, sizeof(addr2));
		addr2.sin_family = AF_INET;

		if(strncmp(dst_ip, myself, 15)==0)
			addr2.sin_port = htons (my_port);
		else
			addr2.sin_port = htons (port_num);

		inet_pton(AF_INET, next_ip, &addr2.sin_addr);

		if(strncmp(dst_ip, myself, 15)==0)
			printf("try to connect to self\n\n");
		else
			printf("try connect to %s:%d\n\n", next_ip, port_num);

		while(ret2==-1){
				ret2 = connect(fd_sock, (struct sockaddr *)&addr2, sizeof(addr2));

			}
		printf("connection established\n");
		if (ret2 == -1) {
				perror("connect");
				close(fd_sock);
				return 0;
		}

		printf("send$ ");
		send(fd_sock,recv_buffer,strlen(recv_buffer),0);

		for(int i=0; i<2; i++){
				int len = 0;
				memset(recv_buffer, 0,1024);
				recv(cli_sockfd, recv_buffer, 1024, 0);
				fsync(cli_sockfd);
				len = strlen(recv_buffer);
				printf("from client (%d) %s\n", len, recv_buffer);
				if (len == 0)	 return NULL;
				printf("----\n");
				printf("send$ ");

				send(fd_sock, recv_buffer, len, 0);
				memset(recv_buffer, 0, sizeof(recv_buffer));

				fflush(NULL);

		}

		close(cli_sockfd);
		ret = 0;
		pthread_exit(&ret);

		close(fd_sock);

}
