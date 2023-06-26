

// LCD module connections
 sbit LCD_RS at RA0_bit;
 sbit LCD_EN at RA1_bit;
 sbit LCD_D4 at RB4_bit;
 sbit LCD_D5 at RB5_bit;
 sbit LCD_D6 at RB6_bit;
 sbit LCD_D7 at RB7_bit;
 sbit LCD_RS_Direction at TRISA0_bit;
 sbit LCD_EN_Direction at TRISA1_bit;
 sbit LCD_D4_Direction at TRISB4_bit;
 sbit LCD_D5_Direction at TRISB5_bit;
 sbit LCD_D6_Direction at TRISB6_bit;
 sbit LCD_D7_Direction at TRISB7_bit;
// End LCD module connections

 sbit MODE at RA2_bit;
 sbit SELECT at RA3_bit;
 sbit ENTER at RA4_bit;
 sbit START at RB0_bit;
 sbit RelaySW at RB3_bit;

// Define Messages
 char MSG1[] = "Device is ";
 char MSG2[] = "OFF";
 char MSG3[] = "ON ";
 char MSG4[] = "HH:MM";

 unsigned short HHMM_Pos[] = {6, 7, 8, 9, 10};
 unsigned short ON_Time[] = {0, 0, 10, 0, 0}; // 10 + 48 is :
 unsigned short OFF_Time[] = {0, 0, 10, 0, 0};

 unsigned short Mode_Select = 0 ; // 0:ON, 1:OFF
 unsigned short i, j, k, Timer_On, Get_Input, Cur_Pos, Cur_On;
 unsigned short temp, refresh, Num, HalfSec, Blink, ChangeMin=0;
 unsigned short OFF_HH, OFF_MM, ON_HH, ON_MM;

 void Disp_First_Row(){
  Lcd_Out(1,1, MSG1);
  if (RelaySW == 0) Lcd_Out(1,11, MSG2);
  if (RelaySW == 1) Lcd_Out(1,11, MSG3);
 }

 void Disp_Char(unsigned short col, unsigned short chr){
  Lcd_Chr(2, col, chr+48);
 }

 void Disp_Time(){

   for(i=0; i<5; i++){
      if(!Mode_Select){
       Lcd_Out(2,1, MSG2);
       Disp_Char(HHMM_Pos[i],OFF_Time[i]);
      }
      if(Mode_Select){
       Lcd_Out(2,1, MSG3);
       Disp_Char(HHMM_Pos[i],ON_Time[i]);
      }
   }
 }

 void play_sound(){
  Sound_Play(2500, 500);
 }

 void debounce(){
  Delay_ms(250);
 }

 void cursor_left(){
   for(j=0; j<5; j++){
      Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
   }
 }

 void disable_timer(){
  INTCON = 0x00;
  RelaySW = 0;
  INTCON.T0IF = 0;
  Timer_On = 0;
  Blink = 0xff;
  Mode_Select = 0;
  Disp_First_Row();
  Disp_Time();
  play_sound();
 }
 

 void interrupt() {
  Num ++;           // Interrupt causes Num to be incremented by 1
  if(Num == 9) {
   HalfSec ++;      // Increase sec
   Num = 0;
   Blink = ~Blink;
   if (HalfSec == 120){
    HalfSec = 0;
    ChangeMin = 1;
   }
  }
  TMR0 = 39;        // TMR0 returns to its initial value
  INTCON.T0IF = 0;  // Bit T0IF is cleared so that the interrupt could reoccur
}

void main() {
  CMCON = 7;           // Disable Comparators
  TRISA = 0b00111100;
  TRISB = 0b00000001;
  Sound_Init(&PORTB,2); // Initialize Buzzer o/p pin
  RelaySW = 0;
  Timer_On = 0;
  Get_Input = 0;
  Cur_Pos = 0;
  Cur_On = 0;
  refresh = 0;
  Num = 0;
  HalfSec = 0;

  Lcd_Init();                // Initialize LCD
  Lcd_Cmd(_LCD_CLEAR);       // Clear display
  Lcd_Cmd(_LCD_CURSOR_OFF);  // Cursor off
  Lcd_Out(1,1, "Copyright @");
  Lcd_Out(2,1, "Embedded-Lab.com");
  i=0;
  while(i<4){
   debounce();
   i ++;
  }
  Lcd_Cmd(_LCD_CLEAR);
  Disp_First_Row();
  Lcd_Out(2,4,"-");
  Lcd_Out(2,12, MSG4);
  Disp_Time();

  do {
   if(!MODE && !Timer_On){
    debounce();
    if(!Get_Input){
     Mode_Select = ~Mode_Select;
     Disp_Time();
    }
    if(Get_Input){
     if(!Mode_Select){
      OFF_time[Cur_Pos] = OFF_time[Cur_Pos]+1;
      temp = OFF_time[Cur_Pos];
      switch (Cur_Pos){
       case 0: if(temp > 9) OFF_time[Cur_Pos]=0;
               break;
       case 1: if(temp > 9) OFF_time[Cur_Pos]=0;
               break;
       case 3: if(temp > 5) OFF_time[Cur_Pos]=0;
               break;
       case 4: if(temp > 9) OFF_time[Cur_Pos]=0;
               break;
      }
      Disp_Char(6+Cur_Pos, OFF_time[Cur_Pos]);

     }

     if(Mode_Select){
      ON_time[Cur_Pos] ++;
      temp = ON_time[Cur_Pos];
      switch(Cur_Pos){

       case 0: if(temp > 9) ON_time[Cur_Pos]=0;
               break;
       case 1: if(temp > 9) ON_time[Cur_Pos]=0;
               break;
       case 3: if(temp > 5) ON_time[Cur_Pos]=0;
               break;
       case 4: if(temp > 9) ON_time[Cur_Pos]=0;
               break;
      }
      Disp_Char(6+Cur_Pos, ON_time[Cur_Pos]);
     }
     Lcd_Cmd(_LCD_MOVE_CURSOR_LEFT);
    }
   }   // END if(!MODE)

   if(!SELECT && !Timer_On){
     debounce();
     Get_Input = 1;

     if(Cur_On) {
       Cur_Pos ++;
       if (Cur_Pos == 2) {
         Lcd_Cmd(_LCD_MOVE_CURSOR_RIGHT);
         Cur_Pos ++;
       }
       if(Cur_Pos > 4) {
         Lcd_Cmd(_LCD_MOVE_CURSOR_RIGHT);
         Cur_Pos = 0;
         cursor_left();
       }
       else Lcd_Cmd(_LCD_MOVE_CURSOR_RIGHT);
     }

     if(!Cur_On) {
       Cur_On = 1;
       cursor_left();
       Lcd_Cmd(_LCD_UNDERLINE_ON);
     }
   }

   if(!ENTER && Get_Input){
    debounce();
    Get_Input = 0;
    Cur_On = 0;
    Cur_Pos = 0;
    Disp_Time();
    Lcd_Cmd(_LCD_CURSOR_OFF);  // Cursor off
   }

   if (!START && !Get_Input){
    debounce();
    switch(Timer_On){
    case 0: play_sound();
            Timer_On = 1;
            OPTION_REG = 0x07; // Prescaler (1:256) is assigned to the timer TMR0
            TMR0 = 39;          // Timer T0 counts from 39 to 255
            INTCON = 0xA0;     // Enable interrupt TMR0 and Global Interrupts
            INTCON.T0IF = 0;
            Mode_Select = 0;
            Blink = 0;
            Disp_Time();
            break;
    case 1: disable_timer();
            break;
    }
   }

   if(Timer_On){
    OFF_HH = OFF_Time[0]*10 + OFF_Time[1];
    OFF_MM = OFF_Time[3]*10 + OFF_Time[4];
    ON_HH = ON_Time[0]*10 + ON_Time[1];
    ON_MM = ON_Time[3]*10 + ON_Time[4];
    switch(Blink){
     case 0: Lcd_Chr(2,8,' ');
             break;
     case 255: Lcd_Chr(2,8,':');
             break;
    }

    if(!OFF_HH && !OFF_MM &&!RelaySW){
     if(ON_HH || ON_MM){
      RelaySW = 1;
      Mode_Select = 0xff;
      Disp_First_Row();
      Disp_Time();
      play_sound();
     }
     else {
      disable_timer();
     }
    }

    if(!ON_HH && !ON_MM && RelaySW){
     disable_timer();
     play_sound();
    }

    if(ChangeMin) {
     switch(Mode_Select){
      case 0: if(OFF_MM == 0 && OFF_HH>0){
               OFF_MM = 59;
               OFF_HH -- ;
              }
              else if (OFF_MM >>0) OFF_MM --;
              OFF_Time[0] = OFF_HH/10;
              OFF_Time[1] = OFF_HH%10;
              OFF_Time[3] = OFF_MM/10;
              OFF_Time[4] = OFF_MM%10;
              break;

      case 255: if(ON_MM == 0 && ON_HH>0){
               ON_MM = 59;
               ON_HH -- ;
              }
              else if(ON_MM >> 0) ON_MM --;
              ON_Time[0] = ON_HH/10;
              ON_Time[1] = ON_HH%10;
              ON_Time[3] = ON_MM/10;
              ON_Time[4] = ON_MM%10;
              break;
     }
     ChangeMin = 0;
     Disp_Time();
    }

   }


  }while(1);
 }