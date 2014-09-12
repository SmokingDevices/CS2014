int incomingByte = 0; 
unsigned long  oldtime=0, time;
boolean ermittleGPS = false;

void setup()
{
  Serial.begin(57600);
  Serial1.begin(9600);
  Serial1.println("$PMTK220,100*2F"); // 10 Hz
  Serial1.println("$PMTK314,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*29"); // Nur GGA ausgeben
/* Geschwindigkeit auf dem COM Port erhöhen??
Serial1.println("$PMTK251,57600*2C"); // Baudrate auf 57600 setzen
  Serial1.end();
  delay(100);
  Serial1.begin(57600);
*/
}

void loop()
{
  String temp = "";
  if (!ermittleGPS)
  {
    temp = GetGGA();
    if (!temp.equals(""))
    {
      time = millis();
      Serial.print(time);
      Serial.print(" - ");
      Serial.print(time-oldtime);
      oldtime = time;
      Serial.print(" - ");
      Serial.println(temp);  
    }
  }
}

String GetGGA()
{
  ermittleGPS = true;
  String output = "";
  boolean GGAready = false;
  boolean start = false;
  boolean ok = false;
  int inByte = 0;
  char inChar;
  
  while (!GGAready) 
  {
    if (Serial1.available())
   { 
     inChar = Serial1.read();
     //Serial.print(inChar); //Debug Ausgabe
     inByte = int(inChar);
     if (start)
     {
       output += inChar;
       if (!ok)
       {// richtigen Header noch nicht erkannt
         if (inChar==',')
         {
           if (output.equals("$GPGGA,")==true)
           { //nun richtiger Header da, von nun an, nur doch auf das Ende warten
             ok = true;
           } else
           { // das war dann wohl der falsche Header, also von vorne
             output = "";
             start = false;
           }
         }
       } else
       { // hier kommen wir nur her, wenn Header richtig ist, also auf Ende warten
         if (inByte==13)
         {
           GGAready=true;
         }
       }
     } else
     { // warten auf 13, 10 im Stream, also Ende der Zeile
       if (inByte==10) 
       {
         start = true;
       }
     }// Ende if (start)
   }// Ende if (available)
 }// Ende while
 ermittleGPS = false;
 return(output); 
}
