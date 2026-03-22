#include <Adafruit_NeoPixel.h>
#define NUMPIXELS 24
#define PIN 7
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

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
motor motorR2{10,11};
motor motorR1{13,12};
motor motorL1{15,14};
motor motorL2{9,8};
void stop(int time)
{
  
  motorR1.speed(0);
  motorL1.speed(0);
  motorR2.speed(0);
  motorL2.speed(0);
  delay(time);
  
}

void setup() {
  Serial.begin(115200);
  Serial.println("Hello");
  pinMode(20, INPUT);
  pixels.begin();
  pixels.setBrightness(50);
  for(int i = 0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(100,100,100));
  }
  pixels.show();
  while(digitalRead(20)==1);
    motorR1.speed(60);
    motorR2.speed(60);
    motorL1.speed(0);
    motorL2.speed(0);
    Serial.println("Hi");

}

void loop(){
  
}
