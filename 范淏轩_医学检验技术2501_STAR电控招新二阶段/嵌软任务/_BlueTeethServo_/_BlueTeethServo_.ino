#include <Servo.h>
#include <SoftwareSerial.h>//导入库

Servo duoJi;
int duoJiPin=9;//定义舵机与引脚

SoftwareSerial BT(2,3);//连接蓝牙模块

void duoJiRotate(int angle){
  duoJi.write(angle);
  Serial.print("舵机转至：");
  Serial.print(angle);
  Serial.println("度");  
}//旋转角度函数

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  BT.begin(9600);
  duoJi.attach(duoJiPin);//连接舵机
}

void loop() {
  // put your main code here, to run repeatedly:
  if(BT.available()){
    String input=BT.readString();//读取angle
    input.trim();//去除空格与换行符等
    int angle=input.toInt();//赋值angle

    if(angle>=0&&angle<=180){
      duoJiRotate(angle);//调用函数
    }else{
      Serial.println("角度应处于0—180范围内");//范围提示
    }
  }
}
