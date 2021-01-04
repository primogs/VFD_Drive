/////////////////////////////////////////////////////////////////////////////////////////
//    This file is part of VFD Drive.
//
//    Copyright (C) 2021 Matthias Hund
//    
//    This program is free software; you can redistribute it and/or
//    modify it under the terms of the GNU General Public License
//    as published by the Free Software Foundation; either version 2
//    of the License, or (at your option) any later version.
//    
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//    
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
/////////////////////////////////////////////////////////////////////////////////////////

const int PIN_NRST = 2;
const int PIN_NCS  = 3;
const int PIN_CLK  = 4;
const int PIN_DIN  = 5;
const int PIN_PUMP = 9;

const byte vfdNumOfDigits = 12;

enum DISP_LIGHT {NORMAL=0,ALL_OFF=1,ALL_ON=2};

void VfdReset()
{
  digitalWrite(PIN_NRST,LOW);
  delayMicroseconds(20);
  digitalWrite(PIN_NRST,HIGH);
  delayMicroseconds(20);
}

void VfdBegin()
{
  digitalWrite(PIN_NCS,LOW);
}

void VfdEnd()
{
  delayMicroseconds(20);
  digitalWrite(PIN_NCS,HIGH);
}

void VfdWriteByte(byte data)
{
  for(byte i=0;i<8;i++)
  {
    if(data & 0x1)
      digitalWrite(PIN_DIN,HIGH);
    else
      digitalWrite(PIN_DIN,LOW);
    digitalWrite(PIN_CLK,LOW);
    delayMicroseconds(1);
    digitalWrite(PIN_CLK,HIGH);
    delayMicroseconds(1);
    data >>=1;
  }
  delayMicroseconds(10);
}

// Data Control RAM 
void VfdWriteDCRAM(byte address, byte data)
{
  VfdBegin();
  const byte cmd = 0b00010000 | (address & 0b1111);
  VfdWriteByte(cmd);
  VfdWriteByte(data);
  VfdEnd();
}

// Character Generator RAM
void VfdWriteCGRAM(byte address, uint16_t data)
{
  VfdBegin();
  const byte cmd = 0b00100000 | (address & 0b1111);
  VfdWriteByte(cmd);
  VfdWriteByte(data>>8);
  VfdWriteByte(data & 0xFF);
  VfdEnd();
}

// Additional Data ROM
void VfdWriteADRAM(byte address, byte data)
{
  VfdBegin();
  const byte cmd = 0b00110000 | (address & 0b1111);
  VfdWriteByte(cmd);
  VfdWriteByte(data);
  VfdEnd();
}

void VfdSetDisplayDuty(byte brithness)
{
  VfdBegin();
  const byte cmd = 0b01010000 | (brithness & 0b1111);
  VfdWriteByte(cmd);
  VfdEnd();
}

void VfdSetNumberOfDigits(byte digits)
{
  VfdBegin();
  const byte cmd = 0b01100000 | (digits & 0b1111);
  VfdWriteByte(cmd);
  VfdEnd();
}

void VfdSetDisplayLight(DISP_LIGHT mode)
{
  VfdBegin();
  const byte cmd = 0b01110000 | (mode & 0b11);
  VfdWriteByte(cmd);
  VfdEnd();
}

void VfdClear()
{
  for(byte i=0;i<vfdNumOfDigits;i++)
  {
    const byte digitAddress = 0b00110000;
    VfdWriteDCRAM(i,digitAddress);
  }
}

byte VfdAscii2DigitAddress(char c)
{
  if(c>=64 and c<=95) // capital letters
    return (byte)(c-48);
  else if(c>=32 and c<=63)  //special character and numbers
    return (byte)(c+16);
  else if(c>=97 and c<=125) // lower case letters -> capital letters
    return (byte)(c-80);  
  else
    return (byte)0b00110000;
}


void VfdPrint(const char *str)
{
  const byte len = strlen(str);
  for(byte i = 0;i<len and i<vfdNumOfDigits;++i)
  {
    byte digitAddress = VfdAscii2DigitAddress(str[len-1-i]);
    VfdWriteDCRAM(i,digitAddress);
  }
}

void StartChargePump()
{    
    // square wave with 16,62 kHz duty cycle 50 % on PIN 9
    TCCR1A = (1<<COM1A1) + (1<<WGM11); // set mode to 14 Fast PWM
    TCCR1B = (1<<WGM13) + (1<<WGM12) + (1<<CS10); // prescaler none; 
    ICR1 = 960;
    OCR1A = 480;
    DDRB |= (1<<PB1);
}

void setup() 
{
    Serial.begin(115200);
    pinMode(PIN_NRST,OUTPUT);
    pinMode(PIN_NCS,OUTPUT);
    pinMode(PIN_CLK,OUTPUT);
    pinMode(PIN_DIN,OUTPUT);

    digitalWrite(PIN_NRST,HIGH);
    digitalWrite(PIN_NCS,HIGH);
    digitalWrite(PIN_CLK,HIGH);
    digitalWrite(PIN_DIN,LOW);

    StartChargePump();
    
    delay(500);
    
    VfdReset();
    VfdSetDisplayDuty(8);
    VfdSetNumberOfDigits(12);
    VfdSetDisplayLight(ALL_OFF);
    delay(500);
    VfdSetDisplayLight(ALL_ON);
    delay(500);
    VfdSetDisplayLight(NORMAL);
    VfdPrint("Messsatz.de");
    delay(5000);
    VfdClear();
    delay(500);
}


void loop() 
{
    static uint16_t count = 0;
    char msg[13];
    snprintf(msg,13,"%u",count);
    VfdPrint(msg);
    count++;
    delay(1000);
} 
