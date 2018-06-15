
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

struct userinfo//�ͻ�����Ϣ
{
	char name[10];
	char buf[BUFFER_SIZE];
};

int sockfd;//socket����ֵ
int client1_fd,client2_fd;//accept����ֵ
struct sockaddr_in server_sockaddr,client1_sockaddr,client2_sockaddr;
int sin_size, recvbytes;
//char buf[BUFFER_SIZE];
char ip1str[16];
char ip2str[16];
struct userinfo user1,user2;


void * thread_func(void * arg)
{
	int thread_num=(int)arg;
	sin_size=sizeof(struct sockaddr);

	if(thread_num==0)
	{	
		while(1)
		{
			memset(user1.buf , 0, sizeof(user1.buf));
			memset(user1.name , 0, sizeof(user1.name));
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
		
		
			while(strncmp(user1.buf,"quit",4)!=0)
			{
				/*����recv�������տͻ��˵�����*/
				memset(user1.buf , 0, sizeof(user1.buf));
				memset(user1.name , 0, sizeof(user1.name));
				if ((recvbytes = recv(client1_fd,&user1,sizeof(user1),0)) == -1)
				{
					perror("recv");
					pthread_exit(NULL);
				}
				printf("%s:%s\n",user1.name,user1.buf);
				if((send(client2_fd,&user1,sizeof(user1),0)) == -1)
				{
					perror("send 1");
					//pthread_exit(NULL);
					break;
				}
				//printf("Received from %s: %s\r",ip1str,buf);
				printf("Received from %s,%s\nmessage:%s\r",ip1str,user1.name,user1.buf);
			}
			close(client1_fd);
			printf("\n\n");
		}
		
		
		
	}
	if(thread_num==1)
	{
		while(1)
		{
			memset(user2.buf , 0, sizeof(user2.buf));
			memset(user2.name , 0, sizeof(user2.name));
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
		
			while(strncmp(user2.buf,"quit",4)!=0)
			{
				/*����recv�������տͻ��˵�����*/
				memset(user2.buf , 0, sizeof(user2.buf));
				memset(user2.name , 0, sizeof(user2.name));
				if ((recvbytes = recv(client2_fd, &user2, sizeof(user2), 0)) == -1)
				{
					perror("recv");
					pthread_exit(NULL);
				}
				printf("%s:%s\n",user2.name,user2.buf);
				if((send(client1_fd, &user2, sizeof(user2), 0)) == -1)
				{
					perror("send 2");
					//pthread_exit(NULL);
					break;
				}
				//printf("Received from %s: %s\r",ip2str,buf);
				printf("Received from %s,%s\nmessage:%s\r",ip2str,user2.name,user2.buf);
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
