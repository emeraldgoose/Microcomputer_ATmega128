#include "project.h"

int main(void)
{
	int pSW=0;
	char msg[20], pCmd='x';
	float vin=0;
	
	init();
	
	while(1) {
		if(Mode == 9) { // Main Menu
			if(pCmd != Cmd) lcd_display_clear(), pCmd = Cmd;
			main_menu();
		}
		else if(Mode == 0) { // Timer
			if(pCmd != Cmd || pSW != SW) lcd_display_clear(), pCmd = Cmd, pSW = SW;
			
			// Counting
			switch(Cmd) {
				case 'u' :
					sprintf(msg,"UP : %5.1f",(float)N_cnt/10.);
					Interval = 1;
					break;
				case 's' :
					sprintf(msg,"STOP : %4.1f",(float)N_cnt/10.);
					Interval = 0;
					break;
				case 'd' :
					sprintf(msg,"DOWN : %4.1f",(float)N_cnt/10.);
					Interval = -1;
					break;
				case 'r' :
					sprintf(msg,"RESET : %4.1f",(float)N_cnt/10.);
					Interval = N_cnt = 0;
					break;
				default: break;
			}
			
			// Setting FND
			FND[0] = SEG[N_cnt/1000];
			FND[1] = SEG[(N_cnt/100)%10];
			FND[2] = SEG[(N_cnt/10)%10]&0x7f;
			FND[3] = SEG[N_cnt%10];
			
			if(SW == 1) { // LCD
				DDRA = 0x00; // FND OFF
				
				lcd_display_position(2,1);
				lcd_string(msg);
				lcd_display_position(1,1);
				lcd_string("LCD : Timer");
			}
			else if(SW == 2) { // FND
				PORTB = 0x00; // FND ON
				DDRA = 0xff;
				
				lcd_display_position(1,1);
				lcd_string("FND : Timer");
				FND_display(100);
			}
			else if(SW == 3) { // USART
				DDRA = 0x00; // FND OFF
				
				lcd_display_position(1,1);
				lcd_string("USART : Timer");
				txd_string("\r");
				txd_string(msg);
			}
			else if(SW == 4) { // ALL
				PORTB = 0x00; // FND ON
				DDRA = 0xff;
				
				lcd_display_position(2,1);
				lcd_string(msg);
				FND_display(100);
				txd_string("\r");
				txd_string(msg);
				lcd_display_position(1,1);
				lcd_string("ALL : Timer");
			}
			else { // 기능만 선택하고 표시 방법을 선택하지 않았을 때 나타남. 기존 입력 때문에 생기는 충돌 방지
				lcd_display_position(1,1);
				lcd_string("Timer");
				lcd_display_position(2,1);
				lcd_string("Select display");
				Interval = 0;
			}
		}
		else if(Mode == 1) { // A/D Converter
			Interval = 0; // 모드 전환 시 타이머 정지
			if(pCmd != Cmd || pSW != SW) lcd_display_clear(), pCmd = Cmd, pSW = SW;
			int idx = Cmd - '1'; // idx : 0 (ADC1), 1 (ADC2), 2 (ADC3), 3 (ADC4)
			
			ADMUX = idx;
			ADCSRA |= 0x40;
			vin = (float)ADval[idx]*VREF/1023.0;
			sprintf(msg,"ADC%d : %.2f[V]",idx+1,vin);
			
			// Setting FND
			FND[0] = SEG[idx+10];
			FND[1] = SEG[(int)vin]&0x7f;
			FND[2] = SEG[(int)(vin*10)%10];
			FND[3] = SEG[(int)(vin*100)%10];
			
			if(SW == 1) { // LCD
				DDRA = 0x00; // FND OFF
				
				lcd_display_position(2,1);
				lcd_string(msg);
				lcd_display_position(1,1);
				lcd_string("LCD : A/D Conv.");
			}
			else if(SW == 2) { // FND
				DDRA = 0xff; // FND ON
				PORTB = 0x00;
				
				lcd_display_position(1,1);
				lcd_string("FND : A/D Conv.");
				FND_display(100);
			}
			else if(SW == 3) { // USART
				DDRA = 0x00; // FND OFF
				
				lcd_display_position(1,1);
				lcd_string("USART : A/D Conv.");
				txd_string("\r");
				txd_string(msg);
			}
			else if(SW == 4) { // ALL
				PORTB = 0x00; // FND ON
				DDRA = 0xff;
				
				lcd_display_position(2,1);
				lcd_string(msg);
				FND_display(100);
				txd_string("\r");
				txd_string(msg);
				lcd_display_position(1,1);
				lcd_string("ALL : A/D Conv.");
			}
			else { // 기능 선택, 표시 미선택 때 나타남.
				lcd_display_position(1,1);
				lcd_string("A/D Converter");
				lcd_display_position(2,1);
				lcd_string("Select display");
			}
		}
		else if(Mode == 2) {
			// Title
			txd_string("\n\rCurrent Value\n\r");
			txd_string("\r");
			
			// Timer
			sprintf(msg,"Timer : %4.1f",(float)N_cnt/10.);
			txd_string(msg);
			
			// ADC
			for(int i=0;i<4;i++) {
				float v = (float)ADval[i]*VREF/1023.0;
				sprintf(msg,"ADC%d : %.2f[V]",i+1,v);
				txd_string("   ");
				txd_string(msg);
			}
			
			// Exit Mode
			txd_string("\n\r");
			Mode = 9; SW = 0;
		}
		else if(Mode == 3) { // Game
			Interval = 0; SW = 0;
			
			game_manual();
			game_loading();
			
			// setting
			DDRA = 0xff; // FND ON
			PORTB = 0x00;	// FND ON
			int is_over = 0, is_ranker = 0, is_start = 0; // 게임 오버 표시, 랭킹 등록 가능 표시, 시작 구분
			int score = 0, level = 1; // 각 라운드 점수, 난이도
			int user_pos = 1; // 사용자 위치
			
			// ranking variable
			int pos_idx = 0, alpha_idx = 0; // Name : position, alphabet
			char name[3];
			
			while(1) {
				if(!is_over && !is_start) { // Game menu
					game_menu();
					
					if(SW == 1) { // Start & initialize
						is_over = 0, is_ranker = 0, is_start = 1;
						score = 0, level = 1;
						user_pos = 1;
						pos_idx = 0, alpha_idx = 0;
						for(int i=0;i<3;i++) name[i] = '?';
						make_map(); // Create map
					}
					else if(SW == 2) { // Exit
						Mode = 9, SW = 0;
						break;
					}
				}
				else if(is_over && is_start) {
					if(!is_ranker) { // game_over
						lcd_display_clear();
						if(score == 50 && level == 6) {
							lcd_display_position(1,1);
							lcd_string("All Stage Clear");
						}
						else if(!is_ranker) {
							lcd_display_position(1,1);
							lcd_string("Game Over");
						}
						lcd_display_position(2,1);
						sprintf(msg,"Score : %d",score+50*(level-1));
						lcd_string(msg);
						_delay_ms(1000);
						
						if(!is_ranker && isRanker(score+50*(level-1))) is_ranker = 1; // 스코어 기록 판단
						lcd_display_clear();
					}
					
					if(is_ranker) { // Update Ranking
						lcd_display_OnOff(1,1,1);
						SW = 0; // 기존 남아 있는 입력 제거
						while(pos_idx < 3) {
							lcd_display_position(2,1);
							lcd_string("Input Name : ");
							lcd_display_position(2,14);
							lcd_string(name);
							
							lcd_display_position(1,1);
							lcd_string("Update Score");
							
							if(SW == 3) { // alphabet++ (A->Z)
								alpha_idx++;
								if(alpha_idx > 25) alpha_idx = 0;
								name[pos_idx] = alpha_idx + 0x41;
								SW = 0;
							}
							else if(SW == 4) { // alphabet-- (Z->A)
								alpha_idx--;
								if(alpha_idx < 0) alpha_idx = 25;
								name[pos_idx] = alpha_idx + 0x41;
								SW = 0;
							}
							else if(SW == 1) {
								pos_idx++;
								SW = 0;
							}
							
							_delay_ms(100);
						}
						update_rank(score+50*(level-1),name);
						lcd_display_OnOff(1,0,0);
						ranking_display();
					}
					is_over = 0, is_ranker = 0, is_start = 0; // Go main menu
				}
				else if(!is_over && is_start) {
					
					for(int i = 0; i < 50; i++) ob[i].y -= 1; // 장애물 이동
					
					// 버튼 인터럽트에 따라 조작
					if(SW == 1) user_pos = 1; // UP
					else if(SW == 2) user_pos = 2; // DOWN
					else if(SW == 4) is_over = 0, is_start = 0; // 게임 중 나가기
					else user_pos = 1; // default
				
					if(!is_over && score == 50 && level < 6) { // harder
						level++;
						if(level <= 5) {
							char smg[16];
							score = 0, make_map(); // 새로운 스테이지 시작 전에 맵, 점수 초기화 진행
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
						if(ob[i].y < 17 && ob[i].y > 0) { // LCD 표시가능 거리
							if(ob[i].y == 1) { // 장애물이 유저와 같은 거리일 때
								if(ob[i].x == user_pos) { // 충돌
									is_over = 1; break;
								}
								else score++;
							}
							lcd_display_position(ob[i].x, ob[i].y);
							lcd_string("o");
						}
						lcd_display_position(user_pos,1);
						lcd_string(">");
					}
				
					// delay & FND display
					FND[0] = SEG[level];
					FND[1] = SEG[(score+50*(level-1))/100];
					FND[2] = SEG[((score+50*(level-1))/10)%10];
					FND[3] = SEG[(score+50*(level-1))%10];
					delay_level(level);
				}
			}
		}
	}
	
	return 0;
}

// Interrupt
ISR(TIMER3_COMPA_vect) {
	N_cnt = N_cnt + Interval;
	if(N_cnt >= 10000) N_cnt = 0;
	if(N_cnt < 0) N_cnt = 9999;
}

ISR(ADC_vect) {
	ADval[Cmd-'1'] = (int)ADCL + ((int)ADCH<<8);
}

ISR(USART0_RX_vect) {
	Cmd = UDR0;
	if(Cmd == 'u' || Cmd == 'd' || Cmd == 's' || Cmd == 'r') Mode = 0; // timer
	else if(Cmd >= '1' && Cmd <= '4') Mode = 1; // fnd
	else if(Cmd == 'i') Mode = 2; // info
	else if(Cmd == 'g') Mode = 3; // game
}

ISR(INT4_vect) {
	SW = 1;
}

ISR(INT5_vect) {
	SW = 2;
}

ISR(INT6_vect) {
	SW = 3;
}

ISR(INT7_vect) {
	SW = 4;
}