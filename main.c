#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <poll.h>

int main(int argc, char **argv)
{
	int fd ;
	unsigned char key_value;
	int ret;
	struct pollfd fds[1];
	fd = open("/dev/button",O_RDWR);
	if(fd < 0)
	{
		printf(" -- open driver open --\n");
		return 0;
	}
	fds[0].fd     = fd;
	fds[0].events = POLLIN;
	
	while(1)
	{	
		ret = poll(fds,1,5000);
		if(0 == ret)
		{
			printf("-- time out--\n");
		}
		else 
		{
			read(fd,&key_value,1);
			printf("key_value = 0x%x\n",key_value);
		}
	}
	return 0;
}