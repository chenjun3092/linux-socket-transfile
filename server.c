/*server.c*/
/*
1.建立socket通道，socket（协议，类型（tcp／udp），0）
2.公开ip地址和端口号 bing(sockid,本地地址，地址长度（sizeof(struct sockaddr)）)
3.侦听（listen(sockid,请求数)）
4.等待客户端的连接（accept(sockid,客户段socked,长度)）
5.send(sockid,buf,长度,0) and recv(sockid,buf,长度,0) 
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>

#define PORT			4321
#define BUFFER_SIZE		1024
#define MAX_QUE_CONN_NM	5



int sockfd;//socket返回值
struct sockaddr_in server_sockaddr,client_sockaddr;
int sin_size, recvbytes;
//char buf[BUFFER_SIZE];
char ipstr[16];

int sendfile(int cli_sockfd,char *cmd)//发送文件
{
	char buf[BUFFER_SIZE];
	char *filename=cmd;
	int sendbytes;
	FILE *fp;
	//char msg[BUFFER_SIZE]
	while(isspace(*filename)==0)//截取文件名
		filename++;
	filename++;
	printf("filename:%s\n",filename);

	if((fp=fopen(filename,"r+"))==NULL)//打开失败就返回
	{
		memset(buf,0,sizeof(buf));
		printf("requrest file not exits!\n\n");
		//sscanf("404","%s",buf);
		strcpy(buf,"404");
		write(cli_sockfd,buf,3);
		return -1;
	}
	else
	{
		memset(buf,0,sizeof(buf));
		strcpy(buf,"200");
		write(cli_sockfd,buf,3);
	}
	int length=0;
	while((length=fread(buf,1,sizeof(buf),fp))>0)//读取数据并发送
	{
	 	printf("send file length :%d \n",length);
	 	//printf("%s\n",buf);
	 	if(send(cli_sockfd,buf,length,0)<0)
	 	{
	 		printf("send file:%s failed\n",filename);
	 		break;
	 	}
	 	memset(buf,0,sizeof(buf));
	}
	//sleep(1);//notes : 加个sleep函数等待上方客户端接受完数据后再接收数据退出
	if(recv(cli_sockfd,buf,sizeof(buf),0)>0)//互斥,与和客户端保持同步
	{
		if(strncmp(buf,"200",3)==0)
		{
			printf("success to send.\n\n");
		}
	}
	/*
	if(send(cli_sockfd,"200",3,0)==-1)//发送成功的标识符
	{
		printf("send error\n");
	}
	*/
	fclose(fp);
	return 0;
}
int recvfile(int cli_sockfd,char *cmd)//接受文件
{
	char buf[BUFFER_SIZE];
	char *filename=cmd;
	int recvbytes;
	FILE *fp;
	while(isspace(*filename)==0)//截取文件名
		filename++;
	filename++;

	if((fp=fopen(filename,"w+"))==NULL)//创建文件失败就返回
	{
		perror("create file error");
		return -1;
	}
	
	while(recvbytes=recv(cli_sockfd,buf,sizeof(buf),0))
	{
		if (recvbytes < 0)  
        {  
            printf("Recieve Data Failed!\n");  
            break;  
        } 
		if(strncmp(buf,"200",3)==0)
		{
			printf("%s\n",buf);
			break;
		}
		printf("recv file length:%d\n",recvbytes);
	 	//printf("%s\n",buf);
	 	if(fwrite(buf,sizeof(char),recvbytes,fp)<recvbytes)
	 	{
	 		printf("(recvfile)write failed\n");
	 		break;
	 	}
		if(recvbytes<sizeof(buf))
		{
			strcpy(buf,"200");
			send(cli_sockfd,buf,strlen(buf),0);
			break;
		}
		bzero(buf,BUFFER_SIZE);
	}
	printf("success to recv.\n\n");
	fclose(fp);
	return 0;
}
int lsfile(int cli_sockfd,char *cmd)//列出当前目录文件并发送到客户端
{
	FILE *fp;
	char buf[BUFFER_SIZE];
	char *ls=cmd;
	int i;
	memset(buf,0,sizeof(buf));
	for(i=0;i<4;i++)
		ls++;
	char *s=" -l";
	strcat(ls,s);
	if((fp=popen(ls,"r"))==NULL)
	{
		perror("Popen error");
		return -1;
	}
	char str[BUFFER_SIZE];
	while((fgets(buf,BUFFER_SIZE,fp))!=NULL)//读取数据并发送
	{
		strcat(str,buf);//字符串连接函数
	 	memset(buf,0,sizeof(buf));
	}
	pclose(fp);
	printf("%s\n",str);
	if(send(cli_sockfd,str,strlen(str),0)<0)
	{
	 	printf("send list failed\n");
	 	return -1;
	}
	printf("success to ls\n\n");
	return 0;
}
void * thread_func(void * arg)
{
	int thread_num=(int)arg;
	sin_size=sizeof(struct sockaddr);
	
	char cmd[BUFFER_SIZE];
	int client_fd;
	//FILE cli1_fd;
	while(1)
	{
		memset(cmd,0,sizeof(cmd));
		/*调用listen函数*/
		if (listen(sockfd, MAX_QUE_CONN_NM) == -1)
		{
			perror("listen");
			exit(1);
		}
		printf("thread %d listen...\n",thread_num);
	
		/*调用accept函数，等待客户端的连接*/
		if ((client_fd = accept(sockfd, (struct sockaddr *)&client_sockaddr, &sin_size)) == -1)//将客户端传入的值传入client_sockaddr,长度为struct sockaddr
		{
			perror("accept 0");
			exit(1);
		}
	
		inet_ntop(AF_INET,(char *)&client_sockaddr.sin_addr.s_addr,ipstr,16);
		printf("thread 0 src ip:%s\nsrc port:%d\n",ipstr,client_sockaddr.sin_port);
	
		while(1)
		{
			/*调用recv函数接收客户端的请求*/
			memset(cmd,0,sizeof(cmd));
			if ((recvbytes = recv(client_fd,cmd,sizeof(cmd),0)) == -1)//接受客户端发送的命令
			{
				perror("recv");
				pthread_exit(NULL);
			}
			//printf("%s\n", buf);
			if(strncmp(cmd,"servls",6)==0)
			{
				lsfile(client_fd,cmd);
			}
			else if(strncmp(cmd,"down ",5)==0)
			{
				sendfile(client_fd,cmd);
			}
			else if(strncmp(cmd,"up ",3)==0)
			{
				recvfile(client_fd,cmd);
			}
			else if(strncmp(cmd,"exit",4)==0)
			{
				break;
			}
			//printf("Received from %s: %s\r",ipstr,buf);
			//printf("Received from %s,%s\nmessage:%s\r",ipstr, buf);
		}
		close(client_fd);
		printf("\n\n");
	}	
}

int main()
{
	pthread_t thread[2];
	int no,res;
	void * thread_ret;
	printf("\t\t****************author by sastar****************\n");
	/*建立socket连接*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0))== -1)
	{
		perror("socket");
		exit(1);
	}
	printf("Socket id = %d\n",sockfd);	

	/*设置sockaddr_in 结构体中相关参数,设置套接字属性*/
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(PORT);
	//inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr)//单独设置可接受的ip
	server_sockaddr.sin_addr.s_addr = INADDR_ANY;
	//bzero(&(server_sockaddr.sin_zero), 8);
	memset(&(server_sockaddr.sin_zero),0,8);

	int i = 1;/* 使得重复使用本地地址与套接字进行绑定 */
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));

	/*绑定函数bind*/
	if (bind(sockfd, (struct sockaddr *)&server_sockaddr, sizeof(struct sockaddr))== -1)//将server_sockaddr结构体的ip和端口号公布
	{
		perror("bind 1");
		exit(1);
	}
	printf("Bind success!\n");


	for(no=0;no<2;no++)
	{
		res=pthread_create(&thread[no],NULL,thread_func,(void *)no);
		if(res==-1)
		{
			printf("create thread failed!\n");
			exit(res);
		}
	}
	for(no=0;no<2;no++)
	{
		if((res=pthread_join(thread[no],&thread_ret))==0)
		{
			printf("\tthread %d joined!\n",no);
		}
		else
			printf("\tthread %d joined failed!\n",no);
	}
	close(sockfd);
	exit(0);
}