
/*Monitoreo de CO2, Presencia y ventilación
 * Envio de datos de los sensores MQ135 (CO2) y Pir Hcsr501 (presencia) por MQTT
 * por: Jose Luis Oviedo Barriga
 * Fecha: 15 de junio de 2022
 * 
 * Este programa lee los sensores MQ135 (CO2) y Pir Hcsr501 (presencia) por MQTT
 * ritmo cardiaco (HR) y oxigenación (SpO2) por MQTT.
 *            GAS                        PRESENCIA                    
 * ESP32      MQ135           ESP32      Pir Hcsr501      ESP32      FLASH     ESP32      FLASH
 * I012 ------ 1 Datos        GND ------ 1 GND            I04 ------1 Datos    I013 ------1 Datos    
 * GND ------- 3 GND          I014 ------2 Datos          VCC -------2 Vin     VCC -------2 Vin  
 * VCC ------- 4 Vin          VCC -------3 Vin            GND ------ 3 GND     GND ------ 3 GND                                                          
 * 
 * Esto es una muestra de la estructura básica de un programa
 */

// Bibliotecas************************************************************************
#include <WiFi.h>  // Biblioteca para el control de WiFi
#include <PubSubClient.h> //Biblioteca para conexion MQTT


// Constantes*************************************************************************
float sensorValue; //CO2Value; variable para guardar el valor analógico del sensor
int relayPin=13;    //RelayFan
int Flash = 4;        //ReleyFocoFlash
int ValorPIR=0;

// Variables**************************************************************************
long timeNow, timeLast; // Variables de control de tiempo no bloqueante
int data = 0; // Contador
int wait = 5000;  // Indica la espera cada 5 segundos para envío de mensajes MQTT

// Definición de objetos**************************************************************
  #define MQ135pin 12     //#define CO2PIN 12
  #define PIR 14

// Condiciones iniciales - Se ejecuta sólo una vez al energizar***********************
void setup() {

     pinMode(PIR, INPUT);
     pinMode(Flash, OUTPUT);
     pinMode (relayPin, OUTPUT);
     
     Serial.begin (115200);
     Serial.println("El sensor de gas se esta pre-calentando");
     delay(5000); // Espera a que el sensor se caliente durante 20 segundos

     timeLast = millis (); // Inicia el control de tiempo     

}// Fin de void setup*****************************************************************


void loop(){     //VOID LOOP**********************************************************

    timeNow = millis(); // Control de tiempo para esperas no bloqueantes
      if (timeNow - timeLast > wait) { // Manda un mensaje por MQTT cada cinco segundos
      timeLast = timeNow; // Actualización de seguimiento de tiempo

      presencia();                          //funcion de presencia  
      gases();                              //Funcion deteccion de CO2
    }// fin del if (timeNow - timeLast > wait)

}// Fin de void loop*****************************************************************




// Funcioes del usuario
//----------------------- PRESENCIA----------------------------
void presencia(){                //Esta funcion realiza el sensado de presencia
  ValorPIR = digitalRead(PIR);   //Lectura del Sensor PIR que se guarda en ValorPIR
if (ValorPIR == HIGH){           //Pregunta si esta en alta
     Serial.println(" | PRESENCIA | ");
   digitalWrite(Flash, 1);       //de ser asi lo enciede (fisicamente parapadea ya que el dispositivo se calibra para que este un tiempo apagado y otro tiempo encendido)
   } else{                       // si esta en bajo
      digitalWrite(Flash, 0);    //Pemanece apagado
      Serial.println(" | AUSENCIA | ");
      }
  } 




//----------------------- GASES ---------------------------

  void gases(){                //Esta funcion realiza el sensado de presencia
  sensorValue = analogRead(MQ135pin); // lectura de la entrada analogica "A0""
  Serial.print("Valor detectado por el sensor: ");
  Serial.print(sensorValue);
  if(sensorValue > 310)
    {
     Serial.print(" | Se ha detectado gas!");
     digitalWrite (relayPin, HIGH);
     delay(2000);
    }
  else{
    Serial.println("");
    digitalWrite (relayPin, LOW);
    delay(2000);
    }
  } 
