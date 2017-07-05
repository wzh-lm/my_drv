#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>


int main(int argc, int **argv)
{
	int fd;
	int ret;
	unsigned char key_val;
	struct pollfd fds[1];
	
	
	fd = open("/dev/irq_drv",O_RDWR);	
	if(fd < 0)
		printf("Error:Cannot open the file!\n");
	
	fds[0].fd     = fd;
	fds[0].events = POLLIN;
	
	while(1)
	{
		ret = poll(fds, 1, 5000);
		if(ret == 0)
		{
			printf("Time out!\n");
		}
		else 
		{
			read(fd,&key_val,1);
			printf("key_val = %d\n",key_val);
		}
	}
	return 0;
}







