#include <minix/drivers.h>
#include "test7.h"

static int proc_args(int argc, char *argv[]);
static unsigned long parse_ulong(char *str, int base);
static long parse_long(char *str, int base);
static void print_usage(char *argv[]);

int main(int argc, char **argv) {

	/* Initialize service */
	sef_startup();

	/* Enable IO-sensitive operations for ourselves */
	//sys_enable_iop(SELF);

	printf("Lab 7: The PC's Serial Port\n");

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
			"\t service run %s -args \"ser_test_conf\n <base_addr>\" \n"
			"\t service run %s -args \"ser_test_set\n <base_addr> <bits> <stop> <parity> <rate>\" \n"
			"\t service run %s -args \"ser_test_poll\n <base_addr> <tx> <bits> <stop> <parity> <rate> <stringc> <*strings[]>\" \n"
			"\t service run %s -args \"ser_test_int\" \n"
			"\t service run %s -args \"ser_test_fifo\" \n",
			argv[0], argv[0], argv[0], argv[0], argv[0]);
}

static int proc_args(int argc, char *argv[]) {
	unsigned short base_addr;
	unsigned long bits, stop, rate;
	long parity;
	unsigned char tx;
	int stringc;
	char *strings;

	if (strncmp(argv[1], "ser_test_conf", strlen("ser_test_conf")) == 0) {
		if (argc != 3) {
			printf("wrong no of arguments for test of ser_test_conf() \n");
			return 1;
		}
		if ((base_addr = parse_ulong(argv[2], 16)) == ULONG_MAX)
			return 1;
		printf("ser_test_conf(%x)\n", base_addr);
		return ser_test_conf(base_addr);
	}
	else if (strncmp(argv[1], "ser_test_set", strlen("ser_test_set")) == 0) {
		if (argc != 7) {
			printf("wrong no of arguments for test of ser_test_set() \n");
			return 1;
		}
		if ((base_addr = parse_ulong(argv[2], 16)) == ULONG_MAX)
			return 1;
		if ((bits = parse_ulong(argv[3], 10)) == ULONG_MAX)
			return 1;
		if ((stop = parse_ulong(argv[4], 10)) == ULONG_MAX)
			return 1;
		if ((parity = parse_long(argv[5], 10)) == LONG_MAX)
			return 1;
		if ((rate = parse_ulong(argv[6], 10)) == ULONG_MAX)
			return 1;
		printf("ser_test_set(%u, %lu, %lu, %ld, %lu)\n", base_addr, bits, stop, parity, rate);
		return ser_test_set(base_addr, bits, stop, parity, rate);
	}
	else if (strncmp(argv[1], "ser_test_poll", strlen("ser_test_poll")) == 0) {
		if (argc != 10) {
			printf("wrong no of arguments for test of ser_test_poll() \n");
			return 1;
		}
		if ((base_addr = parse_ulong(argv[2], 16)) == ULONG_MAX)
			return 1;
		if ((tx = parse_ulong(argv[3], 10)) == ULONG_MAX)
			return 1;
		if ((bits = parse_ulong(argv[4], 10)) == ULONG_MAX)
			return 1;
		if ((stop = parse_ulong(argv[5], 10)) == ULONG_MAX)
			return 1;
		if ((parity = parse_long(argv[6], 10)) == LONG_MAX)
			return 1;
		if ((rate = parse_ulong(argv[7], 10)) == ULONG_MAX)
			return 1;
		if ((stringc = parse_long(argv[8], 10)) == ULONG_MAX)
			return 1;
		strings=argv[9];
		printf("ser_test_poll(%X, %u, %u, %lu, %lu, %lu, %d, %s)\n", base_addr, tx, bits, stop, parity, rate, stringc, strings);
		return ser_test_poll(base_addr, tx, bits, stop, parity, rate, stringc, &strings);
	}
	else if (strncmp(argv[1], "ser_test_int", strlen("ser_test_int")) == 0) {
		if (argc != 2) {
			printf("wrong no of arguments for test of ser_test_int() \n");
			return 1;
		}
		printf("ser_test_int()\n");
		return ser_test_int();
	}
	else if (strncmp(argv[1], "ser_test_fifo", strlen("ser_test_fifo")) == 0) {
		if (argc != 2) {
			printf("wrong no of arguments for test of ser_test_fifo() \n");
			return 1;
		}
		printf("ser_test_fifo()\n");
		return ser_test_fifo();
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
