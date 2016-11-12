#include "mylib.h"
static ssize_t buffer_read(int fd, char *ptr)
{
	if (read_cnt <= 0)
	{
		again:
			if ((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0)
			{
				if (errno == EINTR)
					goto again;
				return(-1);
			} else if (read_cnt == 0)
				return(0);
			read_ptr = read_buf;
	}
	read_cnt--;
	*ptr = *read_ptr++;
	return(1);
}

ssize_t readline(int fd, void *vptr, ssize_t maxlen)
{
	ssize_t n, rc;
	char c, *ptr;

	ptr = vptr;
	for (n = 1; n < maxlen; n++)
	{
		if ((rc = buffer_read(fd, &c)) == 1)
		{
			*ptr++ = c;
			if (c == '\n')
				break;
		}
		else if (rc == 0)
		{
			*ptr = 0;
			return(n - 1);
		} else
			return(-1);
	}
	*ptr = 0;
	return(n);
}

unsigned long hex2dec(const char* str)
{
	return strtol(str, NULL, 16);
}

unsigned long bit2dec(const char* str)
{
	return strtol(str, NULL, 2);
}

unsigned long oct2dec(const char* str)
{
	return strtol(str, NULL, 8);
}