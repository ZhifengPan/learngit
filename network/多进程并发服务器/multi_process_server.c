#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <ctype.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>//STDOUT_FILENO
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SRV_PORT 9999

//回收子进程
void catch_child(int signum)
{
	while((waitpid(0,NULL,WNOHANG)) > 0);
	return ;
}

int main(int argc,char *argv[])
{
	int lfd,n,m,cfd,ret,i;
	struct sockaddr_in srv_addr,clt_addr;
	socklen_t ctl_addr_len;
	pid_t pid;

	char buf[BUFSIZ];

	srv_addr.sin_family = AF_INET;
	srv_addr.sin_port = htons(SRV_PORT);
	srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	lfd = socket(AF_INET,SOCK_STREAM,0);
	if(lfd == -1)
	{
		printf("socket error!\n");
		return 0;
	}

	n = bind(lfd,(struct sockaddr *)&srv_addr,sizeof(srv_addr));
	if(n == -1)
	{
		printf("bind error!\n");
		return 0;
	}	

	m = listen(lfd,128);
	if(m == -1)
	{
		printf("listen error!\n");
		return 0;
	}

	ctl_addr_len = sizeof(clt_addr);

	while (1)
	{
		cfd = accept(lfd,(struct sockaddr *)&clt_addr,&ctl_addr_len);
	
		pid = fork();
		if(pid < 0 )
		{
			printf("fork erro!\n");
			return 0;
		}
		else if(pid == 0)//子进程
		{
			close(lfd);
			break;//处理部分放到外面进行。
		}
		else
		{
			struct sigaction act;//设计信号捕捉，回收子进程资源。

			act.sa_handler = catch_child;
			sigemptyset(&act.sa_mask);
			act.sa_flags = 0;

			ret = sigaction(SIGCHLD,&act,NULL);
			if(ret != 0)
			{
				printf("sigaction error!\n");
				return 0;
			}

			close(cfd);
			continue;
		}
		
		if(pid == 0)
		{
			while(1)
			{
				ret = read(cfd,buf,sizeof(buf));
				if(ret == 0)
				{
					close(cfd);
					exit(1);
				}
				for(i = 0;i < ret;i++)
				{
					buf[i] = toupper(buf[i]);
				}
				write(cfd,buf,ret);
				write(STDOUT_FILENO,buf,ret);
			}	
		}
	}
	return 0;
}
