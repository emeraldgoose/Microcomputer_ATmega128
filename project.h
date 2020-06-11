#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tpk_lcd8m.h"
#define VREF 5.0

// global variable
volatile int SW = 0, Mode = 9;
volatile unsigned char Cmd;
volatile int N_cnt = 0, Interval = 0;
volatile int ADval[4] = {0, 0, 0, 0};
volatile unsigned char SEG[16] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90, 0x88, 0x83, 0xc6, 0xa1, 0x86, 0x8e};
volatile unsigned char FND[4];

// Game variable
struct _obstacle {
	int x, y;
} ob[50];
char rank_name[3][3] = {{'?', '?', '?'}, {'?', '?', '?'}, {'?', '?', '?'}};
int rank_score[3] = {0, 0, 0};

// function
void init();
void txd(char ch);
void txd_string(char *str);
void manual();
void FND_display(int var);
void main_menu();
void bingle_bingle(); // Effect
void game_loading();
void game_menu();
void delay_level();
int isRanker(int score);
void update_rank(int score, char *name);

void init() {
	// DDR & PORT
	DDRA = 0xff;
	DDRB = 0x0f;
	DDRE = 0x00;
	DDRF = 0x00;
	PORTB = 0x00;
	PORTE = 0xf0;
	
	// 0.1 second Timer
	TCCR3A = 0x00;
	TCCR3B = 0x0d;
	TCCR3C = 0x00;
	
	OCR3A = 0x59f;
	ETIMSK = 0x10;
	
	EICRB = 0xaa;
	EIMSK = 0xf0;
	
	// Serial
	UCSR0A = 0x00;
	UCSR0B = 0x98;
	UCSR0C = 0x06;
	UBRR0H = 0;
	UBRR0L = 0x5f; // fosc=14.7456MHz, BAUD=9600bps
	
	// ADC
	ADCSRA = 0xcf;
	
	sei();
	_delay_ms(5);
	
	// LCD
	lcd_init();
	lcd_display_OnOff(1,0,0);
	
	// holding message
	Mode = 0;
	lcd_display_position(1,1);
	lcd_string("MicroComputer");
	_delay_ms(1000);
	Mode = 9;
}

void txd(char ch) {
	while(!(UCSR0A & 0x20));
	UDR0 = ch;
}

void txd_string(char *str) {
	int i=0;
	while(1) {
		if(str[i] == '\0') break;
		txd(str[i++]);
	}
}

void manual() {
	txd_string("\n\n\n");
	txd_string("\n\r***********************************************");
	txd_string("\n\r******    MicroComputer Term Project     ******");
	txd_string("\n\r***********************************************");
	txd_string("\n\r");
	txd_string("\n\r   ------------- Timer Menu -------------------");
	txd_string("\n\r   'u' : up counting       'd' : down counting");
	txd_string("\n\r   's' : stop counting     'r' : reset counting");
	txd_string("\n\r");
	txd_string("\n\r   ---------- A/D Conversion Menu -------------");
	txd_string("\n\r   '1' : ADC CH 0          '2' : ADC CH 1");
	txd_string("\n\r   '3' : ADC CH 2          '4' : ADC CH 3");
	txd_string("\n\r");
	txd_string("\n\r   ---------- Display Menu --------------------");
	txd_string("\n\r   SW1(INT4) : LCD          SW2(INT5) : FND");
	txd_string("\n\r   SW3(INT6) : USART        SW4(INT7) : ALL");
	txd_string("\n\r");
	txd_string("\n\r   ---------- add-on --------------------");
	txd_string("\n\r   'i' : Current Value      'g' : Mini Game");
	txd_string("\n\r   'c' : Screen Clear");
	txd_string("\n\r");
}

void main_menu() { // 모드 변경 시 충돌 방지, Mode = 9
	lcd_display_position(1,1);
	lcd_string("Select Mode");
}

void FND_display(int var) {
	for(int i=0;i<var;i++) {
		switch(i%4) {
			case 3:
			PORTB|=0x0f; PORTA=FND[0]; PORTB&=0xf7; break;
			case 2:
			PORTB|=0x0f; PORTA=FND[1]; PORTB&=0xfb; break;
			case 1:
			PORTB|=0x0f; PORTA=FND[2]; PORTB&=0xfd; break;
			case 0:
			PORTB|=0x0f; PORTA=FND[3]; PORTB&=0xfe; break;
			default: break;
		}
		_delay_ms(1);
	}
}

// Game Mode
void bingle_bingle() { // LCD Effect
	char ld[16];
	for(int i=0;i<16;i++) ld[i] = 0x20;
	ld[8] = 0xff;
	for(int i=8;i<16;i++) {
		ld[i] = 0xff;
		lcd_display_position(1,1);
		lcd_string(ld);
		_delay_ms(70);
	}
	for(int i=0;i<16;i++) ld[i] = 0x20;
	for(int i=15;i>=0;i--) {
		ld[i] = 0xff;
		lcd_display_position(2,1);
		lcd_string(ld);
		_delay_ms(70);
	}
	for(int i=0;i<8;i++) ld[i] = 0x20;
	for(int i=8;i<16;i++) ld[i] = 0xff;
	for(int i=0;i<7;i++) {
		ld[i] = 0xff;
		lcd_display_position(1,1);
		lcd_string(ld);
		_delay_ms(70);
	}
	lcd_display_clear();
}

void game_manual() {
	txd_string("\n\n\n\r");
	txd_string("Game Command\n\r");
	txd_string("SW 1 : UP\n\r");
	txd_string("SW 2 : DOWN\n\r");
	txd_string("SW 4 : END GAME\n\r");
}

void game_loading() {
	bingle_bingle();
	
	lcd_display_position(1,1);
	lcd_string("Mini Game");
	_delay_ms(500);
	
	lcd_display_clear();
}

void game_menu() {
	lcd_display_position(1,1);
	lcd_string("Car Racing");
	lcd_display_position(2,1);
	lcd_string("1) Start 2) Exit");
}

void make_map() { // Create map
	srand(TCNT3);
	ob[0].x = 1; ob[0].y = 17;
	for(int i=1;i<50;i++) {
		ob[i].x = rand()%2+1;
		int dst = rand()%3+2;
		ob[i].y = dst+ob[i-1].y;
	}
}

void delay_level(int level) { // delay & FND display
	FND_display((6-level)*100);
}

// Ranking function
void ranking_display() {
	char msg[16];
	txd_string("\n\r");
	txd_string("Game Ranking\n\r");
	sprintf(msg,"1st. %c%c%c %d",rank_name[0][0],rank_name[0][1],rank_name[0][2],rank_score[0]);
	txd_string(msg);
	txd_string("\n\r");
	sprintf(msg,"2nd. %c%c%c %d",rank_name[1][0],rank_name[1][1],rank_name[1][2],rank_score[1]);
	txd_string(msg);
	txd_string("\n\r");
	sprintf(msg,"3rd. %c%c%c %d",rank_name[2][0],rank_name[2][1],rank_name[2][2],rank_score[2]);
	txd_string(msg);
	txd_string("\n\r");
}

int isRanker(int score) {
	for(int i=0;i<3;i++) {
		if(score>rank_score[i]) return 1;
	}
	return 0;
}

void update_rank(int score, char *name) {
	if(score>rank_score[0]) {
		rank_score[2] = rank_score[1]; 
		for(int i=0;i<3;i++) rank_name[2][i] = rank_name[1][i];
		
		rank_score[1] = rank_score[0]; 
		for(int i=0;i<3;i++) rank_name[1][i] = rank_name[0][i];
		
		rank_score[0] = score; 
		for(int i=0;i<3;i++) rank_name[0][i] = name[i];
	}
	else if(score<rank_score[0] && score>rank_score[1]) {
		rank_score[2] = rank_score[1];
		for(int i=0;i<3;i++) rank_name[2][i] = rank_name[1][i];
		
		rank_score[1] = score;
		for(int i=0;i<3;i++) rank_name[1][i] = name[i];
	}
	else if(score<rank_score[1] && score>rank_score[2]) {
		rank_score[2] = score;
		for(int i=0;i<3;i++) rank_name[2][i] = name[i];
	}
}