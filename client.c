
/*client.c*/

#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>

#define PORT	4321
#define BUFFER_SIZE 1024


int cmd_down(int sockfd,char * cmd)//下载文件
{
	FILE *fp;
	char *filename=cmd;
	int recvbytes=0;
	char buf[BUFFER_SIZE];
	memset(buf,0,sizeof(buf));
	
	while(isspace(*filename)==0)//截取文件名
		filename++;
	filename++;

	if((fp=fopen(filename,"w+"))==NULL)//创建文件失败就返回
	{
		perror("create file error");
		return -1;
	}
	if(send(sockfd,cmd,strlen(cmd),0)==-1)//发送命令
	{
		perror("(down)send error");
		return -1;
	}
	printf("%s\n",filename);
	int length=0;
	while(length=recv(sockfd,buf,sizeof(buf),0)>0)
	{
		if (length < 0)  
        {  
            printf("Recieve Data From Server Failed!\n");  
            break;  
        } 
		printf("recv file length:%d\n",length);
	 	//printf("%s\n",buf);
		 /*
		 if(strncmp(buf,"404",3)==0)//请求资源不存在
		 {
			printf("%s\n",buf);//打印服务器返回的信息
			break;
		 }
		 */
		//printf("%s\n",buf);
		int write_length=fwrite(buf,sizeof(char),strlen(buf),fp);
	 	if(write_length<length)
	 	{
	 		printf("(down)write failed\n");
	 		break;
	 	}
		//printf("hi\n");
		bzero(buf,sizeof(buf));
	}
	printf("Recieve File:\t %s Finished!\n", filename);
	fclose(fp);
	return 0;
}
int cmd_up(int sockfd,char *cmd)//上传文件
{
	FILE *fp;
	char *filename=cmd;
	int sendbytes;
	char buf[BUFFER_SIZE];
	memset(buf,0,sizeof(buf));

	while(isspace(*filename)==0)
		filename++;
	filename++;
	printf("filename:%s\n",filename);
	if((fp=fopen(filename,"r+"))==NULL)//打开文件 判断文件是否存在,不存在则退出
	{
		perror("open file error");
		return -1;
	}
	if(send(sockfd,cmd,strlen(cmd),0)==-1)//发送命令和文件名
	{
		perror("(up)send file error");
		return -1;
	}
	//等待服务器返回可以确认发送的信息.....待定,未做;
	//sleep(1);//等待服务器接收
	
	//向服务器发送文件
	int length=0;
	while((sendbytes=fread(buf,1,sizeof(buf),fp))>0)
	{
	 	printf("send file length :%d \n",sendbytes);
	 	//printf("%s\n",buf);
	 	if((length=send(sockfd,buf,strlen(buf),0))<sendbytes)
	 	{
	 		printf("send file:%s failed\n",filename);
	 		break;
	 	}
	 	printf("length:%d\n",length);
	 	//memset(buf,0,sizeof(buf));
	}
	fclose(fp);
	return 0;
}
int cmd_exit(int sockfd,char * cmd)//发送退出消息
{
	int sendbytes;
	if((sendbytes=send(sockfd,cmd,sizeof(cmd),0))<0)//发送命令
	{
		printf("(exit)send failed\n");
		return -1;
	}
	return 0;
}
void help()
{
	printf("\n");
	printf(" down [filename]:\tdownload file\n");
	printf(" up [filename]:\t\tupload file\n");
	printf(" help:\t\t\tshow cmd\n");
	printf(" exit:\t\t\texit client\n");
	printf("\n");
}
int main(int argc,char *argv[])
{
	int sockfd, sendbytes;
	struct hostent *host;
	struct sockaddr_in serv_addr;

	if(argc<2)
	{
		printf("usage:%s ip\n",argv[0]);
		exit(-1);
	}	
	//char ip[16];
	//char ipstr[16];  //存储服务器ip

	time_t t_start,t_end;
	struct tm *ts;

	char cmd[BUFFER_SIZE];
	int filename[BUFFER_SIZE];
	memset(cmd,0,sizeof(cmd));//清零
	memset(filename,0,sizeof(filename));

	printf("\t\t****************author by sastar****************\n");
	
	//printf("please input ip:");//中文乱码
	//scanf("%s",ip);
	//getchar();//吞掉回车键
	
	/*地址解析函数*/
	if ((host = gethostbyname (argv[1])) == NULL)
	{
		perror("gethostbyname");
		exit(1);
	}
	
	/*创建socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("socket");
		exit(1);
	}

	/*设置sockaddr_in 结构体中相关参数*/
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr = *((struct in_addr *)host->h_addr);//服务器ip地址
	bzero(&(serv_addr.sin_zero), 8);

	/*调用connect函数主动发起对服务器端的连接*/
	if(connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(struct sockaddr))== -1)
	{
		perror("connect");
		exit(1);
	}
	
	t_start=time(NULL);//输出时间
	ts=localtime(&t_start);
	printf("\t\tconnect localtime:%s\n",asctime(ts));

	//inet_ntop(AF_INET,(char *)&serv_addr.sin_addr.s_addr,ipstr,16);
	//printf("server ip:%s\nserver port:%d\n",ipstr,serv_addr.sin_port);

	//命令行下载,上传文件;
	while(1)
	{
		printf("please input your cmd:");
		fgets(cmd,sizeof(cmd),stdin);
		cmd[strlen(cmd)-1]='\0';//去除回车符
		if(strncmp(cmd,"down",4)==0)
		{
			cmd_down(sockfd,cmd);
		}
		else if(strncmp(cmd,"up",2)==0)
		{
			cmd_up(sockfd,cmd);
		}
		else if(strncmp(cmd,"exit",4)==0)
		{
			cmd_exit(sockfd,cmd);
			break;
		}
		else
		{
			help();
		}
	}

	t_end=time(NULL);//输出时间
	ts=localtime(&t_end);
	printf("\t\texit localtime:%s\t\t\tThe process has runing %.0f s\n",asctime(ts),difftime(t_end,t_start));
	close(sockfd);
	exit(0);
	
}