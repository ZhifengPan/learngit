#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <pthread.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define MAXLINE 8192
#define SER_PORT 6666

struct s_info
{
    struct sockaddr_in c_addr;
    int afd;
};

void *do_work(void *arg)
{
    int n , i;
    struct s_info *ts = (struct s_info *)arg;
    char buf[MAXLINE];
    char str[INET_ADDRSTRLEN];//#define INET_ADDRSTRLEN 16 可用"[+d"查看

    while(1)
    {
        n = read(ts->afd,buf,MAXLINE);
        if(n == 0)
        {
            printf("client %d closed...\n",ts->afd);
            break;
        }
        printf("received from %s at port %d\n",
                inet_ntop(AF_INET,&(*ts).c_addr.sin_addr ,str ,sizeof(str)),
                ntohs((*ts).c_addr.sin_port));
        for(i = 0; i < n; i++)
            buf[i] = toupper(buf[i]);

        write(STDOUT_FILENO,buf,n);//屏幕显示
        write(ts->afd,buf,n);//回发给客户端
    }
    close(ts->afd);

    return (void *)0;
}

void main(void)
{
    int listenfd , b_ret , l_ret , afd , i;
    struct sockaddr_in ser_addr , cli_addr;
    socklen_t cli_addr_len;
    struct s_info ts[256];
    pthread_t tid;

    bzero(&ser_addr,sizeof(ser_addr));//清空结构体

    ser_addr.sin_family = AF_INET;
    ser_addr.sin_port = htons(SER_PORT);
    ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    listenfd = socket(AF_INET,SOCK_STREAM,0);
    if(listenfd == -1)
    {
        printf("socket error!\n");
    }

    b_ret = bind(listenfd,(struct sockaddr *)&ser_addr,sizeof(ser_addr));
    if(b_ret == -1)
    {
        printf("bind error!\n");
    }

    l_ret = listen(listenfd,128);
    if(l_ret == -1)
    {
        printf("listen error!\n");
    }

    while(1)
    {
        cli_addr_len = sizeof(cli_addr);
        afd = accept(listenfd,(struct sockaddr *)&cli_addr,&cli_addr_len);
        ts[i].c_addr = cli_addr;
        ts[i].afd = afd;

        pthread_create(&tid,NULL,do_work,(void *)&ts[i]);
        pthread_detach(tid);//子线程分离，防止僵尸线程产生。
        i++;
    }
    return ;
}