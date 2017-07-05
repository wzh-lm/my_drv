
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

static void print_err(char *com)
{
	printf("Please input in right command!\n");
	printf("Usage: %s on/off\n", com);
}

int main(int argc, char **argv)
{
	int fd;
	char buf;
	if(argc != 2)
	{
		print_err(argv[0]);
		return 1;
	}
	fd = open("/dev/wzh_led", O_RDWR);
	if(fd < 0 )
		printf("File cannot be opened!\n");
	if(strcmp(argv[1], "on") == 0)
	{
		buf = 1;
	}
	else if(strcmp(argv[1], "off") == 0)
	{
		buf = 0;	
	}
	else 
	{
		print_err(argv[0]);
		return 1;
	}
	write(fd, &buf, 1);
	return 0;
}


