#include <stdio.h>

void
hwclock(int year, int month, int day, int h, int m)
{
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

