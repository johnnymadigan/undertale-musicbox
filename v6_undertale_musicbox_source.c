/*_________________________________________________

    .x88888x.            x*8888x.:*8888: -"888;
   :8**888888X.  :>     X   48888X/`8888H/`8888H
   f    `888888x./     X8x.  8888X  8888X  8888X;
  '       `*88888~     X8888 X8888  88888  88888;
   \.    .  `?)X.      '*888!X8888  X8888  X8888;
    `~=-^   X88> ~       `?8 `8888  X888X  X888X
           X8888  ~      ~"  '888"  X888   X888
           488888           !888;  !888;  !888;
   .xx.     88888X         888!   888!   888!
  '*8888.   '88888>       88"    88"    88"
    88888    '8888>        "~     "~     "~
    `8888>    `888                           
     "8888     8%           Johnny Madigan
      `"888x:-"    https://johnnymadigan.github.io/




	Tutor:			  Pearl
	Project name:	Undertale Musicbox
	Inspired by:	https://www.youtube.com/watch?v=73RcTpsHYVM
  _________________________________________________
*/

#include <stdio.h>
#include <stdint.h>
#include <avr/io.h> 
#include <string.h>
#include <inttypes.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/*_________________________________________________
        HELPFUL GLOBAL VARIABLES FROM CAB202
  _________________________________________________
*/
//	MACROS to set, clear, and read bits from Topic 7 – Introduction to Microcontrollers
#define SET_BIT(reg, pin)			      (reg) |= (1 << (pin))
#define CLEAR_BIT(reg, pin)		      (reg) &= ~(1 << (pin))
#define WRITE_BIT(reg, pin, value)	(reg) = (((reg) & ~(1 << (pin))) | ((value) << (pin)))
#define BIT_VALUE(reg, pin)		  	  (((reg) >> (pin)) & 1)
#define BIT_IS_SET(reg, pin)		    (BIT_VALUE((reg),(pin))==1)

//	BAUD VARIABLES From Lecture Notes Topic 8 - Serial Communication
#define baud_rate		9600
#define F_CPU			  16000000UL
#define my_baud 		F_CPU/16/baud_rate-1

//	TIMER FREQUENCY AND PRESCALER From Lecture Notes Topic 9 - Timers and Interrupts
#define FREQ (16000000.0)
#define PRESCALE (1024.0)

//	CONFIGURE LCD IN 4-PIN MODE From Lecture Notes Topic 11 - LCD
#define LCD_USING_4PIN_MODE (1)

#define LCD_DATA4_DDR (DDRD)
#define LCD_DATA5_DDR (DDRD)
#define LCD_DATA6_DDR (DDRD)
#define LCD_DATA7_DDR (DDRD)

#define LCD_DATA4_PORT (PORTD)
#define LCD_DATA5_PORT (PORTD)
#define LCD_DATA6_PORT (PORTD)
#define LCD_DATA7_PORT (PORTD)

#define LCD_DATA4_PIN (4)
#define LCD_DATA5_PIN (5)
#define LCD_DATA6_PIN (6)
#define LCD_DATA7_PIN (7)

#define LCD_RS_DDR (DDRB)
#define LCD_ENABLE_DDR (DDRB)

#define LCD_RS_PORT (PORTB)
#define LCD_ENABLE_PORT (PORTB)

#define LCD_RS_PIN (1)
#define LCD_ENABLE_PIN (0)

//	LCD COMMANDS From Lecture Notes Topic 11 - LCD
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

//	Flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

//	Flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

//	Flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

//	Flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

/*_________________________________________________
               FUNCTION DECLARATIONS
  _________________________________________________
*/
// Function declarations
void setup();
void loop();

void create_delay(int note);
void play_note();
void display_flower(int body_part);

void uart_send_string(char message[]);
void uart_init(unsigned int ubrr);
void uart_putbyte(unsigned char data);
void uart_putchar(unsigned char data);
void uart_putstring(unsigned char* s);

char uart_getchar(void);
ISR(TIMER2_OVF_vect);
void reverse(char * str, int len);
int intToStr(int x, char str[], int d);
void ftoa(float n, char * res, int afterpoint);

void lcd_init(void);
void lcd_write_string(uint8_t x, uint8_t y, char string[]);
void lcd_write_char(uint8_t x, uint8_t y, char val);
void lcd_clear(void);
void lcd_home(void);

void lcd_createChar(uint8_t, uint8_t[]);
void lcd_setCursor(uint8_t, uint8_t); 

void lcd_noDisplay(void);
void lcd_display(void);
void lcd_noBlink(void);
void lcd_blink(void);
void lcd_noCursor(void);
void lcd_cursor(void);
void lcd_leftToRight(void);
void lcd_rightToLeft(void);
void lcd_autoscroll(void);
void lcd_noAutoscroll(void);
void scrollDisplayLeft(void);
void scrollDisplayRight(void);

size_t lcd_write(uint8_t);
void lcd_command(uint8_t);

void lcd_send(uint8_t, uint8_t);
void lcd_write4bits(uint8_t);
void lcd_write8bits(uint8_t);
void lcd_pulseEnable(void);

uint8_t _lcd_displayfunction;
uint8_t _lcd_displaycontrol;
uint8_t _lcd_displaymode;

/*_________________________________________________
                MY GLOBAL VARIABLES
  _________________________________________________
*/
//	Musical notes from Arduino's Melody documentation  
//	https://www.arduino.cc/en/tutorial/melody
//	variable    period  frequency equivalent
#define note_c	3830	  // 261 Hz
#define note_e	3038    // 329 Hz
#define note_f	2864    // 349 Hz
#define note_g	2550    // 392 Hz
#define note_a	2272    // 440 Hz
#define	rest	  0		    // rest note (no noise)

//	Debouncing in milliseconds
#define DEBOUNCING_MS	1000

int current_note = 0;				      // Holds the current note
long length  = 0;					        // Holds the length of the song
long overall_tempo = 25000;			  // Holds the overall tempo
volatile int count_overflow = 0;	// Holds the total times the timer overflows 

//	Melody array
int melody[] = {
  note_c, note_g, note_f, note_c, note_e,  rest, note_e,  note_f,
    rest, note_c, note_f, note_c, note_e,  rest, note_e,  note_f,
  note_c, note_g, note_f, note_c, note_e,  rest, note_e,  note_f,
  	rest, note_c, note_f, note_a, note_g,  rest, note_f,  note_g 
};

//	Note length array
int note_length[]  = { 
   6, 6, 6, 6, 6, 3, 6, 6,        
  20, 6, 6, 6, 6, 3, 6, 6,                   	   
   6, 6, 6, 6, 6, 3, 6, 6,                    	  
  20, 6, 6, 6, 6, 3, 6, 6 
};

//	Serial messages array
static char * message[] = {
  "Now playing 'His Theme' from the Undertale OST",
  "Timestamps:",
  "Thank you for listening!",
  "Hi I'm Johnny, the creator of this musicbox.",
  "You can use the dial to change the speed of the song.",
  "As the song plays, a flower will dance on the screen",
  "and LEDs will light up to show which note is playing!",
  "Try it yourself, press the button to begin!"
};

//	Custom LCD characters makes the flower from Undertale
//	Including a dancing animation! (I did this all manually
//	before I knew pixels to bitmaps existed)
uint8_t body[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b01110,
  0b11011
};

uint8_t body_mirror[8] = {
  0b11011,
  0b01110,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

uint8_t chin[8] = {
  0b00000,
  0b00100,
  0b01010,
  0b10001,
  0b10010,
  0b10100,
  0b01001,
  0b01010
};

uint8_t chin_mirror[8] = {
  0b01010,
  0b01001,
  0b10100,
  0b10010,
  0b10001,
  0b01010,
  0b00100,
  0b00000
};

uint8_t face[8] = {
  0b00100,
  0b01010,
  0b10001,
  0b01101,
  0b00010,
  0b00001,
  0b11001,
  0b00001
};

uint8_t face_mirror[8] = {
  0b00001,
  0b11001,
  0b00001,
  0b00010,
  0b01101,
  0b10001,
  0b01010,
  0b00100
};

uint8_t top[8] = {
  0b00000,
  0b10000,
  0b01000,
  0b00100,
  0b00100,
  0b01000,
  0b10000,
  0b00000
};

/*_________________________________________________
                   		MAIN
  _________________________________________________
*/
//	Main method to make the musicbox work
int main (void){
	
  // Setup avr
	setup();

  // Loop
	for ( ;; ) {
		loop();
	}
}

/*_________________________________________________
                    	SETUP
  _________________________________________________
*/
// Configure registers for input and output
void setup() {
  
  SET_BIT(DDRC, 1);		// Setup LED c for output
  SET_BIT(DDRC, 2);		// Setup LED e for output
  SET_BIT(DDRC, 3);		// Setup LED f for output
  SET_BIT(DDRC, 4);		// Setup LED g for output
  SET_BIT(DDRC, 5);		// Setup LED a for output

  SET_BIT(DDRB, 3);		// Setup piezo for output
  
  CLEAR_BIT(DDRD, 2);	// Setup button for input
    
  lcd_init();			    // Setup LCD's rows & columns
  
  // Setup custom LCD characters
  lcd_createChar(1, body);
  lcd_createChar(2, body_mirror);
  lcd_createChar(3, chin);
  lcd_createChar(4, chin_mirror);
  lcd_createChar(5, face);
  lcd_createChar(6, face_mirror);
  lcd_createChar(7, top);

  // Display Flower's body and head
  display_flower(1);
  display_flower(3);
  	
  // Display LCD message
  lcd_write_string(8, 0, "Play me!");
  lcd_write_string(6, 1, "  (button)");
  
  // Starts the serial communication allowing the Arduino 
  // to exchange data at the baud rate (9600 bits per second)
  uart_init(my_baud);
  
  // Enabling PWM timer 2 with prescaler of 1024 (approx 60Hz overflow)
  // Enabling PWM timer overflow and turn on interrupts
  TCCR2A = 0;
  TCCR2B = 5; 
  TIMSK2 = 1;  
  sei();
  
  // Initialise ADC
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
  ADMUX = (1 << REFS0);
  
  // Welcome message
  uart_putchar('\n');
  uart_putchar('\n');
  uart_send_string(message[3]);
  uart_putchar('\n');
  uart_send_string(message[4]);
  uart_putchar('\n');
  uart_send_string(message[5]);
  uart_putchar('\n');
  uart_send_string(message[6]);
  uart_putchar('\n');
  uart_send_string(message[7]);
  uart_putchar('\n');
}

/*_________________________________________________
          COMPOSE MELODY & LOOP THE PROGRAM
  _________________________________________________
*/
void loop() {
  
  // Store the length of the song
  int song_length = sizeof(melody) / sizeof(melody[0]);

  // Buffer
  char temp_buf[64];

  // If the button is pressed, debounce then play the song
  if(BIT_IS_SET(PIND, 2)) {
    
    // Debouncing for 1 millisecond
    _delay_ms(DEBOUNCING_MS);	
	
    // Song is about to play message
    uart_putchar('\n');
    uart_send_string(message[0]);
    uart_putchar('\n');
    uart_send_string(message[1]);
    uart_putchar('\n');
    uart_putchar('\n');
    
    // Change LCD text to show melody is playing
  	lcd_write_string(8, 0, "Go me!  ");
  	lcd_write_string(6, 1, "im dancing");
    
    // Play each note in the melody
    for (int i = 0; i < song_length; i++) {
      
      // Find out the elapsed time
	  double time = (count_overflow * 33.0 + TCNT0) * PRESCALE / FREQ;
    
      // Convert the float into a string so we can print it
      ftoa(time, temp_buf, 2);

	  // Send timestamps to serial monitor
	  uart_putstring((unsigned char *) temp_buf);
	  uart_putchar('\n');
      
      current_note = melody[i];	// Assign the note about to be played
      length = note_length[i] * overall_tempo;	// Finding the timing

      if (melody[i] == note_c){	// If the current note matches
      display_flower(2);		// Display 1st dance move
      SET_BIT(PORTC, 1);		// Turn on the corresponding LED
      play_note();				// Play the note
      CLEAR_BIT(PORTC, 1);  	// Turn off the corresponding LED
	  display_flower(1);		// Display the 2nd dance move
      } 
      else if (melody[i] == note_e){
      display_flower(2);
      SET_BIT(PORTC, 2);
      play_note();
      CLEAR_BIT(PORTC, 2);
      display_flower(1);
      }
      else if (melody[i] == note_f){
      display_flower(2);
      SET_BIT(PORTC, 3);
      play_note();
      CLEAR_BIT(PORTC, 3);
	  display_flower(1);
      }
      else if (melody[i] == note_g){
      display_flower(2);
      SET_BIT(PORTC, 4);
      play_note();
      CLEAR_BIT(PORTC, 4);
	  display_flower(1);
      }
      else if (melody[i] == note_a){
      display_flower(2);     
      SET_BIT(PORTC, 5);
      play_note();
      CLEAR_BIT(PORTC, 5);
	  display_flower(1);
      }
      
	  // Leave a gap between notes by reading the potentiometer
      // Start single conversion by setting ADSC bit in ADCSRA
	  ADCSRA |= (1 << ADSC);
      
      // Wait for ADSC bit to clear, signalling completed conversion
	  while (ADCSRA & (1 << ADSC)) {}
      
      // Assign results & use below to set the tempo
	  uint16_t pot = ADC;

      if (pot >= 0 && pot < 171){
        _delay_us(180000);					// Default tempo (slow)
      }
      else if (pot >= 171 && pot < 342){
        _delay_us(150000);					// 16% faster
      }
      else if (pot >= 342 && pot < 512){
        _delay_us(120000);					// 33% faster
      }
      else if (pot >= 512 && pot < 683){
        _delay_us(90000);					// 50% faster
      }
      else if (pot >= 683 && pot < 854){
        _delay_us(60000);					// 66% faster
      }
      else if (pot >= 854){
        _delay_us(30000);					// 83% faster
      }
      else{
        _delay_us(180000);				// Resort to default tempo as a backup
     }
    }
    
    // Display LCD message
  	lcd_write_string(8, 0, "Play me!");
  	lcd_write_string(6, 1, "  (button)");
    
    // Thank you message
    uart_putchar('\n');
    uart_send_string(message[2]);
    uart_putchar('\n');
  }
}

/*_________________________________________________
                   	 CALL A DELAY
  _________________________________________________
*/
//	Method that creates a delay manually since AMS
//	does not like it when you put an integer variable
//	as the compile time and instead needs the integer
//	directly.

//	This means magic numbers can't be avoided, so I'll
//	just point out the microseconds being used are the period
//	values of the notes divided by 2  debouncing!

void create_delay(int note) {
  
  if (current_note == note_c){
	_delay_us(1915);
  }
  else if (current_note == note_e){
	_delay_us(1519);
  }
    else if (current_note == note_f){
	_delay_us(1432);
  }
    else if (current_note == note_g){
	_delay_us(1275);
  }
    else if (current_note == note_a){
	_delay_us(1136);
  }
    else if (current_note == rest){
	_delay_us(0);
  }
}

/*_________________________________________________
                   	 PLAY A NOTE
  _________________________________________________
*/
//	Method that pulses the piezo to generate a specific tone
//	depending on the note's period / frequency from "melody[i]"
//	The note is played for the duration of "length"

void play_note() {
  
  long time_elapsed = 0;
  
  while (time_elapsed < length) {
     
    // Start buzzing & delay
    SET_BIT(PORTB, 3);
	create_delay(current_note);

    // Stop buzzing & delay
    CLEAR_BIT(PORTB, 3);
	create_delay(current_note);
	
    // Update the time elapsed
    time_elapsed += (current_note);
  }                     
}

/*_________________________________________________
                   FLOWER BODY PARTS
  _________________________________________________
*/
//	Display flower's body parts on the LCD
void display_flower(int body_part){
  
  if (body_part == 1){ 
    // Dance move 2
    lcd_setCursor(0,0);
    lcd_write(1);
    lcd_setCursor(1,1);
    lcd_write(2);
    lcd_setCursor(2,0);
    lcd_write(1);

    // Blank character (removes dance move 1)
    lcd_write_char(0, 1, 0b00100000);
    lcd_write_char(1, 0, 0b00100000);
    lcd_write_char(2, 1, 0b00100000);
  }
  else if (body_part == 2){
    // Dance move 1
    lcd_setCursor(0,1);
    lcd_write(2);
    lcd_setCursor(1,0);
    lcd_write(1);
    lcd_setCursor(2,1);
    lcd_write(2);

    // Blank character (removes dance move 2)
    lcd_write_char(0, 0, 0b00100000);
    lcd_write_char(1, 1, 0b00100000);
    lcd_write_char(2, 0, 0b00100000);
  }
  else if (body_part == 3){
    // Display flower's chin
    lcd_setCursor(3,0);
    lcd_write(3);
    lcd_setCursor(3,1);
    lcd_write(4);

  	// Display Flower's face
    lcd_setCursor(4,0);
    lcd_write(5);
    lcd_setCursor(4,1);
    lcd_write(6);
  
  	// Display Flower's top
    lcd_setCursor(5,0);
    lcd_write(7);
    lcd_setCursor(5,1);
    lcd_write(7);    
  }
}

/*_________________________________________________
               STRING TO SERIAL MONITOR
  _________________________________________________
*/
//	Sends a string to the serial monitor, stopping
//	at the end of the string (end of field '\0')
void uart_send_string(char message[])
{
   int message_size = strlen(message);
   int i = 0;
  
   while(i <= message_size){
     
     unsigned char ch = (unsigned char) (message[i]);
     
     if(message[i] == '\0'){
       uart_putbyte(0);
     }
     else{
       uart_putbyte(ch);
     }
     
     i++;
   }
}

/*_________________________________________________
         HELPFUL FUNCTIONS FROM CAB202 BELOW
     Lecture Notes Topic 8 - Serial Communication
  _________________________________________________
*/
// Initialise UART
void uart_init(unsigned int ubrr){
  UBRR0H = (unsigned char)(ubrr>>8);
  UBRR0L = (unsigned char)(ubrr);
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
  UCSR0C = (3 << UCSZ00);
}

// Send one byte
void uart_putbyte(unsigned char data) {
  while (!(UCSR0A & (1 << UDRE0)));
  UDR0 = data;
}

// Send data
void uart_putchar(unsigned char data){
  while (!( UCSR0A & (1<<UDRE0)));
  UDR0 = data;
}

// Send a string
void uart_putstring(unsigned char* s) {
  while(*s > 0) uart_putchar(*s++);
}

// Receive data
char uart_getchar(void) {
  while ( !(UCSR0A & (1<<RXC0)) );
  return UDR0;
}

/*_________________________________________________
         HELPFUL FUNCTIONS FROM CAB202 BELOW
    Lecture Notes Topic 9 - Timers and Interrupts
  _________________________________________________
*/
//	Used to calculate the time elapsed 
//	(how many times the timer has overflowed)
ISR(TIMER2_OVF_vect) {
	count_overflow++;
}

//	Reverses a string 'str' of length 'len' 
void reverse(char * str, int len) {
  int i = 0, j = len - 1, temp;
  while (i < j) {
    temp = str[i];
    str[i] = str[j];
    str[j] = temp;
    i++;
    j--;
  }
}

//	Converts a given integer x to string str[].  
//	d is the number of digits required in the output.  
//	If d is more than the number of digits in x,  
//	then 0s are added at the beginning. 
int intToStr(int x, char str[], int d) {
  int i = 0;
  while (x) {
    str[i++] = (x % 10) + '0';
    x = x / 10;
  }

  // If number of digits required is more, then 
  // add 0s at the beginning 
  while (i < d)
    str[i++] = '0';

  reverse(str, i);
  str[i] = '\0';
  return i;
}

// Converts a floating-point/double number to a string. 
void ftoa(float n, char * res, int afterpoint) {
  // Extract integer part 
  int ipart = (int) n;

  // Extract floating part 
  float fpart = n - (float) ipart;

  // convert integer part to string 
  int i = intToStr(ipart, res, 0);

  // check for display option after point 
  if (afterpoint != 0) {
    res[i] = '.'; // add dot 

  // Get the value of fraction part upto given no. 
  // of points after dot. The third parameter  
  // is needed to handle cases like 233.007 
  fpart = fpart * pow(10, afterpoint);

  intToStr((int) fpart, res + i + 1, afterpoint);
  }
}

/*_________________________________________________
         HELPFUL FUNCTIONS FROM CAB202 BELOW
            Lecture Notes Topic 11 - LCD
  _________________________________________________
*/
void lcd_init(void){
  //dotsize
  if (LCD_USING_4PIN_MODE){
    _lcd_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  } 
  else {
    _lcd_displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
  }
  
  _lcd_displayfunction |= LCD_2LINE;

  // RS Pin
  LCD_RS_DDR |= (1 << LCD_RS_PIN);
  // Enable Pin
  LCD_ENABLE_DDR |= (1 << LCD_ENABLE_PIN);
  
  #if LCD_USING_4PIN_MODE
    //Set DDR for all the data pins
    LCD_DATA4_DDR |= (1 << LCD_DATA4_PIN);
    LCD_DATA5_DDR |= (1 << LCD_DATA5_PIN);
    LCD_DATA6_DDR |= (1 << LCD_DATA6_PIN);    
    LCD_DATA7_DDR |= (1 << LCD_DATA7_PIN);

  #else
    //Set DDR for all the data pins
    LCD_DATA0_DDR |= (1 << LCD_DATA0_PIN);
    LCD_DATA1_DDR |= (1 << LCD_DATA1_PIN);
    LCD_DATA2_DDR |= (1 << LCD_DATA2_PIN);
    LCD_DATA3_DDR |= (1 << LCD_DATA3_PIN);
    LCD_DATA4_DDR |= (1 << LCD_DATA4_PIN);
    LCD_DATA5_DDR |= (1 << LCD_DATA5_PIN);
    LCD_DATA6_DDR |= (1 << LCD_DATA6_PIN);
    LCD_DATA7_DDR |= (1 << LCD_DATA7_PIN);
  #endif 

  // SEE PAGE 45/46 OF Hitachi HD44780 DATASHEET FOR INITIALIZATION SPECIFICATION!

  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way before 4.5V so we'll wait 50
  _delay_us(50000); 
  // Now we pull both RS and Enable low to begin commands (R/W is wired to ground)
  LCD_RS_PORT &= ~(1 << LCD_RS_PIN);
  LCD_ENABLE_PORT &= ~(1 << LCD_ENABLE_PIN);
  
  //put the LCD into 4 bit or 8 bit mode
  if (LCD_USING_4PIN_MODE) {
    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    lcd_write4bits(0b0111);
    _delay_us(4500); // wait min 4.1ms

    // second try
    lcd_write4bits(0b0111);
    _delay_us(4500); // wait min 4.1ms
    
    // third go!
    lcd_write4bits(0b0111); 
    _delay_us(150);

    // finally, set to 4-bit interface
    lcd_write4bits(0b0010); 
  } 
  else {
    // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
    lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);
    _delay_us(4500);  // wait more than 4.1ms

    // second try
    lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);
    _delay_us(150);

    // third go
    lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);
  }

  // finally, set # lines, font size, etc.
  lcd_command(LCD_FUNCTIONSET | _lcd_displayfunction);  

  // turn the display on with no cursor or blinking default
  _lcd_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  lcd_display();

  // clear it off
  lcd_clear();

  // Initialize to default text direction (for romance languages)
  _lcd_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

/********** high level commands, for the user! */
void lcd_write_string(uint8_t x, uint8_t y, char string[]){
  lcd_setCursor(x,y);
  for(int i=0; string[i]!='\0'; ++i){
    lcd_write(string[i]);
  }
}

void lcd_write_char(uint8_t x, uint8_t y, char val){
  lcd_setCursor(x,y);
  lcd_write(val);
}

void lcd_clear(void){
  lcd_command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  _delay_us(2000);  // this command takes a long time!
}

void lcd_home(void){
  lcd_command(LCD_RETURNHOME);  // set cursor position to zero
  _delay_us(2000);  // this command takes a long time!
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void lcd_createChar(uint8_t location, uint8_t charmap[]) {
  
  location &= 0x7; // we only have 8 locations 0-7
  lcd_command(LCD_SETCGRAMADDR | (location << 3));
  
  for (int i=0; i<8; i++) {
    
    lcd_write(charmap[i]);
  }
}

void lcd_setCursor(uint8_t col, uint8_t row){
  
  if ( row >= 2 ) {
    row = 1;
  }
  
  lcd_command(LCD_SETDDRAMADDR | (col + row*0x40));
}

// Turn the display on/off (quickly)
void lcd_noDisplay(void) {
  _lcd_displaycontrol &= ~LCD_DISPLAYON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}
void lcd_display(void) {
  _lcd_displaycontrol |= LCD_DISPLAYON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

// Turns the underline cursor on/off
void lcd_noCursor(void) {
  _lcd_displaycontrol &= ~LCD_CURSORON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}
void lcd_cursor(void) {
  _lcd_displaycontrol |= LCD_CURSORON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

// Turn on and off the blinking cursor
void lcd_noBlink(void) {
  _lcd_displaycontrol &= ~LCD_BLINKON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}
void lcd_blink(void) {
  _lcd_displaycontrol |= LCD_BLINKON;
  lcd_command(LCD_DISPLAYCONTROL | _lcd_displaycontrol);
}

// These commands scroll the display without changing the RAM
void scrollDisplayLeft(void) {
  lcd_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void scrollDisplayRight(void) {
  lcd_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void lcd_leftToRight(void) {
  _lcd_displaymode |= LCD_ENTRYLEFT;
  lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

// This is for text that flows Right to Left
void lcd_rightToLeft(void) {
  _lcd_displaymode &= ~LCD_ENTRYLEFT;
  lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

// This will 'right justify' text from the cursor
void lcd_autoscroll(void) {
  _lcd_displaymode |= LCD_ENTRYSHIFTINCREMENT;
  lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

// This will 'left justify' text from the cursor
void lcd_noAutoscroll(void) {
  _lcd_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  lcd_command(LCD_ENTRYMODESET | _lcd_displaymode);
}

/*********** mid level commands, for sending data/cmds */

inline void lcd_command(uint8_t value) {
  //
  lcd_send(value, 0);
}

inline size_t lcd_write(uint8_t value) {
  lcd_send(value, 1);
  return 1; // assume sucess
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void lcd_send(uint8_t value, uint8_t mode) {
  
  //RS Pin
  LCD_RS_PORT &= ~(1 << LCD_RS_PIN);
  LCD_RS_PORT |= (!!mode << LCD_RS_PIN);

  if (LCD_USING_4PIN_MODE) {
    lcd_write4bits(value>>4);
    lcd_write4bits(value);
  } 
  else {
    lcd_write8bits(value); 
  } 
}

void lcd_pulseEnable(void) {
  
  //Enable Pin
  LCD_ENABLE_PORT &= ~(1 << LCD_ENABLE_PIN);
  _delay_us(1);    
  LCD_ENABLE_PORT |= (1 << LCD_ENABLE_PIN);
  _delay_us(1);    // enable pulse must be >450ns
  LCD_ENABLE_PORT &= ~(1 << LCD_ENABLE_PIN);
  _delay_us(100);   // commands need > 37us to settle
}

void lcd_write4bits(uint8_t value) {
  
  //Set each wire one at a time
  LCD_DATA4_PORT &= ~(1 << LCD_DATA4_PIN);
  LCD_DATA4_PORT |= ((value & 1) << LCD_DATA4_PIN);
  value >>= 1;

  LCD_DATA5_PORT &= ~(1 << LCD_DATA5_PIN);
  LCD_DATA5_PORT |= ((value & 1) << LCD_DATA5_PIN);
  value >>= 1;

  LCD_DATA6_PORT &= ~(1 << LCD_DATA6_PIN);
  LCD_DATA6_PORT |= ((value & 1) << LCD_DATA6_PIN);
  value >>= 1;

  LCD_DATA7_PORT &= ~(1 << LCD_DATA7_PIN);
  LCD_DATA7_PORT |= ((value & 1) << LCD_DATA7_PIN);

  lcd_pulseEnable();
}

void lcd_write8bits(uint8_t value) {
  
  //Set each wire one at a time
  #if !LCD_USING_4PIN_MODE
    LCD_DATA0_PORT &= ~(1 << LCD_DATA0_PIN);
    LCD_DATA0_PORT |= ((value & 1) << LCD_DATA0_PIN);
    value >>= 1;

    LCD_DATA1_PORT &= ~(1 << LCD_DATA1_PIN);
    LCD_DATA1_PORT |= ((value & 1) << LCD_DATA1_PIN);
    value >>= 1;

    LCD_DATA2_PORT &= ~(1 << LCD_DATA2_PIN);
    LCD_DATA2_PORT |= ((value & 1) << LCD_DATA2_PIN);
    value >>= 1;

    LCD_DATA3_PORT &= ~(1 << LCD_DATA3_PIN);
    LCD_DATA3_PORT |= ((value & 1) << LCD_DATA3_PIN);
    value >>= 1;

    LCD_DATA4_PORT &= ~(1 << LCD_DATA4_PIN);
    LCD_DATA4_PORT |= ((value & 1) << LCD_DATA4_PIN);
    value >>= 1;

    LCD_DATA5_PORT &= ~(1 << LCD_DATA5_PIN);
    LCD_DATA5_PORT |= ((value & 1) << LCD_DATA5_PIN);
    value >>= 1;

    LCD_DATA6_PORT &= ~(1 << LCD_DATA6_PIN);
    LCD_DATA6_PORT |= ((value & 1) << LCD_DATA6_PIN);
    value >>= 1;

    LCD_DATA7_PORT &= ~(1 << LCD_DATA7_PIN);
    LCD_DATA7_PORT |= ((value & 1) << LCD_DATA7_PIN);
    
    lcd_pulseEnable();
  #endif
}

/*_________________________________________________
         	  			      FIN
  _________________________________________________
*/
