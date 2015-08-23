//******************************************************************************
// Calc SD CRC7
//******************************************************************************
int calc_sd_crc7(char *buf) {
	int crc, crc_prev;
	int i, j;
	crc = buf[0];
	for(i=1; i<6; i++) {
		for(j=7; j>=0; j--) {
			crc <<= 1;
			crc_prev = crc;
			if(i<5) crc |= (buf[i]>>j) & 1;
			if(crc & 0x80) { crc ^= 0x89; }	// Generator
		}
	}
	return crc_prev | 1;
}
