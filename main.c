/*
 * File:   main.c
 * Author: shummu
 *
 * Created on 24 September, 2023, 5:44 PM
 */


#include "main.h"

#pragma config WDTE = OFF //watching timer disabled

unsigned char sec =0, min =0,flag =0;//0
int operation_flag = POWER_ON_SCREEN; 
static void init_config(void) {
    //Initialization of CLCD Module
    init_clcd();
    
    //Initialization of MKP Module
    init_matrix_keypad();
    
    //RC2  pin as a output
    FAN_DDR =0;
    FAN =OFF;//turn off the fan
    
    BUZZER_DDR=0;//RC1 pin as a output pin
    BUZZER =OFF;
    
    //Initialization of timer module
     init_timer2();
     
     PEIE = 1;
     GIE =1;
}

void main(void) {
    init_config(); //calling initializing function 
    
    //Variable declaration
 
    unsigned char key;
    int reset_flag;
    while (1) {
        key = read_matrix_keypad(STATE);//1
        if(operation_flag==MENU_DISPLAY_SCREEN)
        {
           //SW 1 is pressed
         if(key == 1)
         {
          operation_flag =MICRO_MODE;
          reset_flag = MODE_RESET;
          clear_screen();
          clcd_print(" power = 900W   ",LINE2(0));
          __delay_ms(3000); //3sec
          clear_screen();
           }
          else if (key == 2) //Grill mode
          {
          operation_flag =GRILL_MODE;
          reset_flag = MODE_RESET;
          clear_screen();
          }
        
           else if (key == 3) //convection
           {
             operation_flag =CONVECTION_MODE;
             reset_flag = MODE_RESET;
             clear_screen();
           }
           else if (key == 4) //start mode
           {
             sec=30;
             min=0;
             FAN=ON;//turning on the fan
             TMR2ON =ON;//turning on timer
             operation_flag =TIME_DISPLAY;
             clear_screen();
            } 
         }
         else if (operation_flag ==TIME_DISPLAY)
         {
             if (key ==4)//start
             {
             //Increment time by 30sec
             sec= sec+ 30;//sec =50+30=80
             //sec==60>60
              if(sec> 59)
             {
                 min++;
                 sec =sec -60;
             }
         }
           else if (key ==5)//pause
           {
              operation_flag=PAUSE; 
           }
           else if (key ==6)//stop
           {
               operation_flag=STOP;
               clear_screen();
           }
         }
         else if(operation_flag== PAUSE)
         {
             if (key ==4)//resume
             {
               FAN=ON;
               TMR2ON =ON;
               operation_flag =TIME_DISPLAY;
             }
         }
       
        switch(operation_flag)
        {
            case POWER_ON_SCREEN:
                power_on_screen();
                operation_flag= MENU_DISPLAY_SCREEN;//menu display screen
                clear_screen();
                break;
            case MENU_DISPLAY_SCREEN:
                menu_display_screen();
                break;
            case GRILL_MODE:
                set_time(key ,reset_flag);
                break;
            case MICRO_MODE:
                set_time(key ,reset_flag);
                break;
            case CONVECTION_MODE:
                if(flag==0)
                {
                    set_temp(key ,reset_flag);
                    if(flag==1)//#key
                    {
                        clear_screen();
                       reset_flag =MODE_RESET; 
                       continue;
                    }
                }
                else if(flag ==1)
                {
                    set_time(key,reset_flag);
                }
                break;
            case TIME_DISPLAY:
                time_display_screen();
                break;    
            case PAUSE:
                 FAN=OFF;
                TMR2ON =OFF;
                break;
            case STOP:
                FAN=OFF;
                TMR2ON =OFF;
                operation_flag= MENU_DISPLAY_SCREEN; 
                break;
        }
        
          reset_flag=RESET_NOTHIMG ;    
    }
}

void time_display_screen(void)          
{
     //Line11 display]
     clcd_print(" TIME =  ", LINE1(0));
     //PRINT MIN AND SEC
     clcd_putch( min/10 + '0', LINE1(9));
     clcd_putch( min%10 + '0', LINE1(10));
     clcd_putch(':', LINE1(11));
    
     //SEC
     clcd_putch( sec/10 + '0', LINE1(12));
     clcd_putch( sec%10 + '0', LINE1(13));
     
     //print options
     clcd_print(" 4.Start/Resume",LINE2(0));
     clcd_print(" 5.Pause ",LINE3(0));
     clcd_print(" 6.Stop ",LINE4(0));
     
     if(sec ==0 && min ==0)
     {
        clear_screen();
        clcd_print(" Time UP !!",LINE2(0));
        BUZZER =ON;
        __delay_ms(3000);//3sec
        clear_screen(); 
        BUZZER =OFF;
        FAN =OFF;//TURN OFF FAN
        TMR2ON=OFF;
        operation_flag= MENU_DISPLAY_SCREEN;
     }
}

void clear_screen(void)
{
   clcd_write(CLEAR_DISP_SCREEN, INST_MODE);
    __delay_us(500);
}
void power_on_screen(void)
{
    unsigned char i;
    //LINE1->printing Bar on line1
    for(i=0; i < 16; i++)
    {
    //clcd_write(0xFF, DATA_MODE);//0b1111 1111
        clcd_putch(BAR, LINE1(i));//i=0 1 2 3 4 5...
        __delay_ms(150);
    }
   //Writing messages in Line 2&3
    clcd_print("  Powering ON   ", LINE2(0));
    clcd_print(" Microwave Oven ", LINE3(0));
    
     for(i=0; i < 16; i++)
     {
         clcd_putch(BAR, LINE4(i));
         __delay_ms(100);
     }
     //Delay
     __delay_ms(3000);//3sec
}

void menu_display_screen(void)
 { 
    clcd_print("1.Micro" ,LINE1(0));//Line1 
    clcd_print("2.Grill" ,LINE2(0));
    clcd_print("3.Convection" ,LINE3(0));
    clcd_print("4.Start" ,LINE4(0));
    
 }

void  set_temp(unsigned char key,int reset_flag)
{
     static unsigned char key_count, blink,temp;
     static int wait; 
     if (reset_flag == MODE_RESET)
     {
         key_count =0;
         wait =0;
         blink =0;
         temp =0;
         flag =0;
         key =ALL_RELEASED;
         clcd_print(" SET TEMP (oC)  ",LINE1(0));
         clcd_print(" TEMP = ", LINE2(0));
         //sec => 0 to 59
         clcd_print("*:CLEAR  #:ENTER", LINE4(0));
     }
     //reading the temp
     if((key !='*')&& (key !='#') && (key!= ALL_RELEASED))
     {
         //key=1 5 4 7 
         key_count++;//1 2 3
                   //1
         if(key_count <=3)//reading number of temp 
         {
             temp = temp*10 + key; // temp=1 
         } 
     }
     else if(key ==  '*')  //sec or min
     {
         temp=0;
         key_count=0;
     }
     else if (key =='#')//enter key
     {
         clear_screen();
        //start the pre-heating
         clcd_print("  Pre-Heating ",LINE1(0));
         clcd_print("Time Rem.= ",LINE3(0));
         TMR2ON=1;//switching on the timer
         sec =180;
         while(sec!=0)//sec==0,after 180sec
         {
              //temp= 123
         clcd_putch((sec/100) + '0',LINE3(11)); //8 =123/100->1
         clcd_putch((sec/10)%10 + '0',LINE3(12));//9 =123/10=12%10=2
         clcd_putch((sec%10) + '0',LINE3(13));//10=3
         }
         //SEC=0
         if(sec ==0)
         {
             flag=1;
             TMR2ON =0;
             //set time screen exactly like grill mode
             //operation_flag =GRILL_MODE;
             
              
         }
     }
    
     if(wait++ == 15)//0 1 ...15
     {
        wait = 0;
        blink = !blink;//0
         //printing temp on the set temp screen
     //temp= 123
     clcd_putch((temp/100) + '0',LINE2(8)); //8 =123/100->1
     clcd_putch((temp/10)%10 + '0',LINE2(9));//9 =123/10=12%10=2
     clcd_putch((temp%10) + '0',LINE2(10));//10=3
   } 
     if(blink)
     {
         clcd_print("   ",LINE2(8)); 
        
      }  
}

void set_time(unsigned char key,int reset_flag)
{
     static unsigned char key_count,blink_pos , blink;
     static int wait;
     if (reset_flag == MODE_RESET)
     {
         key_count =0;
         sec =0;
         min =0;
         blink_pos =0;//SEC
         wait=0;
         blink=0;
         key =ALL_RELEASED;
        clcd_print("SET TIME (MM:SS)",LINE1(0));
        clcd_print("TIME-", LINE2(0));
        //sec => 0 to 59
        clcd_print("*:CLEAR  #:ENTER", LINE4(0));
    
     }
    
     if((key !='*')&& (key !='#') && (key!= ALL_RELEASED))
     {
         //key=1 5 4
         key_count++;//1 2 3
                   
         if(key_count <=2)//reading number of sec
         {
             sec = sec*10 + key; // sec-=15 
             blink_pos =0;
         } 
         else if((key_count> 2) && (key_count <=4))
         {
              min = min*10 + key;
               blink_pos =1;
         }
     }
     else if(key ==  '*')  //sec or min
     {
         if(blink_pos ==0)//To clear sec 
         {
             sec =0;
             key_count=0;
         }
         else if (blink_pos ==1)
         {
             min =0;
             key_count=2;
         }
     }
     else if (key =='#')//enter key
     {
         clear_screen();
         operation_flag =TIME_DISPLAY;
         FAN =ON;//TURN ON THE FAN
         /*Switching off the timer*/
         TMR2ON = ON;
     }
    
     if(wait++ == 15)//0 1 ...15
     {
        wait = 0;
        blink = !blink;//0
         //printing Sec and min on the set line screen
        //Min 
         clcd_putch(min/10 + '0', LINE2(6));
         clcd_putch(min%10 + '0', LINE2(7));
         clcd_putch(':', LINE2(8));
    
         //SEC
         clcd_putch( sec/10 + '0', LINE2(9));
         clcd_putch(sec%10  + '0', LINE2(10));
     }
     
     if(blink)
     {
         switch (blink_pos)
         { 
             case 0://sec
                 clcd_print("  ",LINE2(9));
                 break;
             case 1://min
                 clcd_print("  ",LINE2(6));
                 break;
         }
         clcd_print("  ",LINE2(9));
         clcd_print("  ",LINE2(6));
     }
           
 }  