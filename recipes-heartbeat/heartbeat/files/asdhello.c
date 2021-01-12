#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char* argv[])
{
	const char* dev_name = "/dev/asdkmod0";
	int fd = -1;
	char buf[64];
	
	printf("Hello world!! (asd)\n");
	
	if(argc > 1)
		printf("%s\n", argv[1]);
	
	fd = open(dev_name, O_RDWR);
	if(fd < 0)
	{
		printf("Error: cannot open %s, fd = %d\n", dev_name, fd);
		return -1;
	}
	read(fd, buf, 1);
	printf("read data: %d\n", (int) buf[0]);
	close(fd);
	
	return 0;
}
