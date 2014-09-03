#include <math.h>          //notwendig für log

/***********************************************************************************
 * DEFINITIONEN                                                                    *
 ***********************************************************************************/
#define debug 1 // debug = 1 = Ausgabe auf Serial, = 0 nur Senden
 
#define Vcc 5.0              //Supply voltage 

#define Bitrate 19200

#define druckPin A6
#define tempPin A1
#define tempNTCPin A2
#define beepPin 5        // digitaler Pin für Beeper

// LM35 Temp sensor
#define TmpSens 100          //Sensitivity. Deg.Celsius /volt

// NTC-sensor variables
#define R1 10000.0            //Value of R1 on the shield
#define NTC_Grad 18.1699      //The gradiant value for the NTC sensor
#define NTC_Off 21.0         //The offset value for the NTC sensor
#define TRENNER "|"

/***********************************************************************************
 * VARIABLEN                                                                       *
 ***********************************************************************************/
//Primärmission (1)
long int letzteSendung = 0;
int Intervall = 1000;    

boolean ermittleGPS = false; //Wird gerade GPS ermittelt

int pause = 1000;
int sensorValue = 0;  // variable to store the value coming from the sensor
float druckBoden = 0;
float druckDurchschnitt = 0;      //Variablen setzen zur späteren Verwendung
float hoehe = 0;
float altehoehe=0;
float fall= 0;

//Staubsensor (2)
int dustPin = 5;  //angeschlossen an Pin 6
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
String header = "time,Druck,Temp (int), Temp (ext),Hoehe, Fallgeschw.,Staubwert, Staubspannung, Staubdichte, GPS";

/***********************************************************************************
 * FUNKTIONEN                                                                      *
 ***********************************************************************************/
//Primärmission (1)
float getPressure (int pin) {  //methode für Druck
	float volt = Bit2Volt(pin); //liest pin ein 
	float hPa = 10*(volt/(0.009*Vcc)+(0.095/0.009));  //umrechnung von V zu Pa und dann zu hPa
	return hPa;  //zurückgeben (an aufrufer)
}

float getTemperatureExtern(int pin){  //LM35 an A1
	float volt = Bit2Volt(pin); //liest pin ein
	float result =volt*TmpSens;  //umrechnung
	return result;  //zurückgeben (an aufrufer)
}

float getTemperatureIntern(int pin){
	float volt = Bit2Volt(pin); //liest pin ein
	float Temp=NTC_Grad*volt-NTC_Off; //umrechnen Volt nach Grad Celcius
	return Temp; //zurückgeben
}

float Bit2Volt(int n){    //Function to convert raw ADC-data (0-255) to volt
	int raw = analogRead(n);  //messdaten von pin 'n' lesen
	float volt = (float)raw*5.000/1024;  //umrechnung für volt
	return volt;  //zurückgeben der Voltzahl
}

float calcAltitude(float pressure){    //rechne hoehe
	float A = pressure/druckDurchschnitt; // Gemessenner Druck /Druckdurchschnitt
	float B = 1/5.25588;    // B als Konstante einsetzten (aus Datenbank)
	float C = pow(A,B);    // Logarithmus von A und B
	C = 1 - C;              
	C = C /0.0000225577;
	return C;
}

String GetGGA() {
	String gps_output = "";
	boolean GGAready = false;
	boolean start = false;
	int inByte = 0;
	char inChar;
  
	ermittleGPS = true;
	while (!GGAready) {
		if (Serial3.available()) { 
			inChar = Serial3.read();
			inByte = int(inChar);
			//Serial.print(inChar); //Debug Ausgabe
			//Serial.print(inByte); //Debug Ausgabe
			//Serial.print(", "); //Debug Ausgabe
			if (start) {
				if (inByte==13) { //Abbruch Bedingung erreicht
					GGAready=true;
				} else {
					gps_output += inChar;
				}
			} else { // warten auf 13, 10 im Stream, also Ende der Zeile
				if (inByte==10) {
					start = true;
				}
			}// Ende if (start)
		}// Ende if (available)
	}// Ende while
	ermittleGPS = false;
	return(gps_output); 
}

/***********************************************************************************
 * SETUP ROUTINE                                                                   *
 ***********************************************************************************/
void setup () {  //Voreinstellungen
	if (debug == 1) {
		Serial.begin(Bitrate);  //datenblocks pro sekunde
	}
	Serial1.begin(Bitrate); // initialize serial communication at 19200 bits per second:

	Serial3.begin(9600);  // init GPS an Serial2
	// Serial3.println("$PMTK220,100*2F"); // 10 Hz, ging gut mit Arduino, mit T-Board nicht mehr
	Serial3.println("$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"); // Nur GGA ausgeben

	pinMode (beepPin,OUTPUT);	//LED=verbraucher --> pinmode output
	pinMode (ledPower,OUTPUT);	//LED=verbraucher --> pinmode output
	pinMode (dustPin,OUTPUT);	//Staubsensor ist Verbraucher...s.o.
 
	//Primärmission
	float hPa;
	for (int i = 0; i < 20 ; i++) {    //schleife, 20 mal
		hPa = getPressure(A0);
		if (800 < hPa &&  1500 > hPa){ 
		//zum Berechnen des 0-Druckes...sollten unwahrscheinliche Werte gemessen werden, 
		//sollen diese ignoriert und durch neue ersetzt werden
			druckBoden += hPa;
		} else {
			i--;
		} 
	}
	druckDurchschnitt = druckBoden/20;      //Berechnung des Druckdurchschnittes
	
	if (debug == 1) {
		//Serial.println ("h:"+hoehe);      //debugging
		Serial.print ("hPa:");
		Serial.println (hPa);
		Serial.print ("druckDurchschnitt:");
		Serial.println (druckDurchschnitt);
		Serial.print ("setup done : ");
		Serial.println (hoehe);
		Serial.println(header);
	} // Ende debug
	Serial1.println(header);
}


/***********************************************************************************
 * LOOP ROUTINE                                                                   *
 ***********************************************************************************/
void loop () {  //Schleife <3
	if (millis()-letzteSendung >= Intervall) {  //Sendeintervall = 1s
		letzteSendung = millis();
		String output = "";  
		//Primärmission
		float hPa = getPressure(A0);
		hoehe = calcAltitude(hPa);
		fall = (altehoehe - hoehe) / (pause / 1000) ;
		altehoehe = hoehe;

		Serial.print(letzteSendung / 1000); //anzeigen des Zeitcodes
		Serial.print (TRENNER); //Trennstrich (pipe)
		Serial.print(hPa); //anzeigen des Druckwertes
		Serial.print (TRENNER); //Trennstrich (pipe)
		Serial.print (getTemperatureExtern(A1));  //anzeigen des Temperaturwertes EXT  LM35
		Serial.print (TRENNER); //Trennstrich (pipe)
		Serial.print (getTemperatureIntern(A2)); //anzeigen NTC Temperaturwert INT 
		Serial.print (TRENNER); //Trennstrich (pipe)
		Serial.print (hoehe);
		Serial.print (TRENNER); //Trennstrich (pipe)
		Serial.print (fall);
 
		Serial1.print(letzteSendung / 1000); //anzeigen des Zeitcodes
		Serial1.print (TRENNER); //Trennstrich (pipe)
		Serial1.print(hPa); //anzeigen des Druckwertes
		Serial1.print (TRENNER); //Trennstrich (pipe)
		Serial1.print (getTemperatureExtern(A1));  //anzeigen des Temperaturwertes
		Serial1.print (TRENNER); //Trennstrich (pipe)
		Serial1.print (getTemperatureIntern(A2)); //anzeigen NTC Temperaturwert
		Serial1.print (TRENNER); //Trennstrich (pipe)
		Serial1.print (hoehe);      //Höhenangabe
		Serial1.print (TRENNER); //Trennstrich (pipe)
		Serial1.print (fall);      //Höhenangabe
   
    
		//Staubsensor
		digitalWrite (ledPower, LOW);    //LED anschalten
		delayMicroseconds (280);
		dustValue=analogRead (dustPin);    //Staubsensor auslesen
		delayMicroseconds (40);
		digitalWrite (ledPower, HIGH);    //LED ausschalten
   
		delayMicroseconds (9680);			// Abklingen, vermutlich nicht nötig
 
		voltage = dustValue *0.0048875;    //umrechnen der ausgelesenen Daten zu Volt
		// dustDensity = 0.17*voltage-0.1;
		dustDensity = voltage/5;


		Serial.print (TRENNER); //Trennstrich (pipe)
		Serial.print(dustValue);  //darstellen von gemessener voltzahl
		Serial.print (TRENNER);
		Serial.print(voltage);  //- errechneter voltzahl
		Serial.print (TRENNER);
		Serial.print(dustDensity);  //- letztendliche staubkonzentration
    
		Serial1.print (TRENNER); //Trennstrich (pipe)
		Serial1.print(dustValue);  //darstellen von gemessener voltzahl
		Serial1.print (TRENNER);
		Serial1.print(voltage);  //- errechneter voltzahl
		Serial1.print (TRENNER);
		Serial1.print(dustDensity);  //- letztendliche staubkonzentration
 

		//GPS
		String temp = "";
		if (!ermittleGPS) {
			temp = GetGGA();
			if (!temp.equals("")) {
				Serial.print(TRENNER);
				Serial.print(temp);  
				Serial1.print(TRENNER);
				Serial1.print(temp);  
			}
		}
		/*
		if (debug == 1) {
			Serial.println(output);
		}
		Serial1.println(output);
		*/
		Serial.println(TRENNER);
		Serial1.println (TRENNER);

		// Test Beeper
		digitalWrite (beepPin, HIGH);
		delay(10);
		digitalWrite(beepPin,LOW);
  } // Ende im Sekundentakt Senden
} // Ende Loop