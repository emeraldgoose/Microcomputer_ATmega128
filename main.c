#include "project.h"

int main(void)
{
	int pSW=0;
	char str[20], msg[20], pCmd='x';
	float vin=0;
	
	lcd_init();
	lcd_display_OnOff(1,0,0);
	init();
	
	manual();
	flag=0;
	lcd_display_position(1,1);
	lcd_string("MicroComputer");
	_delay_ms(1000);
	flag=9;
	
	while(1) {
		// Switch Message
		if(flag==9) { // Hub
			if(pCmd!=Cmd) lcd_display_clear(), pCmd=Cmd;
			main_menu();
		}
		else if(flag==0) { // Timer
			if(pCmd!=Cmd || pSW!=SW) lcd_display_clear(), pCmd=Cmd, pSW=SW;
			
			if(Cmd=='u') sprintf(str,"UP : %4.1f",(float)N_cnt/10.), Interval=1;
			else if(Cmd=='s') sprintf(str,"STOP : %4.1f",(float)N_cnt/10.), Interval=0;
			else if(Cmd=='d') sprintf(str,"DOWN : %4.1f",(float)N_cnt/10.), Interval=-1;
			else if(Cmd=='r') sprintf(str,"RESET : %4.1f",(float)N_cnt/10.), Interval=N_cnt=0;
			FND[0]=SEG[N_cnt/1000];
			FND[1]=SEG[(N_cnt/100)%10];
			FND[2]=SEG[(N_cnt/10)%10]&0x7f;
			FND[3]=SEG[N_cnt%10];
			
			if(SW==1) {
				DDRA=0x00;
				lcd_display_position(2,1);
				lcd_string(str);
				lcd_display_position(1,1);
				lcd_string("LCD : Timer");
			}
			else if(SW==2) {
				PORTB=0x00;
				DDRA=0xff;
				lcd_display_position(1,1);
				lcd_string("FND : Timer");
				FND_display();
			}
			else if(SW==3) {
				DDRA=0x00;
				lcd_display_position(1,1);
				lcd_string("USART : Timer");
				txd_string("\r");
				txd_string(str);
			}
			else if(SW==4) {
				PORTB=0x00;
				DDRA=0xff;
				lcd_display_position(2,1);
				lcd_string(str);
				FND_display();
				txd_string("\r");
				txd_string(str);
				lcd_display_position(1,1);
				lcd_string("ALL : Timer");
			}
			else {
				lcd_display_position(1,1);
				lcd_string("Timer");
				lcd_display_position(2,1);
				lcd_string("Select display");
				Interval=0;
			}
		}
		else if(flag==1) { // A/D Converter
			SW=0;
			if(pCmd!=Cmd || pSW!=SW) lcd_display_clear(), pCmd=Cmd, pSW=SW;
			Interval=0;
			int idx=Cmd-'1';
			ADMUX=idx;
			ADCSRA|=0x40;
			vin=(float)ADval[idx]*VREF/1023.0;
			sprintf(msg,"ADC%d : %.2f[V]",idx+1,vin);
			FND[0]=SEG[idx+10];
			FND[1]=SEG[(int)vin]&0x7f;
			FND[2]=SEG[(int)(vin*10)%10];
			FND[3]=SEG[(int)(vin*100)%10];
			
			if(SW==1) {
				DDRA=0x00;
				lcd_display_position(2,1);
				lcd_string(msg);
				lcd_display_position(1,1);
				lcd_string("LCD : A/D Converter");
			}
			else if(SW==2) {
				DDRA=0xff;
				PORTB=0x00;
				lcd_display_position(1,1);
				lcd_string("FND : A/D Converter");
				FND_display();
			}
			else if(SW==3) {
				DDRA=0x00;
				lcd_display_position(1,1);
				lcd_string("USART : A/D Converter");
				txd_string("\r");
				txd_string(msg);
			}
			else if(SW==4) {
				PORTB=0x00;
				DDRA=0xff;
				lcd_display_position(2,1);
				lcd_string(msg);
				FND_display();
				txd_string("\r");
				txd_string(msg);
				lcd_display_position(1,1);
				lcd_string("ALL : A/D Converter");
			}
			else {
				lcd_display_position(1,1);
				lcd_string("A/D Converter");
				lcd_display_position(2,1);
				lcd_string("Select display");
			}
		}
		else if(flag==2) {
			txd_string("\n\rCurrent Value\n\r");
			txd_string("\r");
			sprintf(str,"Timer : %4.1f",(float)N_cnt/10.);
			txd_string(str);
			for(int i=0;i<4;i++) {
				float v=(float)ADval[i]*VREF/1023.0;
				sprintf(str,"ADC%d : %.2f[V]",i+1,v);
				txd_string("   ");
				txd_string(str);
			}
			txd_string("\n\r");
			flag=9; SW=0; // Exit Mode
		}
		else if(flag==3) {
			Interval=0;
			SW=0;
			game_manual();
			game_loading();
			// initialize setting
			DDRA=0xff;
			PORTB=0x00;
			int over_flag=0, is_ranker=0, is_start=0;
			int score=0, level=1;
			int user_pos=1;
			// ranking variable
			int pos_idx=0, alpha_idx=0; // Name : position, alphabet
			char name[3];
			
			while(1) {
				if(!over_flag && !is_start) {
					game_menu();
					if(SW==1) {
						over_flag=0, is_ranker=0, is_start=1;
						score=0, level=1;
						user_pos=1;
						pos_idx=0, alpha_idx=0;
						for(int i=0;i<3;i++) name[i]='?';
						make_map();
					}
					else if(SW==2) {
						flag=9, SW=0;
						break;
					}
				}
				else if(over_flag && is_start) {
					char smg[16];
					if(!is_ranker) { // game_over
						if(score==50 && level==6) {
							lcd_display_clear();
							lcd_display_position(1,1);
							lcd_string("All Stage Clear");
							lcd_display_position(2,1);
							lcd_string("Score : 250");
						}
						else if(!is_ranker) {
							lcd_display_position(1,1);
							lcd_string("Game Over");
							lcd_display_position(2,1);
							sprintf(smg,"Score : %d",score+50*(level-1));
							lcd_string(smg);
						}
						_delay_ms(1000);
						if(!is_ranker && isRanker(score+50*(level-1))) is_ranker=1;
						lcd_display_clear();
					}
					
					if(is_ranker) { // Update Ranking
						lcd_display_OnOff(1,1,1);
						SW=0;
						while(pos_idx<3) {
							lcd_display_position(2,1);
							lcd_string("Input Name : ");
							lcd_display_position(2,14);
							lcd_string(name);
							lcd_display_position(1,1);
							lcd_string("Update Score");
							if(SW==3) {
								alpha_idx++;
								if(alpha_idx>25) alpha_idx=0;
								name[pos_idx]=alpha_idx+0x41;
								SW=0;
							}
							else if(SW==4) {
								alpha_idx--;
								if(alpha_idx<0) alpha_idx=25;
								name[pos_idx]=alpha_idx+0x41;
								SW=0;
							}
							else if(SW==1) {
								pos_idx++;
								SW=0;
							}
							_delay_ms(100);
						}
						update_rank(score+50*(level-1),name);
						lcd_display_OnOff(1,0,0);
						ranking_display();
					}
					over_flag=0, is_ranker=0, is_start=0;
				}
				else if(!over_flag && is_start) {
					for(int i=0;i<50;i++) ob[i].y-=1;
					if(SW==1) user_pos=1;
					else if(SW==2) user_pos=2;
					else if(SW==4) over_flag=0, is_start=0;
					else user_pos=1;
				
					if(!over_flag && score==50 && level<6) { // harder
						level++;
						if(level<=5) {
							char smg[16];
							score=0, make_map();
							lcd_display_clear();
							lcd_display_position(1,1);
							sprintf(smg,"Stage %d",level);
							lcd_string(smg);
							_delay_ms(200);
						}
					}
				
					// Play Game
					lcd_display_clear();
					for(int i=0;i<50;i++) {
						if(ob[i].y<17 && ob[i].y>0) {
							if(ob[i].y==1) {
								if(ob[i].x==user_pos) {
									over_flag=1; break;
								}
								else score++;
							}
							lcd_display_position(ob[i].x,ob[i].y);
							lcd_string("o");
						}
						lcd_display_position(user_pos,1);
						lcd_string(">");
					}
				
					// delay & FND display
					FND[0]=SEG[level];
					FND[1]=SEG[(score+50*(level-1))/100];
					FND[2]=SEG[((score+50*(level-1))/10)%10];
					FND[3]=SEG[(score+50*(level-1))%10];
					delay_level(level);
				}
			}
		}
	}
	
	return 0;
}

// Interrupt
ISR(TIMER3_COMPA_vect) {
	N_cnt=N_cnt+Interval;
	if(N_cnt>=10000) N_cnt=0;
	if(N_cnt<0) N_cnt=9999;
}

ISR(ADC_vect) {
	ADval[Cmd-'1']=(int)ADCL+((int)ADCH<<8);
}

ISR(USART0_RX_vect) {
	Cmd=UDR0;
	if(Cmd=='u' || Cmd=='d' || Cmd=='s' || Cmd=='r') flag=0; // timer
	else if(Cmd>='1' && Cmd<='4') flag=1; // fnd
	else if(Cmd=='i') flag=2; // info
	else if(Cmd=='g') flag=3; // game
}

ISR(INT4_vect) {
	SW=1;
}

ISR(INT5_vect) {
	SW=2;
}

ISR(INT6_vect) {
	SW=3;
}

ISR(INT7_vect) {
	SW=4;
}