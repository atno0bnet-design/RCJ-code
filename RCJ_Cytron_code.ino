

#include <Adafruit_NeoPixel.h>
#define NUMPIXELS 24
#define PIN 7
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#include <Wire.h>
#include <Adafruit_BNO055.h>

Adafruit_BNO055 bno(55,0x28);
float og = 0;
float data = 0;
int condition = 0;
int FL_motor = 0;
int BL_motor = 0;
int FR_motor = 0;
int BR_motor = 0;

enum bub{l1=50,l2=50,r1=50,r2=50};
int cmap(int val){
  int mapped = map(val, 0 , 100, 0, 255);
  return constrain(mapped, 0, 255);

}

struct motor{
  uint8_t fpin, rpin;
  void speed(int val);
};




void motor::speed(int val){
  int map_speed = cmap(abs(val));
    if(val>0){
      analogWrite(fpin, map_speed);
      analogWrite(rpin, 0); 
    }
    else{
      analogWrite(fpin, 0);
      analogWrite(rpin, map_speed);
    }
}
motor motorFR{11,10};
motor motorBR{12,13};
motor motorFL{15,14};
motor motorBL{9,8};

void stop(int time)
{
  
  motorFR.speed(0);
  motorFL.speed(0);
  motorBR.speed(0);
  motorBL.speed(0);
  delay(time);
  
}

void move(int FL, int BL, int FR, int BR){
  motorFL.speed(FL);
  motorBL.speed(BL);
  motorFR.speed(-FR);
  motorBR.speed(-BR);
}

void getYaw(float *data){
    int counter = 0;
    *data = 0;
    while(*data == 0 && counter++<100){
        sensors_event_t n;
        bno.getEvent(&n);
        *data = n.orientation.x;
    }
}

void getRoll(float *data){
    int counter = 0;
    *data = 0;
    while(*data == 0 && counter++<100){
        sensors_event_t n;
        bno.getEvent(&n);
        *data = n.orientation.z;
    }
}


void turnright(float degrees) {
  sensors_event_t sensorData;
  bno.getEvent(&sensorData);
  float startHeading = sensorData.orientation.x;
  float targetHeading = startHeading + degrees;
  if (targetHeading >= 360) targetHeading -= 360;
  Serial.print("Turning from ");
  Serial.print(startHeading);
  Serial.print(" to ");
  Serial.println(targetHeading);

  while (true) {
    bno.getEvent(&sensorData);
    double currentHeading = sensorData.orientation.x;

    float turn = targetHeading - currentHeading;

    if (turn < -180) turn += 360;
    if (turn > 180) turn -= 360;

    if (abs(turn) < 2) break;

    move(25,25,-25,-25);
    delay(10);
  }
  stop(1000);
}

void turnleft(float degrees) {
  sensors_event_t sensorData;
  bno.getEvent(&sensorData);
  float startHeading = sensorData.orientation.x;
  float targetHeading = startHeading - degrees;
  if (targetHeading >= 360) targetHeading -= 360;
  Serial.print("Turning from ");
  Serial.print(startHeading);
  Serial.print(" to ");
  Serial.println(targetHeading);

  while (true) {
    bno.getEvent(&sensorData);
    double currentHeading = sensorData.orientation.x;

    float turn = targetHeading - currentHeading;

    if (turn < -180) turn += 360;
    if (turn > 180) turn -= 360;

    if (abs(turn) < 2) break;

    move(-25,-25,25,25);
    delay(10);
  }
  stop(1000);
}

void setup() {

  Serial.begin(115200);
  Serial.println("Hello");
  Serial1.setRX(17);
  Serial1.setTX(16);
  Serial1.begin(115200);
  delay(2000);
  Wire.setSCL(29);
  Wire.setSDA(28);
  Wire.begin();
  if(bno.begin(OPERATION_MODE_IMUPLUS)==0){
    Serial.println("Bno failed");
    while(1);
  }
  
  pinMode(20, INPUT);
  pinMode(21, INPUT);

  pixels.begin();
  pixels.setBrightness(20);
  for(int i = 0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(255,255,255));
  }
  pixels.show();

  while(digitalRead(21));
  
  Serial1.write('a');
  getRoll(&og);
}

void loop(){
  Serial.println("checking...");
  if(Serial1.available() >0){
    getRoll(&data);
    Serial.print(data);
    Serial.print(":");
    Serial.println(og);
    if(data>og+5){
      Serial1.write('z');
      og = data;  
    }
    if(data<og-5){
      Serial1.write('x');
    }
    
    Serial.println(data);
    condition = Serial1.parseInt();
    FL_motor = Serial1.parseInt();
    BL_motor = Serial1.parseInt();
    FR_motor = Serial1.parseInt();
    BR_motor = Serial1.parseInt();
    
      
    /*
    Serial.print("Cond: ");
    Serial.println(condition);
    Serial.print("FL_motor: ");
    Serial.println(FL_motor);
    Serial.print("BL_motor: ");
    Serial.println(BL_motor);
    Serial.println();
    */
    switch(condition)
    {
        case 1:{
            move(20,20,20,20);
            delay(200);
            turnleft(80);
            move(20,20,20,20);
            delay(400);
            stop(000);
            Serial1.write('a');
            
            break;
        }
        case 2:{
          move(20,20,20,20);
          delay(400);
          turnright(80);
          move(20,20,20,20);
          delay(900);
          stop(1000);
          Serial1.write('a');
          break;
        }
        case 3:{
          turnright(180);
          move(20,20,20,20);
          delay(900);
          stop(1000);
          break;
        }
        default:{
          move(FL_motor, BL_motor, FR_motor, BR_motor);
        }
    }
    

  }
}
