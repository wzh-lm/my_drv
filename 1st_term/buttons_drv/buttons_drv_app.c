#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>

int fd;

void buttons_fasync(int signum)
{
	unsigned char key_val;
	read(fd, &key_val, 1);
	printf("USER:key_val = 0x%x\n", key_val);
}

int main(int argc, int **argv)
{
	int oflag;
	fd = open("/dev/buttons_drv",O_RDWR|O_NONBLOCK);
	if(fd < 0)
	{
		printf("Cannot open the file!\n");
		return -1;
	}
	signal(SIGIO, buttons_fasync);
	fcntl(fd, F_SETOWN, getpid());
	oflag = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, oflag|FASYNC);
	while(1)
	{
		sleep(1000);
	}
	return 0;
}

