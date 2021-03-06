/* bootpackのメイン */



#include "bootpack.h"

#include <stdio.h>

extern struct FIFO8 keyfifo;

extern struct FIFO8 mousefifo;
void HariMain(void)

{

	extern struct FIFO8 keyfifo;
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;

	char s[40], mcursor[256],keybuf[32],mousebuf[128];

	int mx, my,i;
	init_keyboard();
	enable_mouse();

	fifo8_init(&keyfifo,32,keybuf);
	fifo8_init(&mousefifo,128,mousebuf);
	init_gdtidt();

	init_pic();

	io_sti(); /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */



	init_palette();
	

	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);

	mx = (binfo->scrnx - 16) / 2; /* 画面中央になるように座標計算 */

	my = (binfo->scrny - 28 - 16) / 2;

	init_mouse_cursor8(mcursor, COL8_008484);

	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

	sprintf(s, "(%d, %d)", mx, my);

	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);



	io_out8(PIC0_IMR, 0xf9); /* PIC1とキーボードを許可(11111001) */

	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */



	for (;;) {

		io_cli();
		if(fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0){
			io_stihlt();
		}else{
			if(fifo8_status(&keyfifo) != 0){
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s,"%02X",i);
				boxfill8(binfo->vram,binfo->scrnx,COL8_008484,0,16,15,31);
				putfonts8_asc(binfo->vram,binfo->scrnx,0,16,COL8_FFFFFF,s);
			}else if(fifo8_status(&mousefifo)!=0){
				i = fifo8_get(&mousefifo);
				io_sti();
				sprintf(s,"%02X",i);
				boxfill8(binfo->vram,binfo->scrnx,COL8_008484,32,16,47,31);
				putfonts8_asc(binfo->vram,binfo->scrnx,32,16,COL8_FFFFFF,s);
			}
		}

	}

}
void wait_KBC_sendready(void){
	for(;;){
		if((io_in8(PORT_KEYSTA)&KEYSTA_SEND_NOTREADY) == 0){
			break;
		}
	}
	return;
}
void init_keyboard(void){
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,KBC_MODE);
	return;
}

void enable_mouse(void){
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD,KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT,MOUSECMD_ENABLE);
	return;
}