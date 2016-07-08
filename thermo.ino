#include <SindormirSevenSegments.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define tDelay 250
#define ONE_WIRE_BUS A5
#define PUMP A4 //for voltage doubler

unsigned long t0=0;
int count = -99;
double Input;
boolean v2v=false;
int nloop=0;

LED7seg x7seg;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress tempSensor;

#define IRQON(x) (TIMSK2 |= 1<<TOIE2)
#define IRQOFF(x) (TIMSK2 &= ~(1<<TOIE2))

void setup()
{
  x7seg.commonType(CATHODE,true); //inverted cathode drivers
  x7seg.attach_symb(8,9,10); //three digits
  x7seg.attach_segs(0,1,2,3,4,5,6,7); //segment drivers
  pinMode(PUMP, OUTPUT);
  // UNO: Run timer2 interrupt
#if 1
  TCCR2A = 0;
  TCCR2B = 0<<CS22 | 1<<CS21 | 0<<CS20;

  //Timer2 Overflow Interrupt Enable
  //TIMSK2 |= 1<<TOIE2;
  IRQON();
#endif
  //analogWrite(PUMP,128); //prime charge pump
  x7seg.lampTest();
  //delay(200);
  //x7seg.print((readVcc()+5)/10);
  //x7seg.setDot(2);
  sensors.begin();
  Treset();
  x7seg.clearDot();
}

void Treset(void)
{
  int i;
  while(!sensors.getAddress(tempSensor, 0)) {
    IRQON();
    showError("Er0");
    delay(500);
  }
  //IRQOFF();
  sensors.setResolution(tempSensor, 12);
  sensors.setWaitForConversion(true);
  sensors.requestTemperatures();
  IRQON();
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
  nloop++;
  //IRQOFF();
  if (sensors.isConversionAvailable(tempSensor))
  {
    Input = sensors.getTempC(tempSensor);
    sensors.requestTemperatures(); // prime the pump for the next one - but don't wait
    IRQON();
    showTemp(Input);
    nloop=0;
  }
  else if(nloop){
    showError("Er1");
    sensors.requestTemperatures();
  }
  else while(!sensors.isConnected(tempSensor)){
    Treset();
  }
  IRQON();
  //delay(100);
}

void showError(const char *str)
{
  x7seg.setSymbol(2,str[0]);
  x7seg.setSymbol(1,str[1]);
  x7seg.setSymbol(0,str[2]);
  x7seg.clearDot();
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
  IRQOFF();
  x7seg.clearDot();
  if(sign>0 && val>=1000){ // four digits positive
    val = (val+5) / 10; //drop the decimal
    x7seg.setSymbol(2,(val/100)%10);
    x7seg.setSymbol(1,(val/10)%10);
    x7seg.setSymbol(0,val%10);
  }
  else if(sign>0 && val>=100){ // three digits positive
    x7seg.setDot(1);
    x7seg.setSymbol(2,(val/100)%10);
    x7seg.setSymbol(1,(val/10)%10);
    x7seg.setSymbol(0,val%10);
  }
  else if(sign>0){ // two digits positive
    x7seg.setDot(1);
    x7seg.setSymbol(2,BLANK);
    x7seg.setSymbol(1,(val/10)%10);
    x7seg.setSymbol(0,val%10);
  }
  else if(val<100){ //up to 2 digits negative
    x7seg.setDot(1);
    x7seg.setSymbol(2,NEG);
    x7seg.setSymbol(1,(val/10)%10);
    x7seg.setSymbol(0,val%10);
  }
  else{ //three digits negative
    val = (val+5) / 10; //drop the decimal
    x7seg.setSymbol(2,NEG);
    x7seg.setSymbol(1,(val/10)%10);
    x7seg.setSymbol(0,val%10);
  }
  IRQON();
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

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
 
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both
 
  long result = (high<<8) | low;
 
  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

