#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>

static void usage(char *);

int main(int argc,char **argv)
{
	int fd;
	int val;
	char *filename;
	filename = argv[1];
	fd = open(filename,O_RDWR);
	if(argc != 3)
	{
		usage(argv[0]);
	}
	if(strcmp(argv[2],"on") == 0)
	{
		val = 1;
	}
	else if(strcmp(argv[2],"off") == 0)
	{
		val = 0;
	}
	else
	{
		usage(argv[0]);
	}
	write(fd,&val,4);
	return 0;
}

void usage(char *file)
{
	printf("Usage:\n");
	printf("%s /dev/leds <on|off>\n",file);
	printf("%s /dev/led1 <on|off>\n",file);
	printf("%s /dev/led2 <on|off>\n",file);
	printf("%s /dev/led3 <on|off>\n",file);
}


