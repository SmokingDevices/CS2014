/***********************************************************************************
 * DEFINITIONEN                                                                    *
 ***********************************************************************************/
#define debug 1 // debug = 1 = Ausgabe auf Serial, = 0 nur Senden
 
#define Vcc 5.0              //Supply voltage 

#define Bitrate 19200

#define druckPin A6
#define tempPin A1			// int. Sensor LM35 (3 Beine)
#define tempNTCPin A2		// ext. Sensor NTC (2 Beine)
#define beepPin 5			// digitaler D-Pin 5 für Beeper
#define dustPin A9			// angeschlossen an Pin A8
#define dustLEDPin 2		// LED an D-Pin 2 angeschlossen

// LM35 Temp sensor
#define TmpSens 100			// Sensitivity. Deg.Celsius /volt

// NTC-sensor variables
#define NTC_Grad 18.1699	// The gradiant value for the NTC sensor
#define NTC_Off 21.0		// The offset value for the NTC sensor

#define TRENNER ","			// Trennzeichen zwischen den Werten

/***********************************************************************************
 * VARIABLEN                                                                       *
 ***********************************************************************************/
unsigned long letzteSendung = 0;
int Intervall = 1000;    // alle wie viel ms wird eine Messung gemacht und gesendet

//Primärmission (1)
int sensorValue = 0;	// Variable to store the value coming from the sensor
float druckBoden = 0;
float druckDurchschnitt = 0;      // Variablen setzen zur späteren Verwendung
float hoehe = 0;		// aktuelle Hoehe
float altehoehe=0;		// Hoehe bei der letzten Messung
float fall= 0;			// aktuelle Fallgeschwindigkeit

boolean ermittleGPS = false; //Wird gerade GPS ermittelt

// Staubsensor (2)
int dustValue = 0;		// Wert der vom Dust Sensor kommt 
float voltage = 0;		// errechnete Voltzahl
float dustDensity = 0;	// letztendliche Staubkonzentration

// Pieper einschalten
boolean steige   = false;
boolean unter150 = false;

//allgemeine Laufvariable
int i=0;

//Header fuer die Exceltabelle, wir nur einmal am Anfang gesendet
String header = "time,Druck,Temp (int),Temp (ext),Hoehe,Fallgeschw.,Staubwert,Staubspannung,Staubdichte,GPS Header,Uhrzeit,Latitude,N/S,Longitude,E/W,Position,Satelliten,HDOP,MSL Hoehe,M,Geoid Hoehe,M,Age of Diff,Checksumme";

/***********************************************************************************
 * FUNKTIONEN                                                                      *
 ***********************************************************************************/
//Primärmission (1)
float getPressure (int pin) {  //Methode für Druck
	float volt = Bit2Volt(pin); //liest pin ein 
	float hPa = 10*(volt/(0.009*Vcc)+(0.095/0.009));  //Umrechnung von V zu Pa und dann zu hPa
	return hPa;  // zurueckgeben
}

float getTemperatureIntern(int pin){  //LM35 an A1
	float volt = Bit2Volt(pin); //liest pin ein
	float result =volt*TmpSens;  //umrechnung
	return result;  // zurueckgeben
}

float getTemperatureExtern(int pin){
	float volt = Bit2Volt(pin); // liest pin ein
	float Temp=NTC_Grad*volt-NTC_Off; // umrechnen Volt nach Grad Celcius
	return Temp; // zurueckgeben
}

float Bit2Volt(int n){    // Funktion zum Lesen und umwandeln
	int raw = analogRead(n);  // Messdaten von pin 'n' lesen
	float volt = (float)raw*5.000/1023;  // Umrechnung für Volt
	return volt;  // zurückgeben der Voltzahl
}

float calcAltitude(float pressure){    // rechne Hoehe
	float A = pressure/druckDurchschnitt; // Gemessenner Druck /Druckdurchschnitt
	float B = 1/5.25588;    // B als Konstante einsetzten (aus Datenbank)
	float C = pow(A,B);    // Logarithmus von A und B
	C = 1 - C;              
	C = C /0.0000225577;
	return C;
}

String floatToString(float number){
	String stringNumber = "";
	char tempChar[10]; 
	dtostrf(number, 4, 2, tempChar);
	stringNumber += tempChar;
	return stringNumber;
}

String GetGGA() {
  //Serial.println("ermittle gga");
	String gps_output = "";
	boolean GGAready = false;
	boolean start = false;
	int inByte = 0;
	char inChar;
  
	ermittleGPS = true;
	while (!GGAready) {
               // Serial.println(millis());
		if (Serial3.available()) { 
			inChar = Serial3.read();
			inByte = int(inChar);
			//Serial.print(inChar);	// Debug Ausgabe
			//Serial.print(inByte);	// Debug Ausgabe
			//Serial.print(", ");	// Debug Ausgabe
			if (start) {
				if (inByte==13) { // Abbruch Bedingung erreicht
					GGAready=true;
				} else {
					gps_output += inChar;
				}
			} else { // warten auf 10 im Stream, also Ende der Zeile
				if (inByte==10) {
					start = true;
				}
			}// Ende if (start)
		} /*else {
                  Serial.println("nothing available"); 
                }// Ende if (available)*/
	}// Ende while
	ermittleGPS = false;
	return(gps_output); 
}

/***********************************************************************************
 * SETUP ROUTINE                                                                   *
 ***********************************************************************************/
void setup () {  //Voreinstellungen
	pinMode(16, OUTPUT);  // init LED Debug beim Einschalten
	pinMode(17, OUTPUT);  // init LED
	pinMode(18, OUTPUT);  // init LED
	pinMode(19, OUTPUT);  // init LED
	pinMode(20, OUTPUT);  // init LED
	pinMode(21, OUTPUT);  // init LED
	pinMode(22, OUTPUT);  // init LED
	pinMode(23, OUTPUT);  // init LED
	digitalWrite(16, LOW);   // set LED on
	digitalWrite(17, LOW);   // set LED on
	if (debug == 1) {
		Serial.begin(Bitrate);
	}
	digitalWrite(18, LOW);   // set LED on
	Serial1.begin(Bitrate); 	// Initialisiere das Sendmodul (19200 bits per second):

	Serial3.begin(9600);  					// init GPS an Serial2
	// Serial3.println("$PMTK220,100*2F"); // 10 Hz, ging gut mit Arduino, mit T-Board nicht mehr
	Serial3.println("$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"); // Nur GGA ausgeben
	digitalWrite(19, LOW);   // set LED on

	pinMode (beepPin,OUTPUT);				// Piepser zum Laerm machen
	pinMode (dustLEDPin,OUTPUT);			// LED des Staubsensor

	digitalWrite(20, LOW);   // set LED on

	//Primärmission
	float hPa;
        int j = 0;
	for (int i = 0; i < 20 ; i++) { // Schleife, 20 mal
                j++;			
		hPa = getPressure(druckPin);
		if (800 < hPa &&  1500 > hPa){ 
		//zum Berechnen des 0-Druckes...sollten unwahrscheinliche Werte gemessen werden, 
		//sollen diese ignoriert und durch neue ersetzt werden
			druckBoden += hPa;
		} else {
			i--;
		} 
                if (j > 50){
                  i = 20;
                }
	}
	digitalWrite(21, LOW);   // set LED on
	druckDurchschnitt = druckBoden/20;      // Berechnung des Druckdurchschnittes
	
	if (debug == 1) {
		//Serial.println ("h:"+hoehe);      // debugging
		Serial.print ("hPa:");
		Serial.println (hPa);
		Serial.print ("druckDurchschnitt:");
		Serial.println (druckDurchschnitt);
		Serial.print ("setup done : ");
		Serial.println (hoehe);
		Serial.println(header);
	} // Ende debug
	digitalWrite(22, LOW);   // set LED on
	Serial1.println(header);
	digitalWrite(23, LOW);   // set LED on

		digitalWrite (beepPin, HIGH);
		delay(100);
		digitalWrite(beepPin,LOW);

	delay(500);              // warten fuer eine halbe Sekunde
	digitalWrite(16, HIGH);   // set LED off
	digitalWrite(17, HIGH);   // set LED off
	digitalWrite(18, HIGH);   // set LED off
	digitalWrite(19, HIGH);   // set LED off
	digitalWrite(20, HIGH);   // set LED off
	digitalWrite(21, HIGH);   // set LED off
	digitalWrite(22, HIGH);   // set LED off
	digitalWrite(23, HIGH);   // set LED off
	delay(500);              // warten fuer eine halbe Sekunde
	digitalWrite(16, LOW);   // set LED on
	digitalWrite(17, LOW);   // set LED on
	digitalWrite(18, LOW);   // set LED on
	digitalWrite(19, LOW);   // set LED on
	digitalWrite(20, LOW);   // set LED on
	digitalWrite(21, LOW);   // set LED on
	digitalWrite(22, LOW);   // set LED on
	digitalWrite(23, LOW);   // set LED on
	delay(500);              // warten fuer eine halbe Sekunde
	digitalWrite(16, HIGH);   // set LED off
	digitalWrite(17, HIGH);   // set LED off
	digitalWrite(18, HIGH);   // set LED off
	digitalWrite(19, HIGH);   // set LED off
	digitalWrite(20, HIGH);   // set LED off
	digitalWrite(21, HIGH);   // set LED off
	digitalWrite(22, HIGH);   // set LED off
	digitalWrite(23, HIGH);   // set LED off
}


/***********************************************************************************
 * LOOP ROUTINE                                                                    *
 ***********************************************************************************/
void loop () {  //Schleife <3
	if (millis()-letzteSendung >= Intervall) {  //Sendeintervall = 1s
		letzteSendung = millis();
		String output = "";  

		// ******************* PRIMÄRMISSION ************************
		float hPa = getPressure(druckPin);
		hoehe = calcAltitude(hPa);
		fall = (altehoehe - hoehe) / (Intervall / 1000) ;
		altehoehe = hoehe;
                		
		// Steigt die Rakete
		if (steige==false && hoehe>150){
			steige = true;
		}
		// Fällt der CanSat, unter 150m soll der Piepser dann anfangen.
		if (unter150 == false && steige==true && hoehe<150){
			unter150 = true;
		}

		output += (String)(letzteSendung / 1000);	// Anzeigen des Zeitcodes
		output += TRENNER; 
		output += floatToString(hPa); 				// Anzeigen des Druckwertes
		output += TRENNER; 
		output += floatToString(getTemperatureIntern(tempPin));  	// Anzeigen des Temperaturwertes INT  LM35
		output += TRENNER; 
		output += floatToString(getTemperatureExtern(tempNTCPin));	// Anzeigen NTC Temperaturwert EXT 
		output += TRENNER; 
		output += floatToString(hoehe);
		output += TRENNER; 
		output += floatToString(fall);

		// ******************* STAUBSENSOR ************************
		digitalWrite(dustLEDPin, LOW);			// LED anschalten
		delayMicroseconds (280);
		dustValue=analogRead(dustPin);			// Staubsensor auslesen
		delayMicroseconds (40);
		digitalWrite(dustLEDPin, HIGH);			// LED ausschalten
   
		delayMicroseconds (9680);				// Abklingen, vermutlich nicht nötig
 
		voltage = dustValue *0.0048875;			// umrechnen der ausgelesenen Daten zu Volt !!! 5/1023!!!
		if (voltage <= 3.5) {
			dustDensity = voltage /7.5;
		}
		else {
			dustDensity = (voltage-3.5)*2 +0.4;
		}

		output += TRENNER; 
		output += (String)(dustValue);			// Darstellen von gemessener Voltzahl
		output += TRENNER; 
		output += floatToString(voltage);		// errechneter Voltzahl
		output += TRENNER; 
		output += floatToString(dustDensity);	// letztendliche Staubkonzentration

		// ********************* GPS *****************************
/*		String temp = "";
		if (!ermittleGPS) {
			temp = GetGGA();
			if (!temp.equals("")) {
				output += TRENNER; 
				output += temp;
				output += TRENNER; 
			}
		}
*/		
		if (debug == 1) {
			Serial.println(output);
		}
		Serial1.println(output);
		
	} // Ende im Sekundentakt Senden
	
	if (unter150){ // ab 150m soll der Pieper sich bemerkbar machen.
	// Test Beeper
		digitalWrite (beepPin, HIGH);
		delay(100);
		digitalWrite(beepPin,LOW);
	}
} // Ende Loop
