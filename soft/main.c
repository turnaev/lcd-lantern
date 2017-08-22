#define F_CPU 9600000

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>

#define PORT PORTB
#define PIN PINB
#define LED PB0
#define BUTTON PB1

#define _ledOn() (PORT |= _BV(LED))
#define _ledOff() (PORT &= ~_BV(LED))
#define TIMED 800

#define _ledOnTime(t)  _ledOn(); _delay_ms(t)
#define _ledOffTime(t) _ledOff(); _delay_ms(t)
#define _v(v) (OCR0A=255-v)

void delay(int ms){
    for(;ms;ms--) {
        _delay_ms(1);
    }
}

void ledChangeBrightness(int from, int to, int t) {
int v;
	
    if(from == to) {
        return;
		
    } else if(from < to) {
        v = (int)t/(to - from);
        for (;from <= to; from++) {
            delay(v);
            _v(from);
        }
    } else {
        v = (int)t/(from - to);
        for (;from >= to; from--) {
            delay(v);
            _v(from);
        }
    }
}

void timerOff() {
    TCCR0A &= ~_BV(COM0A1) & ~_BV(COM0A0);  // выкл, ШИМ
    _v(0);
}

void timerOn(int v) {
    TCCR0A |= _BV(COM0A1) | _BV(COM0A0) | _BV(WGM00);  // управление выходом, ШИМ
    TCCR0B |= _BV(CS01); // делитель N = 8
    _v(v);
}

enum step {ON, S1, FULL, MIG1, MIG2, OFF};
volatile unsigned char state = 0;

SIGNAL(INT0_vect) {
    cli();
	
    GIMSK &= ~_BV(INT0);  // Запрещение прерываний PCINTn
	
    state++;
    if(state > OFF) {
        state = ON;
    }

    switch (state) {
		
    case ON:
        _ledOnTime(50);
        _ledOffTime(100);
        _ledOnTime(50);
        _ledOffTime(100);
        timerOn(0);
        ledChangeBrightness(0, 40, TIMED);
        break;
		
    case S1:
        ledChangeBrightness(40, 100, TIMED);
        break;

    case FULL: //full
		
        ledChangeBrightness(100, 255, TIMED);
        timerOff();
        _ledOn();
		
        break;

    case MIG1:
        timerOff();
        _ledOn();
        break;
			
    case MIG2:
        _ledOff();
        timerOn(0);
        break;
			
    default: // OFF
        timerOff();
        _ledOnTime(10);
        _ledOffTime(100);
        _ledOnTime(10);
        _ledOffTime(100);
		
        _ledOff();
    }
	
    _delay_ms(300);
    GIMSK |= _BV(INT0);  // Разрешение прерываний PCINTn
    sei();
}

int main(void) {
	
    PORT = 0;
	
    DDRB &= ~_BV(BUTTON);
    DDRB |= _BV(LED);

    MCUCR |= (1 << ISC01) & (0 << ISC00);  // прерывание по входу PCINT1 (PB1)
    GIMSK |= _BV(INT0);  // Разрешение прерываний PCINTn
	 
    _ledOff();
    state = OFF;
	
    sei();

    for (;;) {
        if(state == MIG1) {
            _ledOnTime(20);
            _ledOffTime(100);
            _ledOnTime(20);
            _ledOffTime(100);
            _ledOnTime(20);
            _ledOffTime(1000);
        } else if(state == MIG2) {
            ledChangeBrightness(0, 255, 800);
            _delay_ms(500);
            ledChangeBrightness(255, 0, 800);
            _delay_ms(1000);
        }
    }
	
    return (0);
}

