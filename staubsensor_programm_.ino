
#define Bitrate 19200
int dustPin = 0;  //angeschlossen an Pin 6
int ledPower = 2;  //LED an pin 2 angeschlossen
int delayTime = 500;  //messzeitabstand
int delayTime2 = 80;  //zeit in der gemessen wird ?
float offTime = 9689;  //messpause
int dustValue = 0;  //???????????????????????(kommt nicht im programm vor, oder? o.0)
float calcVoltage = 0; //errechnete Voltzahl
float voltMeasured = 0; //gemessenen Voltzahl im Sensor
float dustDensity = 0; // letztendliche Staubkonzentration
int i=0;
float voltage = 0;
float ppm=0;

void setup () {  //voreinstellgungen
   //Serial1.begin(Bitrate);
   Serial.begin(Bitrate);  //datenblocks pro sekunde
   pinMode (ledPower,OUTPUT);  //LED=verbraucher --> pinmode output
   pinMode (dustPin,OUTPUT); //staubsensor ist verbraucher...s.o.
 }
 

/*float Bit2Volt(int n){    //Function to convert raw ADC-data (0-255) to volt
  int raw = analogRead(n);
  Serial.println(raw);
  float volt = (float)raw*(5/1024);      //vorher 3.3/1024
  return volt;
}*/

 
void loop () {  //schleife <3
 
  /* digitalWrite (ledPower, LOW); //led anschalten
   delayMicroseconds (280);
   voltMeasured = Bit2Volt(dustPin);
   delayMicroseconds (40);
   digitalWrite (ledPower, HIGH);  //led ausschalten
   
   calcVoltage = voltMeasured;  //umrechnen von voltMeasured zu calcVoltage
   dustDensity = (0.17*calcVoltage-0.1); // weiterrechnen mit calcvoltage zu dustdensity
 */
 
   digitalWrite (ledPower, LOW);
   delayMicroseconds (280);
   dustValue=analogRead (dustPin);    
   delayMicroseconds (40);
   digitalWrite (ledPower, HIGH);
   
   delayMicroseconds (9680);
 
  voltage = dustValue *0.0048875;    //vorher ppm/i*0.0049 (5/1023)
//  dustDensity = 0.17*voltage-0.1;
  dustDensity = voltage/5;
 
 
   Serial.print(dustValue);  //darstellen von gemessener voltzahl
   Serial.print ('|');
   Serial.print(voltage);  //- errechneter voltzahl
   Serial.print ('|');
   Serial.println(dustDensity);  //- letztendliche staubkonzentration
   delay (500);
   
   /*Serial1.print(dustValue);  //darstellen von gemessener voltzahl
   Serial1.print ('|');
   Serial1.print(voltage);  //- errechneter voltzahl
   Serial1.print ('|');
   Serial1.println(dustDensity);  //- letztendliche staubkonzentration
   delay (500);*/
 }
 
 
