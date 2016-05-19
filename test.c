//GlobalCharTest.c
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#define DEV_NAME "/dev/ChangeLetter"
int main()
{
	int fd;
	char letter;
	char buf[30] = {0};

	fd = open(DEV_NAME, O_RDWR, S_IRUSR | S_IWUSR);
	if (fd < 0)
	{
		printf("Open Deivece Fail ! \n");
		return -1;
	}
	printf("Please input a small letter and I will give you the big letter : ");
	scanf("%s", buf);
	int len = strlen(buf);
	write(fd, buf, len);
	read(fd, buf, len);
	printf("The big letter  is %s \n", buf); 
	printf("Length is %d\n", len); 
	close(fd);
	return 0;
}
