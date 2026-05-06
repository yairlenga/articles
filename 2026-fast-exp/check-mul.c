#include <stdlib.h>
#include <stdio.h>

double x[1024] ;
const int NX = sizeof(x) / sizeof(*x) ;

static double test_mul(int n)
{
	double s = 1.0 ;
	for (int i=0 ; i<n ; i++) {
		s = 1 ;
		for (int j=0 ; j<NX ; j++) s *= x[j] ;
	} ;
	return s ;
}

int main(int argc, char **argv)
{
	int n = argc > 1 ? atoi(argv[1]) : 1000000 ;
	for (int i=0 ; i<NX ; i++) x[i] = (i+1)*0.01 ;
	double s = test_mul(n) ;
	printf("N=%d, S=%.3f\n", n, s) ;
}
