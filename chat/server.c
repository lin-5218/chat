#include<stdio.h>
#include<sys/socket.h>
#include<string.h>
#include<netinet/in.h>
#include<unistd.h>
#include<stdlib.h>
#include<pthread.h>
#include<time.h>
#include<sqlite3.h>
#include<fcntl.h>  
#include<errno.h>  
#define server_port 8000
#define N 50
enum res{ok,no,log_ok,log_no,reg_ok,reg_no,in_ok,in_no,verify_say_ok,verify_say_fail};

typedef struct login_information{
	    int flag;
	    char state[N];
		int id;
		char name[N];
		char passwd[N];
		int admin;
		char msg[N];
    	char toname[N];
    	char filename[N];
	
}login_infor;

typedef struct online_node{
	int connfd;
	char name[N];
	int id;
	int state;
	struct online_node *next;
	
}online; 

typedef online * Link;

typedef struct connfd_addr{
		int connfd;
		struct sockaddr_in client_addr;
		socklen_t addrlen; 
		sqlite3 *pdb;
		Link head;
}connfd_addr;


int create_link(Link *head){
	*head = (Link)malloc(sizeof(online));
	if(*head == NULL)
	{
		printf("malloc head fail\n");
		return no;
	}
	else
	{
		(*head)->next = NULL;
		return ok; 
	}
} 
int create_node(Link *new_node)
{
	*new_node = (Link )malloc(sizeof(online));
	if(*new_node == NULL)
	{
		printf("malloc node fail\n");
		return no;
	 } 
	 else
	 {
	 	return ok;
	 }
}
void insert_node_tail(Link head,Link new_node)
{
	Link p = head; 
	while(p->next != NULL)
	{
		p = p->next;
	}
	p->next = new_node;
	new_node->next = NULL;
}
void init_infor(login_infor * l_i)
{
	l_i->flag = 0;
	memset(l_i->state,0,sizeof(l_i->state));
    l_i->id = 0;
	memset(l_i->passwd,0,sizeof(l_i->passwd));
	memset(l_i->name,0,sizeof(l_i->name));
	l_i->admin = 0;
	memset(l_i->msg,0,sizeof(l_i->msg));
	memset(l_i->filename,0,sizeof(l_i->filename));

	
}
int create_table(sqlite3 *pdb)
{
		char *sql = NULL;
		char *errmsg = NULL;
		sql="create table if not exists mytable (id integer primary key,name text,passwd text,admin integer);";
		if(SQLITE_OK != sqlite3_exec(pdb,sql,NULL,NULL,&errmsg))
		{
				printf("create table error!%s\n",errmsg);
				return -1;
		}
		else
		{
				return SQLITE_OK;
		}

}
int display(sqlite3 *pdb,int id)//
{

		char sql[100];
		char ** ret_val = NULL;
		char * errmsg = NULL;
		int nrow;
		int ncol;
		int ret;
		int i;
		sprintf(sql, "select  * from mytable where id = %d;",id);
		if(SQLITE_OK == sqlite3_get_table(pdb,sql,&ret_val,&nrow,&ncol,&errmsg))
		{
				printf("nrow = %d ncol = %d\n",nrow,ncol);
				if(nrow==0)
				{
						return -1;
				}
				for(i = 0; i < (nrow + 1) * ncol;i++)
				{
						printf("%10s",ret_val[i]);

						if((i + 1) % ncol == 0)
						{
								printf("\n");
						}
				}
				return SQLITE_OK;

		}
		else
		{
				sqlite3_free_table(ret_val);

				return -1;
		}
}
int insert_record(sqlite3 * pdb,login_infor *l_i)
{
		char sql[1024];
		char *errmsg = NULL;

		sprintf(sql,"insert into mytable (id,name,passwd,admin) values (%d,'%s','%s',%d);",l_i->id,l_i->name,l_i->passwd,l_i->admin);

		if(SQLITE_OK != sqlite3_exec(pdb,sql,NULL,NULL,&errmsg))
		{
				printf("insert fail!%s\n",errmsg);
				return -1;
		}
		printf("insert success!\n");
		return SQLITE_OK;
}
int search_id(sqlite3 * pdb,int id)
{

		if(SQLITE_OK != display(pdb,id))
		{

				return -1;
		}
		else
		{
				return SQLITE_OK;
		}
}

int verify_passwd(sqlite3 * pdb,int id)
{
		char sql[100];
		char ** ret_val = NULL;
		char * errmsg = NULL;
		int nrow;
		int ncol;
		int ret;
		int i;
		sprintf(sql, "select passwd from mytable where id = %d;",id);
		if(SQLITE_OK == sqlite3_get_table(pdb,sql,&ret_val,&nrow,&ncol,&errmsg))
		{

				if(nrow==0)
				{
						return -1;
				}
                else
                {
				   return SQLITE_OK;
				}
				
		}
}
int return_name(sqlite3 * pdb,int id,char **name)
{
		char sql[100];
		char ** ret_val = NULL;
		char * errmsg = NULL;
		int nrow;
		int ncol;
		int ret;
		int i;
		sprintf(sql, "select name from mytable where id = %d;",id);
		if(SQLITE_OK == sqlite3_get_table(pdb,sql,&ret_val,&nrow,&ncol,&errmsg))
		{

				if(nrow == 0)
				{
						return -1;
				}
			
                (*name) = ret_val[1];
				return SQLITE_OK;
		}
}
int  handle_log(connfd_addr ** c_a)
{
    	int connfd,n; 
	
	    login_infor  l_i;
	    connfd = (*c_a)->connfd;
	    Link head = NULL;
	    Link new_node=NULL;
	    head = (*c_a)->head;

		printf("connfd=%d\n",connfd);
		
		sqlite3 *pdb=(*c_a)->pdb;
     
       while(1)
       {
	   	
	  	init_infor(&l_i);
	    n=read(connfd,&l_i,sizeof(login_infor));
	    
	    if(strcmp(l_i.state,"LOG") == 0)
	        {
	        	
				    if(n == 0)
				    {
				        printf("client out\n");
						return in_no;
			   		}
				//	printf("n=%d\n",n);
				//	printf("flag=%d\n",l_i.flag);
				//	printf("id=%d\n",l_i.id);
			    //    printf("passwd=%s\n",l_i.passwd);
			        
			        if(SQLITE_OK != search_id(pdb,l_i.id))
			        {
			          	printf("your id is error,please input again!\n");
			        	l_i.flag=0;
			        	write(connfd,&l_i,sizeof(login_infor));
			        	continue;
					}
			        else
			        {
			        	   if(SQLITE_OK != verify_passwd(pdb,l_i.id))
			  			 {
			  			 	printf("your passwd is error,please input again!\n");
			        	    l_i.flag = 0;
			        	    write(connfd,&l_i,sizeof(login_infor));
			        	    
			        	    continue;
			   		
			  			 }
			  			 else
			  			 {
			  			 	l_i.flag=1;
			  			 	
			  			 	char *username;
			  			 	return_name(pdb,l_i.id,&username);
			  			 	
							if(ok == create_node(&new_node))
							{
							
								new_node->connfd = connfd;
								new_node->state = 1;
								new_node->id = l_i.id;
								strcpy(new_node->name,username);							
								insert_node_tail(head,new_node); 
							}
							strcpy(l_i.name,username);
			  			 	write(connfd,&l_i,sizeof(login_infor));			  			 	
			  			    return  in_ok;
			  			    break;
						 }
			        	
					}
			        		        
				  
			      
			    
			 }	

	
        
        if(strcmp(l_i.state,"REG") == 0)
        {
        	if(n == 0)
				{
				    printf("client out\n");
					return in_no;
			   	}
			//	printf("n=%d\n",n);
			//	printf("flag=%d\n",l_i.flag);
			
			 //   printf("passwd=%s\n",l_i.passwd);
			
			    l_i.admin=0;
			    
			    srand((unsigned)time(NULL));
			    
			    do{
			         l_i.id = rand()%100000;
			     }
			    while(SQLITE_OK == search_id(pdb,l_i.id));
				if(SQLITE_OK != insert_record(pdb,&l_i))
				{
					l_i.flag = 0;
				}	
				else
				{
				     l_i.flag = 1;
				}	
				
				
			    write(connfd,&l_i,sizeof(login_infor));
			    
		
		        continue;
		}
		
		if(strcmp(l_i.state,"SLOG") == 0)
	        {
	        	
				if(n == 0)
				    {
				        printf("client out\n");
						return in_no;
			   		}
				//	printf("n=%d\n",n);
				//	printf("flag=%d\n",l_i.flag);
				//	printf("id=%d\n",l_i.id);
			    //    printf("passwd=%s\n",l_i.passwd);
			        
			        l_i.flag = 1;
			        
				   
			        write(connfd,&l_i,sizeof(login_infor));
		
		            continue;
		    }
        
	

	  
   }

}

 void display_all_list(int connfd,Link head)
 {
 	Link p=head->next;

 	login_infor l_i; 
	  init_infor(&l_i);
 	strcpy(l_i.state,"DOF");
  
 	while(p != NULL)
 	{
 		strcpy(l_i.msg,p->name);
 		
 		write(connfd,&l_i,sizeof(l_i));
 		printf("online name:%s\n",l_i.msg);
        sleep(1);
 		p=p->next;
 		
	}
 	
 	
 }
 
 void release_link(Link *head)
 {
 	
 	Link p=*head;
 	 while(*head != NULL)
    {   
        *head = p->next;
	    free(p);
		p  = *head;
    }

    
}
int verify_say(int connfd,Link head)
{
	Link q=head->next;
	while(q != NULL && q->connfd != connfd)
 			{	
 				q=q->next;
 			
			}
			if(q != NULL)
			{
				if(q->state == 1)
				{
					return verify_say_ok;
				}
				else
				{
					return verify_say_fail;
				}
			     
				
			}
			else
			{
			        printf("ver:no this people\n");
			        	return verify_say_fail;
			}
	
}
 void say_to_all(int connfd,char sendname[],Link head,char msg[])
 {
     login_infor l_i;
 	if(verify_say_ok != verify_say(connfd,head))
 	 {	
	  		strcpy(l_i.state,"STA");
	  		strcpy(l_i.name,"server");
		 	strcpy(l_i.msg,"you have been banned!\n");
 			write(connfd,&l_i,sizeof(l_i));
	 }
	 else
	 {
			 
		 	
		 	strcpy(l_i.state,"STA");
		 	strcpy(l_i.msg,msg);
		 	Link p=head->next;	
		 	while(p != NULL)
		 	{
		 		if(p->connfd != connfd)
		 		{
				    //write(p->connfd,msg,N);
				    strcpy(l_i.name,sendname);
		
				    write(p->connfd,&l_i,sizeof(l_i));
				    printf("%s:%s\n",sendname,msg);
				}
		        sleep(1);
		 		p=p->next;
		 		
			}
}
 	
 	
 }
  void say_to_one(int connfd,char sendname[],Link head,char msg[],char toname[])
 {
 	 login_infor l_i;
 	if(verify_say_ok != verify_say(connfd,head))
 	 {	
	  		strcpy(l_i.state,"STO");
	  		strcpy(l_i.name,"server");
		 	strcpy(l_i.msg,"you have been banned!\n");
 			write(connfd,&l_i,sizeof(l_i));
	 }
	 else
	 {
 	
		 	strcpy(l_i.state,"STO");
		 
		 	Link p=head->next;	
		 	while(p != NULL && strcmp(p->name,toname)!= 0)
		 	{	
		 	   printf("p.anem=%s\n",p->name);
		 		p=p->next;
		 		
			}
			if(p!=NULL)
			{
			        strcpy(l_i.msg,msg);
				   strcpy(l_i.name,sendname);
			    	write(p->connfd,&l_i,sizeof(l_i));
				 printf("%s to %s:%s\n",sendname,toname,msg);
			}
				else
			{
			   strcpy(l_i.name,"server");
				strcpy(msg,"client is offline!\n");
				write(connfd,&l_i,sizeof(l_i));
		
			}
}
 	
 }
 void dis_send_msg(int connfd,char toname[],Link head)
 {
 	    Link p=head->next;
 	    Link q=head->next;
 	    login_infor l_i;
 	    strcpy(l_i.state,"DSM");
 		while(p != NULL && p->id != 11111)
 		{	
 			p=p->next;
 		
 		
		}
		if(p!=NULL)
		{
		
			while(q != NULL && strcmp(q->name,toname) != 0)
 			{	
 				printf("q->%s\n",q->name);
				 q=q->next;
 				
 			
			}
			if(q != NULL)
			{
				q->state=0;
				printf("dis %s send msg\n",q->name);
				strcpy(l_i.msg,"disable  send msg success!");
				write(connfd,&l_i,sizeof(l_i));
			}
			else
			{
				strcpy(l_i.msg,"no this people!");
				write(connfd,&l_i,sizeof(l_i));
			}
		
			
		}
		else
		{
			strcpy(l_i.msg,"you don't have right to dis send msg!");
			write(connfd,&l_i,sizeof(l_i));
			
		}
		
 	
 }
 
  void en_send_msg(int connfd,char toname[],Link head)
 {
 	    Link p=head->next;
 	    Link q=head->next;
 	    login_infor l_i;
 	    strcpy(l_i.state,"DSM");
 		while(p != NULL && p->id != 11111)
 		{	
 			p=p->next;
 		
 		
		}
		if(p!=NULL)
		{
			
			while(q != NULL && strcmp(q->name,toname) != 0)
 			{	
 				printf("q->%s\n",q->name);
				 q=q->next;
 				
 			
			}
			if(q != NULL)
			{
				q->state=1;
				printf("enable  %s send msg\n",q->name);
				strcpy(l_i.msg,"enable  send msg success!");
				write(connfd,&l_i,sizeof(l_i));
			}
			else
			{
				strcpy(l_i.msg,"no this people!");
				write(connfd,&l_i,sizeof(l_i));
			}
		
			
		}
		else
		{
			strcpy(l_i.msg,"you don't have right to dis send msg!");
			write(connfd,&l_i,sizeof(l_i));
			
		}
		
 	
 }
 void release_node(int connfd,Link head)
 {

    Link p1 = head->next;
	Link p2 = head;

	 if(p1 == NULL)
	 {
		 printf("Link is empty!\n");
	 }
     else
     {
         while(p1->next != NULL && p1->connfd != connfd)
         {
             p2 = p1;
             p1 = p1 -> next;
         }

         if( p1 -> next != NULL)
         {
             p2 -> next = p1 -> next;
             free (p1);
		 }
		 else
		 {
			 if(p1->connfd == connfd)
			 {
				 p2->next = NULL;
				 free(p1);
			 }
			 else
			 {
				 printf("not node is attach!\n");
			 }
		 }
      }
}
void send_file(int connfd,char filename[],char msg[],int flag,char toname[],Link head)
{
	  int tofd;
	  
	  if((tofd = open(filename,O_CREAT|O_RDWR|O_APPEND|O_EXCL)) == -1 )
          {
                  perror("open to file error!\n");
  
                    return;
        }
        if(flag != 0)
        {
		  write(tofd,msg,strlen(msg));
      	}
      	close(tofd);
	
}
void handle_mainview(connfd_addr ** c_a)
{
        int connfd=(*c_a)->connfd;
        login_infor  l_i;
        int n;
        while(1)
       {
	   	
	  	init_infor(&l_i);
	    n=read(connfd,&l_i,sizeof(login_infor));

	    if(strcmp(l_i.state,"DOF") == 0)
	        {
	       //	printf("recv=%s\n",l_i.state);
	            display_all_list(connfd,(*c_a)->head);
	        	continue;
			}
			
		if(strcmp(l_i.state,"STA") == 0)
	        {
			 //    printf("recv=%s\n",l_i.state);
			 //    printf("recvmsg=%s\n",l_i.msg);
	             say_to_all(connfd,l_i.name,(*c_a)->head,l_i.msg);
	        	 continue;
			}
		if(strcmp(l_i.state,"STO") == 0)
	        {
			//     printf("recv=%s\n",l_i.state);
	             say_to_one(connfd,l_i.name,(*c_a)->head,l_i.msg,l_i.toname);
	        	 continue;
			}
				if(strcmp(l_i.state,"DSM") == 0)
	        {
			//     printf("recv=%s\n",l_i.state);
		    
	             dis_send_msg(connfd,l_i.toname,(*c_a)->head);
	        	 continue;
			}
				if(strcmp(l_i.state,"ESM") == 0)
	        {
			//     printf("recv=%s\n",l_i.state);
		   
	             en_send_msg(connfd,l_i.toname,(*c_a)->head);
	        	 continue;
			
			}
			    if(strcmp(l_i.state,"SFF") == 0)
			{
				send_file(connfd,l_i.filename,l_i.msg,l_i.flag,l_i.toname,(*c_a)->head);
			    	
			}
			
	    	if(n ==0)
			{
				release_node(connfd,(*c_a)->head);
				printf("the connfd %d client is out\n",connfd);
				
				break;
			}
			
	    }
		 
}

void * work_thread(void * arg)
{
	
	
	    connfd_addr  *c_a;
	    c_a=(connfd_addr *)arg;
	    
	    
	    if(in_ok != handle_log(&c_a))
	    {
	    	printf("in_no");
	    	_exit(0);
		}
		printf("in_ok!\n");
		
		handle_mainview(&c_a);

	
		
	  
}
int main()
{
		int listenfd,connfd;
		char msg[N];
		socklen_t addrlen;
		struct sockaddr_in server_addr,client_addr;
	    
		connfd_addr  c_a;

		pthread_t tid;
		Link head =NULL;
		
		
		if(ok == create_link(&head))
			{
				printf("create link success£¡\n");		
			}
			
	
		sqlite3 *pdb;
		if(SQLITE_OK != sqlite3_open("mydatabase.db",&pdb))
		{
				printf("open dtabase fail!%s\n",sqlite3_errmsg(pdb));
				exit(EXIT_FAILURE);
		}
		else
		{
				printf("open database success!\n");
		}
		if(SQLITE_OK ==  create_table(pdb))
		{
				printf("create table success!\n");
		}
		else
		{
				sqlite3_close(pdb);
				return 0;
		}

		listenfd=socket(AF_INET,SOCK_STREAM,0);

		bzero(&server_addr,sizeof(server_addr));
        server_addr.sin_family=AF_INET;
		server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
		server_addr.sin_port=htons(server_port);

		addrlen=sizeof(server_addr);
		bind(listenfd,(struct sockaddr *)&server_addr,addrlen);


		listen(listenfd,999);
		printf("listening........\n");


        addrlen=sizeof(client_addr);
		

 while(1)
	{		

	   connfd=accept(listenfd,(struct sockaddr*)&client_addr,&addrlen);
	   
	   c_a.connfd=connfd;
	   c_a.client_addr=client_addr;
	   c_a.addrlen=addrlen;
	   c_a.pdb=pdb; 
       c_a.head=head;
	   pthread_create(&tid,NULL,(void *)work_thread,(void *)&c_a);
	   
	}
 
   
		close(listenfd);
			release_link(&head);
		return 0;

}
