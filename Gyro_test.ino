#include<Wire.h>
//#include<SoftwareSerial.h>
//SoftwareSerial radioSerial(2,4); //RX, TX
int red = 11;
int green = 10;
int blue = 9;
float RateRoll, RatePitch, RateYaw;
float RateCalibrationRoll, RateCalibrationPitch, RateCalibrationYaw;
float AccX, AccY, AccZ;
float AngleRoll, AnglePitch;
int RateCalibrationNumber;
uint32_t LoopTimer;
float KalmanAngleRoll=0, KalmanUncertaintyAngleRoll=2*2;
float KalmanAnglePitch=0, KalmanUncertaintyAnglePitch=2*2;
float Kalman1DOutput[]={0,0};

void kalman_1d(float KalmanState,float KalmanUncertainty,float KalmanInput,float KalmanMeasurement){
  KalmanState= KalmanState+(0.004*KalmanInput);
  KalmanUncertainty= KalmanUncertainty+(0.004*0.004*4*4);
  float KalmanGain= KalmanUncertainty*1/(1*KalmanUncertainty+ 3*3);
  KalmanState= KalmanState+KalmanGain*(KalmanMeasurement-KalmanState);
  KalmanUncertainty=(1-KalmanGain)*KalmanUncertainty;
  Kalman1DOutput[0]= KalmanState;
  Kalman1DOutput[1]= KalmanUncertainty;
}

void gyro_signals(void){
  Wire.beginTransmission(0x68); //starting communicaton with the MPU60X0
  Wire.write(0x1A);// Switch on low-pass filter
  Wire.write(0x05);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x1C);//configue accelerometer
  Wire.write(0x10);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x3B);//setting up sensitivity
  Wire.endTransmission();
  Wire.requestFrom(0x68,6);
  int16_t AccXLSB = Wire.read()<<8| Wire.read();
  int16_t AccYLSB = Wire.read()<<8| Wire.read();
  int16_t AccZLSB = Wire.read()<<8| Wire.read();
  Wire.beginTransmission(0x68);
  Wire.write(0x1B);//setting up sensitivity
  Wire.write(0x8);
  Wire.endTransmission();
  Wire.beginTransmission(0x68);
  Wire.write(0x43);
  Wire.endTransmission();
  Wire.requestFrom(0x68,6);
  int16_t GyroX= Wire.read()<<8|Wire.read();
  int16_t GyroY= Wire.read()<<8|Wire.read();
  int16_t GyroZ= Wire.read()<<8|Wire.read();
  RateRoll=(float)GyroX/65.5;
  RatePitch=(float)GyroY/65.5;
  RateYaw=(float)GyroZ/65.5;
  AccX=(float)AccXLSB/4096-0.06;
  AccY=(float)AccYLSB/4096+0.05;
  AccZ=(float)AccZLSB/4096-0.08;
  AngleRoll= atan(AccY/sqrt(AccX*AccX+AccZ*AccZ))*1/(3.142/180);
  AnglePitch= -atan(AccX/sqrt(AccY*AccY+AccZ*AccZ))*1/(3.142/180);
}
void setup() {
  Serial2.begin(57600);
  //radioSerial.begin(57600);
  pinMode(13, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  digitalWrite(13, HIGH);
  digitalWrite(red, LOW);
  digitalWrite(blue, LOW);
  digitalWrite(green, HIGH);
  Wire.setClock(400000);
  Wire.begin();
  delay(250);
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00);
  Wire.endTransmission();
  for(RateCalibrationNumber=0; RateCalibrationNumber <2000; RateCalibrationNumber++){
    gyro_signals();
    RateCalibrationRoll += RateRoll;
    RateCalibrationPitch += RatePitch;
    RateCalibrationYaw += RateYaw;
    delay(1);
  }
  RateCalibrationRoll /= 2000;
  RateCalibrationPitch /= 2000;
  RateCalibrationYaw /= 2000;
  LoopTimer = micros();
  digitalWrite(red, HIGH);
  digitalWrite(blue, HIGH);
  digitalWrite(green, LOW);
}

void loop() {
  gyro_signals();
  RateRoll -= RateCalibrationRoll;
  RatePitch -= RateCalibrationPitch;
  RateYaw -= RateCalibrationYaw;
  
  kalman_1d(KalmanAngleRoll, KalmanUncertaintyAngleRoll,RateRoll, AngleRoll);
  KalmanAngleRoll= Kalman1DOutput[0];
  KalmanUncertaintyAngleRoll= Kalman1DOutput[1];
  kalman_1d(KalmanAnglePitch, KalmanUncertaintyAnglePitch,RatePitch, AnglePitch);
  KalmanAnglePitch= Kalman1DOutput[0];
  KalmanUncertaintyAnglePitch= Kalman1DOutput[1];

  
  //Serial.print("Roll rate [°/s]= ");
  Serial.print(RateRoll);
  Serial.print(",");
  //Serial.print("Pitch rate [°/s]= ");
  Serial.print(RatePitch);
  Serial.print(",");
  //Serial.print("Yaw rate [°/s]= ");
  Serial.print(RateYaw);
  Serial.print(",");
  //radioSerial.print("Acceleration X [g]= ");
  Serial.print(AccX);
  Serial.print(",");
  //radioSerial.print("Acceleration Y [g]= ");
  Serial.print(AccY);
  Serial.print(",");
  //radioSerial.print("Acceleration Z [g]= ");
  Serial.print(AccZ);
  Serial.print(",");
  Serial.print(AngleRoll);
  Serial.print(",");
  Serial.print(AnglePitch);
  Serial.print(",");
  Serial.print(KalmanAngleRoll);
  Serial.print(",");
  Serial.println(KalmanAnglePitch);
  while(micros()-LoopTimer<4000);
  LoopTimer=micros();
  //delay(50);
}
