//************************************************************************
// 16*2 Character LCD 8bit Interface for WinAVR
// RS = PC4, R/W = PC5, EN = PC6, DB7~DB0 : PD7~PD0
//************************************************************************

void lcd_busy(void)
{   

	DDRD = 0x00;
	do {    
		PORTC &= 0b01101111; 	// RS = 0    
		PORTC |= 0b00100000; 	// RW = 1   
		PORTC |= 0b01000000; 	// E = 1   
		_delay_us(1);  
	} while(PIND & 0x80);
    
	PORTC &= 0b10111111; 	// E = 0  
	DDRD = 0xff;

}

void lcd_command(unsigned char cmd)
{
	lcd_busy();
    
	PORTD = cmd;     
	PORTC &= 0b01101111;    	// RS = 0
	PORTC &= 0b01011111;    	// RW = 0 
	PORTC |= 0b01000000;    	// E = 1 
	_delay_us(1);   
	PORTC &= 0b00111111;    	// E = 0

}       

void lcd_init()
{
	DDRD = 0xff;
	DDRC = DDRC | 0xf0;
    
	_delay_ms(15);           	// 15ms
	
	PORTD = 0b00110000;        
	PORTC &= 0b00001111;    	// E,RW,RS = 0,0,0
	_delay_us(4100);         	// 4.1ms 
	PORTC &= 0b00001111;    	// E,RW,RS = 0,0,0
	_delay_us(100);          	// 100us 
	PORTC &= 0b00001111;    	// E,RW,RS = 0,0,0
    
	lcd_command(0b00111000);
	lcd_command(0b00001000);
	lcd_command(0b00000001);
	lcd_command(0b00000110);	

	lcd_command(0b00001111);	
}         

void lcd_data(unsigned char byte)
{
	lcd_busy();
    
	PORTD = byte;  
	PORTC |= 0b00010000; 		// RS=1
	PORTC &= 0b01011111; 		// RW=0 
	PORTC |= 0b01000000; 		// E=1 
	_delay_us(1); 			
	PORTC &= 0b00111111; 		
	_delay_us(45);
}

void lcd_string(char *str)
{
	int i=0;
	while(1) {
		if (str[i] == '\0') break;
		lcd_data(str[i++]);	
	}
}

void lcd_display_clear()
{
	lcd_command(0b00000001);
	_delay_us(1645);
}

void lcd_cursor_home()
{
	lcd_command(0b00000010);
	_delay_us(1645);
}

void lcd_entry_mode(int ID, int S)
{
unsigned char cmd;
	
	cmd = 0b00000100;
	if (ID == 1) cmd = cmd | 0b00000010;
	if (S == 1) cmd = cmd | 0b00000001;
	lcd_command(cmd);
	_delay_us(45);
}

void lcd_display_OnOff(int D, int C, int B)
{
unsigned cmd;
	
	cmd = 0b00001000;
	if (D == 1) cmd = cmd | 0b00000100;
	if (C == 1) cmd = cmd | 0b00000010;
	if (B == 1) cmd = cmd | 0b00000001;
	lcd_command(cmd);
	_delay_us(45);
}

void lcd_cursordisplay_shift(int SC, int RL)
{
unsigned cmd;
	
	cmd = 0b00010000;
	if (SC == 1) cmd = cmd | 0b00001000;
	if (RL == 1) cmd = cmd | 0b00000100;
	lcd_command(cmd);
	_delay_us(45);
}

void lcd_display_position(unsigned char row, unsigned char col)
{
unsigned cmd;

	cmd = 0b10000000 | ((row-1)*0x40+(col-1));
	lcd_command(cmd);
	_delay_us(45);
}