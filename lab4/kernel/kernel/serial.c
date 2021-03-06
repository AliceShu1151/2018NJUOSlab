#include "x86.h"
#include "device.h"

void initSerial(void) {
	outByte(SERIAL_PORT + 1, 0x00);
	outByte(SERIAL_PORT + 3, 0x80);
	outByte(SERIAL_PORT + 0, 0x01);
	outByte(SERIAL_PORT + 1, 0x00);
	outByte(SERIAL_PORT + 3, 0x03);
	outByte(SERIAL_PORT + 2, 0xC7);
	outByte(SERIAL_PORT + 4, 0x0B);
}

static inline int serialIdle(void) {
	return (inByte(SERIAL_PORT + 5) & 0x20) != 0;
}

void putChar(char ch) {
	while (serialIdle() != TRUE);
	outByte(SERIAL_PORT, ch);
}


void updateCursor(int r, int c) {
	uint16_t pos = r * 80 + c;
	outByte(0x3d4, 0x0f);
	outByte(0x3d5, 0xff & pos);
	outByte(0x3d4, 0x0e);
	outByte(0x3d5, pos >> 8);
}

/* print to video segment */
void video_print(char c) {
	static int row = 0, col = 0;
	if (c == '\n') {
		row++;	col = 0;
		return;
	}
	
	asm ("movl %0, %%edi;"::"r"(((80 * row + col) * 2)):"%edi");
	asm ("movw %0, %%eax;"::"r"(0x0c00 | c):"%eax");     // 0x0黑底,0xc红字, 字母ASCII码
	asm ("movw %%ax, %%gs:(%%edi);":::"%edi"); 	     // 写入显存
	
	col++;
	if (col == 80) {
		row++;	col = 0;
	}
	updateCursor(row, col);
}
// clear screen
void initVga() {
	updateCursor(0, 0);
	for (int i = 0; i < 10 * 80; i ++) {
		*((uint16_t *)0xb8000 + i) = (0x0c << 8);
	}
}