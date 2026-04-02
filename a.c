#include <stdio.h>

void foo(int x) { return ; }

int main(void)
{
	printf("S=%ld\n", sizeof foo(1)) ;
}
