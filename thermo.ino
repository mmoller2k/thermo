#include <SindormirSevenSegments.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define tDelay 250
#define ONE_WIRE_BUS A5
#define PUMP 11 //for voltage doubler

unsigned long t0=0;
int count = -99;
double Input;
boolean v2v=false;

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
  //analogWrite(PUMP,128); //generate square wave
  x7seg.lampTest();
  x7seg.delay(500);
  x7seg.clear();
  sensors.begin();
  // UNO: Run timer2 interrupt
#if 1
  TCCR2A = 0;
  TCCR2B = 0<<CS22 | 1<<CS21 | 1<<CS20;

  //Timer2 Overflow Interrupt Enable
  TIMSK2 |= 1<<TOIE2;
#endif  
  Treset();
  x7seg.clear();
}

void Treset(void)
{
  int i;
  while(!sensors.getAddress(tempSensor, 0)) {
    showError();
    delay(500);
  }
  sensors.setResolution(tempSensor, 12);
  sensors.setWaitForConversion(false);
  sensors.requestTemperatures();
}

void loop()
{
  unsigned long now = millis();
  if(now-t0 >= tDelay){
    t0 = now;
    //x7seg.print(count);
    if(count>199)count=-99;
    else count++;
  }
  if (sensors.isConversionAvailable(tempSensor))
  {
    noInterrupts();
    Input = sensors.getTempC(tempSensor);
    interrupts();
    sensors.requestTemperatures(); // prime the pump for the next one - but don't wait
    showTemp(Input);
  }
  else while(!sensors.isConnected(tempSensor)){
    Treset();
  }
  delay(250);
}

void showError(void)
{
  x7seg.setSymbol(2,'E');
  x7seg.setSymbol(1,'r');
  x7seg.setSymbol(0,'r');
}

void showTemp(float T)
{
  int sign=1;
  int val;
  //x7seg.clear();
  if(T<0){
    sign=-1;
    T=-T;
  }
  val=T*10.0+0.5;
  if(sign>0 && val<1000){ //up to 3 digits positive
    x7seg.setDot(1);
  }
  else if(sign<0 && val<100){ //up to 2 digits negative
    x7seg.setDot(1);
  }
  else{
    val = (val+5) / 10; //else drop the decimal
    x7seg.clearDot();
  }
  x7seg.print(val*sign);
}

// ************************************************
// Timer Interrupt Handler
// ************************************************
SIGNAL(TIMER2_OVF_vect) 
{
  x7seg.multiplex();
  digitalWrite(PUMP,v2v);
  v2v ^= 1;
}

