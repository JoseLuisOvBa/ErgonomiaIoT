
/*Monitoreo de CO2, Presencia y ventilación
 * Envio de datos de los sensores MQ135 (CO2) y Pir Hcsr501 (presencia) por MQTT
 * por: Jose Luis Oviedo Barriga, David Garcia Sarmiento
 * Fecha: 24 de junio de 2022
 * 
 * Este programa lee los sensores MQ135 (CO2) y Pir Hcsr501 (presencia) por MQTT
 * ritmo cardiaco (HR) y oxigenación (SpO2) por MQTT.
 *            CO2                       PRESENCIA                    
 * ESP32      MQ135           ESP32      Pir Hcsr501      ESP32      LedInter  ESP32      Rele ventilador
 * I012 ------ 1 Datos        GND ------ 1 GND            I033 -----1 Datos    I013 ------1 Datos    
 * GND ------- 3 GND          I014 ------2 Datos                               VCC -------2 Vin  
 * VCC ------- 4 Vin          VCC -------3 Vin                                 GND ------ 3 GND                                                          
 * 
 **************IOs************
 *04 
 *02  PisV        Piston de Ventana
 *14  Sv1         Sensor 1 de Ventana
 *15  Sv2         Sensor 2 de Ventana
 *13  Venti       Rele  Ventilador
 *12  CO2         Sensor
 *16  PIR         Sensor de Presencia
 *33  Indicador de presencia Led Interno reflejado en el Dashboard
 *
 * Esto es una muestra de la estructura básica de un programa
 */

// Bibliotecas************************************************************************
#include <WiFi.h>  // Biblioteca para el control de WiFi
#include <PubSubClient.h> //Biblioteca para conexion MQTT


// Constantes*************************************************************************
float CO2Value; //CO2Value; variable para guardar el valor analógico del sensor
int Venti=13;    //Rele Ventilador
int LedIn = 33;        //LedInterno
int ValorPIR=0;

// Variables**************************************************************************
long timeNow, timeLast; // Variables de control de tiempo no bloqueante
int data = 0; // Contador
int wait = 5000;  // Indica la espera cada 5 segundos para envío de mensajes MQTT

// Definición de objetos**************************************************************
  #define CO2 12
  #define PIR 16

// Condiciones iniciales - Se ejecuta sólo una vez al energizar***********************
void setup() {

     pinMode(PIR, INPUT);
     pinMode(LedIn , OUTPUT);
     pinMode (Venti, OUTPUT);
     
     Serial.begin (115200);
     Serial.println("El sensor de gas se esta pre-calentando");
     digitalWrite(LedIn, 0); 
     delay(5000); // Espera a que el sensor se caliente durante 20 segundos
     digitalWrite(LedIn, 1); 

     timeLast = millis (); // Inicia el control de tiempo     

}// Fin de void setup*****************************************************************


void loop(){     //VOID LOOP**********************************************************///////////////////

    timeNow = millis(); // Control de tiempo para esperas no bloqueantes
      if (timeNow - timeLast > wait) { // Manda un mensaje por MQTT cada cinco segundos
      timeLast = timeNow; // Actualización de seguimiento de tiempo

      presencia();                          //Funcion de presencia  
      COdos();                              //Funcion deteccion de CO2
    }// fin del if (timeNow - timeLast > wait)

}// Fin de void loop*****************************************************************///////////////////




// Funcioes del usuario
//----------------------- PRESENCIA----------------------------
void presencia(){                //Esta funcion realiza el sensado de presencia
  ValorPIR = digitalRead(PIR);   //Lectura del Sensor PIR que se guarda en ValorPIR
if (ValorPIR == HIGH){           //Pregunta si esta en alta
     Serial.print(" | PRESENCIA | ");
   digitalWrite(LedIn, 0);       //de ser asi lo enciede(0=encendido) logica inversa
   } else{                       // si esta en bajo
      digitalWrite(LedIn, 1);    //Pemanece apagado (1=apagado) logica inversa
      Serial.print(" | AUSENCIA | ");
      }
  } 




//----------------------- CO2 ---------------------------

  void COdos(){                //Esta funcion realiza el sensado de presencia
  CO2Value = analogRead(CO2); // lectura de la entrada analogica "A0""
  Serial.print("Valor detectado por el sensor: ");
  Serial.print(CO2Value );
  if(CO2Value  > 600)   // La OMS sugiere de 400 a 600
    {
     Serial.print("  ¡Se ha detectado CO2!  ");
     digitalWrite (Venti, HIGH);
     delay(2000);
    }
  else{
    Serial.print("");
    digitalWrite (Venti, LOW);
    delay(2000);
    }
  Serial.println("");
  } 
