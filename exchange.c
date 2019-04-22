#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "exchange.h"
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netdb.h>


void main(int argc, char* argv[])
{
	graph_read();
	near_node_info(argv[1]);//pass rip[n].txt
	init_d_table(argv[1][3]);//pass n in rip[n].txt
	print_d_table();//testing print_d_table();
	dijkstra();
}

void dijkstra(){
}

char **  get_nearnode_info(char* destip){

	char ** remote_near_node=(char**)malloc(sizeof(char*)*7);
	int nearnode_index=1;
	fd_sock = socket(AF_INET, SOCK_STREAM,0);
	if(fd_sock ==-1){
		perror("socket");
		return NULL;
	}
	memset(&addr , 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port_num);
	inet_pton(AF_INET,destip,&addr.sin_addr);
	ret=-1;
	while(ret==-1)
	{
		ret=connect(fd_sock, (struct sockaddr *)&addr, sizeof(addr));
	}
	printf("connection established\n");
	send(fd_sock,"request near_node",sizeof("request near_node"),0);
	while(1){
		memset(r_buffer, 0, sizeof(r_buffer));
		len = recv(fd_sock, r_buffer, sizeof(1024),0);
		if(strlen(r_buffer)==0) break;
		remote_near_node[nearnode_index]=(char*)malloc(strlen(r_buffer));
		strcpy(remote_near_node[nearnode_index],r_buffer);
		fflush(NULL);
		nearnode_index++;
	}

	return remote_near_node;
}

void print_d_table(){
	for(int k=1;k<7;k++){
		for(int j=0; j<7;j++)
			printf("%9d\t",d_table[k][j]);
		printf("\n");
	}
}

void init_table(){
	for(int k=0;k<7;k++){
		for(int j=0; j<7;j++){
			d_table[k][j]=INF;
		}
	}
}

void init_d_table(char machine){
	init_table();
	int machine_index=atoi(&machine);
	for(int k =1; k< near_node_sz; k+=2){
		d_table[1][atoi(&near_node[k][14])]=atoi(near_node[k+1]);
	}
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

void graph_read(){
	input=fopen("GRAPH.txt","r");
	if(input == NULL){
		printf("fopen failed\n");
		exit(0);
	}

	i=1;
	while((my_read = getline(&line, &len_new, input) !=-1)){
		i+=2;
	}
	edges_sz=i;
	edges=(char**)malloc(i*sizeof(char*));
	fclose(input);
	input=fopen("GRAPH.txt","r");
	i=1;
	while((my_read=getline(&line, &len_new,input) !=-1)){
		edges[i]=(char*)malloc(15);
		edges[i+1]=(char*)malloc(15);
		line=strtok_r(line,":",&tmp_line);
		strcpy(edges[i+1],tmp_line);
		line=strtok_r(line,":",&tmp_line);
		strcpy(edges[i],line);
		i+=2;
	}
	//free(line);

	fclose(input);
	for(int k =1 ;k<i;k++)
	{
		printf("%s\n",edges[k]);
	}
}
