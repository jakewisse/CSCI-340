/* Jake Wisse
 * CSCI 340 - Operating Systems
 * 16 Jan 2013
 *
 *   fact.c
 */

#include <stdio.h>
#include <stdlib.h>

long int fact(long int num);

int main(int argc, char **argv) {

	char *endptr;
	long int n;

	n = strtol(argv[1], &endptr, 10);

	// After calling strtol(), if endptr is equal to argv[1], no arguments have
	// been entered.

	if (endptr == argv[1]) {
		printf("Huh?\n");
		return 1;
	}

	// If endptr is not equal to argv[1], but is not '\0', then the input began
	// with digits but was not a fully valid integer.

	if (*endptr != '\0') {
		printf("Huh?\n");
		return 2;
	}
	
	if (n <= 0) {
		printf("Huh?\n");
		return 3;
	}

	if (n > 12) {
		printf("Overflow.\n");
		return 4;
	}

	n = fact(n);
	printf("%li\n", n);	

	return 0;
}

long int fact(long int num) {
	if (num > 0) {
		 return (num * fact(num - 1));
	}
	else return 1;

}
