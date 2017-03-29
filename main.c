#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char **argv)
{
	int fd ;
	unsigned char key_value;
	fd = open("/dev/button",O_RDWR);
	if(fd < 0)
	{
		printf(" -- open driver open --\n");
		return 0;
	}
	while(1)
	{
		read(fd,&key_value,1);
		printf("key_value = 0x%x\n",key_value);

	}
	return 0;
}