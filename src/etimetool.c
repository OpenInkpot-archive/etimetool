#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>

void
hwclock(int year, int month, int day, int h, int m)
{
    char *s;
	asprintf(&s, "date %02d%02d%02d%02d20%02d; hwclock --systohc",
            month, day, h, m, year);
	if(s)
    {
    	system(s);
	    free(s);
	}
}


int main(int argc, char **argv)
{
	return 0;
}

