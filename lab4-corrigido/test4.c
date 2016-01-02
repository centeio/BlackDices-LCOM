#include <minix/syslib.h>
#include <minix/drivers.h>
#include "8042.h"
#include "timer.h"

int write_to_kbc(unsigned char port, unsigned char command)
{
	unsigned long stat;
	int ret = 1, limiter = 0;

	while ((limiter < 10) && (ret == 1)) {
		if (sys_inb(STAT_REG, &stat) == OK){
			if ((ret = (stat & IBF)) == 0){
				if (sys_outb(port, command) != OK)
					ret = 1;
			}
		}
		if (ret == 1){
			tickdelay(micros_to_ticks(DELAY_US));
			limiter++;
		}
	}
	return ret;
}

int write_to_mouse(unsigned char port, unsigned char command){
	unsigned long stat;
	int ret = 1, limiter = 0;

	while ((limiter < 10) && (ret == 1)) {
		if (sys_inb(STAT_REG, &stat) == OK){
			if ((ret = (stat & IBF)) == 0){
				if (sys_outb(port, command) != OK)
					ret = 1;
				else{
					if (sys_inb(OUT_BUF, &stat) == OK)
						if (stat != ACK)
							ret = 1;
				}
			}
		}
		if (ret == 1){
			tickdelay(micros_to_ticks(DELAY_US));
			limiter++;
		}
	}
	return ret;
}

int mouse_subscribe_int(unsigned int *hook_id) {
	int ret = 0;

	if (sys_irqsetpolicy(MOUSE_IRQ_LINE, IRQ_REENABLE | IRQ_EXCLUSIVE, hook_id) != OK)
		ret = 1;

	if (sys_irqenable(hook_id) != OK)
		ret = 1;

	return ret;
}

int mouse_unsubscribe_int(unsigned int *hook_id) {
	int ret = 0;

	if (sys_irqdisable(hook_id) != OK)
		ret = 1;

	if (sys_irqrmpolicy(hook_id) != OK)
		ret = 1;

	return ret;
}

int en_mouse()
{
	int ret = 0;

	if (write_to_kbc(KBC_CMD_REG, WRITE_BYTE_TO_MOUSE) != 0)
		ret = 1;

	if (write_to_mouse(IN_BUF, ENABLE_MOUSE) != 0)
		ret = 1;

	return ret;
}

int dis_mouse()
{
	int ret = 0;

	if (write_to_kbc(KBC_CMD_REG, WRITE_BYTE_TO_MOUSE) != 0)
		ret = 1;

	if (write_to_mouse(IN_BUF, DISABLE_MOUSE) != 0)
		ret = 1;

	return ret;
}

int read_from_mouse(unsigned long *packetbyte){
	unsigned long stat;
	int ret = 1, limiter = 0;

	while ((limiter < 10) && (ret == 1)) {
		if (sys_inb(STAT_REG, &stat) == OK)
			if ((stat & OBF) == BIT(0)){
				ret = 0;
				if (sys_inb(OUT_BUF, packetbyte) != OK)
					ret = 1;
			}

		if (ret == 1){
			tickdelay(micros_to_ticks(DELAY_US));
			limiter++;
		}
	}

	return ret;
}

int get_packetbyte(unsigned long *packetbyte, unsigned short position) {
	int ret = 0;

	ret = read_from_mouse(&packetbyte[position]);

	if (position == 0)
		if (packetbyte[position] & BIT(3) != BIT(3))	// bit 3 != 1
			ret = 1;

	return ret;
}

int showconf(unsigned long *p) {

	printf("B1=0x%x ", p[0]);
	printf("B2=0x%x ", (char)p[1] & 0xFFF);
	printf("B3=0x%x ", (char)p[2] & 0xFFF);

	printf("LB=%x ", (p[0] & 1));
	printf("RB=%x ", ((p[0] >> 1) & 1));
	printf("MB=%x ", ((p[0] >> 2) & 1));

	printf("XOV=%x ", ((p[0] >> 6) & 1));
	printf("YOV=%x ", ((p[0] >> 7) & 1));
	printf("X=0x%x ", (char)p[1] & 0xFFF);
	printf("Y=0x%x\n", (char)p[2] & 0xFFF);
}

int test_packet(unsigned short cnt) {
	unsigned long packet[3];
	unsigned short counter = 0, position = 0;
	unsigned int hook_id = 5;
	int irq_set = BIT(hook_id);
	int ipc_status;
	message msg;
	int ret = 0;

	if (mouse_subscribe_int(&hook_id) != 0)
		ret = 1;

	if (en_mouse() != 0)
		ret = 1;

	while (counter < cnt) { /* You may want to use a different condition */
		/* Get a request message. */
		int r = driver_receive(ANY, &msg, &ipc_status);
		if (r != 0) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { /* received notification */
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE: /* hardware interrupt notification */
				if (msg.NOTIFY_ARG & irq_set) {/* subscribed interrupt */
					if (get_packetbyte(&packet[0], position) != 1) {
						if (position == 2) {
							showconf(packet);
							counter++;
						}
						position = (position + 1) % 3;
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

	if (dis_mouse() != 0)
		ret = 1;

	if (mouse_unsubscribe_int(&hook_id) != 0)
		ret = 1;

	unsigned long garbage;
	sys_inb(OUT_BUF, &garbage);

	return ret;
}

int test_async(unsigned short idle_time) {
	unsigned long packet[3];
	unsigned short counter = 0, position = 0;
	unsigned int mouse_hook_id = 5, timer_hook_id = 6;
	int mouse_irq_set = BIT(mouse_hook_id), timer_irq_set = BIT(timer_hook_id);
	int ipc_status;
	message msg;
	int ret = 0;

	if (mouse_subscribe_int(&mouse_hook_id) != 0)
		ret = 1;

	if (en_mouse() != 0)
		ret = 1;

	if (timer_subscribe_int(&timer_hook_id) != 0)
		ret = 1;

	while (counter < 60 * idle_time) { /* You may want to use a different condition */
		/* Get a request message. */
		int r = driver_receive(ANY, &msg, &ipc_status);
		if (r != 0) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { /* received notification */
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE: /* hardware interrupt notification */
				if (msg.NOTIFY_ARG & mouse_irq_set) {/* subscribed interrupt */
					if (get_packetbyte(&packet[0], position) != 1) {
						if (position == 2) {
							showconf(packet);
							counter = 0;
						}
						position = (position + 1) % 3;
					}
				}
				if (msg.NOTIFY_ARG & timer_irq_set)
					counter++;
				break;
			default:
				break; /* no other notifications expected: do nothing */
			}
		}
		else { /* received a standard message, not a notification */
			/* no standard messages expected: do nothing */
		}
	}

	if (timer_unsubscribe_int(&timer_hook_id) != 0)
		ret = 1;

	if (dis_mouse() != 0)
		ret = 1;

	if (mouse_unsubscribe_int(&mouse_hook_id) != 0)
		ret = 1;

	unsigned long garbage;
	sys_inb(OUT_BUF, &garbage);

	return ret;
}

int get_statuspacketbyte(unsigned long *packetbyte, unsigned short position) {
	int ret = 0;

	ret = read_from_mouse(&packetbyte[position]);

	if (position == 0){
		// bit 7 != 0 or bit 3 != 0
		if (((packetbyte[position] & BIT(7)) != 0) || ((packetbyte[position] & BIT(3)) != 0))
			ret = 1;}
	else
		if (position == 1)
			// if packetbyte != 000000xxb
			if ((packetbyte[position] >> 2) != 0)
				ret = 1;

	return ret;
}

int showfriendlyconf(unsigned long *p){
	if (((p[0] >> 6) & 1) == 1){
		printf("Remote (polled) mode.\n");
	}
	else{
		printf("Stream mode.\n");

		if (((p[0] >> 5) & 1) == 1)
			printf("Data reporting enabled.\n");
		else
			printf("Data reporting disabled.\n");
	}
	if (((p[0] >> 4) & 1) == 1)
		printf("Scaling is 2:1\n");
	else
		printf("Scaling is 1:1\n");

	if ((p[0] & 1) == 1)
		printf("Left button pressed.\n");
	else
		printf("Left button not pressed.\n");

	if (((p[0] >> 1) & 1) == 1)
		printf("Right button pressed.\n");
	else
		printf("Right button not pressed.\n");
	if (((p[0] >> 2) & 1) == 1)
		printf("Middle button pressed.\n");
	else
		printf("Middle button not pressed.\n");

	printf("Resolution: %d\n", (p[1] >> 1 & 3));
	printf("Sample rate: %d\n", p[2]);

	return 0;
}

int test_config(void) {
	unsigned long packet[3];
	unsigned short counter = 0, position = 0;
	unsigned int hook_id = 5;
	int irq_set = BIT(hook_id);
	int ipc_status;
	message msg;
	int ret = 0;

	if (mouse_subscribe_int(&hook_id) != 0)
		ret = 1;

	if (dis_mouse() != 0)
		ret = 1;

	if (write_to_kbc(KBC_CMD_REG, WRITE_BYTE_TO_MOUSE) != 0)
		ret = 1;

	if (write_to_mouse(IN_BUF, STATUS_REQUEST) != 0)
		ret = 1;

	while (counter < 3) { /* You may want to use a different condition */
		/* Get a request message. */
		int r = driver_receive(ANY, &msg, &ipc_status);
		if (r != 0) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { /* received notification */
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE: /* hardware interrupt notification */
				if (msg.NOTIFY_ARG & irq_set) {/* subscribed interrupt */
					if (get_statuspacketbyte(&packet[0], position) != 1) {
						if (position == 2)
							showfriendlyconf(packet);
						position = (position + 1) % 3;
						counter++;
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

	if (mouse_unsubscribe_int(&hook_id) != 0)
		ret = 1;

	unsigned long garbage;
	sys_inb(OUT_BUF, &garbage);

	return ret;
}

int test_gesture(short length, unsigned short tolerance) {
	unsigned long packet[3], horizontal = 0, vertical = 0;
	unsigned short position = 0, RIGHT_BUTTON = 0;
	unsigned int hook_id = 5;
	int irq_set = BIT(hook_id);
	int ipc_status;
	message msg;
	int ret = 0;

	if (mouse_subscribe_int(&hook_id) != 0)
		ret = 1;

	if (en_mouse() != 0)
		ret = 1;

	// !(vertical >= length && RIGHT_BUTTON != 0)
	while (vertical < length || RIGHT_BUTTON == 0) { /* You may want to use a different condition */
		/* Get a request message. */
		int r = driver_receive(ANY, &msg, &ipc_status);
		if (r != 0) {
			printf("driver_receive failed with: %d", r);
			continue;
		}
		if (is_ipc_notify(ipc_status)) { /* received notification */
			switch (_ENDPOINT_P(msg.m_source)) {
			case HARDWARE: /* hardware interrupt notification */
				if (msg.NOTIFY_ARG & irq_set) {/* subscribed interrupt */
					if (get_packetbyte(&packet[0], position) != 1) {
						if (position == 2) {
							showconf(packet);
							if (fabs((char)packet[1]) < tolerance){
								horizontal = fabs((char)packet[1]);
								vertical = fabs((char)packet[2]);
								RIGHT_BUTTON = packet[0] & BIT(1);	// RIGHT_BUTTON = 2 if pressed
							}
							else {
								horizontal = 0;
								vertical = 0;
								RIGHT_BUTTON = 0;
							}
						}
						position = (position + 1) % 3;
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

	if (dis_mouse() != 0)
		ret = 1;

	if (mouse_unsubscribe_int(&hook_id) != 0)
		ret = 1;

	unsigned long garbage;
	sys_inb(OUT_BUF, &garbage);

	return ret;
}
