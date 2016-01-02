#include <minix/drivers.h>
#include "test4.h"

static int proc_args(int argc, char *argv[]);
static unsigned long parse_ulong(char *str, int base);
static long parse_long(char *str, int base);
static void print_usage(char *argv[]);

int main(int argc, char **argv) {

	/* Initialize service */
	sef_startup();

	printf("lab4: The PC's Mouse\n");

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
			"\t service run %s -args \"test_packet <counter>\" \n"
			"\t service run %s -args \"test_async <idle_time>\" \n"
			"\t service run %s -args \"test_config\" \n"
			"\t service run %s -args \"test_gesture <length> <tolerance>\" \n",
			argv[0], argv[0], argv[0], argv[0]);
}

static int proc_args(int argc, char *argv[]) {
	unsigned short counter, idle_time, length, tolerance;

	if (strncmp(argv[1], "test_packet", strlen("test_packet"))== 0) {
		if (argc != 3) {
			printf("wrong no of arguments for test of test_packet() \n");
			return 1;
		}
		if ((counter = parse_ulong(argv[2], 10)) == ULONG_MAX)
			return 1;
		printf("test_packet(%u)\n", counter);
		return test_packet(counter);
	}
	else if (strncmp(argv[1], "test_async", strlen("test_async")) == 0) {
		if (argc != 3) {
			printf("wrong no of arguments for test of test_async() \n");
			return 1;
		}
		if ((idle_time = parse_ulong(argv[2], 10)) == ULONG_MAX)
			return 1;
		printf("test_async(%u)\n", idle_time);
		return test_async(idle_time);
	}
	else if (strncmp(argv[1], "test_config",strlen("test_config")) == 0) {
		if (argc != 2) {
			printf(
					"wrong no of arguments for test of test_config() \n");
			return 1;
		}
		printf("test_config()\n");
		return test_config();
	}
	else if (strncmp(argv[1], "test_gesture", strlen("test_gesture")) == 0) {
		if (argc != 4) {
			printf("wrong no of arguments for test of test_gesture() \n");
			return 1;
		}
		if ((length = parse_ulong(argv[2], 10)) == ULONG_MAX)
			return 1;
		if ((tolerance = parse_ulong(argv[3], 10)) == ULONG_MAX)
			return 1;
		printf("test_gesture(%u, %u)\n", length, tolerance);
		return test_gesture(length, tolerance);
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
		printf("mouse: parse_ulong: no digits were found in %s \n", str);
		return ULONG_MAX;
	}

	/* Successful conversion */
	return val;
}
