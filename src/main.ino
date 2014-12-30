#define DOT_MSEC 200

#include <stdint.h>
#include <string.h>
#include <EEPROM.h>
#include "tiny_morse_code_library.h"

#define MODE_NONE 0
#define MODE_SEQ_INTERVAL 1

uint8_t mode = MODE_NONE;
uint8_t num = 0;
unsigned long start_millis;
uint8_t eeprom_write_done = 0;

#define DASH_MSEC (DOT_MSEC*3)
#define BETWEEN_DOT_MSEC (DOT_MSEC*1)
#define BETWEEN_CHAR_MSEC (DOT_MSEC*3)

void
real_panic()
{
	for (;;) {
		digitalWrite(13, HIGH);
		delay(300);
		digitalWrite(13, LOW);
		delay(300);
	}

	return;
}

void
disp_morse_dot(const uint8_t te)
{
	digitalWrite(13, HIGH);

	switch (te) {
		case M_O:
			delay(DOT_MSEC);
			break;
		case M_A:
			delay(DASH_MSEC);
			break;
	}

	digitalWrite(13, LOW);

	delay(BETWEEN_DOT_MSEC);

	return;
}

void
disp_morse_char(const uint8_t t[8])
{
	int i;

	for (i = 0; i < 8 && t[i] != M_N; i++) 
		disp_morse_dot(t[i]);
	
	delay(BETWEEN_CHAR_MSEC);

	return;
}

void
disp_morse_str(const char *s)
{
	int i;
	struct morse_flat_t st;

	for (i = 0; i < strlen(s); i++)
		if (!morse_char_to_flat(s[i], &st))
			disp_morse_char(st.t);

	return;
}

void
panic(const char *s)
{
	for (;;)
		disp_morse_str(s);
	
	return;
}

uint8_t
read_mode_from_eeprom()
{
	return EEPROM.read(0);
}

void
write_mode_to_eeprom(uint8_t v)
{
	EEPROM.write(0, v);

	return;
}

uint8_t
read_num_from_eeprom()
{
	return EEPROM.read(1);
}

void
write_num_to_eeprom(uint8_t v)
{
	EEPROM.write(1, v);

	return;
}

void
setup()
{
	char s[0xff];

	pinMode(13, OUTPUT);

	pinMode(2, INPUT_PULLUP);

	if (digitalRead(2) == LOW)
		mode = MODE_SEQ_INTERVAL;
	else {
		switch (mode = read_mode_from_eeprom()) {
			case MODE_SEQ_INTERVAL:
				break;
			case MODE_NONE:
			default:
				panic("Bad mode val in EEPROM");
		}
	}

	sprintf(s, "m%d", mode);
	disp_morse_str(s);

	pinMode(8, INPUT_PULLUP);
	pinMode(9, INPUT_PULLUP);
	pinMode(10, INPUT_PULLUP);
	pinMode(11, INPUT_PULLUP);
	pinMode(12, INPUT_PULLUP);

	num = (digitalRead(8) == LOW ? 1<<0 : 0) | (digitalRead(9) == LOW ? 1<<1 : 0) | (digitalRead(10) == LOW ? 1<<2 : 0) | (digitalRead(11) == LOW ? 1<<3 : 0) | (digitalRead(12) == LOW ? 1<<4 : 0);

	if (num == 0) 
		if ((num = read_num_from_eeprom()) == 0)
			panic("Bad num val in EEPROM");

	sprintf(s, "n%d", num);
	disp_morse_str(s);

	pinMode(3, INPUT_PULLUP);
	pinMode(A0, OUTPUT);
	pinMode(A1, OUTPUT);
	pinMode(A2, OUTPUT);
	pinMode(A3, OUTPUT);
	pinMode(A4, OUTPUT);
	pinMode(A5, OUTPUT);

	digitalWrite(A0, HIGH);
	digitalWrite(A1, HIGH);
	digitalWrite(A2, HIGH);
	digitalWrite(A3, HIGH);
	digitalWrite(A4, HIGH);
	digitalWrite(A5, HIGH);
	delay(5000);
	digitalWrite(A0, LOW);
	digitalWrite(A1, LOW);
	digitalWrite(A2, LOW);
	digitalWrite(A3, LOW);
	digitalWrite(A4, LOW);
	digitalWrite(A5, LOW);

	start_millis=millis();

	return;
}

void loop()
{
	if ((!eeprom_write_done) && millis() - start_millis > 10000) {
		if (read_mode_from_eeprom() != mode) {
			disp_morse_str("write mode");
			write_mode_to_eeprom(mode);
		}
		if (read_num_from_eeprom() != num) {
			disp_morse_str("write num");
			write_num_to_eeprom(num);
		}
		eeprom_write_done = !0;
		disp_morse_str("EEPROM write done");
	}

	while (digitalRead(3) == LOW) {
		disp_morse_str("D");
		digitalWrite(A0, HIGH);
		digitalWrite(A1, HIGH);
		digitalWrite(A2, HIGH);
		digitalWrite(A3, HIGH);
		digitalWrite(A4, HIGH);
		digitalWrite(A5, HIGH);
	}

	switch (mode) {
		case MODE_SEQ_INTERVAL:
			delay(num*60*1000UL);
			digitalWrite(A0, HIGH);
			digitalWrite(A1, HIGH);
			digitalWrite(A2, HIGH);
			digitalWrite(A3, HIGH);
			digitalWrite(A4, HIGH);
			digitalWrite(A5, HIGH);
			delay(5000);
			digitalWrite(A0, LOW);
			digitalWrite(A1, LOW);
			digitalWrite(A2, LOW);
			digitalWrite(A3, LOW);
			digitalWrite(A4, LOW);
			digitalWrite(A5, LOW);
			break;
	}

	return;
}
