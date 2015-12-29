#include <minix/drivers.h>
#include "test3.h"

static int proc_args(int argc, char *argv[]);
static unsigned long parse_ulong(char *str, int base);
static long parse_long(char *str, int base);
static void print_usage(char *argv[]);

int main(int argc, char **argv) {

	/* Initialize service */
	sef_startup();

	/* Enable IO-sensitive operations for ourselves */
	sys_enable_iop(SELF);

	printf("lab3: The PC's Keyboard\n");

	if (argc == 1) {
		print_usage(argv);
		return 0;
	}
	else {
		proc_args(argc, argv);
	}

	return 0;

}

static void print_usage(char *argv[]) {
	printf("Usage: one of the following:\n"
		"\t service run %s -args \"kbd_test_scan <asm>\" \n"
		"\t service run %s -args \"kbd_test_leds <n> <*toggle>\" \n"
		"\t service run %s -args \"kbd_test_timed_scan <n>\" \n",
		argv[0], argv[0], argv[0]);
}

static int proc_args(int argc, char *argv[]) {
	unsigned short assm, n, toggle;

	if (strncmp(argv[1], "kbd_test_scan", strlen("kbd_test_scan"))
		== 0) {
		if (argc != 3) {
			printf("wrong no of arguments for test of kbd_test_scan() \n");
			return 1;
		}
		if ((assm = parse_ulong(argv[2], 10)) == ULONG_MAX)
			return 1;
		printf("kbd_test_scan(%u)\n", assm);
		return kbd_test_scan(assm);
	}
	else if (strncmp(argv[1], "kbd_test_leds", strlen("kbd_test_leds")) == 0) {
		if (argc != 4) {
			printf("wrong no of arguments for test of kbd_test_leds() \n");
			return 1;
		}
		if ((n = parse_ulong(argv[2], 10)) == ULONG_MAX)
			return 1;
		printf("kbd_test_leds(%u, ", n);

		unsigned short leds[n];
		unsigned int i;
		for (i = 0; i < n; i++)
		{
			if ((toggle = argv[3][i] - '0') > 2 || (toggle < 0))
			{
				printf("%u)\nwrong led number\n", toggle);
				return 1;
			}
			leds[i] = toggle;
			printf("%u", leds[i]);
		}

		printf(")\n");
		return kbd_test_leds(n, leds);
	}
	else if (strncmp(argv[1], "kbd_test_timed_scan",
		strlen("kbd_test_timed_scan")) == 0) {
		if (argc != 3) {
			printf(
				"wrong no of arguments for test of kbd_test_timed_scan() \n");
			return 1;
		}
		if ((n = parse_ulong(argv[2], 10)) == ULONG_MAX)
			return 1;
		printf("kbd_test_timed_scan(%u)\n", n);
		return kbd_test_timed_scan(n);
	}
	else {
		printf("non valid function \"%s\" to test\n", argv[1]);
		return 1;
	}
}


static unsigned long parse_ulong(char *str, int base) {
	char *endptr;
	unsigned long val;

	val = strtoul(str, &endptr, base);

	if ((errno == ERANGE && val == ULONG_MAX)
		|| (errno != 0 && val == 0)) {
		perror("strtol");
		return ULONG_MAX;
	}

	if (endptr == str) {
		printf("kbd: parse_ulong: no digits were found in %s \n", str);
		return ULONG_MAX;
	}

	/* Successful conversion */
	return val;
}

static long parse_long(char *str, int base) {
	char *endptr;
	unsigned long val;

	val = strtol(str, &endptr, base);

	if ((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))
		|| (errno != 0 && val == 0)) {
		perror("strtol");
		return LONG_MAX;
	}

	if (endptr == str) {
		printf("kbd: parse_long: no digits were found in %s \n", str);
		return LONG_MAX;
	}

	/* Successful conversion */
	return val;
}
