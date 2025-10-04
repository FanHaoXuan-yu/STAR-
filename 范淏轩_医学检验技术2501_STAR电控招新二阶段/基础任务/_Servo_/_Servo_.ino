#include <Servo.h>//引入舵机库

Servo duoJi;//创建舵机
int duoJiPin=9;//定义舵机引脚
int angle[]={0,30,45,90,135};//角度数组，避免反复麻烦

void setup() {
  // put your setup code here, to run once:
  duoJi.attach(duoJiPin);//连接舵机与引脚
}

void loop() {
  // put your main code here, to run repeatedly:
  for(int i=0;i<5;i++){
    duoJi.write(angle[i]);
    delay(1000);
  }//遍历数组
}