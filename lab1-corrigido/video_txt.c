#include <minix/drivers.h>
#include <sys/video.h>
#include <sys/mman.h>

#include <assert.h>

#include "vt_info.h"

#include "video_txt.h"

/* Private global variables */

static char *video_mem;		/* Address to which VRAM is mapped */

static unsigned scr_width;	/* Width of screen in columns */
static unsigned scr_lines;	/* Height of screen in lines */

void vt_fill(char ch, char attr) {

	unsigned int i, j;
	char *it = video_mem;
	for (i = 0; i < scr_lines; i++) {
		for (j = 0; j < scr_width; j++) {
			*it = ch;
			*(it + 1) = attr;
			it += 2;
		}
	}
}

void vt_blank() {

	vt_fill(0, 0);
}

int vt_print_char(char ch, char attr, int r, int c) {	// ecrã começa na linha 1 e coluna 1
	char *it = video_mem;
	it = it + (scr_width*(r - 1) + c - 1) * 2;
	*it = ch;
	*(it + 1) = attr;
	return 0;
}

int vt_print_string(char *str, char attr, int r, int c) {
	unsigned int i;
	do {
		vt_print_char(*str, attr, r, c);
		str++;
		c++;
		if (c == scr_width + 1)
		{
			r++;
			c = 1;
		}
	} while (*str != 0);
	return 0;
}

int vt_print_int(int num, char attr, int r, int c) {

	int nChars = 0, number[10];	// valor máximo para int tem 10 algarismos na arquitectura x86

	do
	{
		number[nChars] = num % 10;
		nChars++;
		num = num / 10;
	} while (num != 0);

	do
	{
		vt_print_char(number[nChars - 1] + 48, attr, r, c);
		nChars--;
		c++;
		if (c == scr_width + 1)
		{
			r++;
			c = 1;
		}
	} while (nChars > 0);

	return 0;
}

int vt_draw_frame(int width, int height, char attr, int r, int c) {

	//attr = 0x0F;	// recomendado descomentar
	unsigned int i;
	vt_print_char(UL_CORNER, attr, r, c);
	vt_print_char(LL_CORNER, attr, r + height - 1, c);
	vt_print_char(UR_CORNER, attr, r, c + width - 1);
	vt_print_char(LR_CORNER, attr, r + height - 1, c + width - 1);
	for (i = c + 1; i < c + width - 1; i++) {
		vt_print_char(HOR_BAR, attr, r, i);
	}
	for (i = c + 1; i < c + width - 1; i++) {
		vt_print_char(HOR_BAR, attr, r + height - 1, i);
	}
	for (i = r + 1; i < r + height - 1; i++) {
		vt_print_char(VERT_BAR, attr, i, c);
	}
	for (i = r + 1; i < r + height - 1; i++) {
		vt_print_char(VERT_BAR, attr, i, c + width - 1);
	}

	return 0;
}

/*
* THIS FUNCTION IS FINALIZED, do NOT touch it
*/

char *vt_init(vt_info_t *vi_p) {

	int r;
	struct mem_range mr;

	/* Allow memory mapping */

	mr.mr_base = (phys_bytes)(vi_p->vram_base);
	mr.mr_limit = mr.mr_base + vi_p->vram_size;

	if (OK != (r = sys_privctl(SELF, SYS_PRIV_ADD_MEM, &mr)))
		panic("video_txt: sys_privctl (ADD_MEM) failed: %d\n", r);

	/* Map memory */

	video_mem = vm_map_phys(SELF, (void *)mr.mr_base, vi_p->vram_size);

	if (video_mem == MAP_FAILED)
		panic("video_txt couldn't map video memory");

	/* Save text mode resolution */

	scr_lines = vi_p->scr_lines;
	scr_width = vi_p->scr_width;

	return video_mem;
}
