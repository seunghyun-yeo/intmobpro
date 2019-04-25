#define port_num 8179
#define prefix_addr "220.149.244."
#define INF 100000000

FILE * input;
char ** near_node;
char * line = NULL;
char * tmp_line = NULL;
int line_num;
char ** edges;
size_t len_new=0;
ssize_t my_read;
size_t near_node_sz=0;
size_t edges_sz=0;
int i=1;
int d_table[7][7];
int fd_sock, cli_sock;
int ret;
struct sockaddr_in addr;
int len;
size_t getline_len;

char r_buffer[1024];





//////////////////////function
void graph_read();
void near_node_info(char* argv);
void init_d_table(char machine);
char ** get_nearnode_info(char* destip);
void init_table();
void print_d_table();
void dijkstra();
int find_min_w(int row);
void update_d_table(char ** remote_near_node,int i);
