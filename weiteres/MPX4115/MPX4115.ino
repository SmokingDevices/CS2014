/*  A simple air pressure sensor check program
    The circuit uses one point calibration.
    Adjust the pot to give the known atmospheric pressure.
   1070 mb = 107 kpa = 802 mm Hg 
 */

const int analogpin = 6;  // Analog input pin 

String floatToString(float number){
	String stringNumber = "";
	char tempChar[10]; 
	dtostrf(number, 4, 2, tempChar);
	stringNumber += tempChar;
	return stringNumber;
}

float getPressure (int pin) {  //Methode für Druck
	float volt = Bit2Volt(pin); //liest pin ein 
	float hPa = 10*(volt/(0.009*5.00)+(0.095/0.009));  //Umrechnung von V zu Pa und dann zu hPa
Serial.print('(');
Serial.print(hPa);
Serial.print(')');
hPa = (float)(((analogRead(pin)/1024.0)+0.095)/0.0009);
hPa = hPa+(hPa*0.02);
//float hPa = (float)(((volt/5.0)+0.095)/0.0009)*10.0;
//float hPa = ((volt/(5.0*0.009))+1+(0.0095/0.009))*10;
	return hPa;  // zurueckgeben
}

float Bit2Volt(int n){    // Funktion zum Lesen und umwandeln
	int raw = analogRead(n);  // Messdaten von pin 'n' lesen
  Serial.print("sensor = " );                         
  Serial.print(raw);    // print the adc value to the serial monitor:
	float volt = (float)raw*5.000/1024.0;  // Umrechnung für Volt
  Serial.print("\tvolt = " );                         
  Serial.print(volt);    // print the adc value to the serial monitor:
	return volt;  // zurückgeben der Voltzahl
}

void setup() 
{
  Serial.begin(9600);   // initialize serial communications at 9600 bps:
}

void loop() 
{ 
  //long pressure = 950 + adcvalue / 8.5;  //convert it to milli bars
  long pressure = getPressure (analogpin);
  Serial.print("\t pressure = ");      
  Serial.print(floatToString(pressure));   // print the pressure 
  Serial.println(" hPa");
  delay(1000);   // wait 1 second before next sample                    
}


