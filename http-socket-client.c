#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h> 
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/ioctl.h>  
#include <netdb.h>
#include <error.h>





#define VOS_ERR -1
#define VOS_OK 0
#define PORT 8080
#define URL_LEN 1024
#define SNDTIMEOUT 20
#define RCVTIMEOUT 30
#define URL "www.baidu.com"
#define SLEEP  sleep(30)


int Init()
{
	//	printf("wanname :%s\n",wanname);
	return 0;
}

void safe_gettimeofday(struct timeval *now)
{
	int len = sizeof(struct timeval);
	memset(now,0,sizeof(struct timeval));
	return;
}

int count_logtime(struct timeval start,struct timeval end)
{
	int temp = 0;
	temp = 1000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec)/1000;
	return temp;
}

int http_Socket_Connect_server(char * mserver) 
{
	int ulfd = -1;
	struct hostent *url;
	int error = 0;	
	fd_set set;
	struct timeval tm;
	unsigned long ul = 1;
	int ret = 1;
	
	struct sockaddr_in stSocketAddr;

	printf("\r\n%s %d http_Socket_Connect_server begin\r\n",__FILE__,__LINE__);

	//1.new socket
	ulfd = socket(AF_INET, SOCK_STREAM, 0);
	if(ulfd < 0)
    {
		printf("\r\n new socket fail");
		return VOS_ERR;
	}
	
	url = gethostbyname(mserver);//�����������������õ�ip��ַ
	if(url == NULL)
	{	
		close(ulfd);
		printf("gethostbyname fail");
		return VOS_ERR;
	}

	//2.����Ŀ�� socket ip and port
	memset(&stSocketAddr,0,sizeof(stSocketAddr));
	stSocketAddr.sin_family = AF_INET;
	stSocketAddr.sin_addr.s_addr =  inet_addr("192.168.3.178");   
	//stSocketAddr.sin_addr = *((struct in_addr *)url->h_addr);
	stSocketAddr.sin_port = htons(PORT);//htons ���ֽ���ת���������ֽ���


	//ioctl(ulfd, FIONBIO, &ul); //����Ϊ������ģʽ
	
	//3.����TCP���� �����������
	if(connect(ulfd, (struct sockaddr *)&stSocketAddr, sizeof(stSocketAddr)) == VOS_ERR)
	{
		//����ʧ�ܷ���-1
		perror("connect socket fail");
		close(ulfd);
		return VOS_ERR;
	}
	
//	ul = 1;
//	ioctl(ulfd, FIONBIO, &ul); //����Ϊ����ģʽ

	return ulfd; 

}



int HTTP_Send(int ulfd, char *szbuf, int ulLen)
{

	char szSendBUff[URL_LEN] = {0};
	int ulRet = 0;
		
	//��ʱ����
	struct timeval tv =  {SNDTIMEOUT, 0}; 
	int nzero = 0;
	
	printf("\r\n%s %d HTTP_Send begin\r\n",__FILE__,__LINE__);
	//�������������������Ҫ����socket�Ĺؼ���
	setsockopt(ulfd,SOL_SOCKET,SO_SNDBUF,(char *)&nzero,sizeof(nzero));
	setsockopt(ulfd,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(tv));
	
	//�������ݣ�buf����д��http�����ݡ�
	ulRet = send(ulfd, szSendBUff, ulLen, 0);

	if(ulRet != ulLen)
	{
		return VOS_ERR;
	}
	
	return VOS_OK;	
}

int HTTP_Receive(int ulfd, char *szBuf)
{
	int ulRet = 0;
	char *szBuffer=szBuf;
	int ulLen = 0;
	int ulRecevSize = 0;
	
	printf("\r\n%s %d HTTP_Receive begin\r\n",__FILE__,__LINE__);
	//��ʱ����
	struct timeval sttv =  {RCVTIMEOUT, 0}; 
	setsockopt(ulfd,SOL_SOCKET,SO_RCVTIMEO,&sttv,sizeof(sttv));
	//http ��������
	//ѭ���ȴ��ջ�����
	while(0 < (ulLen = recv(ulfd, szBuf+ulRecevSize, 1024, 0))) 
	{
		ulRecevSize +=ulLen; 
		if(ulRecevSize > 9216)
		{
		    break;
		}
	}
	szBuf[ulRecevSize] = '\0';

	return ulRecevSize;

}

/*--------------------------------------------------------------------------------

 *name    :http_get_head
 *function:get http head
 *arg    :
buf:recv data
head:http head
 *return  :
 ---------------------------------------------------------------------------------*/
int http_get_head(char * buf)
{
	char * head = NULL;
	char *ch = NULL;
	if((NULL == (ch = strstr(buf,"\r\n\r\n")))&&(NULL == (ch = strstr(buf,"\n\n")))){
		return -1;
	}
	if('\r' == *ch)
		ch +=4;
	else
		ch +=2;
	*ch = '\0';

	return 0;
}
int http_get_responsecode(char * head,char * responsecode)
{

	char *ch = NULL;
	if((NULL == head)||(NULL == responsecode))
		return -1;
	if((NULL == (ch = strstr(head,"\r\n")))&&(NULL == (ch = strstr(head,"\n")))){
		return -1;
	}
	strncpy(responsecode,head,ch-head);
	return 0;
}
 
int Client_HeartBeatSend()                                                                                           
{                                                                                                                         
	int ulfd = -1;
	int ulSize = 0;
	char szSendBuff[1024] = {0};
	char szRecBuff[1024] = {0};	
	char responsecode[256] = {0};

    printf("\n\n%s %d \n\n",__FILE__,__LINE__);

	//����socket����																													  
	ulfd = http_Socket_Connect_server(URL); //URL������
	if(ulfd == VOS_ERR)
	{
		return VOS_ERR;		
	}

	//��װhttp���ݲ�����
	ulSize = sprintf(szSendBuff,"GET HTTP/1.1\r\n"
			"HOST: s%\r\n"
			"User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux x86_64; rv:39.0) Gecko/20100101 Firefox/39.0\r\n"
			"Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
			"Accept-Language:zh-CN,zh;q=0.8,en-US;q=0.5,en;q=0.3\r\n"
			"Accept-Encoding:gzip, deflate\r\n"
			"Connection: close\r\n\r\n");
	if(VOS_ERR == HTTP_Send(ulfd,szSendBuff,ulSize))
	{
		close(ulfd);
		
		printf("\r\n%s %d HTTP_Send return fail\r\n",__FILE__,__LINE__);
		return VOS_ERR;
	}

	//���շ��������ص�����
	if(HTTP_Receive(ulfd,szRecBuff) <= 0)		
	{
		close(ulfd);	
		printf("\r\n%s %d recvbuf fail\r\n",__FILE__,__LINE__);
		return VOS_ERR;
	}

	//�ر�socket	
	close(ulfd);

    //�������������ص�����
	http_get_head(szRecBuff);
	http_get_responsecode(szRecBuff,responsecode);
	if(NULL == strstr(responsecode,"200"))		
	{   //����������ʧ��
		
		printf("%s %d Client_HeartBeatSend server return fail\n\n",__FILE__,__LINE__);
		return VOS_ERR;
	}
	
	printf("\n\n%s %d Client_HeartBeatSend return ok\n\n",__FILE__,__LINE__);

    return VOS_OK;                                                                                                             
}


int main()
{
	printf("\n\n main %s %d\n\n",__FILE__,__LINE__);
	while(1)
	{       
		printf("\n\n%s %d\n\n",__FILE__,__LINE__);

		if(-1 == Client_HeartBeatSend())
		{			
			printf("\n\n%s %d\n\n",__FILE__,__LINE__);
			SLEEP;
			continue;
		}
	}
	return 0;
}



