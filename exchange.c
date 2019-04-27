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
	near_node_info(argv[1]);//pass rip[n].txt
	init_d_table(argv[1][3]);//pass n in rip[n].txt
	print_d_table();//testing print_d_table();
	dijkstra((210+atoi(argv[1])));
}

void dijkstra(int addr){
	char* local_addr =(char*)malloc(sizeof(char)*15);
	char** remote_near_node;
	int tmp=addr;///최신화 필요 숫자형 주소값
	int rdistance;
	for(int i=1; i<7; i ++){
		distance[1][i]=INF;
	}

	for(int i=1; i<7; i++){
		pre[1][i]=0;
	}
	for(int i=1; i<7; i++){
	if(distance[0][i]==addr){
		distance[0][i]=0;
		break;
	}
	}
	for(int i=1; i<7;i ++){
		tmp=find_min_w(distance[0][tmp]);
		rdistance=distance[1][tmp];
		sprintf(local_addr,"%s%d",prefix_addr,distance[0][tmp]);//make full address
		remote_near_node=get_nearnode_info(local_addr);
		update_table(remote_near_node,rdistance,distance[0][tmp]);
	}
}

void update_table(char ** remote_near_node, int rdistance,int current){
	
	for(int i=1;i<7;i++){
		for(int k=1;k<12;k+=2)
		{
			if(atoi(&remote_near_node[k][12])==distance[0][i]){
				if(distance[1][i]>(rdistance+atoi(remote_near_node[k+1]))){
					pre[1][i]=current;
				}
			}
		}
	}
}

int find_min_w(int current){
	int min=INF;
	int index;
	int tmp, col;

	for(int i=1; i< 7;i++){
		if(min>distance[1][i]){
			min=distance[1][i];
			index=i;
		}
	}
	tmpdistance[1][index]=distance[1][index];
	distance[1][index]=INF;
//	pre[1][index]=current;
	return index;
}


void print_d_table(){
	for(int k=0;k<7;k++){
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
	for(int k =1; k< 7; k++){
		d_table[0][k]=210 + k;
		//newly added
		distance=(int**)malloc(sizeof(int*)*2);
		pre=(int**)malloc(sizeof(int*)*2);
		for(int i=0; i<2; i++){
			distance[i]=(int*)malloc((sizeof(int)*7));
			pre[i]=(int*)malloc((sizeof(int)*7));
		}
		distance[0][k]=210 + k;
		pre[0][k]=210 +k;
		//////////
	}
	d_table[0][0] = 0;
	d_table[0][6] = 144;
}

void init_d_table(char machine){
	init_table();
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

	char ** remote_near_node=(char**)malloc(sizeof(char*)*11);
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
	}
	printf("connection established\n");
	send(fd_sock,"request near_node",sizeof("request near_node"),0);
	while(1){
		memset(r_buffer, 0, sizeof(r_buffer));
		len = recv(fd_sock, r_buffer, sizeof(1024),0);
		if(strlen(r_buffer)==0) break;
		remote_near_node[nearnode_index]=(char*)malloc(strlen(r_buffer));//sender must send line
		strcpy(remote_near_node[nearnode_index],r_buffer);               //by line
		fflush(NULL);
		nearnode_index++;
	}

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
/*
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
}*/
/*
void dijkstra(){
	char* local_addr = (char*)malloc(sizeof(char)*15);
	char ** remote_near_node;
	int tmp;
	for(int i=1;i<6;i++){
		tmp=find_min_w(i);
		d_table[i+1][0]=tmp;
		sprintf(local_addr,"%s%d",prefix_addr,find_min_w(i));//make full address
		remote_near_node=get_nearnode_info(local_addr);
		update_d_table(remote_near_node, i);

	}
}*/
/*
int find_min_w(int row){
	int min= INF;
	int mark;
	int tmp,col;

	for(int i=1;i<7;i++){
		if(d_table[row][0]==d_table[0][i]){
			tmp=d_table[row][i];
			col=i;
			d_table[row][i]=INF;
			break;
		}
	}

	for(int i=1;i<7;i++){
		if(min>d_table[row][i]){
			min=d_table[row][i];
			mark=d_table[0][i];
		}
	}

	d_table[row][col]=tmp;
	return mark;
}*/
