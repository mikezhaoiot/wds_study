#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

/* 
*
*/
int main(int argc, char **argv)
{
	int fd;
	int val = 1;
	fd = open("/dev/xyz",O_RDWR);
	if(fd < 0)
	{
		printf("-- open fail (%s(%d)--<%s) \n",__FILE__,__LINE__,__FUNCTION__);
	}
	if(argc != 2)
	{
		printf("Usage :");
		// <> 尖括号表示不可省略
		printf("%s <on|off>\n",argv[0]);
		return 0;
	}
	if(strcmp(argv[1],"on") == 0)
	{
		val = 0;	
	}
	else
	{
		val = 1;	
	}
	write(fd,&val,4);
}



