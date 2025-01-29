#include<Wire.h>
#include <SoftwareSerial.h>

SoftwareSerial XBee(10, 11);
const int MPU=0x68; 
int16_t AcX,AcY,AcZ,Tmp;

void  setup(){
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);  
  Wire.write(0);    
  Wire.endTransmission(true);
  Serial.begin(9600);
  XBee.begin(9600);
}
void  loop(){
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);  
  Wire.endTransmission(false);
  Wire.requestFrom(MPU,12,true);  
  AcX=Wire.read()<<8|Wire.read();    
  AcY=Wire.read()<<8|Wire.read();  
  AcZ=Wire.read()<<8|Wire.read();  
  
  /*Serial.print("Accelerometer: ");
  Serial.print("X = "); Serial.print(AcX);
  Serial.print(" | Y = "); Serial.print(AcY);
  Serial.print(" | Z = ");  Serial.println(AcZ); */

  String stringAcX = String(AcX, DEC); 
  String stringAcY = String(AcY, DEC); 
  String stringAcZ = String(AcZ, DEC); 
  String stringFrame = String("X="+ stringAcX + ",Y=" + stringAcY + ",Z=" + stringAcZ); 
  XBee.print(stringFrame);

  delay(1500);
}