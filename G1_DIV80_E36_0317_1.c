/*
     Project:   Trigger Divider
     O3Code:    G1.DIV80.E36.0317.1
     MCU:       PIC16F628A
     Author:    Vitaliy Postavnichiy
     Date:      09.06.2017
     Compiller: MicroC
*/

#define LEDIN PORTA.RA0
#define LEDBL PORTA.RA3
#define LEDGR PORTA.RA2
#define LEDRE PORTA.RA1
#define BTN   PORTA.RA6 // IN
#define RESET PORTB.RB1 // IN
#define DIV2  PORTB.RB2
#define DIV3  PORTB.RB3
#define DIV4  PORTB.RB4
#define DIV5  PORTB.RB5
#define DIV6  PORTB.RB6
#define DIV7  PORTB.RB7
#define DIV8  PORTA.RA7
#define MAX68 24        // Maximum for 2,3,4,6,8 after that cycle repeat
#define MAX57 35        // Maximum for 5,7       after that cycle repeat

bit  refresh;                  // Refresh flag
bit  btn_block;
bit  reset_flg;                // When reset was up
char counter68, counter57;     // counters for two main cycles
char modes[4] = {1,2,4,8};     // subdivider modes
char mode;                     // subdivider mode value, accessed by button
char mode_counter;             // current mode count value

void clearout(){
     PORTB = 0;
     DIV8 = 0;
     LEDIN = 0;
}

void reset(){
     counter68 = 0;
     counter57 = 0;
     reset_flg = 1;
     refresh   = 1;
}

void set_mode_leds(){
     LEDBL=0;
     LEDGR=0;
     LEDRE=0;
     switch(mode){
          case 1: LEDBL = 1; break;
          case 2: LEDGR = 1; break;
          case 3: LEDRE = 1; break;
     }
}

void modeup(){
     mode = (mode == 3)?0:mode+1;
     mode_counter = 0;
     set_mode_leds();
}

void init(){
     TRISA  = 0x40;
     TRISB  = 0x03;
     PORTA  = 0;
     PORTB  = 0;
     CMCON  = 0x07;
     T1CON  = 0x30;  // T1 On
     OPTION_REG = 0xC7;
     INTCON = 0xF0; // maybe need set the INT on UP-Front
     PIE1   = 0x01; // Enable Timer1 interrupt
}

void interrupt(){
     if(INTCON.INTF){ // INT pin interrupt
          LEDIN = 1;
          if(modes[mode] == mode_counter+1){
               counter68 = (counter68 < MAX68)?(counter68+1):1;
               counter57 = (counter57 < MAX57)?(counter57+1):1;
               mode_counter = 0;
               refresh = 1;
          }else{
               mode_counter++;
          }
          
          INTF_bit = 0;
     }
     if(INTCON.T0IF){ // Timer0 interrupt
          clearout();
          TMR0IF_bit = 0;
     }
     if(PIR1.TMR1IF){ // Timer1 interrupt (for button)
          TMR1ON_bit = 0;
          if(BTN) modeup();
          TMR1IF_bit = 0;
     }
}

void main() {
     init();
     reset();
     mode = 0;
     mode_counter = 0;
     set_mode_leds();
     btn_block = 0;
     reset_flg = 0;
     
     while(1){
          if(RESET && !reset_flg) reset(); // If UP on RESET pin
          if(!RESET && reset_flg) reset_flg = 0;
          if(refresh){
               if(counter68!=0){
                   if(counter68 % 2 == 0){ // If divides on 2,4,8
                        DIV2 = 1;
                        if(counter68 % 4 == 0){
                            DIV4 = 1;
                            if(counter68 % 8 == 0){
                                 DIV8 = 1;
                            }
                        }
                   }
                   if(counter68 % 3 == 0){
                        DIV3 = 1;
                        if(counter68 % 6 == 0){
                             DIV6 = 1;
                        }
                   }
               }

               if(counter57!=0){
                    if(counter57 % 5 == 0) DIV5 = 1;
                    if(counter57 % 7 == 0) DIV7 = 1;
               }

               TMR0 = 0xF0; // Trigger Time
               refresh = 0;
          }
          if(!btn_block && BTN){
               btn_block = 1;
               TMR1H = 0xFF;
               TMR1L = 0x00;
               TMR1ON_bit = 1;
          }else if(btn_block && !BTN){
               btn_block = 0;
          }
          
     }
}