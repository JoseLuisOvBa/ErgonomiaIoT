
/*Monitoreo de CO2, Presencia y ventilación
 * Envio de datos de los sensores MQ135 (CO2) y Pir Hcsr501 (presencia) por MQTT
 * por: Jose Luis Oviedo Barriga, David Garcia Sarmiento
 * Fecha: 28 de junio de 2022
 * 
 * Este programa lee los sensores MQ135 (CO2) y Pir Hcsr501 (presencia) por MQTT
 * ritmo cardiaco (HR) y oxigenación (SpO2) por MQTT.
 *            CO2                        PRESENCIA                    
 * ESP32      MQ135PinAtras      ESP32      Pir Hcsr501   ESP32      Ventilador
 * I012 ------    1 Datos        GND ------ 1 GND         I013 ------1 Datos    
 * GND -------    3 GND          I016 ------2 Datos        
 * VCC -------    4 Vin          VCC -------3 Vin                                                                  
 *        
 * Asignacion de pines ESP32CAM
 * IO4    Abrir     En 1 Abrir ventana
 * IO2    Cerrar    En 1 Cerrar ventana
 * IO14   sV2       Sensor de ventana compeltamente cerrada
 * IO15   sV1       Sensor de ventana compltamente abierta
 * IO13   Venti     Rele Ventilador  
 * IO12   CO2       Sensor CO2
 * IO16   PIR       Sensor Presencia
 * IO33   LedIn     LEDinterno (Acuse de Presencia)
 * 
 */

// Bibliotecas************************************************************************
#include <WiFi.h>  // Biblioteca para el control de WiFi
#include <PubSubClient.h> //Biblioteca para conexion MQTT


// Constantes*************************************************************************
float ValorCO2; // variable para guardar el valor analógico del sensor
int Venti=13;    //RelayFan
int LedInt = 33;        //LED interno
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
     pinMode(LedInt, OUTPUT);
     pinMode (Venti, OUTPUT);
     
     Serial.begin (115200);
     Serial.println("El sensor de gas se esta pre-calentando");
     delay(5000); // Espera a que el sensor se caliente durante 20 segundos

     timeLast = millis (); // Inicia el control de tiempo     

}// Fin de void setup*****************************************************************


void loop(){     //VOID LOOP**********************************************************///////////////////

    timeNow = millis(); // Control de tiempo para esperas no bloqueantes
      if (timeNow - timeLast > wait) { // Manda un mensaje por MQTT cada cinco segundos
      timeLast = timeNow; // Actualización de seguimiento de tiempo

      presencia();                          //funcion de presencia  
      COdos();                              //Funcion deteccion de CO2
    }// fin del if (timeNow - timeLast > wait)

}// Fin de void loop*****************************************************************///////////////////




// Funcioes del usuario
//----------------------- PRESENCIA----------------------------
void presencia(){                //Esta funcion realiza el sensado de presencia
  ValorPIR = digitalRead(PIR);   //Lectura del Sensor PIR que se guarda en ValorPIR
if (ValorPIR == HIGH){           //Pregunta si esta en alta
     Serial.print(" | PRESENCIA | ");
   digitalWrite(LedInt, 0);       //de ser asi lo enciede
   } else{                       // si esta en bajo
      digitalWrite(LedInt, 1);    //Pemanece apagado
      Serial.print(" | AUSENCIA | ");
      }
  } 



//----------------------- CO2---------------------------

  void COdos(){                //Esta funcion realiza el sensado de presencia
  ValorCO2 = analogRead(CO2); // lectura de la entrada analogica "A0""
  Serial.print("Valor detectado por el sensor: ");
  Serial.print(ValorCO2);
  if(ValorCO2 > 600)   // La OMS sugiere de 400 a 600
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
