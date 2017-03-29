#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>




int fd;
void my_signal_fun(int signum)
{
	unsigned char key_value;
	read(fd,&key_value,1);
	printf("key_value = 0x%x\n",key_value);
}
int main(int argc, char **argv)
{
	
	
	int ret;
	int Oflags;
	signal(SIGIO,my_signal_fun);

	fd = open("/dev/button",O_RDWR);
	if(fd < 0)
	{
		printf(" -- open driver open --\n");
		return 0;
	}

	fcntl(fd,F_SETOWN,getpid());
	
	Oflags = fcntl(fd,F_SETFL);
	fcntl(fd,F_SETFL, Oflags | FASYNC);

	while(1)
	{	
		sleep(1000);
	}
	return 0;
}