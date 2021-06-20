/*                      Master Clock
                   Vernon Billingsley c2021 

              
                       
            arduino   ATmega
              pin      pin        Function
              --------------------------------------
              D2\       4         Data
              D3/       5         Clock Rotaty Encoder
              D4        6         Encoder Button  Step x10 or x1


              D8        14        Clock / 2
              D9        15        Main Clock Out
              D10       16        Clock * 2

              D11       17        CLK/2 LED
              D12       18        CLK LED
              D13       19        CLK*2 LED

              A4        27        I2C  SDA
              A5        28        I2C  SCL
              
              Display Adafruit .56" 7 Segment
              and Arduino backpack

*/

#include <Wire.h>
#include <RotaryEncoder.h>
#include "Note_Timer.h"
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <elapsedMillis.h>

//Setup the display
Adafruit_7segment matrix = Adafruit_7segment();

/*Setup the clocks */
NoteTimer main_clock;


/*Setup the encoders */
RotaryEncoder encode1(2, 3);



/************************* Defines ********************************/
#define DEBUG 1

#if DEBUG == 1
#define dprint(expression) Serial.print("# "); Serial.print( #expression ); Serial.print( ": " ); Serial.println( expression )
#define dshow(expression) Serial.println( expression )
#else
#define dprint(expression)
#define dshow(expression)
#endif

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

/************************** Variables *****************************/
/*Default BPM */
uint16_t BPM = 120;
uint8_t clock_byte;

/*Hold position on the encoders */
int16_t right = BPM;

/*Mode */
boolean clock_mode = true;

/*Encoder Step Mode */
boolean step_by_ten = true;


/**************************  Functions ****************************/
void set_bpm(uint16_t new_bpm) {
  /*Timer One is running at 1000 hertz */

  uint16_t temp_bpm = (60000 / new_bpm) / 4;

  /*Base clock speed */
  main_clock.setCompare(temp_bpm);

}/*end set bpm */


/******************************************************************/
/*************************** Setup ********************************/
/******************************************************************/
void setup() {
  if (DEBUG) {
    Serial.begin(115200);
  }
  encode1.setPosition(BPM);


  /*Set up the main clock */
  set_bpm(BPM);

  /************************* Setup Pins ***************************/
  /*Encoder Button */
  DDRD &= ~_BV (4); // pinMode (4, INPUT);
  /*Clock / 2 */
  DDRB |= _BV (0); // pinMode (8, OUTPUT);
  /*Main Clock */
  DDRB |= _BV (1); // pinMode (9, OUTPUT);
  /*Clock * 2 */
  DDRB |= _BV (2); // pinMode (10, OUTPUT);
  /*Clock / 2 LED */
  DDRB |= _BV (3); // pinMode (11, OUTPUT);
  /*Main Clock LED */
  DDRB |= _BV (4); // pinMode (12, OUTPUT);
  /*Clock * 2 LED */
  DDRB |= _BV (5); // pinMode (13, OUTPUT);

  /************************* Setup the display  **************/
  matrix.begin(0x70);
  matrix.setBrightness(3);     //0 least  15 max
  matrix.print(BPM);
  matrix.writeDisplay();

  /*************************  Setup Timer1 ************************/
  cli();                //stop interrupts
  //set timer1
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register
  OCR1A = 14; //
  // turn on CTC mode
  sbi(TCCR1B, WGM12);
  /*Set prescaler to 1024 */
  sbi(TCCR1B, CS12);
  cbi(TCCR1B, CS11);
  sbi(TCCR1B, CS10);
  // enable timer compare interrupt
  sbi(TIMSK1, OCIE1A);
  sei();

}/**************************  End Setup **************************/
ISR(TIMER1_COMPA_vect) {

  main_clock.count();

}

/*Debounce the button */
elapsedMillis button_time;

/******************************************************************/
/**************************** Loop ********************************/
/******************************************************************/
void loop() {

  if (main_clock.check()) {
    /*Increment the clock count */
    clock_byte ++;
    uint8_t fast = bitRead(clock_byte, 0);
    uint8_t main = bitRead(clock_byte, 1);
    uint8_t slow = bitRead(clock_byte, 2);
    /*Set the pins */
    if (fast) {
      PORTB |= _BV (2); // digitalWrite (10, HIGH);
      PORTB &= ~_BV (5); // digitalWrite (13, LOW);
    } else {
      PORTB &= ~_BV (2); // digitalWrite (10, LOW);
      PORTB |= _BV (5); // digitalWrite (13, HIGH);
    }
    if (main) {
      PORTB |= _BV (1); // digitalWrite (9, HIGH);
      PORTB &= ~_BV (4); // digitalWrite (12, LOW);
    } else {
      PORTB &= ~_BV (1); // digitalWrite (9, LOW);
      PORTB |= _BV (4); // digitalWrite (12, HIGH);
    }
    if (slow) {
      PORTB |= _BV (0); // digitalWrite (8, HIGH);
      PORTB &= ~_BV (3); // digitalWrite (11, LOW);
    } else {
      PORTB &= ~_BV (0); // digitalWrite (8, LOW);
      PORTB |= _BV (3); // digitalWrite (11, HIGH);
    }

    main_clock.init();
  }


  /*Get the encoder pos */
  encode1.tick();
  int16_t new_right = encode1.getPosition();
  
  /*Has it changed */
  if (new_right != right) {
    /*Which step mode */
    if (step_by_ten) {
      /*Is the new number larger or smaller than old */
      if (new_right > right) {
        /*If larger add 9 */
        new_right += 9;
        encode1.setPosition(new_right);
      } else {
        /*If smaller subtract 9 */
        new_right -= 9;
        encode1.setPosition(new_right);
      }
    }
    /*If not in step by 10 mode just count */

    /*Check the range */
    if (new_right <= 10) {
      new_right = 10;
      encode1.setPosition(new_right);
    }
    if (new_right >= 1000) {
      new_right = 1000;
      encode1.setPosition(new_right);
    }
    /*Store the new BPM */
    BPM = new_right;
    /*Set the new BPM */
    set_bpm(new_right);
    /*Update the display */
    matrix.print(BPM);
    matrix.writeDisplay();
    /*Store the new position for compare */
    right = new_right;
  }
  /*If the encoder button is pressed switch between
     count by 10 and count by 1
  */
  if (!(PIND & _BV (4)) && button_time > 200) {
    step_by_ten = !step_by_ten;
    button_time = 0;
  }

}/*************************** End Loop *****************************/
