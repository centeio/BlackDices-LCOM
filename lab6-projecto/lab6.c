#include "test6.h"

static int proc_args(int argc, char *argv[]);
static void print_usage(char *argv[]);

int main(int argc, char **argv) {

	/* Initialize service */
	sef_startup();

	/* Enable IO-sensitive operations for ourselves */
	sys_enable_iop(SELF);

	printf("Lab 6: The PC's Real Time Clock\n");

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
			"\t service run %s -args \"rtc_test_conf\" \n"
			"\t service run %s -args \"rtc_test_date\" \n"
			"\t service run %s -args \"rtc_test_int\" \n",
			argv[0], argv[0], argv[0]);
}

static int proc_args(int argc, char *argv[]) {
	unsigned short counter, idle_time, length, tolerance;

	if (strncmp(argv[1], "rtc_test_conf", strlen("rtc_test_conf"))== 0) {
		if (argc != 2) {
			printf("wrong no of arguments for test of rtc_test_conf() \n");
			return 1;
		}
		printf("rtc_test_conf()\n");
		return rtc_test_conf();
	}
	else if (strncmp(argv[1], "rtc_test_date", strlen("rtc_test_date")) == 0) {
		if (argc != 2) {
			printf("wrong no of arguments for test of rtc_test_date() \n");
			return 1;
		}
		printf("rtc_test_date()\n");
		return rtc_test_date();
	}
	else if (strncmp(argv[1], "rtc_test_int",strlen("rtc_test_int")) == 0) {
		if (argc != 2) {
			printf("wrong no of arguments for test of rtc_test_int() \n");
			return 1;
		}
		printf("rtc_test_int()\n");
		return rtc_test_int();
	}
	else {
		printf("non valid function \"%s\" to test\n", argv[1]);
		return 1;
	}
}
