#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>
#include <stdbool.h>
#include "exchange.h"
void main(int argc, char* argv[]){

	printf("%d\n",INF);
	near_node_info(argv[1]);//pass rip[n].txt
	for(unsigned int k=0; k< timebuffer;k++);
	pthread_create(&tids[thds],NULL,srv, NULL);//trigger srv daemon
	thds++;
	init_d_table(argv[1][3]);//pass n in rip[n].txt
	dijkstra(210+(atoi(&argv[1][3])));
	pthread_join(tids[1],(void **)&ret);
}

void * srv(){

	int srv_sock, cli_sock;
	int lret;
	struct sockaddr_in addr;

	srv_sock= socket(AF_INET, SOCK_STREAM, 0);
	if(srv_sock ==-1){

		perror("srver socket create fail");
		exit(0);
	}

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htons (INADDR_ANY);
	addr.sin_port = htons(srv_port_num);

	lret = bind(srv_sock, (struct sockaddr *)&addr,sizeof(addr));

	if(lret==-1){
		perror("BIND error!!");
		close(srv_sock);
		return 0;
	}

	for(;;)
	{
		lret = listen(srv_sock, 100);

		if(lret ==-1)
		{
			perror("LISTEN standby mode fail");
			//close(srv_sock);
			continue;
		}

		cli_sock = accept(srv_sock, (struct sockaddr *)NULL, NULL);
		if (cli_sock ==-1)
		{
			perror("cli_sock connect ACCEPT fail");
			//close(srv_sock);
			continue;
		}
		printf("accepted\n");
		for(unsigned int k=0; k< timebuffer;k++);	
		pthread_create(&tids[thds],NULL, handle, &cli_sock);
		visited++;
		if(visited==6) break;
	}
	close(srv_sock);
	ret=0;
	pthread_exit(&ret);
}

static void * handle(void * arg){

	printf("handle opened\n");
	int cli_sockfd=*(int*)arg;
	int lret=-1;
	char send_buffer[1024];
	char recv_buffer[2];
	char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];
	int len=0;
	struct sockaddr peer_addr;
	socklen_t peer_addr_len;
	memset(&peer_addr, 0, sizeof(peer_addr));
	peer_addr_len = sizeof(peer_addr);
	lret = getpeername(cli_sockfd, &peer_addr, &peer_addr_len);
	lret = getnameinfo(&peer_addr, peer_addr_len,
			hbuf,sizeof(hbuf),sbuf,sizeof(sbuf),NI_NUMERICHOST | NI_NUMERICSERV);
	if (lret !=0){
		lret =-1;
		pthread_exit(&lret);
	}

	memset(send_buffer,0,1024);
	sprintf(send_buffer,"%d",near_node_sz);
	printf("%s\n",send_buffer);
	send(cli_sockfd,&near_node_sz,sizeof(int),0);
	fsync(cli_sockfd);
	fflush(NULL);
	while(len!=1)len=recv(cli_sockfd,recv_buffer,sizeof(recv_buffer),0);
len=0;
	fsync(cli_sockfd);
	fflush(NULL);
	for(int k=1;k<near_node_sz;k++,len=01){
		memset(send_buffer, 0, 1024);
		//for(unsigned int y=0; y<timebuffer;y++);///time buffer
		sprintf(send_buffer,"%s",near_node[k]);
		send(cli_sockfd,send_buffer,1024,0);
		fsync(cli_sockfd);
		fflush(NULL);
		while(len!=1)len=recv(cli_sockfd,recv_buffer,sizeof(recv_buffer),0);
		fsync(cli_sockfd);
		fflush(NULL);
	}
	close(cli_sockfd);
	lret =0;
	pthread_exit(&lret);
}


void dijkstra(int addr){

	char* local_addr =(char*)malloc(sizeof(char)*15);
	char** remote_near_node;
	int laddr = addr;
	if(laddr==216) laddr=144;
	int tmp=addr;
	int rdistance;
	for(int i=1; i<7; i ++){

		distance[1][i]=INF;
	}

	for(int i=1; i<7; i++){

		pre[1][i]=0;
	}

	for(int i=1; i<7; i++){

		if(distance[0][i]==laddr){

			distance[1][i]=0;
			break;
		}
	}

	for(int i=1; i<7;i ++){

		tmp=find_min_w(distance[0][tmp]);
		rdistance=tmpdistance[1][tmp];
		printf("rdistance : %d\n",rdistance);
		sprintf(local_addr,"%s%d",prefix_addr,distance[0][tmp]);//make full address
		printf("local_addr : %s\n",local_addr);
		remote_near_node=get_nearnode_info(local_addr);
		printf("after get_nearnode_info\n");
		update_table(remote_near_node,rdistance,distance[0][tmp]);
	}
	printf("============final table================\n");
	print_d_table();
}

void update_table(char ** remote_near_node, int rdistance,int current){
	printf("update_table function\n");
	char* saddr = (char*)malloc(sizeof(char)*4);
	//printf("%s",remote_near_node[1]);
	for(int i=1;i<7;i++){
		for(int k=1;k<r_near_node_sz;k+=2)
		{
			strncpy(saddr,&(remote_near_node[k][12]),4);
			saddr[3]='\0';
			//	printf("%s\n",saddr);
			if(atoi(saddr)==distance[0][i]){
				if(distance[1][i]>(rdistance+atoi(remote_near_node[k+1]))){
					//printf("updated\n");
					distance[1][i]=(rdistance+atoi(remote_near_node[k+1]));
					pre[1][i]=current;
				}
			}
		}
	}
	free(saddr);
}

int find_min_w(int current){
	int min=INF; // When the last value is INF, then we have to choose INF node
	int index=1;
	int tmp, col;

	for(int i=1; i< 7;i++){
		if(min>distance[1][i]&&visit[i]==false){
			min=distance[1][i];
			index=i;
		}
	}
	print_d_table();
	tmpdistance[1][index]=distance[1][index];
	//	distance[1][index]=INF;
	visit[index]=true;
	return index;
}


void print_d_table(){

	for(int k=0;k<2;k++){
		for(int j=1; j<7;j++)
			printf("%9d\t",distance[k][j]);
		printf("\n");
	}
}

void init_table(){

	for(int k=0;k<7;k++){
		for(int j=0; j<7;j++){
			d_table[k][j]=INF;
		}
	}
	distance=(int**)malloc(sizeof(int*)*2);
	pre=(int**)malloc(sizeof(int*)*2);
	for(int i=0; i<2; i++){
		distance[i]=(int*)malloc((sizeof(int)*7));
		pre[i]=(int*)malloc((sizeof(int)*7));
	}

	for(int k =1; k< 7; k++){
		d_table[0][k]=210 + k;
		//newly added
		distance[0][k]=210 + k;
		//		printf("%09d\t",distance[0][k]);
		//		printf("\n");
		pre[0][k]=210 +k;
		//////////
	}
	pre[0][6] = 144;
	distance[0][6] = 144;
}

void init_d_table(char machine){

	init_table();
	memset(visit,0,sizeof(visit));
	int machine_index=atoi(&machine);
	int col_num;
	for(int k =1; k< near_node_sz; k+=2){
		d_table[1][atoi(&near_node[k][14])]=atoi(near_node[k+1]);
	}
	if(machine_index == 6)
		col_num = 144;
	else
		col_num = 210 + machine_index;

	d_table[1][0] = col_num;
}

char **  get_nearnode_info(char* destip){
	char ** remote_near_node;//=(char**)malloc(sizeof(char*)*11);
	int nearnode_index=1;
	char r_buffer[1024];
	char s_buffer='1';
	int lnearnodesz=0;
	struct sockaddr_in addr;
	int llen=0;

	fd_sock = socket(AF_INET, SOCK_STREAM,0);
	if(fd_sock ==-1){ 
		perror("socket");
		return NULL;
	}
	memset(&addr , 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(srv_port_num);
	inet_pton(AF_INET,destip,&addr.sin_addr);
	ret=-1;
	printf("before connect : %s:%d\n",destip,srv_port_num);
	
	while(ret==-1)
	{
		ret=connect(fd_sock, (struct sockaddr*)&addr, sizeof(addr));
	}
	printf("connection established\n");
	//send(fd_sock,"request near_node",sizeof("request near_node"),0);
	memset(r_buffer,0,1024);
	while(len!=0)len = recv(fd_sock, &lnearnodesz, sizeof(int),0);
	fsync(fd_sock);
	fflush(NULL);
	r_near_node_sz=lnearnodesz;
	printf("r_near_node_sz : %d\n",r_near_node_sz);
	send(fd_sock, &s_buffer, sizeof(s_buffer),0);
	fsync(fd_sock);
	fflush(NULL);
	remote_near_node=(char**)malloc(sizeof(char*)*r_near_node_sz);
	
	while(1){
		memset(r_buffer, 0, 1024);
		for(unsigned int k=0; k<timebuffer; k++);
		while(len!=0)len = recv(fd_sock, r_buffer,1024,0);
		len=0;
		fsync(fd_sock);
		fflush(NULL);
		send(fd_sock, &s_buffer, sizeof(s_buffer),0);		
		fsync(fd_sock);
		fflush(NULL);
		if(strlen(r_buffer)==0) break;
		remote_near_node[nearnode_index]=(char*)malloc(strlen(r_buffer));//sender must send line
		strcpy(remote_near_node[nearnode_index],r_buffer);		//by line
		printf("remote_near_node[%d] : %s\n",nearnode_index,remote_near_node[nearnode_index]);
		fflush(NULL);
		nearnode_index++;
	}
	close(fd_sock);
	return remote_near_node;
}


void near_node_info(char* argv){

	input=fopen(argv,"r");

	if(input ==NULL){
		printf("main : fopen failed\n");
		return;
	}
	i=1;
	while((my_read = getline(&line, &len_new, input) !=-1)){
		i+=2;
	}
	near_node_sz=i;
	near_node=(char**)malloc(i*sizeof(char*));
	fclose(input);
	input = fopen(argv,"r");

	i=1;
	while((my_read = getline(&line, &len_new,input) != -1)){
		near_node[i]=(char*)malloc(16);
		near_node[i+1]=(char*)malloc(2);
		line=strtok_r(line," ",&tmp_line);
		strcpy(near_node[i],line);
		strcpy(near_node[i+1],tmp_line);
		i+=2;
	}
	//free(line);
	fclose(input);
	for(int k =1;k<i;k++)
	{
		printf("%s\n",near_node[k]);
	}

}
