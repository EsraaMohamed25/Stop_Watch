/*
 mini_proj2.c
 */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

unsigned char g_sec = 0;
unsigned char g_min= 0;
unsigned char g_hour = 0;

void timer1_Init_CTC(void){
	TCNT1= 0;                       //set initial value to 0
	OCR1A = 1000;                   //set compare Value
	TIMSK |= (1<<OCIE1A);          //enable Timer1 Comp A INT
	/* Configure timer control register TCCR1A
	 * 1. Disconnect OC1A and OC1B  COM1A1=0 COM1A0=0 COM1B0=0 COM1B1=0
	 * 2. FOC1A=1 FOC1B=0
	 * Configure timer control register TCCR1B
	 * 1. CTC Mode WGM12=1 WGM13=0 (Mode Number 4)
	 * 2. Pre-scaler = F_CPU/1024 CS10=1 CS11=0 CS12=1
	 */
	TCCR1A = (1<<FOC1A);
	TCCR1B = (1<<WGM12) | (1<<CS10) | (1<<CS12);
	DDRD |= (1<<5) ;
}

ISR(TIMER1_COMPA_vect)
{
	if(g_sec != 60){
		g_sec++;}
	else{
		g_sec =0;
		if(g_min != 60)
			g_min ++;
		else{
			g_min =0;
			if(g_hour != 24)
				g_hour++;
			else
				g_hour =0;
		}
	}
}

void INT0_Init(void){
	DDRD &= (~(1<<PD2)) ;          //configure D2 as input pin
	PORTD |= (1<<PD2);         //activate the internal pull up resistor at PD2
	GICR |= (1<<INT0) ;           //enable pin INT0
	//trigger INT0 with the falling edge
	MCUCR |= (1<<ISC01) ;
	MCUCR &= (~(1<<ISC00)) ;
	SREG |= (1<<7) ;            //enable INT by set I-bit
}

ISR(INT0_vect){         //reset stop watch
	while(!(PIND & (1<<PD2))){
	   PORTA |= 0x3F; //set first 4 pins of PORTC with value 0 at the beginning
		g_sec =0;
		g_min=0;
		g_hour =0;
	}
}

void INT1_Init(void){
	DDRD &= (~(1<<PD3)) ;          //configure D3 as input pin
	GICR |= (1<<INT1) ;           //enable pin INT1
	MCUCR |= (1<<ISC10) | (1<<ISC11) ;    //trigger INT1 with the rising edge
	SREG |= (1<<7) ;                //enable INT by set I-bit
}

ISR(INT1_vect){
	TCCR1B &= (~(1<<CS10)) & (~(1<<CS11)) &(~(1<<CS12));     //pause the time
}

void INT2_Init(void){
	DDRB &= (~(1<<PB2)) ;          //configure B2 as input pin
	PORTB |= (1<<PB2);         //activate the internal pull up resistor at PB2
	GICR |= (1<<INT2) ;        //enable pin INT2
	MCUCSR &= (~(1<<ISC2)) ;    //trigger INT2 with the falling edge
	SREG |= (1<<7) ;            //enable INT by set I-bit
}

ISR(INT2_vect){
	TCCR1B |= (1<<CS12) | (1<<CS10);      //resume the time
}

int main(void){

	INT0_Init();
	INT1_Init();
	INT2_Init();
	SREG |=(1<<7);     //enable global interrupts in MC
	DDRC  |= 0x0F;     //configure first 4 pins of PORTC as output pins(enables)
	PORTC &= 0xF0;    //set first 4 pins of PORTC with value 0 at the beginning

	DDRA |= 0x3F;       //configure first 5 pins of PORTA as output pins
	timer1_Init_CTC();

	while(1){
		PORTA =0x01   ;      //enable first 7-seg
		PORTC = (PORTC & 0xF0) | ((g_sec %10 ) & 0x0F);  //display sec
		_delay_ms(2);
		PORTA = 0x02 ;      //enable second 7-seg
		PORTC = (PORTC & 0xF0) | ((g_sec /10) & 0x0F);   //display sec
		_delay_ms(2);
		PORTA = 0x04 ;      //enable third 7-seg
		PORTC = (PORTC & 0xF0) | ((g_min %10) & 0x0F);  //display min
		_delay_ms(2);
		PORTA = 0x08 ;      //enable fourth 7-seg
		PORTC = (PORTC & 0xF0) | ((g_min /10) & 0x0F);  //display min
		_delay_ms(2);
		PORTA = 0x10 ;      //enable fifth 7-seg
		PORTC = (PORTC & 0xF0) | ((g_hour %10) & 0x0F);  //display hour
		_delay_ms(2);
		PORTA = 0x20 ;     //enable last 7-seg
		PORTC = (PORTC & 0xF0) | ((g_hour/10) & 0x0F);  //display hour
		_delay_ms(2);
	}
}
