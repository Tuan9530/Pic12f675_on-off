#include <12F675.h>
#fuses INTRC_IO,NOWDT,NOPUT,NOPROTECT,NOCPD,NOMCLR
#use delay(clock=4000000)

#define DATA_INFR      PIN_A2
#define led2           PIN_A5
#define led1           PIN_A4
#define led            PIN_A1
#define BTON1          PIN_A0
#define BTON2          PIN_A3

unsigned int8 key, tam1 = 0, tam2 = 0, hl1 = 0, hl2 = 0, step = 0, dem1 = 0;
unsigned int32 DataIR = 0, dem = 0, dem2 = 0, cmd = 0xffffffff;      // Ma cac kenh cua remote 

#byte T1CON = 0x10
#bit TMR1ON = T1CON.0

#int_timer1
void interrupt_timer1() // 100us
{
   dem1++;
   dem2++;
   set_timer1(65479);  
} 

void nhanma()
{
   TMR1ON = 0;   // Timer1 off
   dem1 = 0;
   dem2 = 0;
   step = 0;
   cmd = 0;
   
   while(dem2 < 10000) // moi lan nhan hong ngoai toi da 1s
   {
      if(step == 0)  //bit start co do dai xung thap > 2000us
      {
         set_timer1(65479);
         TMR1ON = 1;   // Timer1 on
         
         while(input(DATA_INFR) == 0)
         {
            if(dem1 > 20 && dem1 <= 200) // dung la xung start
               step = 1;
            
            else if(dem1 > 200) // xung start dai toi da 20ms
            {
               step = 0;
               break;   // bit start qua dai, giai ma that bai
            }  
         }
         
         if(dem1 > 200) // neu xung start qua dai, thoat khoi ham giai ma
            break;
            
         else  // neu dung la xung start
         {
            TMR1ON = 0;   // Timer1 off
            dem1 = 0;
            set_timer1(65479);
            TMR1ON = 1;   // Timer1 on
         }
         
         while(input(DATA_INFR) == 1)
         {
            if(dem1 > 200) // chong treo chuong trinh sau 20ms
            {
               step = 2;
               break;
            }
         }
         TMR1ON = 0;   // Timer1 off
         dem1 = 0;
         set_timer1(65479);
         TMR1ON = 1;   // Timer1 on
      }
      
      //--------------------------------------------------------------------------------------------------------------------- 
      else if(step == 1) // bit 1 có tong do dai xung thap va cao < 1500us, bit 0 có tong do dai xung thap va cao > 1500us
      {
         output_high(led);
         
         while(input(DATA_INFR) == 0)
         {
            if(dem1 > 200) // chong treo chuong trinh sau 20ms
            {
               step = 2;
               break;
            }
         }
         while(input(DATA_INFR) == 1)
         {
            if(dem1 > 20) // bit stop có xung cao > 2000 us
            {
               step = 2;
               break;
            }
         }
         
         if(dem1 > 15)
         {
            cmd = cmd << 1;   // bit 0
            
            TMR1ON = 0;   // Timer1 off
            dem1 = 0;
            set_timer1(65479);
            TMR1ON = 1;   // Timer1 on
         }
         
         else if(dem1 <= 15)
         {
            cmd = cmd + 1; // bit 1
            
            TMR1ON = 0;   // Timer1 off
            dem1 = 0;
            set_timer1(65479);
            TMR1ON = 1;   // Timer1 on
         }
      }
      
      //-------------------------------------------------------------------------------------------------------------------------
      else if(step == 2)  // phat hien bit stop
      {
         TMR1ON = 0;   // Timer1 off
         disable_interrupts(INT_TIMER1); // cam ngat timer1 de luu ma IR nhan duoc
         output_low(led);
         break;
      }
   }
   delay_ms(100); // delay chong nhieu
}

//========================Doc du lieu tu eeprom========================
unsigned int32 read_code(char BT)
{
unsigned char i, x[4];
unsigned int32 tmp, DataIRR;
  DataIRR = 0;
  for(i = 0; i < 4; i++) {
    x[i] = read_eeprom(i+BT*4);
    tmp = x[i];  // kiem tra lai gia tri luu vao
    tmp = tmp<<(24-8*i);
    DataIRR = DataIRR + tmp;
    }
  return DataIRR;
}

//!//=======================Luu du lieu vao eeprom========================
void save_code(char BT)
{
char i, x[4];
  for(i = 0; i < 4; i++) {
    x[i] = DataIR>>(24-8*i); 
    write_eeprom(i+BT*4,x[i]);
    }
}

//==========================Kiem tra nut nhan==========================
unsigned char scan_BT()
{
unsigned char i;  
  if(input(BTON1) == 0) // kiem tra BT1
  {      
    delay_us(100);     // chong nhieu
    if(tam1 == 0) 
    {
      tam1 = 1;
      for(i=0;i<30;i++) // kiem tra BT nhan giu 3s
      {  
        delay_ms(100);
        if(input(BTON1)==1) 
          return 1;
      }
      return 11;
    }
  }

 else if(input(BTON2) == 0) // kiem tra BT1
  {      
    delay_us(100);     // chong nhieu
    if(tam2 == 0) 
    {
      tam2 = 1;
      for(i = 0; i < 30; i++) // kiem tra BT nhan giu 3s
      {  
        delay_ms(100);
        if(input(BTON2)==1) 
          return 2;
      }
      return 22;
    }
  }
  else 
   tam1 = tam2 = 0;
   return 0;
}

//=========================Chuong trinh chinh==========================
void main()
{
   set_tris_a (0b11001101); // 0 la output, 1 la input
   setup_comparator (NC_NC_NC_NC); // tat so sanh
   setup_adc_ports (NO_ANALOGS); // tat dau vao tuong tu
   setup_adc (ADC_OFF); // vô hieu hóa A2D
   port_a_pullups(0b00000101); // dien tro noi keo len, 1 la bat, 0 la tat
   output_low(led);
   output_low(led1);
   output_low(led2);
   
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1); //T0 dem xung noi, ti le chia 2
   enable_interrupts(GLOBAL);
   set_timer1(65479); // Thach anh 4 MHz => CK may = 4/4 = 1us
   
   while(TRUE)
   {
      key = scan_BT();  // quét bàn phím
      if(input(DATA_INFR) == 0)
      {
         enable_interrupts(INT_TIMER1); // cho phep ngat timer1
         nhanma();
         
         if(cmd == read_code(1))
         {
            output_toggle(led1);
            delay_ms(50);
            cmd = 0;
         }
         
         if(cmd == read_code(2))
         {
            output_toggle(led2);
            delay_ms(50);
            cmd = 0;
         }  
      }  
      
      if(key != 0)
      {
         if(key == 1 )
         {
            output_toggle(led1);
            delay_ms(200);
         }
         
         if(key == 2)
         {
            output_toggle(led2);
            delay_ms(200);
         }
         
            
         if(key == 11 || key == 22)
         {
            output_low(led1);
            output_low(led2);
            output_high(led);
            hl1 = 1;
         
            if(hl1 == 1)
            {
               cmd = 0;
               dem = 0;
               while(cmd == 0)  // bat dau hoc
               { 
                   while(dem < 115384)
                   {
                      if(input(DATA_INFR ) == 0)
                      {
                        enable_interrupts(INT_TIMER1); // cho phep ngat timer1
                        nhanma();
                      }
                        
                      if(cmd != 0)
                      {
                        DataIR = cmd;
                        save_code(1);
                        output_low(led);
                        delay_ms(100);
                        output_high(led);
                        hl1 = 0;
                        hl2 = 1;
                        break;
                      }
                      delay_us(1);
                      dem++;
                   }
                   if(cmd == 0)
                   {
                      output_low(led);
                      hl1 = hl2 = 0;
                      break;
                   }
               } // hoc xong kenh 1

            }
            
            if(hl2 == 1)
            {
               cmd = 0;
               DataIR = 0;
               
               while(cmd == 0)  // bat dau hoc
               {         
                   for(dem = 0; dem < 115384; dem++)
                   {
                      if(input(DATA_INFR ) == 0)
                      {
                        enable_interrupts(INT_TIMER1); // cho phep ngat timer1
                        nhanma();
                      } 
                        
                      if(cmd != 0)
                      {
                        DataIR = cmd;
                        save_code(2);
                        output_low(led);
                        hl1 = 0;
                        hl2 = 0;
                        break;
                      }
                      delay_us(1);
                   }
                   if(cmd == 0)
                   {
                      output_low(led);
                      hl1 = hl2 = 0;
                      break;
                   }
               } // hoc xong kenh 2
            }           
            cmd = 0;
         }
         key = 0;
      }
   }
}
