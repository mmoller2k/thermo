#include <SindormirSevenSegments.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define tDelay 250
#define ONE_WIRE_BUS 0
#define PUMP 11 //for voltage doubler

unsigned long t0=0;
int count = -99;
double Input;

LED7seg x7seg;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempSensor;

void setup()
{
  x7seg.commonType(CATHODE,true); //inverted cathode drivers
  x7seg.attach_segs(0,1,2,3,4,5,6,7); //segment drivers
  x7seg.attach_symb(8,9,10); //three digits
  pinMode(PUMP, OUTPUT);
  analogWrite(PUMP,128); //generate square wave
  delay(500); //wait for pump to charge
  sensors.begin();
  while(!sensors.getAddress(tempSensor, 0)) 
  {
     showerror();
     delay(500);
  }
  sensors.setResolution(tempSensor, 12);
  sensors.setWaitForConversion(false);
}

void loop()
{
  unsigned long now = millis();
  if(now-t0 >= tDelay){
    t0 = now;
    x7seg.print(count);
    if(count>199)count=-99;
    else count++;
  }
  if (sensors.isConversionAvailable(0))
  {
    Input = sensors.getTempC(tempSensor);
    sensors.requestTemperatures(); // prime the pump for the next one - but don't wait
    showTemp(Input);
  }
  x7seg.multiplex();
}

void showerror(void)
{
  x7seg.setSymbol(2,'E');
  x7seg.setSymbol(1,'r');
  x7seg.setSymbol(0,'r');
}

void showTemp(float T)
{
  int sign=0;
  int val;
  if(T<0){
    sign=1;
    T=-T;
  }
  val=T*10.0+0.5;
  if(val<1000){
    x7seg.setDot(0);
  }
  else{
    val /= 10;
    x7seg.clearDot();
  }
}

