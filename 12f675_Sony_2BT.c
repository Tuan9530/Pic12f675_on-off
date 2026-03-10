#include <12F675.h>
#fuses INTRC_IO,NOWDT,NOPUT,NOPROTECT,NOCPD,NOMCLR
#use delay(clock=4000000)

#define DATA_INFR      PIN_A2
#define led2           PIN_A5
#define led1           PIN_A4
#define led            PIN_A1
#define BTON1          PIN_A0
#define BTON2          PIN_A3

unsigned char key, i, a = 0, tam1 = 0, tam2 = 0, hl1 = 0, hl2 =0, hl3 = 0;
unsigned int32 DataIR = 0, dem = 0, dem1 = 0, cmd = 0xfffffff;      // Ma cac kenh cua remote 

void nhanma()
{
   cmd = 0;
   dem1 = 0;
   for(i = 0; i < 2; ++i)
   {
      delay_us(750);
      if(input(DATA_INFR)) return; // thoat khoi vong lap
   }
   while(!input(DATA_INFR));
   output_high(led);
   for(i = 0; i < 24; ++i)
   {
      cmd >>= 1; // dich sang fai 1 bit
      while(input(DATA_INFR))
      {
         while(dem1 < 28846 && input(DATA_INFR))
         {
            delay_us(1);
            dem1++;
         }
         break;
      }
      delay_us(750);
      if(!input(DATA_INFR))
      {
         cmd |= 0x800000; // cmd = cmd|0x800000 phep OR tung bit cmd voi 0x800000(=0b 1000 0000 0000 0000 0000 0000)
         while(!input(DATA_INFR));
      }
   }
   cmd >>= 1; // dich sang fai 1 bit
   output_low(led);
   delay_ms(100);
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
   
   while(TRUE)
   {
      key = scan_BT();  // quét bàn phím
      if(input(DATA_INFR ) == 0)
      {
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
         
         if(cmd == read_code(3))
         {
            if(a == 0)
            {
               output_high(led1);
               output_high(led2);
               a++;
               delay_ms(50);
               cmd = 0;
            }
            
            else if(a == 1)
            {
               output_low(led1);
               output_low(led2);
               a = 0;
               delay_ms(50);
               cmd = 0;
            }
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
                        nhanma(); 
                        
                      if(cmd != 0)
                      {
                        DataIR = cmd;
                        save_code(1);
                        output_low(led);
                        delay_ms(100);
                        output_high(led);
                        hl1 = hl3 = 0;
                        hl2 = 1;
                        break;
                      }
                      delay_us(1);
                      dem++;
                   }
                   if(cmd == 0)
                   {
                      output_low(led);
                      hl1 = hl2 = hl3 = 0;
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
                        nhanma(); 
                        
                      if(cmd != 0)
                      {
                        DataIR = cmd;
                        save_code(2);
                        output_low(led);
                        delay_ms(100);
                        output_high(led);
                        hl1 = 0;
                        hl2 = 0;
                        hl3 = 1;
                        break;
                      }
                      delay_us(1);
                   }
                   if(cmd == 0)
                   {
                      output_low(led);
                      hl1 = hl2 = hl3 = 0;
                      break;
                   }
               } // hoc xong kenh 2
            }
            
            if(hl3 == 1)
            {
               cmd = 0;
               DataIR = 0;
               
               while(cmd == 0)  // bat dau hoc
               {         
                   for(dem = 0; dem < 115384; dem++)
                   {
                      if(input(DATA_INFR ) == 0)
                        nhanma(); 
                        
                      if(cmd != 0)
                      {
                        DataIR = cmd;
                        save_code(3);
                        output_low(led);
                        hl1 = 0;
                        hl2 = 0;
                        hl3 = 0;
                        break;
                      }
                      delay_us(1);
                   }
                   if(cmd == 0)
                   {
                      output_low(led);
                      hl1 = hl2 = hl3 = 0;
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
