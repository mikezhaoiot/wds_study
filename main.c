#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>


int main(int argc, char **argv)
{
	int fd;
	int val = 1;
	fd = open("/dev/xxx",O_RDWR);
	if(fd < 0)
	{
		printf("-- open fail (%s(%d)--<%s) \n",__FILE__,__LINE__,__FUNCTION__);
	}
	write(fd,&val,4);
}



