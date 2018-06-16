
/*server.c*/
/*
1.����socketͨ����socket��Э�飬���ͣ�tcp��udp����0��
2.����ip��ַ�Ͷ˿ں� bing(sockid,���ص�ַ����ַ���ȣ�sizeof(struct sockaddr)��)
3.������listen(sockid,������)��
4.�ȴ��ͻ��˵����ӣ�accept(sockid,�ͻ���socked,����)��
5.send(sockid,buf,����,0) and recv(sockid,buf,����,0) 
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



int sockfd;//socket����ֵ
struct sockaddr_in server_sockaddr,client1_sockaddr,client2_sockaddr;
int sin_size, recvbytes;
//char buf[BUFFER_SIZE];
char ip1str[16];
char ip2str[16];

int sendfile(int cli_sockfd,char *cmd)//�����ļ�
{
	char buf[BUFFER_SIZE];
	char *filename=cmd;
	int sendbytes;
	FILE *fp;
	//char msg[BUFFER_SIZE]
	while(isspace(*filename)==0)//��ȡ�ļ���
		filename++;
	filename++;
	printf("filename:%s\n",filename);

	if((fp=fopen(filename,"r+"))==NULL)//��ʧ�ܾͷ���
	{
		memset(buf,0,sizeof(buf));
		printf("requrest file not exits!\n");
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
	while((length=fread(buf,1,sizeof(buf),fp))>0)//��ȡ���ݲ�����
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
	sleep(1);//notes : �Ӹ�sleep�����ȴ��Ϸ��ͻ��˽��������ݺ��ٽ��������˳�
	if(send(cli_sockfd,"200",3,0)==-1)//���ͳɹ��ı�ʶ��
	{
		printf("send error\n");
	}
	//close(cli_sockfd);
	printf("success to send.\n");
	fclose(fp);
	return 0;
}
int recvfile(int cli_sockfd,char *cmd)//�����ļ�
{
	char buf[BUFFER_SIZE];
	char *filename=cmd;
	int recvbytes;
	FILE *fp;
	while(isspace(*filename)==0)//��ȡ�ļ���
		filename++;
	filename++;

	if((fp=fopen(filename,"w+"))==NULL)//�����ļ�ʧ�ܾͷ���
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
		bzero(buf,BUFFER_SIZE);
	}
	printf("success to recv.\n");
	fclose(fp);
	return 0;
}
void * thread_func(void * arg)
{
	int thread_num=(int)arg;
	sin_size=sizeof(struct sockaddr);

	if(thread_num==0)
	{	
		char cmd[BUFFER_SIZE];
		int client1_fd;
		//FILE cli1_fd;
		while(1)
		{
			memset(cmd,0,sizeof(cmd));
			/*����listen����*/
			if (listen(sockfd, MAX_QUE_CONN_NM) == -1)
			{
				perror("listen");
				exit(1);
			}
			printf("thread %d listen...\n",thread_num);
		
			/*����accept�������ȴ��ͻ��˵�����*/
			if ((client1_fd = accept(sockfd, (struct sockaddr *)&client1_sockaddr, &sin_size)) == -1)//���ͻ��˴����ֵ����client1_sockaddr,����Ϊstruct sockaddr
			{
				perror("accept 0");
				exit(1);
			}
		
			inet_ntop(AF_INET,(char *)&client1_sockaddr.sin_addr.s_addr,ip1str,16);
			printf("thread 0 src ip:%s\nsrc port:%d\n",ip1str,client1_sockaddr.sin_port);
		
			while(1)
			{
				/*����recv�������տͻ��˵�����*/
				memset(cmd,0,sizeof(cmd));
				if ((recvbytes = recv(client1_fd,cmd,sizeof(cmd),0)) == -1)//���ܿͻ��˷��͵�����
				{
					perror("recv");
					pthread_exit(NULL);
				}
				//printf("%s\n", buf);
				if(strncmp(cmd,"down",4)==0)
				{
					sendfile(client1_fd,cmd);
					
				}
				else if(strncmp(cmd,"up",2)==0)
				{
					recvfile(client1_fd,cmd);
				}
				else if(strncmp(cmd,"exit",4)==0)
				{
					break;
				}
				//printf("Received from %s: %s\r",ip1str,buf);
				//printf("Received from %s,%s\nmessage:%s\r",ip1str, buf);
			}
			close(client1_fd);
			printf("\n\n");
		}
		
		
		
	}
	if(thread_num==1)
	{
		char cmd[BUFFER_SIZE];
		int client2_fd;
		while(1)
		{
			memset(cmd , 0, sizeof(cmd));
			/*����listen����*/
			if (listen(sockfd, MAX_QUE_CONN_NM) == -1)
			{
				perror("listen");
				exit(1);
			}
			printf("thread %d listen...\n",thread_num);
			
			if ((client2_fd = accept(sockfd, (struct sockaddr *)&client2_sockaddr, &sin_size)) == -1)//���ͻ��˴����ֵ����client1_sockaddr,����Ϊstruct sockaddr
			{
				perror("accept 1");
				exit(1);
			}
		
			inet_ntop(AF_INET,(char *)&client2_sockaddr.sin_addr.s_addr,ip2str,16);
			printf("thread 1 src ip:%s\nsrc port:%d\n",ip2str,client2_sockaddr.sin_port);
		
			while(1)
			{
				/*����recv�������տͻ��˵�����*/
				memset(cmd,0,sizeof(cmd));
				if ((recvbytes = recv(client2_fd,cmd,sizeof(cmd),0)) == -1)
				{
					perror("recv");
					pthread_exit(NULL);
				}
				//printf("%s\n",buf);
				if(strncmp(cmd,"down",4)==0)
				{
					sendfile(client2_fd,cmd);
				}
				else if(strncmp(cmd,"up",2)==0)
				{
					recvfile(client2_fd,cmd);
				}
				else if(strncmp(cmd,"exit",4)==0)
				{
					break;
				}
			}
			close(client2_fd);
			printf("\n\n");
		}
		
			
		//close(sockfd);
	}
}

int main()
{
	pthread_t thread[2];
	int no,res;
	void * thread_ret;
	printf("\t\t****************author by senge****************\n");
	/*����socket����*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0))== -1)
	{
		perror("socket");
		exit(1);
	}
	printf("Socket id = %d\n",sockfd);	

	/*����sockaddr_in �ṹ������ز���,�����׽�������*/
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(PORT);
	//inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr)//�������ÿɽ��ܵ�ip
	server_sockaddr.sin_addr.s_addr = INADDR_ANY;
	//bzero(&(server_sockaddr.sin_zero), 8);
	memset(&(server_sockaddr.sin_zero),0,8);

	int i = 1;/* ʹ���ظ�ʹ�ñ��ص�ַ���׽��ֽ��а� */
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));

	/*�󶨺���bind*/
	if (bind(sockfd, (struct sockaddr *)&server_sockaddr, sizeof(struct sockaddr))== -1)//��server_sockaddr�ṹ���ip�Ͷ˿ںŹ���
	{
		perror("bind 1");
		exit(1);
	}
	printf("thread Bind success!\n");


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
