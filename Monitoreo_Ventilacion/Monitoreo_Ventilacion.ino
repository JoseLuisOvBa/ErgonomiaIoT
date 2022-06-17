
/*Monitoreo de CO2, Presencia y ventilación
 * Envio de datos de los sensores MQ135 (CO2) y Pir Hcsr501 (presencia) por MQTT
 * por: Jose Luis Oviedo Barriga
 * Fecha: 15 de junio de 2022
 * 
 * Este programa lee los sensores MQ135 (CO2) y Pir Hcsr501 (presencia) por MQTT
 * ritmo cardiaco (HR) y oxigenación (SpO2) por MQTT.
 *            
 * ESP32      MQ135           ESP32      Pir Hcsr501      ESP32      Pir Hcsr501 
 * I012 ------ 1 Datos        GND ------ 1 GND            I04 ------1 Datos    
 * GND ------- 3 GND          I014 ------2 Datos          VCC -------2 Vin  
 * VCC ------- 4 Vin          VCC -------3 Vin            GND ------ 3 GND                                                               
 * 
 * Esto es una muestra de la estructura básica de un programa
 */

// Bibliotecas

// Constantes
float sensorValue; //CO2Value; variable para guardar el valor analógico del sensor
int relayPin=13;    //RelayFan
int Flash = 4;        //ReleyFocoFlash
int ValorPIR=0;


// Definición de objetos
  #define MQ135pin 12     //#define CO2PIN 12
  #define PIR 14

// Condiciones iniciales - Se ejecuta sólo una vez al energizar
void setup() {

     pinMode(PIR, INPUT);
     pinMode(Flash, OUTPUT);
     pinMode (relayPin, OUTPUT);
     
     Serial.begin (115200);
     Serial.println("El sensor de gas se esta pre-calentando");
     delay(5000); // Espera a que el sensor se caliente durante 20 segundos

}// Fin de void setup


void loop(){                 //VOID LOOP
presencia();                          //funcion de presencia
gases();                              //Funcion deteccion de CO2
}                            // Fin de void loop




// Funcioes del usuario
//----------------------- PRESENCIA----------------------------
void presencia(){                //Esta funcion realiza el sensado de presencia
  ValorPIR = digitalRead(PIR);   //Lectura del Sensor PIR que se guarda en ValorPIR
if (ValorPIR == HIGH){           //Pregunta si esta en alta
   digitalWrite(Flash, 1);       //de ser asi lo enciede (fisicamente parapadea ya que el dispositivo se calibra para que este un tiempo apagado y otro tiempo encendido)
   } else{                       // si esta en bajo
      digitalWrite(Flash, 0);    //Pemanece apagado
      }
  } 




//----------------------- GASES ---------------------------

  void gases(){                //Esta funcion realiza el sensado de presencia
  sensorValue = analogRead(MQ135pin); // lectura de la entrada analogica "A0""
  Serial.print("Valor detectado por el sensor: ");
  Serial.print(sensorValue);
  if(sensorValue > 100)
  {
   Serial.print(" | Se ha detectado gas!");
   digitalWrite (relayPin, HIGH);
   delay(2000);
  }
  Serial.println("");
  digitalWrite (relayPin, LOW);
  delay(2000);
  } 
