#include <CapacitiveSensor.h>
#include <Process.h>
#include <yunOSC.h>

#define IP_ADR "192.168.240.190"
#define PORT 9001

CapacitiveSensor cs_2_3 = CapacitiveSensor(2,3);

int pressed[12];

int flip[12];

unsigned long capStd[12];

unsigned long cap[12];

unsigned short ldr[12];

unsigned short runs;

unsigned int channels[8][3] =
{
  {0x0, 0x0, 0x0},
  {0x1, 0x0, 0x0},
  {0x0, 0x1, 0x0},
  {0x1, 0x1, 0x0},
  {0x0, 0x0, 0x1},
  {0x1, 0x0, 0x1},
  {0x0, 0x1, 0x1},
  {0x1, 0x1, 0x1}
};

void setup() {
  Bridge.begin();
  osc.begin(IP_ADR, PORT);
  cs_2_3.set_CS_AutocaL_Millis(0xFFFFFFFF);
  Serial.begin(9600);
  //PIN A0 reads from LDR's
  //PIN A1 reads from LDR 10
  //PIN A2 reads from LDR 12
  //PIN 2 sends to Z
  //PIN 3 revieves from Z
  pinMode(4,OUTPUT); // FIRST MUX - E
  pinMode(5,OUTPUT); // FIRST MUX - S0
  pinMode(6,OUTPUT); // FIRST MUX - S1
  pinMode(7,OUTPUT); // FIRST MUX - S2
  
  pinMode(12,OUTPUT); // SECOND MUX - E
  pinMode(9,OUTPUT); //  SECOND MUX - S0
  pinMode(10,OUTPUT); // SECOND MUX - S1
  pinMode(11,OUTPUT); // SECOND MUX - S2
  
  for(int i = 0; i < 12; i++){
    pressed[i] = false;
    flip[i] = false;
  }
  
  runs = 0;
}

void loop() {
  if(runs < 10){ //Getting initial standard values
  
    digitalWrite(4,LOW);//Enabling first MUX's
    digitalWrite(12,HIGH);//Disabling second MUX's
    for(int i = 0; i < 8; i++){
      for(int j = 0; j < 3; j++){
        digitalWrite(j+5,channels[i][j]);
      }
      capStd[i] = cs_2_3.capacitiveSensor(30);
    }
    
    digitalWrite(4,HIGH);//Enabling first MUX's
    digitalWrite(12,LOW);//Disabling second MUX's
    for(int i = 0; i < 3; i++){
      for(int j = 0; j < 3; j++){
        digitalWrite(j+9,channels[i][j]);
      }
      capStd[i+8] = cs_2_3.capacitiveSensor(30);
    }
    
    runs++;
  }
  
  else if(runs >= 10){
    for(int i = 0; i < 12; i++){
      if(i < 8){
        digitalWrite(4,LOW);
        digitalWrite(12,HIGH);
        for(int j = 0; j < 3; j++)
          digitalWrite(j+5,channels[i][j]);
      }
      else{
        digitalWrite(4,HIGH);
        digitalWrite(12,LOW);
        for(int j = 0; j < 3; j++)
          digitalWrite(j+9,channels[i-8][j]);
      }
      cap[i] = cs_2_3.capacitiveSensor(30);
      if(i == 9)
        ldr[i] = analogRead(A1);
      else if(i == 11)
        ldr[i] = analogRead(A2);
      else
        ldr[i] = analogRead(A0);
      
      if(capStd[i] <= 3 + cap[i] * 1.04 && capStd[i] >= -3 + cap[i] * 0.96)
        capStd[i] = cap[i];
      if(ldr[i] < 300)
        flip[i] = 1;
      else
        flip[i] = 0;
      if(cap[i] > capStd[i] * 1.35 && cap[i] > 150)
        pressed[i] = 1;
      else
        pressed[i] = 0;
    }
  }
  
  else{
    runs++;
  }
  
  short incoming = 0;
  if(Serial.available()>0)
    incoming = Serial.read();
  Serial.flush();
  if(incoming - 48 == 5)
    runs = 0;
  
  int capLol[12];
  int ldrLol[12];
  
  for(int i = 0; i < 12; i++){
    capLol[i] = cap[i];
    ldrLol[i] = ldr[i];
  }
  
  osc.send("/cap", pressed, 12);
  osc.send("/ldr", flip, 12);
  Console.print("CAP:\t");
  for(int i = 0; i < 12; i++){
    Console.print(cap[i]);
    Console.print("\t");
  }
  Console.print("LDR:\t");
  for(int i= 0; i < 12; i++){
    Console.print(ldr[i]);
    Console.print("\t");
  }
  Console.print("\n");
}
