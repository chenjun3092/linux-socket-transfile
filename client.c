
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

struct userinfo
{
	char name[10];
	char buf[BUFFER_SIZE];
};

int sockfd, sendbytes;
//char buf[BUFFER_SIZE];
struct hostent *host;
struct sockaddr_in serv_addr;

struct userinfo user;
	
char ip[16];
//char ipstr[16];  //�洢������ip
char name[10];

time_t t_start,t_end;
struct tm *ts;

void * thread_func(void * arg)
{
	int thread_num=(int)arg;
	if(thread_num==0)//������Ϣ
	{
		while(strncmp(user.buf,"quit",4)!=0)
		{
			memset(user.buf, 0, sizeof(user.buf));
			//printf("please input data:");
			fgets(user.buf,sizeof(user.buf),stdin);
			/*������Ϣ����������*/
			if ((sendbytes = send(sockfd, &user, sizeof(user), 0)) == -1)
			{
				perror("send");
				pthread_exit(NULL);
			}
		}
	}
	if(thread_num==1)//������Ϣ
	{
		while(strncmp(user.buf,"quit",4)!=0)
		{
			memset(user.buf, 0, sizeof(user.buf));
			if(recv(sockfd,&user,sizeof(user),0)==-1)
			{
				perror("recv");
				pthread_exit(NULL);
			}
			//printf("receive:%s\n",buf);
			user.buf[strlen(user.buf)-1]='\0';
			printf("\t\t\t\t%s:%s\n",user.name,user.buf);
			strcpy(user.name,name);//�������ֵ�һ�£������޸�
		}
	}
	
}
int main()
{
	pthread_t thread[2];
	int no,res;
	void * ret;
	printf("\t\t****************author by senge****************\n");
	printf("please input your name:");//������Ϣ
	scanf("%s",name);
	strcpy(user.name,name);
	
	printf("please input ip:");//��������
	scanf("%s",ip);
	getchar();//�̵��س���
	
	/*��ַ��������*/
	if ((host = gethostbyname(ip)) == NULL)
	{
		perror("gethostbyname");
		exit(1);
	}
	
	/*����socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		perror("socket");
		exit(1);
	}

	/*����sockaddr_in �ṹ������ز���*/
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr = *((struct in_addr *)host->h_addr);//������ip��ַ
	bzero(&(serv_addr.sin_zero), 8);

	/*����connect������������Է������˵�����*/
	if(connect(sockfd,(struct sockaddr *)&serv_addr, sizeof(struct sockaddr))== -1)
	{
		perror("connect");
		exit(1);
	}
	
	t_start=time(NULL);
	ts=localtime(&t_start);
	printf("\t\tconnect localtime:%s\n",asctime(ts));

	//inet_ntop(AF_INET,(char *)&serv_addr.sin_addr.s_addr,ipstr,16);
	//printf("server ip:%s\nserver port:%d\n",ipstr,serv_addr.sin_port);

	for(no=0;no<2;no++)
	{
		if(pthread_create(&thread[no],NULL,thread_func,(void *)no)==-1)
		{
			perror("pthread_create");
			exit(0);
		}
		printf("create thread %d success!\n",no);
	}
	for(no=0;no<2;no++)
	{
		if(pthread_join(thread[no],&ret)==0)
			printf("thread %d joined!\n",no);
		else
			printf("thread %d join failed!\n",no);
	}
	t_end=time(NULL);
	ts=localtime(&t_end);
	printf("\t\texit localtime:%s\t\t\tThe process has runing %.0f s\n",asctime(ts),difftime(t_end,t_start));
	close(sockfd);
	exit(0);
	
}
