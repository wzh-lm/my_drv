#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <poll.h>
#include <signal.h>


int fd;

static void my_signal_fun(int signum)
{
	unsigned char key_val;
	read(fd, &key_val, 1);
	printf("key_val = %d\n", key_val);
}

int main(int argc, int **argv)
{
	int ret;
	int oflag;
	signal(SIGIO,my_signal_fun);
	
	fd = open("/dev/irq_drv",O_RDWR);	
	
	if(fd < 0)
		
		printf("Error:Cannot open the file!\n");

	fcntl(fd, F_SETOWN, getpid());

	oflag = fcntl(fd, F_GETFL);

	fcntl(fd, F_SETFL, oflag | FASYNC);
	
	while(1)
	{
		sleep(1000);
	}
	return 0;
}







