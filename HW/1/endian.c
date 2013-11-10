#include <stdio.h>

typedef unsigned char *pointer;

void show_bytes(pointer start, int len)
{
	int i;
	for (i = 0 ; i < len ; i++)
		printf("0x%p\t0x%.2x\n", start+i, start[i]);
	printf("\n");
}

int main(void)
{
	int a = 15213;
	printf("int a = %d\t0x%x\n", a, a);
	show_bytes((pointer) &a, sizeof(int));
	return 0;
}