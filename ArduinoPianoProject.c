#include <stdint.h>
  #include <stdio.h>
  #include <avr/io.h> 
  #include <avr/interrupt.h>
  #include <util/delay.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>

  //declare functions
  void setup(void);
  void process(void);
  void uart_init(unsigned int ubrr);
  unsigned char uart_getchar(void);
  void uart_putchar(unsigned char data);

  #define SET_BIT(reg, pin)			(reg) |= (1 << (pin))
  #define CLEAR_BIT(reg, pin)			(reg) &= ~(1 << (pin))
  #define WRITE_BIT(reg, pin, value)	(reg) = (((reg) & ~(1 << (pin))) | ((value) << (pin)))
  #define BIT_VALUE(reg, pin)			(((reg) >> (pin)) & 1)
  #define BIT_IS_SET(reg, pin)		(BIT_VALUE((reg),(pin))==1)


  

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00  

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


  #define BAUD (9600)
  #define MYUBRR (F_CPU/16/BAUD-1)

#define C 262
#define D 294
#define E 330
#define F 349
#define G 392
#define A 440

  unsigned char rx_buf;

 // int frequency[] = {262, 294, 330, 349, 392, 440, 494};

//_______________________________________________________________________

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
uint8_t comparator;




void lcd_init(void){
  //dotsize
  if (LCD_USING_4PIN_MODE){
    _lcd_displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  } else {
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
  } else {
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
  } else {
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






  //____________________________________________________________________

  void setup(void){

    uart_init(MYUBRR);


    //output
    SET_BIT(DDRB, 5);
    //piezo
    SET_BIT(DDRB, 2);


    
    //timer0
    TCCR0A = 0;
    TCCR0B = 4;
    TIMSK0 = 1; 
    sei();

    //timer2
    TCCR2A = 0;
    TCCR2B = 2;
    TIMSK2 = 1;
    sei();



/*
    OCR2A = 128;
    TCCR2A |= (1 << COM0A1);
    TCCR2B = (1 << CS22);
    TIMSK2 |= (1<< OCIE2A)
    TCCR2A |= (1 << WGM2);
*/
   

    //ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    //ADMUX = (1 << REFS0);

    
    //input
    CLEAR_BIT(DDRC, 0);
    CLEAR_BIT(DDRC, 1);
    CLEAR_BIT(DDRD, 2);
    CLEAR_BIT(DDRD, 3);
    CLEAR_BIT(DDRB, 3);
    CLEAR_BIT(DDRB, 4);

  }

  volatile unsigned int switch_counter1 = 0;
  volatile unsigned int switch_state1 = 0;

  volatile unsigned int switch_counter2 = 0;
  volatile unsigned int switch_state2 = 0;

  volatile unsigned int switch_counter3 = 0;
  volatile unsigned int switch_state3 = 0;

  volatile unsigned int switch_counter4 = 0;
  volatile unsigned int switch_state4 = 0;

  volatile unsigned int switch_counter5 = 0;
  volatile unsigned int switch_state5 = 0;

  volatile unsigned int switch_counter6 = 0;
  volatile unsigned int switch_state6 = 0;

  volatile unsigned int switch_counter7 = 0;
  volatile unsigned int switch_state7 = 0;


  volatile uint8_t ISRcounter = 0;

  ISR(TIMER2_OVF_vect)
  {
    if(switch_state1 == 0 && switch_state2 == 0 && switch_state3 == 0 && switch_state4 == 0 && switch_state5 == 0 && switch_state6 == 0){
    comparator = 0;	
      CLEAR_BIT(PORTB, 2);
    }
    
	if(ISRcounter < comparator / 2) {
		//set pin state high
    SET_BIT(PORTB, 2);
     
	}else if (ISRcounter < comparator) {
      CLEAR_BIT(PORTB, 2);

    }else{
     ISRcounter = 0;
    }

	ISRcounter++;
  } 


  ISR(TIMER0_OVF_vect) {
	  uint8_t mask = 0b01111111;				

  //button1
    switch_counter1 = ((switch_counter1 << 1) & mask) | BIT_VALUE(PINC, 0);
       
    if (switch_counter1 == mask){
        switch_state1 = 1;
    }
    
    if (switch_counter1 == 0){
        switch_state1 = 0;
    }
 

  //button 2
    switch_counter2 = ((switch_counter2 << 1) & mask) | BIT_VALUE(PINC, 1);

    if (switch_counter2 == 0){
        switch_state2 = 0;
    } 
    else if (switch_counter2 == mask){
        switch_state2 = 1;
    } 

  //button 3
    switch_counter3 = ((switch_counter3 << 1) & mask) | BIT_VALUE(PIND, 2);

    if (switch_counter3 == 0){
        switch_state3 = 0;
    } 
    else if (switch_counter3 == mask){
        switch_state3 = 1;
    }

    //button 4
    switch_counter4 = ((switch_counter4 << 1) & mask) | BIT_VALUE(PIND, 3);

    if (switch_counter4 == 0){
        switch_state4 = 0;
    } 
    else if (switch_counter4 == mask){
        switch_state4 = 1;
    }

    //button 5
    switch_counter5 = ((switch_counter5 << 1) & mask) | BIT_VALUE(PINB, 3);

    if (switch_counter5 == 0){
        switch_state5 = 0;
    } 
    else if (switch_counter5 == mask){
        switch_state5 = 1;
    }

    //button 6
    switch_counter6 = ((switch_counter6 << 1) & mask) | BIT_VALUE(PINB, 4);

    if (switch_counter6 == 0){
        switch_state6 = 0;
    } 
    else if (switch_counter6 == mask){
        switch_state6 = 1;
    }
  }

  void playnote(unsigned int frequency, unsigned long duration){
    uint8_t prescalarbits = 0b001;
    uint32_t ocr = 0;

    OCR2A = F_CPU / frequency / 8 - 1;


  }

  void process(void){
    if(switch_state1 == 1){
      //define a character to sent
      static char sent_char = 'A';
    //send serial data
      uart_putchar(sent_char);
      lcd_write_char(0, 0, sent_char);
      comparator = 7812.5 / A * 2;
		//comparator = 1000;
       
       SET_BIT(PORTB, 5);
    

    }else if(switch_state1 == 0){
     CLEAR_BIT(PORTB, 5);
    }

    //_____________________________________________

    if(switch_state2 == 1){
      //define a character to sent
      static char sent_char = 'G';
    //send serial data
      uart_putchar(sent_char);
      lcd_write_char(0, 0, sent_char);
      comparator = 7812.5 / G * 2;


       
       SET_BIT(PORTB, 5);
    

    }else if(switch_state2 == 0){
      
      CLEAR_BIT(PORTB, 5);
    }

    //_____________________________________________

    if(switch_state3 == 1){
      //define a character to sent
      static char sent_char = 'F';
    //send serial data
      uart_putchar(sent_char);
      lcd_write_char(0, 0, sent_char);
      comparator = 7812.5 / F * 2;

       
       SET_BIT(PORTB, 5);
    

    }else if(switch_state3 == 0){
      
      CLEAR_BIT(PORTB, 5);
    }
    
    //_____________________________________________

    if(switch_state4 == 1){
      //define a character to sent
      static char sent_char = 'E';
    //send serial data
      uart_putchar(sent_char);
      lcd_write_char(0, 0, sent_char);
      
      comparator = 7812.5 / E * 2;


       
       SET_BIT(PORTB, 5);
    

    }else if(switch_state4 == 0){
      //CLEAR_BIT(PORTB, 2);
      CLEAR_BIT(PORTB, 5);
    }

    //_____________________________________________

    if(switch_state5 == 1){
      //define a character to sent
      static char sent_char = 'D';
    //send serial data
      uart_putchar(sent_char);
      lcd_write_char(0, 0, sent_char);
      comparator = 7812.5 / D * 2;
       
       SET_BIT(PORTB, 5);
    

    }else if(switch_state5 == 0){
      
      CLEAR_BIT(PORTB, 5);
    }

    //_____________________________________________

    if(switch_state6 == 1){
      //define a character to sent
      static char sent_char = 'C';
    //send serial data
      uart_putchar(sent_char);
      lcd_write_char(0, 0, sent_char);
      comparator = 7812.5 / C * 2;
       
       SET_BIT(PORTB, 5);
    

    }else if(switch_state6 == 0){
      
      CLEAR_BIT(PORTB, 5);
    }

    //_____________________________________________
  }

  //Obtained from Topic 8 - Lecture Notes - Example 1
  void uart_init(unsigned int ubrr){

    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char)(ubrr);
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C =(3 << UCSZ00);

  }

  //transmit data
  void uart_putchar(unsigned char data){

    while (!( UCSR0A & (1<<UDRE0))); /* Wait for empty transmit buffer*/

      UDR0 = data;            /* Put data into buffer, sends the data */
                                      
    
  }

  //receive data
  unsigned char uart_getchar(void){
    
    /* Wait for data to be received */ 
      while ( !(UCSR0A & (1<<RXC0)) );
    

    return UDR0;
    
  }

int intToStr(int x, unsigned char str[], int d) {
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

void uart_putstring(unsigned char* s)
{
    // transmit character until NULL is reached
    while(*s > 0) uart_putchar(*s++);
}

void reverse(unsigned char * str, int len) {
  int i = 0, j = len - 1, temp;
  while (i < j) {
    temp = str[i];
    str[i] = str[j];
    str[j] = temp;
    i++;
    j--;
  }
}

  int main(void){
    lcd_init();
    setup();
    

    for(;;){
      process();
      _delay_ms(100);

    }

    return 0;
  }

