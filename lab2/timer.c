#include <minix/syslib.h>
#include <minix/drivers.h>
#include "i8254.h"
#include "timer.h"
#include <math.h>

int hook_id;
unsigned int counter;

int timer_set_square(unsigned long timer, unsigned long freq) {
	int ret = 0;

	if (timer >= 3) // it is unsigned
		ret = 1;
	else
	{
		unsigned char mem_pos = timer + TIMER_0;
		unsigned long increment_value = TIMER_FREQ / freq;
		unsigned char lsb = increment_value & 0xFF, msb = (increment_value >> 8) & 0xFF;

		unsigned char command;
		command = TIMER_SEL0;
		command |= TIMER_LSB_MSB;
		command |= TIMER_SQR_WAVE;
		command |= TIMER_BIN;

		if (sys_outb(TIMER_CTRL, command) != OK || sys_outb(mem_pos, lsb) != OK || sys_outb(mem_pos, msb) != OK)
			ret = 1;
	}

	return ret;
}

int timer_subscribe_int(void) {
	int ret = 0;
	if (sys_irqsetpolicy(0, IRQ_REENABLE, &hook_id) != OK)
		ret = -1;
	if (ret == 0){
		if (sys_irqenable(&hook_id) != OK)
			ret = -1;
	}
	return ret;
}

int timer_unsubscribe_int() {
	int ret = 0;
	if (sys_irqrmpolicy(&hook_id) != OK)
		ret = -1;
	if (ret == 0){
		if (sys_irqdisable(&hook_id) != OK)
			ret = -1;
	}
	return ret;
}

void timer_int_handler() {
	counter++;
}

int timer_get_conf(unsigned long timer, unsigned char *st) {
	int ret = 0;

	if (timer >= 3)	// it is unsigned
		ret = 1;

	// indicate to the i8254 that we want to realize the read-back command
	if (ret == 0)
	{
		unsigned char command = TIMER_RB_CMD | TIMER_RB_COUNT_ | TIMER_RB_STATUS_ | TIMER_RB_SEL(timer);

		if ((((command) >> 6) & 3) == 3)	// verify if the two MSBs are 1
			ret = 0;
		else
			ret = 1;

		if (ret == 0){
			if (sys_outb(TIMER_CTRL, command) != OK)
				ret = 1;
		}
	}

	// receive from the i8254 the configuration
	if (ret == 0)
	{
		unsigned char timer_address = timer + TIMER_0;
		unsigned long config;
		if (sys_inb(timer_address, &config) != OK)
			ret = 1;
		*st = config;
	}

	return ret;
}

int timer_display_conf(unsigned char conf) {
	int i;
	unsigned int conf_binary[8];

	// show conf in binary
	printf("Timer's configuration: ");
	for (i = 7; i >= 0; i--){ // most significant bit to least significant bit
		int bit;
		if ((conf >> (i + 1)) == (conf / pow(2, i + 1)))
			bit = 0;
		else
			bit = 1;

		conf_binary[i] = bit;

		printf("%u", bit);
		if (i == 0)
			printf("\n");
	}

	// show the value of each part of the configuration
	printf("Counter: %u\n", ((conf >> 6) & 3));

	printf("Type of access: ");
	if (conf_binary[5] == 0 && conf_binary[4] == 1)
		printf("LSB\n");
	else
		if (conf_binary[5] == 1 && conf_binary[4] == 0)
			printf("MSB\n");
		else
			if (conf_binary[5] == 1 && conf_binary[4] == 1)
				printf("LSB followed by MSB\n");

	printf("Operating mode: ");
	if (((conf >> 1) & 7) <= 5)
		printf("%u\n", (conf >> 1) & 7);
	else
		if ((conf >> 1 & 7) == 6)
			printf("2\n");
		else
			if ((conf >> 1 & 7) == 7)
				printf("3\n");

	printf("Counting mode: ");
	if ((conf >> 1) == (conf / 2))
		printf("Binary\n"); // the bit was 0
	else
		printf("BCD\n"); // the bit was 1

	return 0;
}

int timer_test_square(unsigned long freq) {
	int ret = 0;

	if (timer_set_square(0, freq) != 0)
		ret = 1;

	return ret;
}

int timer_test_int(unsigned long time) {
	int ret = 0;

	int irq_set = BIT(timer_subscribe_int());
	int ipc_status;
	message msg;

	counter = 0;

	while (counter < time * 60) { /* You may want to use a different condition */
		/* Get a request message. */
		int r = driver_receive(ANY, &msg, &ipc_status);
		if (r != 0) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)){ /* received notification */
			switch (_ENDPOINT_P(msg.m_source)){
			case HARDWARE: /* hardware interrupt notification */
				if (msg.NOTIFY_ARG & irq_set){ /* subscribed interrupt */
					timer_int_handler();
					if (counter % 60 == 0)
						printf("Hello\n");
				}
				break;
			default:
				break; /* no other notifications expected: do nothing */
			}
		}
		else { /* received a standard message, not a notification */
			/* no standard messages expected: do nothing */
		}
	}

	if (timer_unsubscribe_int() != OK) // If an error has occurred before, it will show up now and return 1
		ret = 1;

	return ret;
}

int timer_test_config(unsigned long timer) {
	int ret = 0;

	unsigned char st;
	if (timer_get_conf(timer, &st) != 0) // "timer" validation (from 0 to 2) in timer_get_conf
		ret = 1;

	if (ret == 0)
		timer_display_conf(st);

	return ret;
}
