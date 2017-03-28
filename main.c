#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

void print_usage(char *file)
{
	printf("Usage:\n");
	printf("%s <dev> <on | off>\n",file);
}

int main(int argc, char **argv)
{
	int fd;
	char val;
	char *filename;
	if(argc != 3)
	{
		print_usage(argv[0]);
		return 0;
	}
	filename = argv[1];
	fd = open(filename,O_RDWR);
	if(fd < 0)
	{
		printf("-- open fail (%s(%d)--<%s) \n",__FILE__,__LINE__,__FUNCTION__);
	}
	if(!strcmp(argv[2],"on"))
	{
		//ÁÁµÆ
		val = 0;
		write(fd,&val,1);	
	}
	else if(!strcmp(argv[2],"off"))
	{
		//ÃðµÆ
		val = 1;
		write(fd,&val,1);		
	}
	else
	{
		print_usage(argv[0]);
	}
	return 0;
}



