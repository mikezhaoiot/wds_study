#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char **argv)
{
	int fd ;
	unsigned char key_value[4];
	fd = open("/dev/button",O_RDWR);
	if(fd < 0)
	{
		printf(" -- open driver open --\n");
		return 0;
	}
	while(1)
	{
		read(fd,key_value,sizeof(key_value));
		if(!key_value[0] || !key_value[1] || !key_value[2] || !key_value[3])
		{
			printf("key pressed: %d %d %d %d\n",key_value[0],key_value[1],key_value[2],key_value[3]);
		}
	}
	return 0;
}