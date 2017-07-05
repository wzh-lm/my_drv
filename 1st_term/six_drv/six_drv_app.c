#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>





int main(int argc, int **argv)
{
	int ret;
	int fd;
	unsigned char key_val;
	
	fd = open("/dev/irq_drv",O_RDWR);		
	if(fd < 0)		
	{
		printf("Error:Cannot open the file!\n");
		return -1;
	}
	while(1)
	{
		read(fd, &key_val, 1);
		printf("key_val = %d \n", key_val);
	}
	return 0;
}







