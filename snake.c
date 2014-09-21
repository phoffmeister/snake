#include <avr/io.h>
#include <avr/interrupt.h>
#include "../5110_lcd_driver/lcd_driver.h"
#define MAX_SNAKE_LEN 130
#define SNAKE_SPEED 800

uint8_t pix[84][6];
uint8_t gameover;
uint8_t dir;
uint8_t body[2][MAX_SNAKE_LEN];
uint8_t len;
uint8_t startscreen;
uint8_t fc;
uint16_t score;
uint8_t food[2];


uint8_t inline read_keys()
{
	uint8_t val;
	val = 0;
	
	if ( !(PINC & (1<<PINC2)) ) 
	{//rechts
		val |= 1;
  	}
  	
  	if ( !(PINC & (1<<PINC4)) ) 
	{//hoch
		val |= (1<<1);
  	}
  	
  	if ( !(PINC & (1<<PINC5)) ) 
	{//links
		val |= (1<<2);
  	}
  	
  	if ( !(PINC & (1<<PINC3)) ) 
	{//runter
		val |= (1<<3);
  	}
  	
  	if ( !(PINC & (1<<PINC1)) ) 
	{//A
		val |= (1<<4);
  	}
  	
  	if ( !(PINC & (1<<PINC0)) ) 
	{//B
		val |= (1<<5);
  	}
  	
  	
	return val;
}

void inline evalkeys( uint8_t k)
{
	if ( k & 1 && (dir != 2)) 
	{//rechts
		dir = 0;
		return;
	}
	
	if ( k & (1<<1) && (dir != 3)  ) 
	{//hoch
		dir = 1;
		return;
	}
	
	if ( k & (1<<2) && (dir != 0)  ) 
	{//links
		dir = 2;
		return;
	}
	
	if ( k & (1<<3) && (dir != 1)  ) 
	{//runter
		dir = 3;
		return;
	}
}

void snake_hw_init()
{
	//PC2-PC5 are inputs
	DDRC &= ~((1<<PC5) | (1<<PC4) | (1<<PC3) | (1<<PC2) | (1<<PC1) | (1<<PC0));
	//activate internal pullups
	PORTC |= (1<<PC5) | (1<<PC4) | (1<<PC3) | (1<<PC2) | (1<<PC1) | (1<<PC0);
}

void inline snake_init()
{	
	uint8_t n;
	gameover = 0;
	dir = 3;
	len = 5;
	for(n=0;n<MAX_SNAKE_LEN;n++)
	{
		body[0][n]=100;
		body[1][n]=100;
	}
	
	body[0][0]=42;body[1][0]=24;
	body[0][1]=41;body[1][1]=24;
	body[0][2]=40;body[1][2]=24;	
	
	score = 0;
	fc = 0;
	food[0] = 100;
	food[1] = 100;
}

void inline draw_field()
{
	uint8_t n;
	
	//top border
	for(n=0; n<84; n++)
		lcd_modifyPixel(pix, n, 9, 1);
	
	//bottom border
	for(n=0; n<84; n++)
		lcd_modifyPixel(pix, n, 47, 1);
	
	//left border
	for(n=9; n<48; n++)
		lcd_modifyPixel(pix, 0, n, 1);
		
	//right border
	for(n=9; n<48; n++)
		lcd_modifyPixel(pix, 83, n, 1);
		
	lcd_write_num(pix,0,0,score);
}

void inline draw_snake()
{
	uint8_t n;
	for(n=0; n<len; n++)
	{
		lcd_modifyPixel(pix, body[0][n], body[1][n], 1);
	}
}

void inline draw_gameover()
{
	lcd_clearAll(pix);
	lcd_write_string(pix, 3, 2, "GAMEOVER");
	lcd_write_num(pix, 10,3, score);
}

void inline move_snake()
{
	uint8_t tmp[2];
	uint8_t n;
	
	for(n=len; n>0; n--)
	{
		tmp[0] = body[0][n];
		tmp[1] = body[1][n];
		
		body[0][n] = body[0][n-1];
		body[1][n] = body[1][n-1];
		
		body[0][n-1] = tmp[0];
		body[1][n-1] = tmp[1];
	}
	
	if( dir == 0 )
	{//rechts		
		body[0][0] = body[0][1] + 1;
		body[1][0] = body[1][1];
	}
	else if( dir == 1 )
	{//hoch
		body[0][0] = body[0][1];
		body[1][0] = body[1][1] - 1;
	}
	else if( dir == 2 )
	{//links
		body[0][0] = body[0][1] - 1;
		body[1][0] = body[1][1];
	}
	else if( dir == 3 )
	{//runter
		body[0][0] = body[0][1];
		body[1][0] = body[1][1] + 1;
	}
	
	
}

void setup_timer()
{
	TCCR1B |= (1<<WGM12) | (1<<CS12) | (1<<CS10);
	OCR1A = SNAKE_SPEED;
	TIMSK |= (1<<OCIE1A);
}

void is_gameover()
{
	uint8_t x,y,n;
	
	x = body[0][0];
	y = body[1][0];
	
	// hitting any wall
	if(x==1)
		gameover = 1;
	if(x==83)
		gameover = 1;
	if(y==9)
		gameover = 1;
	if(y==47)
		gameover = 1;
		
	//hitting yourself
	for(n=1;n<len;n++)
	{
		if( x==body[0][n] && y==body[1][n])
			gameover = 1;
	}
}

void inline draw_startscreen()
{
	lcd_write_string(pix, 4,2,"SNAKE");
	lcd_write_string(pix, 1,4,"PUSH TO START");
}

void inline add_food()
{
	uint8_t fx;
	uint8_t fy;
	uint8_t not_good = 1;
	uint8_t n;
	
	while( not_good ) 
	{
		fx = (rand() % 82) + 1;
		fy = (rand() % 36) + 10;
		not_good = 0;
		for(n=0; n<len; n++)
		{
			if(fx == body[0][n] && fy == body[1][n])
				not_good = 1;
		}
	}
	
	food[0] = fx;
	food[1] = fy;
}

void inline draw_food()
{
		if(food[0] != 100)
		{
			lcd_modifyPixel(pix, food[0], food[1], 1);
		}
}

ISR(TIMER1_COMPA_vect)
{
	fc++;
	if(startscreen)
	{
		draw_startscreen();
		if(read_keys())
		{
			startscreen=0;
			srand(TCNT1);
		}
	}
	else
	{
		
		//check for game over
		is_gameover();
		if(gameover)
		{
			
			draw_gameover();
			if(read_keys())
			{
				snake_init();
			}
		}
		else
		{
			//collision with food
			if( body[0][0] == food[0] && body[1][0] == food[1] )
			{
				score+= 1;
				food[0] = 100;
				food[1] = 100;
				//enlarge
				if(len<MAX_SNAKE_LEN)
					len++;
			}
			//read key values
			evalkeys(read_keys());
			//move
			move_snake();
			
			//add food after severel frames
			if(fc > 100)
			{
				add_food();
				fc=0;
			}
			
			//update screen
			lcd_clearAll(pix);
			if(read_keys() & (1<<4))
				lcd_write_string(pix, 7,0,"A");
			if(read_keys() & (1<<5))
				lcd_write_string(pix, 8,0,"B");
			draw_field();
			draw_food();
			draw_snake();
		}
	}	
	lcd_updateDisplay(pix);
}

int main(){
	lcd_setup();
	snake_hw_init();
	
	lcd_clearAll(pix);
	
	lcd_updateDisplay(pix);
	sei();
	snake_init();
	startscreen = 1;
	setup_timer();
	
	
	while(1)
	{
	}
}
