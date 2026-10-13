#include<LPC21xx.H>
#include"lcd.h"
#include"keypad_init.h"
#include"project_title.h"
#include"uart.h"
#define IN1 1<<23
#define IN2 1<<24
#define EN1 1<<25
unsigned char key_lut[4][4]={{7,8,9,'/'},{4,5,6,'x'},{1,2,3,'-'},{'@',0,'=','+'}};
unsigned int batch;
volatile unsigned int count=0;
unsigned int key_flag=0; 
unsigned int batch_count=0;
volatile unsigned int motor_flag=0;
//count_isr
void COUNT_ISR(void) __irq
{
EXTINT=0x01;
	if(motor_flag==1)
	count++;
VICVectAddr=0;
}
//end of count isr
//kill_switch_isr
void kill(void) __irq
{
EXTINT=0x02;
		LCD_COMMAND(0X01);
		LCD_STRING("TERMINATED");
		delay_ms(1000);
		LCD_COMMAND(0X01);
VICVectAddr=0;
}
//end of kill isr
int main()
{
PINSEL1|=0X01;
PINSEL0|=(1<<29);
PINSEL0&=~(1<<28);
VICIntSelect=0x00;
VICVectAddr1=(unsigned long)COUNT_ISR;
VICVectCntl1=(0x20)|14;
VICVectAddr0=(unsigned long)kill;
VICVectCntl0=(0x20)|15;
EXTMODE=0x03;
EXTPOLAR=0x03;
VICIntEnable=(1<<14)|(1<<15);
proj_title();
uart_config();
while(1){	
batch_count++;
LCD_COMMAND(0X01);
LCD_STRING("ENTER BATCH ");
LCD_DATA(batch_count+48);
LCD_DATA(':');
LCD_COMMAND(0XC0);
while(key_flag==0){
keypad_init();
value=key_lut[row_val][col_val];	
	if(value=='@'){
		LCD_COMMAND(0X01);
		LCD_STRING("TERMINATED");
		delay_ms(1000);
		LCD_COMMAND(0X01);
		return 0;
	}
if(value!='='){	
	if(value<10){
		LCD_DATA(value+48);
		batch=(batch*10)+value;
	}
	else{
		LCD_COMMAND(0X01);
		LCD_STRING("INVALID KEY");
		delay_ms(1000);
		LCD_COMMAND(0X01);
		LCD_STRING("ENTER BATCH ");
		LCD_DATA(batch_count+48);
		LCD_DATA(':');
		LCD_COMMAND(0XC0);
		batch=0;
		}
	}
if(value=='='){
	LCD_COMMAND(0X01);
	LCD_STRING("BATCH:");
	LCD_COMMAND(0XC0);
	LCD_INT(batch);
	delay_ms(2000);
	LCD_COMMAND(0X01);
	key_flag=1;
	LCD_STRING("MOTOR ON");
	break;
}
}

while(count<batch){
	IODIR0|=IN1|IN2|EN1;
	IOSET0=IN1|EN1;
	IOCLR0=IN2;;
	motor_flag=1;
}
	LCD_COMMAND(0X01);
	LCD_STRING("MOTOR OFF");
	IOCLR0 = EN1;
	motor_flag=0;
	delay_ms(1000);
	LCD_COMMAND(0X01);
	LCD_STRING("sending sms...");
    uart_str("AT\r\n");
    delay_ms(1000);
    uart_str("AT+CMGF=1\r\n");
    delay_ms(1000);
		uart_str("AT+CMGS=\"+919876543210\"\r\n");
    delay_ms(1000);
    uart_str("Batch ");
		uart_int(batch_count);
		uart_str(" completed.\r\nQuantity: ");
		uart_int(count);
		uart_str("\r\n");
    uart_tx(26);  


	key_flag=0;
	count=0;
	batch=0;
}
}


