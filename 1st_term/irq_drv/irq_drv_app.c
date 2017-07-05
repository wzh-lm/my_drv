#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc,char **argv)
{
	int fd;
	unsigned char key_val;
	
	fd = open("/dev/irq_drv",O_RDWR);
	if(fd < 0)
		printf("Cannot open the file!\n");
	while(1)
	{
		read(fd,&key_val,1);
		printf("key_val = %d \n",key_val);
	}
	return 0;
}
