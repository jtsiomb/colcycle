#include <stdio.h>
#include <dos.h>

static int quit;

int main(int argc, char **argv)
{
	printf("hello!\n");
	return 0;
}

void app_quit(void)
{
	quit = 1;
}

void set_palentry(int idx, unsigned char r, unsigned char g, unsigned char b)
{
}
