/*  MEDIDOR SIMPLE DE ESR Y CAPACIDAD. Solamente necesario  Arduino y una resistencia de 1k.
EN CONDENSADORES de >= 1 uF.

Basado en los trabajos de:
Paul Badger 2008 (https://docs.arduino.cc/tutorials/generic/capacitance-meter).
system: https://forum.arduino.cc/t/capacimetro-con-arduino/106928
Electgpl: https://youtu.be/pFPTO1TySSM
j-rpm: http://j-rpm.com/2022/12/capacimetro-esr-con-arduino-v2/

El condensador se conecta entre puerto A5 y Tierra.
Se genera una corriente pulsante/alterna, de periodo T=4  us (2+2) => f= 250 KHz).
La idea es que en corriente alterna el condensador equivale a un cortocircuito, por lo que VA5 = VRESR.
Se está despreciando la reactancia capacitiva en alterna que tiene un valor = Xc=  1/ (2.PI.f.C).
Para un condensador de 1uF, y a la frecuencia de ¼ MHz, sería de Xc= 0,6366  Ω.
Para un condensador de 10uF sería Xc = 0,06366 Ω y para un condensador 100 uF sería de Xc = 0,006366 Ω. 

IMPORTANTE:
----------
Los condensadores deben estar descargados o con tensión <= 5v

byjosemaria@gmail.com
 */


#define analogPin    A5 // Pin analógico para medir ESR
#define pulsePin     4  //Pin digital para mandar frecuencia pulsante pulso en la medida ESR
float esrSamples;
double Resr;
byte muestras = 4; //el número de muestras afectan al ESR, mayor número --> mayor ESR
int Ventrada = 0; //(medida en milivoltios)
int Vsalida; //(medida en milivoltios)
int resistorPULSO = 1000.0;// medida en ohmios (Rp)
unsigned long startTime;
unsigned long elapsedTime;
float microFarads;   // floating point variable to preserve precision, make calculations


//Medimos la tensión de entrada REAL.
int refADC(){
   long result;
   // Lee 1.1V de referencia interna 
   ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
   delay(2); 
   ADCSRA |= _BV(ADSC);
   while (bit_is_set(ADCSRA,ADSC));
   result = ADCL;
   result |= ADCH<<8;
   result = 1125300L / result; // Calcula la tensión AVcc en mV
   return result;
}



void setup()
{
Serial.begin(9600); // se establece a 9600bps la comunicación por el puerto serie
pinMode(pulsePin, OUTPUT);
pinMode(analogPin, INPUT);

Serial.println(); // Muestra un línea en blanco.
Serial.println("CAPACITOR Y ESR EN CONDENSANDORES"); // Muestra por el puerto serie el mensaje CAPACIMETRO
Serial.println("---------------------------------");  // Muestra por el puerto serie el mensaje ----------------- en otra línea de texto
Serial.println();
Ventrada = refADC();
Serial.print("ADCvRef - Tension entrada: ");
Serial.print(Ventrada);
Serial.println(" mV");
Serial.println();
delay(2000);
}

void loop() {

medidaESR();
medidaCapacidad();

}




//Descarga del condensador
void descarga()
{

//Serial.println(" Descargando condensador...");
digitalWrite(pulsePin,LOW); //
//delay(1000);
while(analogRead(analogPin) > 0){ }        // Otra opción para descargar wait until capacitor is completely discharged
//Serial.println(analogRead(analogPin));
//delay(200);

}


//Medida de la capacidad
void medidaCapacidad() {

descarga();


pinMode(analogPin, INPUT);
digitalWrite(pulsePin, HIGH);   
startTime = micros();
while(analogRead(analogPin) < (647)){       // 647 is 63.2% of 1023, corresponde con 1T = R.C
  }


elapsedTime= micros() - startTime;
digitalWrite(pulsePin, LOW);   
microFarads = ((float)elapsedTime / resistorPULSO) ;
  if (microFarads > 1){
    Serial.print(microFarads);       // print the value to serial port
    Serial.println(" uF");         // print units and carriage return 
    delay (2000); //tiempo de espera para poder ver el resultado en el monitor serial
  }

  else{
    // if value is smaller than one microFarad, convert to nanoFarads (10^-9 Farad).
    Serial.print((microFarads * 1000.0));   // print the value to serial port
    Serial.println(" nF");  // print units and carriage return
    delay (2000); //tiempo de espera para poder ver el resultado en el monitor serial

  }
Serial.println();  
}


//calcula ESR 
void medidaESR()
{

pinMode(analogPin, INPUT);
Resr = 0;
esrSamples =0;

//DESCARGA del condensador
descarga();


//GENERACION DE SEÑAL CUADRADA, ALTA FRECUENCIA , MEDIDA y repetición para el muestreo
for (int i=0;i<muestras;i++){
digitalWrite(pulsePin,HIGH); //
delayMicroseconds(2);
digitalWrite(pulsePin,LOW); //
delayMicroseconds(2);
esrSamples = esrSamples + analogRead(analogPin); //medimos Vsalida al final de cada periodo
}

esrSamples = float(esrSamples)/muestras;

Vsalida = (esrSamples * (float)Ventrada) / 1023.0; //Convertimos Vsalida a milivoltios
Resr = (resistorPULSO * (float)Vsalida)/((float)Ventrada -Vsalida); //voltage divider RESR=(Rp*Vo)/(Vi-Vo)


if(Resr<20)
{
Serial.print("ESR (Ω) = ");
Serial.println(Resr,3);
}
else
{
Serial.print("ESR-fuera de rango (OL)(Ω) = ");
Serial.println(Resr,3);

}

delay(200);
}
