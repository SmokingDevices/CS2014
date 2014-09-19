const int analogpin = 6;  // Analog input pin 
long letzteSendung = 0;

String floatToString(float number){
	String stringNumber = "";
	char tempChar[10]; 
	dtostrf(number, 4, 2, tempChar);
	stringNumber += tempChar;
	return stringNumber;
}

float getPressure (int pin) {  //Methode für Druck
  float hPa=0;
  for (int i=1; i<=10; i++) {
    hPa += (float)(((analogRead(pin)/1024.0)+0.095)/0.0009);
    hPa += 23.0;
  }
//  hPa = hPa/5.0;
  return hPa/10.0;  // zurueckgeben
}

void setup() 
{
  Serial.begin(19200);   // initialize serial communications at 9600 bps:
}

void loop() 
{ 
  if (millis()-letzteSendung >= 1000) {  //Sendeintervall = 1s
    letzteSendung = millis();

    float pressure = getPressure (analogpin);
    Serial.print(millis());      
    Serial.print(",");      
    Serial.println(floatToString(pressure));   // print the pressure 
  }
}


