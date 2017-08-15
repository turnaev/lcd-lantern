#define F_CPU 9600000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define PORT PORTB
#define PIN PINB
#define LED PB0
#define BUTTON PB1

#define _ledOn  (PORT &= ~_BV(LED));
#define _ledOff (PORT |= _BV(LED));

#define _ledOnTime(t)  _ledOn _delay_ms(t);
#define _ledOffTime(t) _ledOff _delay_ms(t);

void blink() {
	_ledOnTime(50);
	_ledOffTime(100);
}

void slowUpLed(int from, int to) {
	for (int i = from;i<to;i++) {
		_delay_us(200);
		OCR0A=i;
	}
}

void slowDownLed(int from, int to) {
	for (int i = from;i>to;i--) {
		_delay_us(500);
		OCR0A=i;
	}
}

void off() {
	_ledOff
	blink();
	blink();
	blink();
	blink();
	_ledOff
}

volatile unsigned char state = 1;

int main(void) {
	//TCCR0A |= (1 << COM0A1) | (1 << COM0A0) | (1 << WGM00);  // управление выходом, ШИМ
	//TCCR0B |= (1 << CS01); // делитель N = 8
	PORT = 0;
	
	DDRB |= (0 << BUTTON);
	DDRB &= (1 << LED);
	
	MCUCR |= (1 << ISC01) & (0 << ISC00);  // прерывание по входу PCINT1 (PB1)
	GIMSK |= (1 << INT0);  // Разрешение прерываний PCINTn
	
	sei();
		
	while (1) {
		if (state==7) {
			
			_delay_ms(30);
			slowUpLed(0, 255);
			_delay_ms(40);
			slowDownLed(255, 10);
			_delay_ms(30);
			slowUpLed(10, 255);
			_delay_ms(40);
			slowDownLed(255, 0);
			_delay_ms(500);
			
		} else if(state==8) {
			
			_ledOnTime(20);
			_ledOffTime(70);
			_ledOnTime(20);
			_ledOffTime(70);
			_ledOnTime(20);
			_ledOffTime(700);
	
		} else if(state==9) {
			_ledOnTime(50);
			_ledOffTime(1200);
		}
	}
}


SIGNAL(INT0_vect) {
	cli();
	//GIMSK &= ~(1 << INT0);  // Запрещение прерываний PCINTn
	
	switch (state)
	{
		case 1:
		TCCR0A |= (1 << COM0A1) | (1 << COM0A0) | (1 << WGM00);  // управление выходом, ШИМ
		TCCR0B |= (1 << CS01); // делитель N = 8
		slowUpLed(0, 30);
		
		break;
		case 2:
		slowUpLed(30, 80);
		break;
		case 3:
		slowUpLed(80, 120);
		break;
		case 4:
		slowUpLed(120, 170);
		break;
		
		case 5:
		slowDownLed(170, 10);
		_delay_ms(300);
		slowUpLed(10, 255);
		
		OCR0A=0;
		TCCR0A &= (0 << COM0A1) & (0 << COM0A0);  // выкл, ШИМ
		
		_ledOn
		break;
		case 6:
		TCCR0A |= (1 << COM0A1) | (1 << COM0A0) | (1 << WGM00);  // управление выходом, ШИМ
		TCCR0B |= (1 << CS01); // делитель N = 8
		break;
		default:
		OCR0A=0;
		TCCR0A &= (0 << COM0A1) & (0 << COM0A0);  // выкл, ШИМ
		_ledOff
		;
	}
	
	state++;
	
	if(state>9) {
		
		off();
		state=1;
	}
	
	_delay_ms(400);
	//GIMSK |= (1 << INT0);  // Разрешение прерываний PCINTn
	sei();
}