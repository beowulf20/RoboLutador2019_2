

#include <Arduino.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif


BluetoothSerial SerialBT;

#define CMD_UP    'w'
#define CMD_LEFT  'a'
#define CMD_DOWN  's'
#define CMD_RIGHT 'd'
#define CMD_STOP  '0'

#define ME_EN1 18
#define ME_EN2 5 
#define MD_EN1 22 
#define MD_EN2 21

char cmd_buff = '0';
size_t cmd_buff_len = 0;
void setup(){

  pinMode(ME_EN1,OUTPUT);
  pinMode(ME_EN2,OUTPUT);
  pinMode(MD_EN1,OUTPUT);
  pinMode(MD_EN2,OUTPUT);
  
  Serial.begin(9600);
  SerialBT.begin("RoboBrutus");
  Serial.println("Device started");


}

typedef enum {
  MOTOR_DIR_FORWARD = 0,
  MOTOR_DIR_BACKWARD,
  MOTOR_DIR_LEFT,
  MOTOR_DIR_RIGHT,
  MOTOR_DIR_STOP,
  MOTOR_DIR_MAX
} MOTOR_DIR;

void motor_set_dir(MOTOR_DIR dir){  
  if (dir > MOTOR_DIR_MAX || dir < 0){
    Serial.println("Invalid direction");
    return;
  }
  byte me1,me2,md1,md2;
  switch (dir)
  {
  case MOTOR_DIR_FORWARD:        
    me1 = 1;
    me2 = 0;
    md1 = 1;
    md2 = 0;
    break;
  case MOTOR_DIR_BACKWARD:    
    me1 = 0;
    me2 = 1;
    md1 = 0;
    md2 = 1;
    break;
  case MOTOR_DIR_LEFT:    
    me1 = 0;
    me2 = 1;
    md1 = 1;
    md2 = 0;
    break;
  case MOTOR_DIR_RIGHT:    
    me1 = 1;
    me2 = 0;
    md1 = 0;
    md2 = 1;
    break;
  case MOTOR_DIR_STOP:    
    me1 = 0;
    me2 = 0;
    md1 = 0;
    md2 = 0;
    break;
  default:    
    break;
  }
  digitalWrite(ME_EN1,me1);
  digitalWrite(ME_EN2,me2);
  digitalWrite(MD_EN1,md1);
  digitalWrite(MD_EN2,md2);
}
void loop(){  
  if(SerialBT.available())  {    
    cmd_buff_len = SerialBT.readBytes(&cmd_buff,1);    
    MOTOR_DIR dir = MOTOR_DIR_MAX;
    switch (cmd_buff){      
      case CMD_UP:
        // Serial.println("going up motherfucker");
        dir = MOTOR_DIR_FORWARD;
        break;
      case CMD_LEFT:
        // Serial.println("going left motherfucker");
        dir = MOTOR_DIR_LEFT;
        break;
      case CMD_DOWN:
        // Serial.println("going down motherfucker");
        dir = MOTOR_DIR_BACKWARD;
        break;
      case CMD_RIGHT:
        // Serial.println("going right motherfucker");
        dir = MOTOR_DIR_RIGHT;
        break;
      case CMD_STOP:
        // Serial.println("going stop motherfucker");
        dir = MOTOR_DIR_FORWARD;
      default:
        break;
    }
    motor_set_dir(dir);
  }
  delay(20);
}