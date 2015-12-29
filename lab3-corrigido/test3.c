#include <minix/syslib.h>
#include <minix/drivers.h>
#include "8042.h"
#include "timer.h"

#define ESCmc 0x01	// not used
#define ESCbc 0x81

void testscan();
extern unsigned char obf;
extern unsigned char err;

int kbd_subscribe(unsigned int *hook_id) {
	int ret = 0;

	if (sys_irqsetpolicy(KBD_IRQ_LINE, IRQ_REENABLE | IRQ_EXCLUSIVE, hook_id) != OK)
		ret = 1;

	if (ret == 0) {
		if (sys_irqenable(hook_id) != OK)
			ret = 1;
	}
	return ret;
}

int kbd_unsubscribe(unsigned int *hook_id) {
	int ret = 0;

	if (sys_irqdisable(hook_id) != OK)
		ret = 1;

	if (ret == 0) {
		if (sys_irqrmpolicy(hook_id) != OK)
			ret = 1;
	}

	return ret;
}

int verify_IBF_flag() {
	int ret = 1, limiter = 0;
	unsigned long status;

	while ((limiter < 10) && (ret == 1)) {
		sys_inb(STATUS_REG, &status); /* assuming it returns OK */
		/* loop while 8042 input buffer is not empty */
		if ((status & (PARITY_ERROR | TIMEOUT_ERROR | IBF)) != 0) {	// BIT(1) must not be set
			tickdelay(micros_to_ticks(DELAY_US));
			limiter++;
		}
		else
			ret = 0;
	}

	return ret;
}

unsigned long kbd_read()
{
	unsigned long ch, status;
	int ret = 1, limiter = 0;

	while ((limiter < 10) && (ret == 1)) {
		sys_inb(STATUS_REG, &status); /* assuming it returns OK */
		/* loop while 8042 output buffer is empty */
		if ((status & (PARITY_ERROR | TIMEOUT_ERROR | OBF)) != BIT(0)) {
			tickdelay(micros_to_ticks(DELAY_US));
			limiter++;
		}
		else {
			sys_inb(OUT_BUF, &ch);
			ret = 0;
		}
	}

	return ch;
}

void show_char(unsigned short ch)
{
	if (ch != 0xE0)	// two char scancode beginning
	{
		if ((ch & BIT(7)) == 0)
			printf("Makecode: %x\n", ch);
		else
			printf("Breakcode: %x\n", ch);
	}
}

int kbd_test_scan(unsigned short ass) {
	unsigned long ch = 0;
	unsigned int hook_id = 2;
	int irq_set = BIT(hook_id);
	int ipc_status;
	unsigned long status;
	message msg;
	int ret = 0;

	if (kbd_subscribe(&hook_id) != 0)
		ret = 1;

	while (ch != ESCbc) { /* You may want to use a different condition */
		/* Get a request message. */
		int r = driver_receive(ANY, &msg, &ipc_status);
		if (r != 0) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { /* received notification */
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE: /* hardware interrupt notification */
				if (msg.NOTIFY_ARG & irq_set) { /* subscribed interrupt */
					switch (ass) {
					case 0:
						if (ch == 0xE0)	// two char scancode beginning
							ch = (ch << 8) | kbd_read();
						else
							ch = kbd_read();
						show_char(ch);
						break;
					case 1:
						testscan();
						if (ch == 0xE0)	// two char scancode beginning
							ch = (ch << 8) | obf;
						else
							ch = obf;
						show_char(ch);
						if (err != 0)
							ret = 1;
						break;
					default:
						break;
					}
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

	if (kbd_unsubscribe(&hook_id) != 0)
		ret = 1;

	return ret;
}

int kbd_test_leds(unsigned short n, unsigned short *toggle) {
	int states[3] = { 0, 0, 0 }; // 0-scroll lock, 1-number lock, 2-caps lock (all considered off at the beginning)
	unsigned long kbc_return;
	int ret = 0;

	if (timer_subscribe_int() != 0)
		ret = 1;
	if (timer_test_square(60) != 0)
		ret = 1;

	if (ret != 0)
		printf("Problem assigning Timer 0 interruptions.\n(Preventing the risk of the interruptions become locked.)\n");
	else {
		int i;
		for (i = 0; i < n; i++) {
			timer_test_int(1);

			if (verify_IBF_flag() != 0)
				ret = 1;
			else {
				sys_outb(IN_BUF, SET_INDICATORS); // indicate that we want to set the leds
				sys_inb(OUT_BUF, &kbc_return); // receive information from the kbc to verify if it is possible to change leds

				if (kbc_return == RESEND_CODE) // resend
				{
					i--;
					printf("Resending request.\n");
				}
				else if (kbc_return == ERROR_CODE)	// error
				{
					i = 0;
					printf("Error detected. Reinitiating sequence.\n");
				}
				else {
					// change leds (consider that ACK was sent)
					states[toggle[i]] = (states[toggle[i]] + 1) % 2;
					unsigned long command = states[0] | (states[1] << 1) | (states[2] << 2);

					if (verify_IBF_flag() != 0)
						ret = 1;
					else {
						sys_outb(IN_BUF, command);// send command to change leds
						sys_inb(OUT_BUF, &kbc_return);// receive information from the kbc to verify if it was possible to change leds

						if (kbc_return == RESEND_CODE) // resend
						{
							i--;
							printf("Resending request.\n");
						}
						else if (kbc_return == ERROR_CODE)	// error
						{
							i = 0;
							printf("Error detected. Reinitiating sequence.\n");
						}
						else {
							// consider that ACK was sent
							if (toggle[i] == 0) {
								if (states[toggle[i]] == 0)
									printf("Scroll Lock Off\n");
								else
									printf("Scroll Lock On\n");
							}
							else if (toggle[i] == 1) {
								if (states[toggle[i]] == 0)
									printf("Number Lock Off\n");
								else
									printf("Number Lock On\n");
							}
							else if (toggle[i] == 2) {
								if (states[toggle[i]] == 0)
									printf("Caps Lock Off\n");
								else
									printf("Caps Lock On\n");
							}
						}
					}
				}
			}
		}
	}

	if (timer_unsubscribe_int() != 0)
		ret = 1;

	return ret;
}


int kbd_test_timed_scan(unsigned short n) {
	unsigned long ch = 0, status;
	unsigned int kbd_hook_id = 2;
	unsigned int timer_hook_id = 3;
	int kbd_irq_set = BIT(kbd_hook_id);
	int timer_irq_set = BIT(timer_hook_id);
	int ipc_status;
	message msg;
	int ret = 0;

	int counter = 0;

	if (kbd_subscribe(&kbd_hook_id) != 0)
		ret = 1;

	set_timer_hook_id(timer_hook_id);
	if (timer_subscribe_int() != 0)
		ret = 1;

	if (ret == 0)
		while (ch != ESCbc && counter < n * 60) { /* You may want to use a different condition */
			/* Get a request message. */
			int r = driver_receive(ANY, &msg, &ipc_status);
			if (r != 0) {
				printf("driver_receive failed with: %d", r);
				continue;
			}
			if (is_ipc_notify(ipc_status)) { /* received notification */
				switch (_ENDPOINT_P(msg.m_source)) {
				case HARDWARE: /* hardware interrupt notification */
					if (msg.NOTIFY_ARG & kbd_irq_set) { /* subscribed interrupt */
						if (ch == 0xE0)	// two char scancode beginning
							ch = (ch << 8) | kbd_read();
						else
							ch = kbd_read();
						show_char(ch);
						counter = 0;
					}
					if (msg.NOTIFY_ARG & timer_irq_set)
					{
						counter++;
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

	if (timer_unsubscribe_int() != 0)
		ret = 1;

	if (kbd_unsubscribe(&kbd_hook_id) != 0)
		ret = 1;

	if (counter == n * 60)
		printf("The time is over.\n");
	else
		if (ch == ESCbc)
			printf("ESC was press.\n");

	return ret;
}
