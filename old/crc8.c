#ifndef CRC8_C_INC
#define CRC8_C_INC 100
#
#include"crc8.h"
#include<stdlib.h>
//CRC8用関数
//記号定数
#define crc8_KEY (0xD5)// x8 + x7 + x6 + x4 + x2 + x1 + x0
//CRC8テーブルバッファ
unsigned char* x_crc8_Table=0;
//CRC8テーブルのmallocフラグ
unsigned char x_crc8_IsMallocTable=0;
//CRC8テーブル有効性確認
unsigned char crc8_table_is_valid() {
	return x_crc8_Table!=0;
}
//CRC8テーブルの初期化
void crc8_table_initialize() {
	unsigned char value;
        int n ;
        int i;
	
	if(crc8_table_is_valid())return;
	x_crc8_Table=malloc(256);
	x_crc8_IsMallocTable=1;
	if(!crc8_table_is_valid())return;

	for(n= 0; n < 256; n++) {
		value = (unsigned char)(n);
		// value = n;
		for(i = 0; i < 8; i++) {
			if(value & 0x80) {
				value <<= 1;
				value ^= crc8_KEY;
			} else { 
				value <<= 1;
			}
		}
		x_crc8_Table[n] = value;
	}
}
//CRC8テーブルのメモリ割り当て初期化
void crc8_table_placement_initialize(unsigned char Table[256]) {
	unsigned char value;
        int n ;
        int i;

	if(crc8_table_is_valid())return;
	x_crc8_Table=Table;
	x_crc8_IsMallocTable=0;
	if(!crc8_table_is_valid())return;

	for(n= 0; n < 256; n++) {
		value = (unsigned char)(n);
		// value = n;
		for(i = 0; i < 8; i++) {
			if(value & 0x80) {
				value <<= 1;
				value ^= crc8_KEY;
			} else {
				value <<= 1;
			}
		}
		x_crc8_Table[n] = value;
	}
}
//CRC8テーブルの終端化
void crc8_table_finalize() {
	if(x_crc8_IsMallocTable)free(x_crc8_Table);
	x_crc8_Table=0;
	x_crc8_IsMallocTable=0;
}
//ビットシフトによるCRC8の単バイト計算
unsigned char crc8_bitshift_putc(unsigned char crc8, unsigned char data) {
	unsigned char i;

	crc8 ^= data;
	for(i=0; i<8; i++) {
		if(crc8 & 0x80) {
			crc8<<=1;
			crc8^=crc8_KEY;
		} else {
			crc8<<=1;
		}
	}
	return crc8;
}
//ビットシフトによるCRC8の複数バイト計算
unsigned char crc8_bitshift_puts(unsigned char crc8, const void *buff, unsigned int size) {
	unsigned char *itr = (unsigned char *)buff;

	while((size--) != 0) crc8 = crc8_bitshift_putc(crc8, *(itr++));

	return crc8;
}
//CRC8テーブルによるCRC8の単バイト計算
unsigned char crc8_table_putc(unsigned char crc8, unsigned char data) {
	return x_crc8_Table[crc8 ^data];
}
//CRC8テーブルによるCRC8の複数バイト計算
unsigned char crc8_table_puts(unsigned char crc8, const void *buff, unsigned int size) {
	unsigned char *itr = (unsigned char *)buff;

	while((size--) != 0) crc8 = x_crc8_Table[crc8 ^ *(itr++)];

	return crc8;
}
//CRC8の単バイト計算
unsigned char crc8_putc(unsigned char crc8, unsigned char data) {
	if(crc8_table_is_valid())return crc8_table_putc(crc8, data);
	else return crc8_bitshift_putc(crc8, data);
}
//CRC8の複数バイト計算
unsigned char crc8_puts(unsigned char crc8, const void *buff, unsigned int size) {
	if(crc8_table_is_valid())return crc8_table_puts(crc8, buff, size);
	else return crc8_bitshift_puts(crc8, buff, size);

}
#
#endif
