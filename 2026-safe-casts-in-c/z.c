#include <string.h>
#include <stdio.h>

void main(int argc, char **argv)
{
    // validate at x has 3 or less characters than y
    if ( strlen(argv[1]) - strlen(argv[2]) <= 3 ) {
	printf("fail\n") ;
	return ;
    } ;
    printf("pass\n") ;
}
