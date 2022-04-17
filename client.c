#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>
#include <arpa/inet.h>
#include<string.h>
#include<fcntl.h>  
#include<errno.h>  
#define server_port 8000
#define N 50
enum res{ log_ok,log_fail,reg_ok,reg_fail,dof_ok,dof_fail,sta_ok,sta_fail,sto_ok,sto_fail,dsm_ok,dsm_fail,esm_ok,esm_fail,sff_ok,sff_fail,vch_ok,vch_fail,save_ok,save_fail};
char Name[N];
int Id;
typedef struct login_information
{
	    int flag;
	    char state[N];
		int id;
		char name[N];
		char passwd[N];
		int admin;
		char msg[N];
		char toname[N];
		char filename[N];
		int s_or_r;
}login_infor;

void init_infor(login_infor * l_i)
{
	l_i->flag=0;
	memset(l_i->state,0,sizeof(l_i->state));
    l_i->id=0;
	memset(l_i->passwd,0,sizeof(l_i->passwd));
	memset(l_i->name,0,sizeof(l_i->name));
	l_i->admin=0;
    memset(l_i->msg,0,sizeof(l_i->msg));
	memset(l_i->filename,0,sizeof(l_i->filename));
	l_i->s_or_r=0;
}

void welcome()
{
		printf("-----------------------------------------------------\n");
		printf("|                                                   |\n");
		printf("|               WELCOME!                            |\n");
		printf("|                                                   |\n");
		printf("|               1.login(Normal user)      [LOG]     |\n");
		printf("|               2.reg  (Regist new user)  [REG]     |\n");
		printf("|               3.login(Super  user)      [SLOG]    |\n");
		printf("|                                                   |\n");
		printf("|                                                   |\n");
		printf("-----------------------------------------------------\n");
}
void main_view()
{
	system("clear");
	printf("---------------------------------------------------------\n");
	printf("|                                                       |\n");
	printf("|             1.Display the online  list  !    [DOF]    |\n");
	printf("|             2.Send message to all friend!    [STA]    |\n");
	printf("|             3.Send message to  a  friend!    [STO]    |\n");
    printf("|             4.Disable  send  message    !    [DSM]    |\n");
    printf("|             5.Enable   send  message    !    [ESM]    |\n");
    printf("|             6.Send  file  to  a  friend !    [SFF]    |\n");
    printf("|             7.View  chat   history      !    [VCH]    |\n");
	printf("|                                                       |\n"); 
	printf("|                                                       |\n");
	printf("---------------------------------------------------------\n");
}

int save_chat(char name[],char msg[])
{
		
	
		int fromfd;
		char filename[N];
		sprintf(filename,"%s.txt",name);
		
		if((fromfd = open(filename,O_CREAT|O_RDWR|O_APPEND,0777)) == -1 )
	          {
	                  perror("open from file error!");
	  
	                 return save_fail;
	          }
	          
	    
	    int length=0;
	  
	    msg[strlen(msg)]='\n';
	    msg[strlen(msg)+1]='\0';
	    if( (length = write(fromfd,msg, strlen(msg))) > 0 )    
	    {   

	         return save_ok;
	    }
	    else
	    {
	    	return save_fail;
		}
	   

}
void * read_thread(void* var)
{
    login_infor l_i;
	int sockfd;
	int n;
    
	sockfd = *((int*)var);
	


    while(1)
	{ 
        init_infor(&l_i);
		n = read(sockfd,&l_i,sizeof(l_i));

		if(n == 0)
		{
		
			printf("server is out\n");
			exit(-1);
		}
  
       if(strcmp(l_i.state,"STA")==0)
       {
       	    char msg[1024];
       	    sprintf(msg,"%s:%s.\n",l_i.name,l_i.msg);
       	    if(save_ok == save_chat(l_i.name,msg))
       	    {
       	    	 printf("%s:%s.\n",l_i.name,l_i.msg);
			}
			else
			{
				printf("save fail\n");
			}
       	   
	   }
	     if(strcmp(l_i.state,"STO")==0)
       {
       	
       		char msg[1024];
       	    sprintf(msg,"%s say to %s (private):%s.\n",l_i.name,l_i.toname,l_i.msg);
       	    if(save_ok == save_chat(l_i.name,msg))
       	    {
                printf("%s say to you (private):%s.\n",l_i.name,l_i.msg);
			}
			else
			{
				printf("save fail\n");
			}
       	    
	   }
	   if(strcmp(l_i.state,"DOF")==0)
	   {
	   	  
	   	   printf("the %s is online\n",l_i.msg);
	     
	   }
	   if(strcmp(l_i.state,"DSM")==0)
	   {
	   	  
	   	   printf("%s\n",l_i.msg);
	     
	   }
	    if(strcmp(l_i.state,"ESM")==0)
	   {
	   	  
	   	   printf("%s\n",l_i.msg);
	     
	   }
	    if(strcmp(l_i.state,"SFF")==0) 
	   {
	   
	   	  
		   	 if(l_i.s_or_r == 1)   
		   	 {
		   	 		int tofd;
		   	 		
				   	if((tofd = open(l_i.filename,O_CREAT|O_RDWR|O_APPEND,0777)) == -1 )
			        {
			            perror("open to file error!\n");
			  
			             continue;
			        }
			        
			        if(l_i.flag == 1)//flag==1,recv file no complete
			        {
					  	write(tofd,l_i.msg,strlen(l_i.msg));
					  
			      	}
			    close(tofd);
			}
				  
			 else
			 {
			 		printf("%s\n",l_i.msg);
			 }
		     
		   
	   
    }



}	
pthread_exit(NULL);
}
int  login(int connfd)
{
		
	    login_infor  l_i; 
	    init_infor(&l_i);
		int id;
		char passwd[N];
		char buf[N];
		printf("input the id:");
		scanf("%d",&id);
        Id=id;
        getchar();
        
        
		printf("input the passwd:");
		fgets(passwd,50,stdin);
        passwd[strlen(passwd)-1]='\0';
        
		l_i.id=id;
		strcpy(l_i.passwd,passwd);
		strcpy(l_i.state,"LOG");
		l_i.flag=0;

	
	    write(connfd,&l_i,sizeof(login_infor));
	    read(connfd,&l_i,sizeof(login_infor));
	//	printf("id=%d\n",l_i.id);
	//	printf("pass=%s\n",l_i.passwd);
	//	printf("flag=%d\n",l_i.flag);
	    strcpy(Name,l_i.name);
		if(l_i.flag==1)
	    {
	        printf("login ok\n");
			return log_ok;	
		}
		else
		{
			printf("login fail\n");
			return log_fail;
		}




}
int reg(int connfd)
{
	    char newname[N];
		char newpasswd[N];
		
		printf("input the new name:");
	    fgets(newname,N,stdin);
     	newname[strlen(newname)-1]='\0';

		printf("input the passwd:");
		fgets(newpasswd,N,stdin);
		newpasswd[strlen(newpasswd)-1]='\0';

		
		login_infor  l_i; 
		init_infor(&l_i);
		strcpy(l_i.name,newname);
		strcpy(l_i.passwd,newpasswd);
		strcpy(l_i.state,"REG");
		l_i.flag=0;
		write(connfd,&l_i,sizeof(login_infor));
		
		read(connfd,&l_i,sizeof(login_infor));
	//	printf("id=%d\n",l_i.id);
	//	printf("flag=%d\n",l_i.flag);
	    if(l_i.flag==1)
	    {
	        printf("regist success,your id is %d\n",l_i.id);
			return reg_ok;	
		}
		else
		{
			printf("regist fail\n");
			return 	reg_fail;
		}
				
		
}
int  slog(int connfd)
{
	
			
	     login_infor  l_i; 
	    init_infor(&l_i);
		int id;
		char passwd[N];
		char buf[N];
		printf("input the id:");
		scanf("%d",&id);

        getchar();
        
        
		printf("input the passwd:");
		fgets(passwd,50,stdin);
        passwd[strlen(passwd)-1]='\0';
        
		l_i.id=id;
		strcpy(l_i.passwd,passwd);
		strcpy(l_i.state,"LOG");
		l_i.flag=0;

	
	    write(connfd,&l_i,sizeof(login_infor));
	    read(connfd,&l_i,sizeof(login_infor));
	//	printf("id=%d\n",l_i.id);
	//	printf("pass=%s\n",l_i.passwd);
	//	printf("flag=%d\n",l_i.flag);
	    strcpy(Name,l_i.name);
	    Id=l_i.id;
		if(l_i.flag==1)
	    {
	        printf("login ok\n");
			return log_ok;	
		}
		else
		{
			printf("login fail\n");
			return log_fail;
		}
}

void client_log(int socketfd)
{	

     char buf[N];
	 while(1)
		{ 
		        system("clear");
			    memset(buf,0,sizeof(buf));

		        welcome();
                printf("input the order:");
              
                

                fgets(buf,50,stdin);
				buf[strlen(buf)-1]='\0';
				if(strcmp(buf,"LOG") == 0)
				{
                      if(log_ok == login(socketfd))
                      {
                      	 break;
					  }
					  else
					  {
					  	  printf("log fail,please login again!wait 4s.....\n");
					  	  sleep(4);
					  	 
					  } 
					  continue;
						
				}
				if(strcmp(buf,"REG") == 0)
				{
                    if(reg_ok == reg(socketfd))
                    {
                    	printf("please input the id and paasswd to login in !\n");
                    	sleep(4);
                    
					}
					else
					{
					   	printf("regist fail,please regist again!wait 3s.....\n");
					  	sleep(3);
					   
					} 
					continue;
				
				}
						 
				
				 if(strcmp(buf,"SLOG") == 0)
				{
				
                      if(log_ok == slog(socketfd))
                      {
                      	 break;
					  }
					  else
					  {
					  	  printf("log fail,please login again!wait 4s.....\n");
					  	  sleep(4);
					  	 
					  } 
					  continue;

				  }
				
						
				printf("input the corrent order!\n");
				sleep(1);
				continue;
							
		}
}
int dof(int connfd)
{
	 	login_infor  l_i; 
	    init_infor(&l_i);
	    
	    strcpy(l_i.state,"DOF");
	
		int w;
	    w=write(connfd,&l_i,sizeof(login_infor));
		if(w == 0)
		{
			return dof_fail;
		}	
		else
		{
			return dof_ok;
		}
      
}
int sta(int connfd)
{
    	login_infor  l_i; 
	    init_infor(&l_i);
	    char msg[N];
	    
	
    	printf("input the msg:");
	    fgets(msg,N,stdin);
	    msg[strlen(msg)-1]='\0';
	    
	    strcpy(l_i.state,"STA");
	    strcpy(l_i.msg,msg);
	    strcpy(l_i.name,Name);
	    
		int w;
	    w=write(connfd,&l_i,sizeof(login_infor));
		if(w == 0)
		{
			return sta_fail;
		}	
		else
		{
			return sta_ok;
		}
	
}
int sto(int connfd,char sendname[])
{
		login_infor  l_i; 
	    init_infor(&l_i);
	    char msg[N];
	    char name[N];
	    
	   	printf("input the msg:");
	    fgets(msg,N,stdin);
	    msg[strlen(msg)-1]='\0';
	    
	    printf("input the name:");
	    fgets(name,N,stdin);
	     name[strlen(name)-1]='\0';
	    
	    
	    strcpy(l_i.state,"STO");
	    strcpy(l_i.msg,msg);
	    strcpy(l_i.toname,name);
	    strcpy(l_i.name,sendname);
    
	    
		int w;
	    w=write(connfd,&l_i,sizeof(login_infor));
		if(w == 0)
		{
			return sto_fail;
		}	
		else
		{
			return sto_ok;
		}
}
int dsm(int connfd)
{
        login_infor  l_i; 
	    init_infor(&l_i);

	    char toname[N];
	    
	   	printf("input the dsm name:");
	    fgets(toname,N,stdin);
	    toname[strlen(toname)-1]='\0';

	    strcpy(l_i.toname,toname);
	    strcpy(l_i.state,"DSM");
	    
	    int w;
	    w=write(connfd,&l_i,sizeof(login_infor));
	    
		if(w == 0)
		{
			return dsm_fail;
		}	
		else
		{
			return dsm_ok;
		}
	    
	    
	return dsm_ok;
}
int esm(int connfd)
{
   		login_infor  l_i; 
	    init_infor(&l_i);

	    char toname[N];
	    
	   	printf("input the esm name:");
	    fgets(toname,N,stdin);
	    toname[strlen(toname)-1]='\0';

	    strcpy(l_i.toname,toname);
	    strcpy(l_i.state,"ESM");
	    
	    int w;
	    w=write(connfd,&l_i,sizeof(login_infor));
	    
		if(w == 0)
		{
			return esm_fail;
		}	
		else
		{
			return esm_ok;
		}

}
int sff(int connfd,char sendname[])
{
	char filename[N];
	printf("input the filename:");
	fgets(filename,N,stdin);
	filename[strlen(filename)-1]='\0';
	
	char toname[N];
	printf("input the toname:");
	fgets(toname,N,stdin);
	toname[strlen(toname)-1]='\0';
	
	login_infor l_i;
	


	
	int fromfd;
	if((fromfd = open(filename,O_RDONLY)) == -1 )
          {
                  perror("open from file error!\n");
  
                 return sff_fail;
          }
          
    char msg[N];
    memset(msg,0,sizeof(msg));
    int length=0;
  
    while( (length = read(fromfd,msg, sizeof(msg))) > 0 )    
        {   
			    l_i.flag=1;
			    strcpy(l_i.name,sendname);
			    strcpy(l_i.toname,toname);
			    strcpy(l_i.state,"SFF");  
			    strcpy(l_i.filename,filename);
                 strcpy(l_i.msg,msg); 
			
                if( write(connfd, &l_i, sizeof(l_i)) < 0) 
                { 
                    printf("Send File:%s Failed.\n", filename); 
					close(fromfd);
                    return sff_fail; 
                } 
               
                sleep(2);
                memset(msg,0, sizeof(msg) );
        }
        
      
        
		l_i.flag=0;
        memset(l_i.msg,0,sizeof(l_i.msg));
        strcpy(l_i.name,sendname);
		strcpy(l_i.toname,toname);
		strcpy(l_i.state,"SFF");  
		strcpy(l_i.filename,filename);
        write(connfd,&l_i,sizeof(l_i));
        
      	close(fromfd);
        return sff_ok; 
        

    
    
    
	
	
	
}
int vch(int connfd,char username[])
{
	int fromfd;
	char filename[N];
	sprintf(filename,"%s.txt",username);
	
	if((fromfd = open(filename,O_RDONLY)) == -1 )
          {
                  perror("open from file error!");
  
                 return vch_fail;
          }
          
    char msg[N];
    memset(msg,0,sizeof(msg));
    int length=0;
  
    while( (length = read(fromfd,msg, sizeof(msg))) > 0 )    
        {   
            printf("%s",msg);    
        }
    return vch_ok;
	
}
void client_say(int socketfd)
{
	char buf[N]; 	
	system("clear");
	main_view();
	
    printf("your name is %s\n",Name);
    printf("your id is %d\n",Id);
    
    char username[N];
    strcpy(username,Name);
    int userid = Id;
    
	 while(1)
		{ 
		       
			    memset(buf,0,sizeof(buf));		            
                fgets(buf,50,stdin);
				buf[strlen(buf)-1] = '\0';
				
				
				if(strcmp(buf,"DOF") == 0)
				{
				    printf("the online list:\n");
                    if(dof_ok != dof(socketfd))
                    {
					   printf("no online!\n");
							   
					}					 
					else
					{
			           sleep(2);  
					} 
					continue;
				}		
			
			
				
				if(strcmp(buf,"STA") == 0)
				{
                   if(sta_ok == sta(socketfd))
                    {
					    sleep(2);										   
					}
					  					 
					else
					{
					   printf("sta fail!\n");
					  	sleep(2);
					  
					 } 
					continue;
				
				}
						 
				
				
				
				if(strcmp(buf,"STO") == 0)
				{	
					if(sto_ok == sto(socketfd,username))
                    {
					    sleep(2);										   
					}
					  					 
					else
					{
					  	printf("sto fail!\n");
					  	sleep(2);
					  
					} 
				 	continue; 
				}
				
				
				
				if(strcmp(buf,"DSM") == 0)
				{	
					if(userid==11111)
					{
						
						if(dsm_ok == dsm(socketfd))
	                    {
						    sleep(2);			
						}
						else
						{
						 	sleep(2);
						}
					}
					  					 
					else
					{
					  	printf("you don't have right to disable send msg'!\n");
					  	sleep(2);
					  
					} 
				 		 continue; 
				}
				
				
				
			    if(strcmp(buf,"ESM") == 0)
				{	
				    if(userid == 11111)
				    {
					
					 	if(esm_ok == esm(socketfd))
                      		{
					   		 sleep(2);
							   
					 		}
			    	}
					  					 
					else
					{
					  	printf("you don't have right to enable send msg'!\n");
					  	sleep(2);
					  
					 } 
				 		 continue; 
				}
				
						
				if(strcmp(buf,"SFF") == 0)
				{	
					if(sff_ok == sff(socketfd,username))
                    {
					    sleep(2);
						printf("send success!\n");										   
					}
					  					 
					else
					{
					  	printf("send fail!\n");
					  	sleep(2);
					  
					} 
				 	continue; 
				}
				
				
				
						if(strcmp(buf,"VCH") == 0)
				{	
					if(vch_ok == vch(socketfd,username))
                    {
					    sleep(2);
													   
					}
					  					 
					else
					{
					  	printf("view fail!\n");
					  	sleep(2);
					  
					} 
				 	continue; 
				}
				
				
				printf("input the corrent order!\n");
				sleep(1);
				continue;
							
		}
}
int main(int argc,char **argv)
{
		int socketfd;
		char buf[N];
		int n;
		struct sockaddr_in client_addr,server_addr;
		socklen_t addr_len;
		int c;

		pthread_t tid_read;

		if(argc != 2)
		{
				printf("input the ip!\n");
				exit(-1);
		}
		socketfd = socket(AF_INET,SOCK_STREAM,0);

		bzero(&server_addr,sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = inet_addr(argv[1]);
		server_addr.sin_port = htons(server_port);
        
		addr_len = sizeof(server_addr);
		c = connect(socketfd,(struct sockaddr *)&server_addr,sizeof(server_addr));

	    client_log(socketfd);
	   
	    c = pthread_create(&tid_read,NULL,(void*)read_thread,(void*)(&socketfd));
	    
    	client_say(socketfd); 
   
      

		
       pthread_join(tid_read,NULL);
      

		close(socketfd);
		return 0;


}
