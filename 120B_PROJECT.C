//code works for 1 shift reg shifting through LEds
#include <avr/io.h>
#include "io.h"
#include "bit.h"
#include <avr/interrupt.h>
#include <stdio.h>


unsigned long int findGCD(unsigned long int a, unsigned long int b)
{
    unsigned long int c;
    while(1)
    {
        c = a%b;
        if(c==0){return b;}
        a = b;
        b = c;
    }
    return 0;
}

typedef struct _task 
{
    /*Tasks should have members that include: state, period,
    a measurement of elapsed time, and a function pointer.*/
    signed char state; //Task's current state
    unsigned long int period; //Task period
    unsigned long int elapsedTime; //Time elapsed since last task tick
    int (*TickFct)(int); //Task tick function
} task;

//VARIABLES~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Declare an array of tasks
static task task1, task2, task3, task4, task5, task6, task7, task8, task9, task10, task11, task12, task13, task14, task15, task16, task17, task18;
task *tasks[] = { &task1, &task2, &task3, &task4, &task5, &task6, &task7, &task8, &task9, &task10, &task11, &task12, &task13, &task14, &task15, &task16, &task17, &task18};
const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

const unsigned char GCD = 1;                   // PERIOD OF WHOLE STATE MACHINE 
const unsigned char ButtonPeriod = 10;
const unsigned char JoyStickPeriod = 10;
const unsigned char PlayerPeriod = 10;
const unsigned char EnemyPeriod = 10;
const unsigned char LEDPeriod = 1;
const unsigned char LaserPeriod = 10;
const unsigned char GamePeriod = 10;
const unsigned char PowerPeriod = 10;
const unsigned char MetiorePeriod = 10;

//Max limits of the Matrix
const unsigned char MAX = 8;         // LAST LED TO WHICH THE PLAYER CAN MOVE TO UP
const unsigned char MIN = 1;         // LAST LED TO WHICH THE PLAYER CAN MOVE TO DOWN

//Enemy movement speed
const unsigned char WaitSpeed = 3;	// how many times slower then lazer the enemies MOVES
const unsigned char BossSpeed = 10; // how many times slower than the lazer the boss moves
const unsigned char WaveTime = 30;		//Time between waves
const unsigned char win_condition = 10;

const unsigned char PowerSpawnTimer = 100; // how long it take for power to spawn
const unsigned char PowerLastTimer = 30; //how long power lasts
const unsigned char PowerOnFieldTimer = 40; // how long power is on the LEDBoard to be hit

const unsigned char MetioreFallRate = 10; // How much slower the Metiore falls compared to the Lazer
const unsigned char MetioreSpawnRate = 40;

unsigned char KEYPAD_INPUT;
unsigned char JoyStick_INPUT;       // 1 is up 2 is down 0 is nothing
unsigned char BUTTON_1;     //make 1 on realease of button press --> then bak to 0  RESTART/start
unsigned char BUTTON_2;     //make 1 on realease of button press --> then bak to 0  shoot laser


unsigned char GameStart;    // is turned on when Game start button pressed

unsigned char LIVES;        // set to 3  ** Decrement whenever enemy make it to first collumn
unsigned char WAVE;
unsigned char ENEMIESNUM;   // Number of enemies left for the wave
unsigned char ENEMIESON;    // if 1 odd, 2 even, 3 boss, 0 off
unsigned char power;		// if 1 there is a power up on the field
unsigned char metiore;		// if 1 there is a metiore on the field

unsigned char got_power;	// if 1 you have power_up tri shot, 2 if split shot, 0 if nothing



unsigned char RANDOM = 0;


unsigned char PlayerLOC;
unsigned char EnemyLoc1;
unsigned char EnemyLoc2;
unsigned char EnemyLoc3;
unsigned char LaserLoc1;
unsigned char LaserLoc2;
unsigned char LaserLoc3;

unsigned char PowerLoc;
unsigned char MetioreLoc = 0;

unsigned char GameStatus;

unsigned short cur_ADC;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Random spawning locations for Metiores and Power ups
unsigned char LOCATIONS[] = {18,27,42,35,63,54,62,11,12,17,16,48,26,19,50,28,60};
const unsigned char  LOC_ARR_SIZE = 17;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//RED ENEMIES
unsigned long red[] = { 0x8FF0FFDF, 0x8FF0FFBF, 0x8FF0FF7F, 0x8FF0FFFE, 0x8FF0FFFD, 0x8FF0FFFB, 0x8FF0FFF7, 0x8FF0FFEF, 0x4FF0FFFE, 0x4FF0FFFD, 0x4FF0FFFB, 0x4FF0FFF7, 0x4FF0FFEF, 0x4FF0FFDF, 0x4FF0FFBF, 0x4FF0FF7F, 0x2FF0FFFE, 0x2FF0FFFD, 0x2FF0FFFB, 0x2FF0FFF7, 0x2FF0FFEF, 0x2FF0FFDF, 0x2FF0FFBF, 0x2FF0FF7F, 0x1FF0FFFE, 0x1FF0FFFD, 0x1FF0FFFB, 0x1FF0FFF7, 0x1FF0FFEF, 0x1FF0FFDF, 0x1FF0FFBF, 0x1FF0FF7F, 0x0FF8FFFE, 0x0FF8FFFD, 0x0FF8FFFB, 0x0FF8FFF7, 0x0FF8FFEF, 0x0FF8FFDF, 0x0FF8FFBF, 0x0FF8FF7F, 0x0FF4FFFE, 0x0FF4FFFD, 0x0FF4FFFB, 0x0FF4FFF7, 0x0FF4FFEF, 0x0FF4FFDF, 0x0FF4FFBF, 0x0FF4FF7F, 0x0FF2FFFE, 0x0FF2FFFD, 0x0FF2FFFB, 0x0FF2FFF7, 0x0FF2FFEF, 0x0FF2FFDF, 0x0FF2FFBF, 0x0FF2FF7F, 0x0FF1FFFE, 0x0FF1FFFD, 0x0FF1FFFB, 0x0FF1FFF7, 0x0FF1FFEF, 0x0FF1FFDF, 0x0FF1FFBF, 0x0FF1FF7F};
//USE BLUE FOR PEOPLE & SHOT
unsigned long blue[] = {0x8FF0FEFF, 0x8FF0FDFF, 0x8FF0FBFF, 0x8FF0F7FF, 0x8FF0EFFF, 0x8FF0DFFF, 0x8FF0BFFF, 0x8FF07FFF, 0x4FF0FEFF, 0x4FF0FDFF, 0x4FF0FBFF, 0x4FF0F7FF, 0x4FF0EFFF, 0x4FF0DFFF, 0x4FF0BFFF, 0x4FF07FFF, 0x2FF0FEFF, 0x2FF0FDFF, 0x2FF0FBFF, 0x2FF0F7FF, 0x2FF0EFFF, 0x2FF0DFFF, 0x2FF0BFFF, 0x2FF07FFF, 0x1FF0FEFF, 0x1FF0FDFF, 0x1FF0FBFF, 0x1FF0F7FF, 0x1FF0EFFF, 0x1FF0DFFF, 0x1FF0BFFF, 0x1FF07FFF, 0x0FF8FEFF, 0x0FF8FDFF, 0x0FF8FBFF, 0x0FF8F7FF, 0x0FF8EFFF, 0x0FF8DFFF, 0x0FF8BFFF, 0x0FF87FFF, 0x0FF4FEFF, 0x0FF4FDFF, 0x0FF4FBFF, 0x0FF4F7FF, 0x0FF4EFFF, 0x0FF4DFFF, 0x0FF4BFFF, 0x0FF47FFF, 0x0FF2FEFF, 0x0FF2FDFF, 0x0FF2FBFF, 0x0FF2F7FF, 0x0FF2EFFF, 0x0FF2DFFF, 0x0FF2BFFF, 0x0FF27FFF, 0x0FF1FEFF, 0x0FF1FDFF, 0x0FF1FBFF, 0x0FF1F7FF, 0x0FF1EFFF, 0x0FF1DFFF, 0x0FF1BFFF, 0x0FF17FFF};
//USE FOR item/ metiorite
unsigned long green[] = { 0x8FE0FFFF, 0x8FD0FFFF, 0x8FB0FFFF, 0x8F70FFFF, 0x8EF0FFFF, 0x8DF0FFFF, 0x8BF0FFFF, 0x87F0FFFF, 0x4FE0FFFF, 0x4FD0FFFF, 0x4FB0FFFF, 0x4F70FFFF, 0x4EF0FFFF, 0x4DF0FFFF, 0x4BF0FFFF, 0x47F0FFFF, 0x2FE0FFFF, 0x2FD0FFFF, 0x2FB0FFFF, 0x2F70FFFF, 0x2EF0FFFF, 0x2DF0FFFF, 0x2BF0FFFF, 0x27F0FFFF, 0x1FE0FFFF, 0x1FD0FFFF, 0x1FB0FFFF, 0x1F70FFFF, 0x1EF0FFFF, 0x1DF0FFFF, 0x1BF0FFFF, 0x17F0FFFF, 0x0FE8FFFF, 0x0FD8FFFF, 0x0FB8FFFF, 0x0F78FFFF, 0x0EF8FFFF, 0x0DF8FFFF, 0x0BF8FFFF, 0x07F8FFFF, 0x0FE4FFFF, 0x0FD4FFFF, 0x0FB4FFFF, 0x0F74FFFF, 0x0EF4FFFF, 0x0DF4FFFF, 0x0BF4FFFF, 0x07F4FFFF, 0x0FE2FFFF, 0x0FD2FFFF, 0x0FB2FFFF, 0x0F72FFFF, 0x0EF2FFFF, 0x0DF2FFFF, 0x0BF2FFFF, 0x07F2FFFF, 0x0FE1FFFF, 0x0FD1FFFF, 0x0FB1FFFF, 0x0F71FFFF, 0x0EF1FFFF, 0x0DF1FFFF, 0x0BF1FFFF, 0x07F1FFFF };

//for metiore
unsigned long teal[] = {0x8FE0FEFF, 0x8FD0FDFF, 0x8FB0FBFF, 0x8F70F7FF, 0x8EF0EFFF, 0x8DF0DFFF, 0x8BF0BFFF, 0x87F07FFF, 0x4FE0FEFF, 0x4FD0FDFF, 0x4FB0FBFF, 0x4F70F7FF, 0x4EF0EFFF, 0x4DF0DFFF, 0x4BF0BFFF,0x47F07FFF, 0x2FE0FEFF, 0x2FD0FDFF, 0x2FB0FBFF, 0x2F70F7FF, 0x2EF0EFFF, 0x2DF0DFFF, 0x2BF0BFFF,0x27F07FFF,0x1FE0FEFF, 0x1FD0FDFF, 0x1FB0FBFF, 0x1F70F7FF, 0x1EF0EFFF, 0x1DF0DFFF, 0x1BF0BFFF, 0x17F07FFF, 0x0FE8FEFF, 0x0FD8FDFF, 0x0FB8FBFF, 0x0F78F7FF, 0x0EF8EFFF, 0x0DF8DFFF, 0x0BF8BFFF,0x07F87FFF, 0x0FE4FEFF, 0x0FD4FDFF, 0x0FB4FBFF, 0x0F74F7FF, 0x0EF4EFFF, 0x0DF4DFFF, 0x0BF4BFFF, 0x07F47FFF, 0x0FE2FEFF, 0x0FD2FDFF, 0x0FB2FBFF, 0x0F72F7FF, 0x0EF2EFFF, 0x0DF2DFFF, 0x0BF2BFFF,0x07F27FFF,0x0FE1FEFF,0x0FD1FDFF, 0x0FB1FBFF, 0x0F71F7FF, 0x0EF1EFFF, 0x0DF1DFFF, 0x0BF1BFFF, 0x07F17FFF};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void transmit_data(unsigned long data)
{
	int i;
	PORTC = 0x00;
	for(i = 32; i >= 0; i--)
	{
		// Sets SRCLR to 1 allowing data to be set
		// Also clears SRCLK in preparation of sending data
		PORTC = 0x08;
		// set SER = next bit of data to be sent.
		PORTC |= ((data >> i) & 0x01);
		// set SRCLK = 1. Rising edge shifts next bit of data into the shift register
		PORTC |= 0x04;
	}
	// set RCLK = 1. Rising edge copies data from “Shift” register to “Storage” register
	PORTC |= 0x02;
	// clears all lines in preparation of a new transmission
	
}



volatile unsigned char TimerFlag = 0; // TimerISR() sets this to 1. C programmer should clear to 0.
// Internal variables for mapping AVR's ISR to our cleaner TimerISR model.
unsigned long _avr_timer_M = 1; // Start count from here, down to 0. Default 1 ms.
unsigned long _avr_timer_cntcurr = 0; // Current internal count of 1ms ticks

void TimerOn()
{
	// AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B;// bit3 = 0: CTC mode (clear timer on compare)
	
	// bit2bit1bit0=011: pre-scaler /64
	// 00001011: 0x0B
	// SO, 8 MHz clock or 8,000,000 /64 = 125,000 ticks/s
	// Thus, TCNT1 register will count at 125,000 ticks/s
	// AVR output compare register OCR1A.
	
	OCR1A = 125; // Timer interrupt will be generated when TCNT1==OCR1A
	
	// We want a 1 ms tick. 0.001 s * 125,000 ticks/s = 125
	// So when TCNT1 register equals 125,
	// 1 ms has passed. Thus, we compare to 125.
	// AVR timer interrupt mask register
	
	TIMSK1 = 0x02; // bit1: OCIE1A -- enables compare match interrupt
	
	//Initialize avr counter
	TCNT1=0;
	_avr_timer_cntcurr = _avr_timer_M;
	
	// TimerISR will be called every _avr_timer_cntcurr milliseconds
	//Enable global interrupts
	SREG |= 0x80; // 0x80: 1000000
}

void TimerOff()
{
	TCCR1B = 0x00; // bit3bit1bit0=000: timer off
}

void TimerISR()
{
	TimerFlag = 1;
}

// In our approach, the C programmer does not touch this ISR, but rather TimerISR()
ISR(TIMER1_COMPA_vect)
{
	// CPU automatically calls when TCNT1 == OCR1 (every 1 ms per TimerOn settings)
	_avr_timer_cntcurr--; // Count down to 0 rather than up to TOP
	
	if (_avr_timer_cntcurr == 0)
	{ // results in a more efficient compare
		TimerISR(); // Call the ISR that the user uses
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

// Set TimerISR() to tick every M ms
void TimerSet(unsigned long M)
{
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}


//JOYSTICK INPUT~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ADC_init() {
	ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
	// ADEN: setting this bit enables analog-to-digital conversion.
	// ADSC: setting this bit starts the first conversion.
	// ADATE: setting this bit enables auto-triggering. Since we are
	// in Free Running Mode, a new conversion will trigger whenever
	// the previous conversion completes.
}
enum JoyStickState { startJS,waitJS } StickState;
int JoyStick (int StickState)
{
    switch(StickState)
    {
        case startJS:
            StickState = waitJS;
            break;
        case waitJS:
            break;
        default:
            break;
    }
    switch(StickState)
    {
        case startJS:
            break;
        case waitJS:
        /*
        - use this variable JoyStick_INPUT
        - use an if statement and where joystick will be plugged in
            use that input to get left up down
        
        */
			if((char)ADC >= 0xF0)
			{
				JoyStick_INPUT = 1;
			}
			else if((char)ADC <= 0x13)
			{
				JoyStick_INPUT = 2;
			}
			else
			{
				JoyStick_INPUT = 0;
			}
            break;
        default:
            break;
        
    }
    return StickState;
}


// Button INPUT SM
//Start/Restart
enum Button1_STATES{start_button1, wait_button1, on_button1, off_button1}BUT1_state;
int Button1_SM(int BUT1_state)
{
	unsigned char button1;
	switch(BUT1_state)
	{
		case start_button1:
		BUT1_state = wait_button1;
		break;
		case wait_button1:
		button1 = ~PINB & 0x01; // button is connected to A0
		// 2) Perform Computation
		if (button1)
		{			
			BUT1_state = on_button1;
		}
		break;
		case on_button1:
		button1 = ~PINB & 0x01; // button is connected to A0
		// 2) Perform Computation
		if (!button1)
		{
			BUT1_state = off_button1;
		}
		break;
		case off_button1:
		BUT1_state = wait_button1;
		break;
		default:
		break;
	}
	switch(BUT1_state)
	{
		case start_button1:
		button1 = 0;
		BUTTON_1 = 0;
		break;
		case wait_button1:
		BUTTON_1 = 0;
		break;
		case on_button1:
		break;
		case off_button1:
		BUTTON_1 = 1;
		break;
	}
	return BUT1_state;
}
// LED MATRIX SM
enum StateLED{ start_LED, LEDB, LEDR, LEDG} LEDMatrix;
int LEDSM(int LEDMatrix)
{
	switch (LEDMatrix)
	{
		case start_LED:
			if(GameStart == 1)//GameStart)
			{
				PlayerLOC = 1;
				LEDMatrix = LEDB;
			}
			break;
		case LEDB:
			LEDMatrix = LEDR;
			break;
		
		case LEDR:
			if((PowerLoc != 0) || (MetioreLoc != 0))
			{
				LEDMatrix = LEDG;
			}
			else
			{
				LEDMatrix = LEDB;
			}
			break;
		case LEDG:
			LEDMatrix = LEDB;
			break;
		default:
			LEDMatrix = start_LED;
			break;
	}
	switch (LEDMatrix)
	{
		case start_LED:
			break;
		case LEDB:
			if(PlayerLOC != 0)
			{
				transmit_data(blue[PlayerLOC - 1]);
			}			
			if(LaserLoc1 != 0)
			{
				transmit_data(blue[LaserLoc1 - 1]);
			}
			if(LaserLoc2 != 0)
			{
				transmit_data(blue[LaserLoc2 - 1]);
			}
			if(LaserLoc3 != 0)
			{
				transmit_data(blue[LaserLoc3 - 1]);
			}
			
			if(GameStatus == 1)
			{
				transmit_data(0xF00FFFFF);
			}
			break;
		
		case LEDR:
	
			if(EnemyLoc1 > 8)
			{
				transmit_data(red[EnemyLoc1 - 1]);
			}
			if(EnemyLoc2 > 8)
			{
				transmit_data(red[EnemyLoc2 - 1]);
			}
			if(EnemyLoc3 > 8)
			{
				transmit_data(red[EnemyLoc3 - 1]);
			}
			if(GameStatus == 2)
			{
				transmit_data(0xF00FFF00);
			}
			
			break;
		case LEDG:
			if(PowerLoc > 0)
			{
				transmit_data(green[PowerLoc - 1]);
			}
			if(MetioreLoc > 0)
			{
				transmit_data(teal[MetioreLoc -1]);
			}
			break;		
		default:
			break;
			
	}
	return LEDMatrix;
}

//PLAYER MOVEMENT SM~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

enum PlayerMoveState { startPM, waitPM,up, down } PlayerState;
int PlayerSM(int PlayerState)
{
	switch(PlayerState)
	{
		case startPM:
		if(GameStart)
		{
			PlayerState = waitPM;
		}
		break;
		case waitPM:
		if(GameStart)
		{
			if(JoyStick_INPUT == 1)
			{
				PlayerState = up;
			}
			if(JoyStick_INPUT == 2)
			{
				PlayerState = down;
			}
		}
		else
		{
			PlayerState = startPM;
		}
		break;
		case up:
		PlayerState = waitPM;
		break;
		case down:
		PlayerState = waitPM;
		break;
		default:
		PlayerState = startPM;
		break;
	}
	switch(PlayerState)
	{
		case startPM:
		PlayerLOC = 0;
		break;
		case waitPM:
		if(PlayerLOC == 0)
		{
			PlayerLOC = 1;
		}
		break;
		case up:
		if( PlayerLOC != MAX)
		{
			//PUT IN CODE TO MOVE PLAYER ONE LED RIGHT
			PlayerLOC = PlayerLOC + 1;
		}
		break;
		case down:
		if( PlayerLOC != MIN)
		{
			//PUT IN CODE TO MOVE PLAYER ONE LED LEFT
			PlayerLOC = PlayerLOC - 1;
		}
		break;
		default:
		break;
	}
	return PlayerState;
}

//Game SM~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
enum StateGame{ start_game, wait_game, lose_game, win_game, between_game} GAME;
int GameSM(int GAME)
{
	switch(GAME)
	{
		case start_game:
		if(BUTTON_1)
		{
			GameStart = 1;
			PlayerLOC = 1;
			GAME = wait_game;
		}
		break;
		case wait_game:
		if(LIVES == 0)
		{
			GAME = lose_game;
			GameStart = 0;
		}
		else if(WAVE >  win_condition)
		{
			GAME = win_game;
			GameStart = 0;
		}
		break;
		case lose_game:
		if(BUTTON_1)
		{
			GAME = between_game;
			GameStart = 1;
		}
		break;
		case win_game:
		if(BUTTON_1)
		{
			GAME = between_game;
			GameStart = 1;
		}
		break;
		case between_game:
		GAME = wait_game;
		break;
		default:
		GAME = start_game;
		break;
	}
	switch(GAME)
	{
		case start_game:
		GameStatus = 0;
		LIVES = 3;
		break;
		case wait_game:
		break;
		case lose_game:
		WAVE = 0;
		GameStatus = 2;
		// LOSE GAME ACTION DO HERE
		break;
		case win_game:
		WAVE = 0;
		GameStatus = 1;
		//DO WIN ACTION HERE
		break;
		case between_game:
		GameStatus = 0;
		LIVES = 3;
		//RESET ALL THE STATES TO START STATE except GAMESM
		break;
		default:
		break;
	}
	return GAME;
}

//BUTTON 2
enum Button2_STATES{start_button2, wait_button2, on_button2, off_button2}BUT2_state;
int Button2_SM(int BUT2_state)
{
	unsigned char button2;
	switch(BUT2_state)
	{
		case start_button2:
		BUT2_state = wait_button2;
		break;
		case wait_button2:
		button2 = ~PINB & 0x02; // button is connected to A0
		// 2) Perform Computation
		if (button2)
		{
			BUT2_state = on_button2;
		}
		break;
		case on_button2:
		button2 = ~PINB & 0x02; // button is connected to A0
		// 2) Perform Computation
		if (!button2)
		{
			BUT2_state = off_button2;
		}
		break;
		case off_button2:
		BUT2_state = wait_button2;
		break;
		default:
		break;
	}
	switch(BUT2_state)
	{
		case start_button2:
		button2 = 0;
		BUTTON_2 = 0;
		break;
		case wait_button2:
		BUTTON_2 = 0;
		break;
		case on_button2:
		break;
		case off_button2:
		BUTTON_2 = 1;
		break;
	}
	return BUT2_state;
}


//LAZER you can shoot only one lazer at a time unless power up is avaliable only if all lazer shots go away you can shoot again
enum Lazers { start_lazer, wait_lazer, shoot_lazer, shoot_duo_lazer1, shoot_duo_lazer2, shoot_split_lazer, reload_lazer} LazerState; //ADD different power up states
int LazerSM (int LazerState)
{
	switch(LazerState)
	{
		case start_lazer:
			if(GameStart == 1)
			{
				LazerState = wait_lazer;
			}
			break;
		case wait_lazer:
			if(GameStart)
			{
				if(BUTTON_2)
				{
					//ADD STATEMENTS TO CHANGE TO DIFFERNET LAZER STATES IF POWER UP
					if(got_power == 0)
					{	
						LazerState = shoot_lazer;
					}
					else if(got_power == 1)
					{
						LazerState = shoot_duo_lazer1;
					}
					else if(got_power == 2)
					{
						LazerState = shoot_split_lazer;
					}
				}
			}
			else
			{
				LazerState = start_lazer;
			}
			break;
		case shoot_lazer:
			LazerState = reload_lazer;
			break;
		case shoot_duo_lazer1:
			LazerState = shoot_duo_lazer2;
			break;
		case shoot_duo_lazer2:
			LazerState = reload_lazer;
			break;
		case shoot_split_lazer:
			LazerState = reload_lazer;
			break;
		case reload_lazer:
			if((LaserLoc1 == 0) && (LaserLoc2 == 0) && (LaserLoc3 == 0) )     // CHECK FOR Laserloc 2 and 3
			{
				LazerState = wait_lazer;
			}
			break;
		default:
		LazerState = start_lazer;
		break;
	}
	switch(LazerState)
	{
		case start_lazer:
		break;
		case wait_lazer:
		break;
		case shoot_lazer:
		// ADD CODE TO START LaserLoc1 = PlayerLoc + 1
			LaserLoc1 = PlayerLOC + 8;
			break;
		case shoot_duo_lazer1:
			LaserLoc1 = PlayerLOC + 8;
			break;
		case shoot_duo_lazer2:
			if(LaserLoc1 != 0)
			{
				LaserLoc1 = LaserLoc1 + 8;
			}
			LaserLoc2 = PlayerLOC +8;
			break;
		case shoot_split_lazer:
			if(PlayerLOC % 8 != 0)
			{
				LaserLoc3 = PlayerLOC + 9;
			}
			if(PlayerLOC %8 != 1)
			{
				LaserLoc2 = PlayerLOC + 7;
			}
			LaserLoc1 = PlayerLOC+ 8;
			break;
		case reload_lazer:
			// add code to increase LaserLoc if LaserLoc != 0
			if(LaserLoc1 != 0)
			{
				LaserLoc1 = LaserLoc1 + 8;
			}
			if(LaserLoc2 != 0)
			{
				LaserLoc2 = LaserLoc2 + 8;
			}
			if(LaserLoc3 != 0)
			{
				LaserLoc3 = LaserLoc3 + 8;
			}
			//check if Laser is still in the LED matrix
			if(LaserLoc1 > 64)
			{
				LaserLoc1 = 0;
			}
			if(LaserLoc2 > 64)
			{
				LaserLoc2 = 0;
			}
			if(LaserLoc3 > 64)
			{
				LaserLoc3 = 0;
			}
			break;
		default:
		break;
	}
	return LazerState;
}

//Power SM if 
//*power = 1 duo *power = 2 split *power = 0 normal
enum Powers {start_power, wait_power, spawn_power1, hit_power1, spawn_power2, hit_power2, power_active}PowerState;
int PowerSM ( int PowerState)
{
	static char time;
	static char i;
	switch(PowerState)
	{
		case start_power:
			if(GameStart == 1)
			{
				PowerState = wait_power;
			}
			break;
		case wait_power:
			if(GameStart == 1)
			{
				if(time >= PowerSpawnTimer)
				{
					if(i % 3 == 1)
					{
						PowerState = spawn_power2;
						i++;
					}
					else
					{
						PowerState = spawn_power1;
						i++;
					}
					time = 0;
				}
			}
			else
			{
				PowerState = start_power;
			}
			break;
		case spawn_power1:
			PowerState = hit_power1;
			break;
		case hit_power1:
			if(LIVES <= 0 )
			{
				PowerState = start_power;
			}
			if(time >= PowerOnFieldTimer )
			{
				PowerState = wait_power;
				PowerLoc = 0;
			}
			else if(PowerLoc == LaserLoc1)
			{	
				PowerLoc = 0;
				LaserLoc1 = 0;
				got_power = 1;
				PowerState = power_active;
			}
			break;
		case spawn_power2:
			PowerState = hit_power2;
			break;
		case hit_power2:
			if(LIVES <= 0 )
			{
				PowerState = start_power;
			}
			if(time >= PowerOnFieldTimer )
			{
				PowerState = wait_power;
				PowerLoc = 0;
				time = 0;
			}
			else if(PowerLoc == LaserLoc1)
			{
				PowerLoc = 0;
				LaserLoc1 = 0;
				got_power = 2;
				PowerState = power_active;
				time = 0;
			}
			break;
		case power_active:
			if(time >= PowerLastTimer)
			{
				PowerState = wait_power;
				time = 0;
			}
			break;
		default:
			break;
	}
	switch(PowerState)
	{
		case start_power:
			time = 0;
			i = 0;
			PowerLoc = 0;
			break;
		case wait_power:
			got_power = 0;
			time ++;
			break;
		case spawn_power1:
			PowerLoc = (((i * 13) % 8) * 5) + 10;
			break;
		case hit_power1:
			time ++;
			break;
		case spawn_power2:
			PowerLoc = (((i * 13) % 8) * 5) + 10;
			break;
		case hit_power2:
			time ++;
			break;
		case power_active:
			time ++;
			break;
		default:
			break;
	}
	return PowerState;
}

//Metiore SM
enum Metiores {start_met, wait_met, spawn_met, hit_met}MetioreState;
int MetioreSM( int MetioreState)
{
	static char i;
	static char j;
	switch( MetioreState)
	{
		case start_met:
			if(GameStart == 1)
			{
				MetioreState = wait_met;
			}
			break;
		case wait_met:
			if(GameStart == 1)
			{	
				if(i >= MetioreSpawnRate)
				{
					MetioreState = spawn_met;
					i =0;
				}
			}
			else
			{
				MetioreState = start_met;
			}
			break;
		case spawn_met:
			MetioreState = hit_met;
			break;
		case hit_met:
			if(GameStart == 1)
			{
				if(LaserLoc1 == MetioreLoc)
				{
					LaserLoc1 = 0;
					MetioreLoc = 0;
					MetioreState = wait_met;
				}
				else if(LaserLoc2 == MetioreLoc)
				{
					LaserLoc2 = 0;
					MetioreLoc = 0;
					MetioreState = wait_met;
				}
				else if(LaserLoc3 == MetioreLoc)
				{
					LaserLoc3 = 0;
					MetioreLoc = 0;
					MetioreState = wait_met;
				}
				else if(MetioreLoc == PlayerLOC)
				{
					LIVES --;
					MetioreLoc = 0;
					MetioreState = wait_met;
				}
				else if(MetioreLoc < 8 )
				{
					MetioreLoc = 0;
					MetioreState = wait_met;
				}
			}
			else
			{
				MetioreState = start_met;
			}
			break;
		default:
			break;
	}
	switch(MetioreState)
	{
		case start_met:
			MetioreLoc = 0;
			j = 0;
			i = 0;
			break;
		case wait_met:
			i++;
			break;
		case spawn_met:
			i = 0;
			j++;
			if(j%8 == 0)
			{
				j++;
			}
			MetioreLoc = ((j * 13)%8 * 7)+5;
			break;
		case hit_met:
			i++;
			if( i >= MetioreFallRate)
			{
				MetioreLoc = MetioreLoc - 8;
				i = 0;
			}
			break;
		default:
		break;
	}
	
	return MetioreState;
}

//ENEMIES SM~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// CONTROLER FOR ENEMY WAVES
enum EnemyWaveState { startEW, waitEW, WaitBetweenWave, odd, even, boss } EnemyState;
int EnemyWave(int EnemyState)
{
	static unsigned char i;
	static unsigned char k;
	switch(EnemyState)
	{
		case startEW:
		if(GameStart == 1)
		{
			EnemyState = waitEW;
		}
		break;
		case waitEW:
		if(GameStart == 1)
		{
			if(ENEMIESNUM == 0)
			{
				if(((WAVE+1)%6) == 0)
				{
					EnemyState = boss;
				}
				else if(k == 0)
				{	
					EnemyState = even;
					k = 1;
				}
				else if(k == 1)
				{
					EnemyState = odd;
					k = 0;
				}
			}
		}
		else
		{
			EnemyState = startEW;
		}
		break;
		case odd:
		EnemyState = WaitBetweenWave;
		break;
		case even:
		EnemyState = WaitBetweenWave;
		break;
		case boss:
		EnemyState = WaitBetweenWave;
		break;
		case WaitBetweenWave:
			if(GameStart == 1)
			{
				if(ENEMIESNUM == 0)
				{
					if(i == 0)
					{
						WAVE ++;
					}
					if(i > WaveTime)
					{
						EnemyState = waitEW;
					}
					i++;
				}
			}
			else
			{
				EnemyState = startEW;
			}
			break;
		default:
		EnemyState = waitEW;
		break;
	}
	switch(EnemyState)
	{
		case startEW:
		i = 0;
		k = 0;
		ENEMIESNUM = 0;
		ENEMIESON = 0;
		WAVE = 0;
		EnemyLoc1 = 0;
		EnemyLoc2 = 0;
		EnemyLoc3 = 0;
		break;
		case waitEW:
		i = 0;
		break;
		case odd:
		ENEMIESON = 1;
		break;
		case even:
		ENEMIESON = 2;
		break;
		case boss:
		ENEMIESON = 3;
		break;
		case WaitBetweenWave:
		ENEMIESON = 0;
		break;
		default:
		break;
	}
	return EnemyState;
}

// Enemies SM must increment ENEMIESNUM count by 1 for each SM
// decrement when enemy dies


// ODD ENEMIES          GO DOWN LEFT TO RIGHT
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//ENEMY ODD 1
enum EnemyWaveOdd1 { startEOdd1, On1, Oddmove1} OddEnemyState1;
int OddEnemyWave1(int OddEnemyState1)
{
	static char wait;
	switch(OddEnemyState1)
	{
		case startEOdd1:
		if(ENEMIESON == 1)
		{
			OddEnemyState1 = On1;
		}
		break;
		case On1:
		OddEnemyState1 = Oddmove1;
		break;
		
		case Oddmove1:
		if(LIVES <= 0)
		{
			OddEnemyState1 = startEOdd1;
			EnemyLoc1 = 0;
		}
		if(EnemyLoc1 == 0)
		{
			OddEnemyState1 = startEOdd1;
		}
		else if( EnemyLoc1 == LaserLoc1 || EnemyLoc1 == LaserLoc2 || EnemyLoc1 == LaserLoc3 || EnemyLoc1 < 8)
		{
			if(EnemyLoc1 <= 8)
			{
				LIVES --;
			}
			else if( EnemyLoc1 == LaserLoc1 )
			{
				LaserLoc1 = 0;
			}
			else if(  EnemyLoc1 == LaserLoc2 )
			{
				LaserLoc2 = 0;
			}
			else
			{
				LaserLoc3 = 0;
			}
			OddEnemyState1 = startEOdd1;
			ENEMIESNUM = ENEMIESNUM -1;
			EnemyLoc1 = 0;
		}
		break;
		default:
		break;
	}
	switch(OddEnemyState1)
	{
		case startEOdd1:
		break;
		case On1:
		wait = 0;
		ENEMIESNUM = ENEMIESNUM + 1;
		EnemyLoc1 = 63;
		break;
		case Oddmove1:
		// write whatever code needed to move EnemyLoc1 by one
			if(wait >= WaitSpeed)
			{
				EnemyLoc1 = EnemyLoc1 - 1;
				wait = 0;
			}
			wait ++;
		break;
		
		default:
		break;
	}
	return OddEnemyState1;
}

//ENEMY ODD 2
enum EnemyWaveOdd2 { startEOdd2, On2, Oddmove2} OddEnemyState2;
int OddEnemyWave2(int OddEnemyState2)
{
	static char wait;
	switch(OddEnemyState2)
	{
		case startEOdd2:
		if(ENEMIESON == 1)
		{
			OddEnemyState2 = On2;
		}
		break;
		case On2:
		OddEnemyState2 = Oddmove2;
		break;
		case Oddmove2:
		if(LIVES <= 0)
		{
			OddEnemyState2 = startEOdd2;
			EnemyLoc2 = 0;
		}
		if(EnemyLoc2 == 0)
		{
			OddEnemyState2 = startEOdd2;
		}
		else if(EnemyLoc2 == LaserLoc1 ||EnemyLoc2 == LaserLoc2 ||EnemyLoc2 == LaserLoc3 || EnemyLoc2 < 8)
		{
			if(EnemyLoc2 <= 8)
			{
				LIVES --;
			}
			else if( EnemyLoc2 == LaserLoc1 )
			{
				LaserLoc1 = 0;
			}
			else if( EnemyLoc2 == LaserLoc2)
			{
				LaserLoc2 = 0;
			}
			else 
			{
				LaserLoc3 = 0;
			}
			OddEnemyState2 = startEOdd2;
			ENEMIESNUM = ENEMIESNUM -1;
			EnemyLoc2 = 0;
		}
		break;
		default:
		break;
	}
	switch(OddEnemyState2)
	{
		case startEOdd2:

		break;
		case On2:
		wait = 0;
		ENEMIESNUM = ENEMIESNUM + 1;
		EnemyLoc2 = 60; 
		break;
		case Oddmove2:
			if(wait >= WaitSpeed)
			{
				EnemyLoc2 = EnemyLoc2 - 1;
				wait = 0;
			}
			wait ++;
				//use for loop to make EnemyLoc1 be set to highest and go down
		break;
		default:
		break;
	}
	return OddEnemyState2;
}

//ENEMY ODD 3
enum EnemyWaveOdd3 { startEOdd3, On3, Oddmove3} OddEnemyState3;
int OddEnemyWave3(int OddEnemyState3)
{
	static char wait;
	switch(OddEnemyState3)
	{
		case startEOdd3:
		if(ENEMIESON == 1)
		{
			OddEnemyState3 = On3;
		}
		break;
		case On3:
		OddEnemyState3 =Oddmove3;
		break;
		
		case Oddmove3:
		if(LIVES <= 0)
		{
			OddEnemyState3 = startEOdd3;
			EnemyLoc3 = 0;
		}
		if(EnemyLoc3 == 0)
		{
			OddEnemyState3 = startEOdd1;
		}
		else if(EnemyLoc3 == LaserLoc1 ||EnemyLoc3 == LaserLoc3 ||EnemyLoc3 == LaserLoc2 || EnemyLoc3 < 8)
		{
			if(EnemyLoc3 <= 8)
			{
				LIVES --;
			}
			else if(EnemyLoc3 == LaserLoc1 )
			{
				LaserLoc1 = 0;
			}
			else if(EnemyLoc3 == LaserLoc2 )
			{
				LaserLoc2 = 0;
			}
			else 
			{
				LaserLoc3 = 0;
			}
			OddEnemyState3 = startEOdd3;
			ENEMIESNUM = ENEMIESNUM -1;
			EnemyLoc3 = 0;
		}
		break;
		default:
		break;
	}
	switch(OddEnemyState3)
	{
		case startEOdd3:
		break;
		case On3:
		//use for loop to make EnemyLoc1 be set to highest and go down
		ENEMIESNUM = ENEMIESNUM + 1;
		EnemyLoc3 = 58;
		wait = 0;
		break;
		case Oddmove3:
			//use for loop to make EnemyLoc1 be set to highest and go down
			if(wait >= WaitSpeed)
			{
				EnemyLoc3 = EnemyLoc3 - 1;
				wait = 0;
			}
			wait ++;
			break;
		default:
		break;
	}
	return OddEnemyState3;
}


// EVEN ENEMIES          Diagonal
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//ENEMY Even1
enum EnemyWaveEven1 { startEEven1, OnEven1, Evenmove1} EvenEnemyState1;
int EvenEnemyWave1(int EvenEnemyState1)
{
	static char wait;
	static char i;
	switch(EvenEnemyState1)
	{
		case startEEven1:
		if(ENEMIESON == 2)
		{
			EvenEnemyState1 = OnEven1;
		}
		break;
		case OnEven1:
		EvenEnemyState1 = Evenmove1;
		break;
		
		case Evenmove1:
		if(LIVES <= 0)
		{
			EvenEnemyState1 = startEEven1;
			EnemyLoc1 = 0;
		}
		if(EnemyLoc1 == 0)
		{
			EvenEnemyState1 = startEEven1;
		}
		else if( EnemyLoc1 == LaserLoc1 || EnemyLoc1 == LaserLoc2 || EnemyLoc1 == LaserLoc3 || EnemyLoc1 < 8)
		{
			if(EnemyLoc1 <= 8)
			{
				LIVES --;
			}
			else if( EnemyLoc1 == LaserLoc1 )
			{
				LaserLoc1 = 0;
			}
			else if(  EnemyLoc1 == LaserLoc2 )
			{
				LaserLoc2 = 0;
			}
			else
			{
				LaserLoc3 = 0;
			}
			EvenEnemyState1 = startEEven1;
			ENEMIESNUM = ENEMIESNUM -1;
			EnemyLoc1 = 0;
		}
		break;
		default:
		break;
	}
	switch(EvenEnemyState1)
	{
		case startEEven1:
		i = 0;
		break;
		case OnEven1:
		wait = 0;
		ENEMIESNUM = ENEMIESNUM + 1;
		EnemyLoc1 = 63;
		i = 0;
		break;
		case Evenmove1:
		// write whatever code needed to move EnemyLoc1 by one
		if(wait >= WaitSpeed)
		{
			if(i >= 1)
			{
				EnemyLoc1 = EnemyLoc1 + 7;
				i = 0;
			}
			else
			{
				EnemyLoc1 = EnemyLoc1 - 9;
				i++;
			}
			wait = 0;
		}
		wait ++;
		break;
		
		default:
		break;
	}
	return EvenEnemyState1;
}
//ENEMY EVEN 2
enum EnemyWaveEven2 { startEEven2, OnEven2, Evenmove2} EvenEnemyState2;
int EvenEnemyWave2(int EvenEnemyState2)
{
	static char wait;
	static char i;
	switch(EvenEnemyState2)
	{
		case startEEven2:
		if(ENEMIESON == 2)
		{
			EvenEnemyState2 = OnEven2;
		}
		break;
		case OnEven2:
		EvenEnemyState2 = Evenmove2;
		break;
		
		case Evenmove2:
		if(LIVES <= 0)
		{
			EvenEnemyState2 = startEEven2;
			EnemyLoc2 = 0;
		}
		if(EnemyLoc2 == 0)
		{
			EvenEnemyState2 = startEEven2;
		}
		else if( EnemyLoc2 == LaserLoc1 || EnemyLoc2 == LaserLoc2 || EnemyLoc2 == LaserLoc3 || EnemyLoc2 < 8)
		{
			if(EnemyLoc2 <= 8)
			{
				LIVES --;
			}
			else if( EnemyLoc2 == LaserLoc1 )
			{
				LaserLoc1 = 0;
			}
			else if(  EnemyLoc2 == LaserLoc2 )
			{
				LaserLoc2 = 0;
			}
			else
			{
				LaserLoc3 = 0;
			}
			EvenEnemyState2 = startEEven2;
			ENEMIESNUM = ENEMIESNUM -1;
			EnemyLoc2 = 0;
		}
		break;
		default:
		break;
	}
	switch(EvenEnemyState2)
	{
		case startEEven2:
		i = 0;
		break;
		case OnEven2:
		wait = 0;
		ENEMIESNUM = ENEMIESNUM + 1;
		EnemyLoc2 = 53;
		i = 0;
		break;
		case Evenmove2:
		// write whatever code needed to move EnemyLoc2 by one
		if(wait >= WaitSpeed)
		{
			if(i >= 1)
			{
				EnemyLoc2 = EnemyLoc2 + 7;
				i = 0;
			}
			else
			{
				EnemyLoc2 = EnemyLoc2 - 9;
				i++;
			}
			wait = 0;
		}
		wait ++;
		break;
		
		default:
		break;
	}
	return EvenEnemyState2;
}

enum EnemyWaveEven3 { startEEven3, OnEven3, Evenmove3} EvenEnemyState3;
int EvenEnemyWave3(int EvenEnemyState3)
{
	static char wait;
	static char i;
	switch(EvenEnemyState3)
	{
		case startEEven3:
		if(ENEMIESON == 2)
		{
			EvenEnemyState3 = OnEven3;
		}
		break;
		case OnEven3:
		EvenEnemyState3 = Evenmove3;
		break;
		
		case Evenmove3:
		if(LIVES <= 0)
		{
			EvenEnemyState3 = startEEven3;
			EnemyLoc3 = 0;
		}
		if(EnemyLoc3 == 0)
		{
			EvenEnemyState3 = startEEven3;
		}
		else if( EnemyLoc3 == LaserLoc1 || EnemyLoc3 == LaserLoc2 || EnemyLoc3 == LaserLoc3 || EnemyLoc3 < 8)
		{
			if(EnemyLoc3 <= 8)
			{
				LIVES --;
			}
			else if( EnemyLoc3 == LaserLoc1 )
			{
				LaserLoc1 = 0;
			}
			else if(  EnemyLoc3 == LaserLoc2 )
			{
				LaserLoc2 = 0;
			}
			else
			{
				LaserLoc3 = 0;
			}
			EvenEnemyState3 = startEEven3;
			ENEMIESNUM = ENEMIESNUM -1;
			EnemyLoc3 = 0;
		}
		break;
		default:
		break;
	}
	switch(EvenEnemyState3)
	{
		case startEEven3:
		i = 0;
		break;
		case OnEven3:
		wait = 0;
		ENEMIESNUM = ENEMIESNUM + 1;
		EnemyLoc3 = 43;
		i = 0;
		break;
		case Evenmove3:
		// write whatever code needed to move EnemyLoc3 by one
		if(wait >= WaitSpeed)
		{
			if(i >= 1)
			{
				EnemyLoc3 = EnemyLoc3 + 7;
				i = 0;
			}
			else
			{
				EnemyLoc3 = EnemyLoc3 - 9;
				i++;
			}
			wait = 0;
		}
		wait ++;
		break;
		
		default:
		break;
	}
	return EvenEnemyState3;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//BOSS ENEMIES BIG
enum EnemyWaveBoss { startEBoss, OnBoss, moveBoss} EnemyStateBoss;
int EnemyWaveBoss(int EnemyStateBoss)
{
	static char wait;
	switch(EnemyStateBoss)
	{
		case startEBoss:
		if(ENEMIESON == 3)
		{
			EnemyStateBoss = OnBoss;
		}
		break;
		case OnBoss:
		EnemyStateBoss = moveBoss;
		break;
		
		case moveBoss:
			if(LIVES <= 0)
			{
				EnemyStateBoss = startEBoss;
				EnemyLoc3 = 0;
				EnemyLoc2 = 0;
				EnemyLoc1 = 0;
				ENEMIESNUM = 0;
			}
			else if( (EnemyLoc3 != 0) && (EnemyLoc3 == LaserLoc1 || EnemyLoc3 == LaserLoc2 || EnemyLoc3 == LaserLoc3  || EnemyLoc3 < 8))
			{
				if(EnemyLoc3 <= 8)
				{
					LIVES --;
					EnemyLoc2 = 0;
					EnemyLoc1 = 0;					
					EnemyStateBoss = startEBoss;
					ENEMIESNUM = 0;
				}
				else if( EnemyLoc3 == LaserLoc1 )
				{
					LaserLoc1 = 0;
				}
				else if(  EnemyLoc3 == LaserLoc2 )
				{
					LaserLoc2 = 0;
				}
				else
				{
					LaserLoc3 = 0;
				}
				EnemyLoc3 = 0;
			}
			else if((EnemyLoc3 == 0)&& (EnemyLoc2 != 0) && (EnemyLoc2 == LaserLoc1 || EnemyLoc2 == LaserLoc2 || EnemyLoc2 == LaserLoc3  || EnemyLoc2 < 8) )
			{
				if(EnemyLoc2 <= 8)
				{
					LIVES --;
					EnemyLoc1 = 0;					
					EnemyStateBoss = startEBoss;
					ENEMIESNUM = 0;
				}
				else if( EnemyLoc2 == LaserLoc1 )
				{
					LaserLoc1 = 0;
				}
				else if(  EnemyLoc2 == LaserLoc2 )
				{
					LaserLoc2 = 0;
				}
				else
				{
					LaserLoc3 = 0;
				}
				EnemyLoc2 = 0;
			}
			else if((EnemyLoc2 == 0) && (EnemyLoc1 == LaserLoc1 || EnemyLoc1 == LaserLoc2 || EnemyLoc1 == LaserLoc3  || EnemyLoc1 < 8) )
			{
				if(EnemyLoc1 <= 8)
				{
					LIVES --;
				}
				else if( EnemyLoc1 == LaserLoc1 )
				{
					LaserLoc1 = 0;
				}
				else if(  EnemyLoc1 == LaserLoc2 )
				{
					LaserLoc2 = 0;
				}
				else
				{
					LaserLoc3 = 0;
				}
				EnemyLoc1 = 0;				
				EnemyStateBoss = startEBoss;
				ENEMIESNUM = 0;
			}
			break;
		default:
		break;
	}
	switch(EnemyStateBoss)
	{
		case startEBoss:
		break;
		case OnBoss:
		wait = 0;
		ENEMIESNUM = ENEMIESNUM + 1;
		EnemyLoc1 = 64;
		EnemyLoc2 = 63;
		EnemyLoc3 = 62;
		break;
		case moveBoss:
		if(wait >= BossSpeed)
		{
			if((EnemyLoc3 % 8) == 1)
			{
				EnemyLoc1 = EnemyLoc1 - 3;
				EnemyLoc2 = EnemyLoc2 - 3;
				EnemyLoc3 = EnemyLoc3 - 3;
			}
			else if((EnemyLoc2 % 8) == 1)
			{
				EnemyLoc1 = EnemyLoc1 - 2;
				EnemyLoc2 = EnemyLoc2 - 2;
			}
			else
			{
				if(EnemyLoc1 > 8)
				{
					EnemyLoc1 = EnemyLoc1 - 1;
				}
				if(EnemyLoc2 > 8)
				{
					EnemyLoc2 = EnemyLoc2 - 1;
				}
				if(EnemyLoc3 > 8)
				{
					EnemyLoc3 = EnemyLoc3 - 1;
				}
			}
			wait = 0;
		}
		wait ++;
		break;
		
		default:
		break;
	}
	return EnemyStateBoss;
}

//OUTPUTLIVES
enum LivesCur {startLives, onLives} LivesState;
int LivesSM(int LivesState)
{
	static char curD;
	switch( LivesState )
	{
		case startLives:
			if(GameStart == 1)
			{
				LivesState = onLives;
			}
			break;
		case onLives:
			break;
		default:
			LivesState = startLives;
			break;
	}
	switch( LivesState )
	{
		case startLives:
			curD = 0;
			PORTD = curD;
			break;
		case onLives:
			if(LIVES == 3)
			{
				curD = 0x07;
			}
			else if(LIVES == 2)
			{
				curD = 0x03;
			}
			else if(LIVES == 1)
			{
				curD = 0x01;
			}
			else
			{
				curD = 0x00;
			}
			PORTD = curD;
			break;
		default:
			break;
	}
	return LivesState;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main()
{
	unsigned char i = 0;
	DDRC = 0xFF; // Set port C to output
	PORTC = 0x00; // Init port C to 0s
	
	DDRD = 0xFF; // Set port C to output
	PORTD = 0x00; // Init port C to 0s
	
	DDRA = 0x00; // Set port A to input
	PORTA = 0xFF; // Init port A to 1s
	
	DDRB = 0x00; // Set port A to input
	PORTB = 0xFF; // Init port A to 1s
	
	//TASK INITIALIZATION~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	
	task1.state = start_button1;//Task initial state.
	task1.period = ButtonPeriod;//Task Period.
	task1.elapsedTime = 0;//Task current elapsed time.
	task1.TickFct = &Button1_SM;//Function pointer for the tick.	
			
	task2.state = start_button2;//Task initial state.
	task2.period = ButtonPeriod;//Task Period.
	task2.elapsedTime = 0;//Task current elapsed time.
	task2.TickFct = &Button2_SM;//Function pointer for the tick.
	
	task3.state = startJS;//Task initial state.
	task3.period = JoyStickPeriod;//Task Period.
	task3.elapsedTime = 0;//Task current elapsed time.
	task3.TickFct = &JoyStick;//Function pointer for the tick.

	task4.state = start_game;//Task initial state.
	task4.period = GamePeriod;//Task Period.
	task4.elapsedTime = 0;//Task current elapsed time.
	task4.TickFct = &GameSM;//Function pointer for the tick.

	task5.state = startPM;//Task initial state.
	task5.period = PlayerPeriod;//Task Period.
	task5.elapsedTime = 0;//Task current elapsed time.
	task5.TickFct = &PlayerSM;//Function pointer for the tick.
	
	task6.state = LazerState;//Task initial state.
	task6.period = LaserPeriod;//Task Period.
	task6.elapsedTime = 0;//Task current elapsed time.
	task6.TickFct = &LazerSM;//Function pointer for the tick.
	
	task7.state = EnemyState;//Task initial state.
	task7.period = EnemyPeriod;//Task Period.
	task7.elapsedTime = 0;//Task current elapsed time.
	task7.TickFct = &EnemyWave;//Function pointer for the tick
	
	task8.state = OddEnemyState1;//Task initial state.
	task8.period = EnemyPeriod;//Task Period.
	task8.elapsedTime = 0;//Task current elapsed time.
	task8.TickFct = &OddEnemyWave1;//Function pointer for the tick	

	task9.state = OddEnemyState2;//Task initial state.
	task9.period = EnemyPeriod;//Task Period.
	task9.elapsedTime = 0;//Task current elapsed time.
	task9.TickFct = &OddEnemyWave2;//Function pointer for the tick

	task10.state = OddEnemyState3;//Task initial state.
	task10.period = EnemyPeriod;//Task Period.
	task10.elapsedTime = 0;//Task current elapsed time.
	task10.TickFct = &OddEnemyWave3;//Function pointer for the tick		
	
	task11.state = start_power;//Task initial state.
	task11.period = PowerPeriod;//Task Period.
	task11.elapsedTime = 0;//Task current elapsed time.
	task11.TickFct = &PowerSM;//Function pointer for the tick
	
	task12.state = MetioreState;//Task initial state.
	task12.period = MetiorePeriod;//Task Period.
	task12.elapsedTime = 0;//Task current elapsed time.
	task12.TickFct = &MetioreSM;//Function pointer for the tick
	
	task13.state = EvenEnemyState1;//Task initial state.
	task13.period = EnemyPeriod;//Task Period.
	task13.elapsedTime = 0;//Task current elapsed time.
	task13.TickFct = &EvenEnemyWave1;//Function pointer for the tick
	
	task14.state = EvenEnemyState2;//Task initial state.
	task14.period = EnemyPeriod;//Task Period.
	task14.elapsedTime = 0;//Task current elapsed time.
	task14.TickFct = &EvenEnemyWave2;//Function pointer for the tick
		
	task15.state = EvenEnemyState3;//Task initial state.
	task15.period = EnemyPeriod;//Task Period.
	task15.elapsedTime = 0;//Task current elapsed time.
	task15.TickFct = &EvenEnemyWave3;//Function pointer for the tick
	
	task16.state = EnemyStateBoss;//Task initial state.
	task16.period = EnemyPeriod;//Task Period.
	task16.elapsedTime = 0;//Task current elapsed time.
	task16.TickFct = &EnemyWaveBoss;//Function pointer for the tick
		
	task17.state = start_LED;//Task initial state.
	task17.period = LEDPeriod;//Task Period.
	task17.elapsedTime = 0;//Task current elapsed time.
	task17.TickFct = &LEDSM;//Function pointer for the tick
	
	task18.state = startLives;//Task initial state.
	task18.period = LaserPeriod;//Task Period.
	task18.elapsedTime = 0;//Task current elapsed time.
	task18.TickFct = &LivesSM;//Function pointer for the tick
	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	TimerSet(GCD);
	ADC_init();
	TimerOn();
	
	while(1)
	{		
		// Scheduler code
		for ( i = 0; i < numTasks; i++ )
		{
			// Task is ready to tick
			if ( tasks[i]->elapsedTime == tasks[i]->period )
			{
				// Setting next state for task
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				// Reset the elapsed time for next tick.
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 1;
		}
		while (!TimerFlag); // Wait 1 sec
		TimerFlag = 0;
	}
}