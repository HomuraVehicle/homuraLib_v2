#ifndef CRC8_INC
#define CRC8_INC 100
#
/*
---CRC8---
巡回冗長性検査、CRC8のバイト生成機能を提供するライブラリ

crc8_bitshift_putc/puts
	ビットシフト型crc8演算関数
	速度は遅いが、メモリを消費しない
crc8_table_putc/puts
	テーブル型crc8演算関数
	高速だが、テーブル用メモリ256byteを消費する
	initialize/finalizeが必要となる
	is_valid関数でテーブルが有効かを確認できる
crc8_putc/puts
	テーブルが使用可能ならテーブル型、そうでなければビットシフト型を用いてcrc8演算を行う

===crc8===
v1_00/140115 hmIto
	作成
*/
#ifdef __cplusplus
extern "C"{
#endif
	//ビットシフトによるCRC8の単バイト計算
	unsigned char crc8_bitshift_putc(unsigned char crc8, unsigned char data);
	//ビットシフトによるCRC8の複数バイト計算
	unsigned char crc8_bitshift_puts(unsigned char crc8, const void *buff, unsigned int size);
	//CRC8テーブルによるCRC8の単バイト計算
	unsigned char crc8_table_putc(unsigned char crc8, unsigned char data);
	//CRC8テーブルによるCRC8の複数バイト計算
	unsigned char crc8_table_puts(unsigned char crc8, const void *buff, unsigned int size);
	//CRC8の単バイト計算
	unsigned char crc8_putc(unsigned char crc8, unsigned char data);
	//CRC8の複数バイト計算
	unsigned char crc8_puts(unsigned char crc8, const void *buff, unsigned int size);
	//CRC8テーブル有効性確認
	unsigned char crc8_table_is_valid();
	//CRC8テーブルの初期化
	void crc8_table_initialize();
	//CRC8テーブルのメモリ割り当て初期化
	void crc8_table_placement_initialize(unsigned char Table[256]);
	//CRC8テーブルの終端化
	void crc8_table_finalize();
#ifdef __cplusplus
}
#endif
#
#endif
