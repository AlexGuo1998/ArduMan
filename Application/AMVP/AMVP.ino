#include <SPI.h>
#include <SdFat.h>
#include <avr/pgmspace.h>
#include "timing.h"

SdFat SD;

const uint8_t OLEDCS = 12, DS = 4, RST = 6, SDCS = 23; //ArduMan

const uint8_t initseq[] PROGMEM = {
	0x0ae,				/* display off, sleep mode */
	0x0d5, 0x0f0,		/* clock divide ratio (0x00=1) and oscillator frequency (0x8) */
	0x0a8, 0x03f,		/* */
	
	0x0d3, 0x000,		/* set diaplay offset = 0 */
	
	0x040,				/* start line = 0 */
	
	0x08d, 0x014,		/* [2] charge pump setting (p62): 0x014 enable, 0x010 disable */
	
	0x020, 0x000,		/* */
	0x0a1,				/* segment remap a0/a1*/
	0x0c8,				/* c0: scan dir normal, c8: reverse */
	0x0da, 0x012,		/* com pin HW config, sequential com pin config (bit 4), disable left/right remap (bit 5) */
	0x081, 0x07f,		/* [2] set contrast control */
	0x0d9, 0x0f1,		/* [2] pre-charge period 0x022/f1*/
	0x0db, 0x040,		/* vcomh deselect level */
	
	0x02e,				/* 2012-05-27: Deactivate scroll */ 
	0x0a4,				/* output ram to display */
	0x0a6,				/* none inverted normal display mode */
	0x0af,				/* display on */
};

counter coled, cmovie;

void bootoled() {
	digitalWrite(OLEDCS, HIGH);
	digitalWrite(DS, LOW);
	digitalWrite(RST, LOW);
	digitalWrite(SDCS, HIGH);
	delay(1000);
	digitalWrite(RST, HIGH);
	digitalWrite(OLEDCS, LOW);
	digitalWrite(DS, LOW);
	for (uint8_t i = 0; i < sizeof(initseq) / sizeof(initseq[0]); i++) {
		SPI.transfer(pgm_read_byte_near(&initseq[i]));
	}
	digitalWrite(DS, HIGH);
}

void loadFrame(File &f, uint32_t &nmax, FatPos_t &pmax, uint32_t nnow, FatPos_t *p) {
	//Serial.print("Load: ");
	//Serial.print(nmax);
	//Serial.print(" -> ");
	//Serial.println(nnow);
	nnow = nnow & (~3);
	f.setpos(&pmax);
	if (nmax < nnow) {
		//Serial.println("warning");
		//f.seekCur(128 * 8 * (nnow - nmax));
	}
	f.seekCur(128 * 8);
	f.getpos(&p[0]);
	f.seekCur(128 * 8);
	f.getpos(&p[1]);
	f.seekCur(128 * 8);
	f.getpos(&p[2]);
	f.seekCur(128 * 8);
	f.getpos(&p[3]);
	pmax = p[3];
	nmax = nnow + 3;
}

double screen_freq = 134.85;

void playmovie(char *filename, double freq) {
	digitalWrite(OLEDCS, HIGH);
	digitalWrite(DS, HIGH);
	if (!SD.begin(SDCS)) {
	    Serial.println("Card failed, or not present");
	    return;
  	}
	File f = SD.open(filename);
	if(!f) {		
		Serial.println("error opening file");
		return;
	}
	
	FatPos_t p[4], pmax;
	f.getpos(&pmax);

	uint32_t nnow = 0;
	uint32_t nmax = 0;	
	{
		unsigned long now = micros();
		initcounter(&coled, now, screen_freq);
		initcounter(&cmovie, now, freq);
	}
	loadFrame(f, nmax, pmax, 1, p);
	while (f.available()) {
		unsigned long now = micros();
		if (nextframe(&cmovie, now)) {
			nnow += 4;
			loadFrame(f, nmax, pmax, nnow, p);
		}
		if (nextframe(&coled, now)) {
			nnow = (nnow & (~3)) | ((nnow + 1) & 3);
			uint8_t dat[512];
			f.setpos(&p[nnow & 3]);
			f.read(dat, 512);
			digitalWrite(OLEDCS, LOW);
			for (int i = 0; i < 512; i++) SPI.transfer(dat[i]);
			digitalWrite(OLEDCS, HIGH);
			f.read(dat, 512);
			digitalWrite(OLEDCS, LOW);
			for (int i = 0; i < 512; i++) SPI.transfer(dat[i]);
			digitalWrite(OLEDCS, HIGH);
		}
	}
	f.close();
	digitalWrite(OLEDCS, LOW);
}

void setup() {
	pinMode(OLEDCS, OUTPUT);
	pinMode(DS, OUTPUT);
	pinMode(RST, OUTPUT);
	pinMode(SDCS, OUTPUT);
	SPI.begin();
	SPI.beginTransaction(SPISettings(16000000, MSBFIRST, SPI_MODE0));// div 2
	Serial.begin(250000);
	//while(!Serial);
	bootoled();
}

void loop() {
	playmovie((char *)"omr-5.bin", 23.98);
}

