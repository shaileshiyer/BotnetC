#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <time.h>


#define PORT 9999
#define BUF_SIZE 2048
#define CLADDR_LEN 100
#define CMD_SIZE 100

//server can listen to several ports
static int *s_accept=NULL ,n_ports=0;
static int   *sport_type = NULL, n_port_types = 0;
static int   *s_comm = NULL, n_comm = 0;


/*helper functions for managing connection entries */
static int find_empty_entry (int *slist,int n)
{
	int i=0;

	while (i<n && slist[i] != -1 )
	{
			i++;
	}
	return i;
}

static int add_handler(int **slist , int *n)
{
	int i;

	if ((i=find_empty_entry(*slist,*n)) == *n )
	{
		(*n)++;
		*slist = realloc(*slist,(*n) * sizeof(int) );
	}
	return i;
}


int send_toclients (int ex_tcp,char* buffer,int len)
{
	int sa_len = sizeof (struct sockaddr_in);
	int k;
	for (k=0;k<n_comm;k++)
	{
		if(ex_tcp != k)
		{
			if(write(s_comm[k],buffer,len)<0)
					perror("error tcp send");
		}
	}

	return 0;
}

int main()
{
	 fd_set	rfds;
	 struct timeval	tv;
	 int i,max,n_res,len,ilen,j,k,l,r;
	 int	use_sin = 1, loop=1;
	 char buffer[BUF_SIZE], ibuffer[BUF_SIZE];

	 socklen_t	sa_len = sizeof (struct sockaddr_in);
	 struct sockaddr_in	server,client;
	 int **list ,*n;
	 int g_len ;
	 int sock1,sock2 = 0,ops=1;

	 //create initial socket`
	 list = &s_accept;
	 n = &n_ports;
	 g_len = sizeof(struct sockaddr_in);

	 server.sin_addr.s_addr = INADDR_ANY;
	 server.sin_family = AF_INET ;
 	 server.sin_port = htons(PORT);

	 sock1 = add_handler(list , n);
	 (*list)[sock1] = socket (PF_INET,SOCK_STREAM,0);

	 //check if socket is created properly
	 if ((*list)[sock1] < 0 )	perror ("socket(NET):");
	 //i is sock2 and j is sock1
	 sock2 = add_handler(&sport_type,&n_port_types);
	 sport_type[sock2] = SOCK_STREAM;

	 setsockopt((*list)[sock1],SOL_SOCKET,SO_REUSEADDR,&ops,sizeof(ops));

	 sock2 = bind((*list)[sock1],(struct sockaddr *) &server ,g_len);
	 if (sock2 < 0 )	perror("bind: ");

	 sock2 = listen((*list)[j],10);

	 //now starts the main loop
	 while (loop)
	 {
		 if (n_ports == 0 && n_comm == 0) break;

		 FD_ZERO(&rfds);

		 ilen = max = 0;

		 for (i = 0; i < n_ports; i++)
		 {
			 FD_SET(s_accept[i], &rfds);
			 if (s_accept[i] >= max)
			 		max = s_accept[i] + 1;
		 }

		 for (i = 0; i < n_comm; i++)
		 {
			 if (s_comm[i] != -1)
			 		FD_SET(s_comm[i], &rfds);
			 if (s_comm[i] >= max)
			  	max = s_comm[i] + 1;
		 }
		 /* Add stdin to rdset and set timeout*/
		 if (use_sin) FD_SET(0, &rfds);

		 /* 4 sec Timeout*/
		 tv.tv_sec = 4;
		 tv.tv_usec = 0;
		 if ((n_res = select(max,&rfds,NULL, NULL , &tv)) < 0 )
		 {
		 		perror("select:");
		 }
		 else
		 {

			 if (FD_ISSET(0,&rfds))
			 {
				 	//get stdin data to send to all clients
					ilen= read (0,ibuffer,BUF_SIZE);
					if (ilen == 0)
					{
						loop=0;
						continue;
					}

					ibuffer[ilen] = 0;
			 }
			 if (n_res)
			 {
				 /* check client sockets */
				 /*---------------------------------------------------*/
				 for(i=0; i < n_comm ; i++)
				 {
					 	if (s_comm[i]== -1 )
							continue;

						if(FD_ISSET(s_comm[i], &rfds))
						{
							if ((len = read(s_comm[i],buffer,BUF_SIZE)) <= 0 )
							{
								//0 bytes read removing socket
								s_comm[i]= -1;
							}
							buffer[len] = 0;
							write (1,buffer , len);
							send_toclients(i,buffer,len);
						}

						if ( ilen )
						{
							if ( write(s_comm[i],ibuffer,ilen) < 0)
							{
								close (s_comm[i]);
								s_comm[i] = -1 ;

							}
						}
				 }

				 /* Check accept sockets */
				 /*--------------------------------------------*/

				 for ( i=0 ; i < n_ports ; i++)
				 {
					 if (FD_ISSET(s_accept[i],&rfds))
					 {
						 /* process tcp connections */
						 j = add_handler(&s_comm, &n_comm);
						 s_comm[j] = accept (s_accept[i] , (struct sockaddr*) &client,&sa_len);
						 use_sin = s_comm[j];
					 }
					 else
					 {
						 /*add idle operation here*/
						//  PDEBUG("TIMEOUT!!!")
					 }
				 }
			 }
		 }
	 }
	 return 0 ;
}
