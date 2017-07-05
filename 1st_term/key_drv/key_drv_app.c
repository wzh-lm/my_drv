#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main(int argc,char **argv)
{
	int fd;
	unsigned char *key_val;
	unsigned int count = 0 ;
	static unsigned int stat = 0;
	fd = open("/dev/Buttons",O_RDWR);
	if(fd < 0)
	{
		printf("Can't open the file!");
	}
	while(1)
	{
		read(fd,key_val,sizeof(key_val));
		if((key_val[0] == 0 || key_val[1] == 0 || key_val[2] == 0 || key_val[3] == 0)&&(stat == 0))
		{
			printf("test-num-%d : %d %d %d %d\n",count++,key_val[0],key_val[1],key_val[2],key_val[3]);
			stat = 1;
		}
		else if((key_val[0] == 1 && key_val[1] == 1 && key_val[2] == 1 && key_val[3] == 1)&&(stat = 1))
		{
			stat = 0;
		}
	}
	return 0;
}
