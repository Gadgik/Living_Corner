#include <StepperMotor.h>
#include <Wire.h>

#define TRANS_ID 8

String com;

#define turb_pin A2
#define soil1_pin A0
#define soil2_pin A1

void receiveEvent(int howMany) {
  com = "";
  while (Wire.available()) { 
    char c = Wire.read();  // Читаем байт
    com += c;  // Добавляем к строке
  }
  Serial.println(com); // Выводим строку
}

StepperMotor motor1(8, 7, 9,3);// движение цветов
StepperMotor motor2(5, 4, 6);// кормушка хомяка
StepperMotor motor3(3, 4, 2);// кормушка рыб
StepperMotor motor4(11, 10, 12,4);// движение рыб

void setup() {
  pinMode(turb_pin,INPUT);
  pinMode(soil1_pin,INPUT);
  pinMode(soil2_pin,INPUT);

  Serial.begin(115200);
  motor1.begin();
  motor2.begin();
  motor3.begin();
  motor4.begin();
  Wire.begin(TRANS_ID);
  Wire.onReceive(receiveEvent);
}

void feed_hams(){
  motor2.rotate(700);
  Serial.println("hums1");
}

void feed_fish(int pos){
  if(pos == 2){
    motor4.rotate(1100);
    delay(500);
    motor3.rotate(-30);
    delay(500);
    motor3.rotate(500);
    delay(500);
    motor4.rotate(-1100);
    Serial.println("fish1");
  }else{
    motor4.rotate(2250);
    delay(500);
    motor3.rotate(-30);
    delay(500);
    motor3.rotate(500);
    delay(500);
    motor4.rotate(-2250);
    Serial.println("fish2");
  }
}
#define water_time 1000*8
#define foster_time 1000*2

void flowers(int pos,int move){
  if(pos == 1){
    motor1.rotate(1200);
    delay(2000);
    if(move == 1){
      delay(water_time);
    }else{
      delay(foster_time); 
    }
    delay(2000);
    motor1.rotate(-1200);
  }else{
    motor1.rotate(1700);
    delay(2000);
    if(move == 1){
      delay(water_time);
    }else{
      delay(foster_time);
    }
    delay(2000);
    motor1.rotate(-1700);
  }
}

void loop() {
  if(com != ""){
    if(com =="feed hamster"){
      feed_hams();
    } else if(com =="feed fish 1"){
      feed_fish(1);
    } else if(com =="feed fish 2"){
      feed_fish(2);
    } else if(com =="water plant 1"){
      flowers(1,1);
    } else if(com =="water plant 2"){
      flowers(2,1);
    } else if(com =="felt plant 1"){
      flowers(1,2);
    } else if(com =="felt plant 2"){
      flowers(2,2);
    }
    com = "";
  }

}
