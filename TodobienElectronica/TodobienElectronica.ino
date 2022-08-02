
/*Monitoreo de CO2, Presencia y ventilación
   Este programa tiene por objetivo comprobar el funcionamiento de los sensores MQ135 (CO2) y Pir Hcsr501 (presencia) 
   por: Jose Luis Oviedo Barriga, David Garcia Sarmiento, Erika Barojas Payán
   Fecha: 29 de junio de 2022

   Este programa lee los sensores MQ135 (CO2) y Pir Hcsr501 (presencia) 

              CO2                        PRESENCIA
   ESP32      MQ135PinAtras      ESP32      Pir Hcsr501   ESP32      Ventilador
   I012 ------    1 Datos        GND ------ 1 GND         I015 ------1 Datos
   GND -------    3 GND          I013 ------2 Datos
   VCC -------    4 Vin          VCC -------3 Vin

   Asignacion de pines DISPONIBLES ESP32CAM
   IO4    Abrir     En 1 Abrir ventana
   IO2    Cerrar    En 1 Cerrar ventana
   IO14   Sv        Sensor de ventana compeltamente Abierta/Cerrada (compuerta EXOR)
   IO15   Venti     Rele Ventilador
   IO12   CO2       Sensor CO2
   IO13   PIR       Sensor Presencia
   IO33   LedIn     LEDinterno (Acuse de Presencia)

*/

// Constantes*************************************************************************
float ValorCO2 = 0;   // Variable para guardar el valor digital del sensor de CO2
int ValorPIR = 0;     // Variable para guardar el valor digital del sensor de presencia
int ValorSv = 0;      // Variable para guardar el valor digital del sensor ventana activa
int EdoVen = 0;       // Condicion inicial Ventana cerrada  EdoVen=1 Abierta // EdoVen=0 Cerrada

// Variables**************************************************************************
long timeNow, timeLast;    // Variables de control de tiempo no bloqueante
int data = 0;              // Contador
int wait = 5000;           // Indica la espera cada 5 segundos para envío de mensajes MQTT

// Definición de objetos**************************************************************

#define LedInt 33   // Pin del LED interno
#define PIR 13      // Pin del sensor de presencia
#define Sv 14       // Pin de la EXOR
#define Venti 15    // Pin de Relevador que activará el ventilador 
#define CO2 12      // Pin del sensor de CO2
#define Abrir 4     // Pin de salida al puente H que controla el motor para abrir la ventana
#define Cerrar 2    // Pin de salida al puenta H que controla el motor para cerrar la ventana

// Condiciones iniciales - Se ejecuta sólo una vez al energizar***********************
void setup() {

  pinMode(Abrir, OUTPUT);      // Salida del ESP32 de apertura de ventana y entrada 1 de puente H
  pinMode(Cerrar, OUTPUT);     // Salida del ESP32 de cierre de ventana y entrada 2 de puente H
  pinMode(Venti, OUTPUT);      // Salida de encendido de ventiladaro
  pinMode(LedInt, OUTPUT);     // Salida al LED interno del ESP32
  pinMode(Sv, INPUT);          // Entrada de la EXOR que indica que la ventana esta completamente Abierta/Cerrada
  pinMode(PIR, INPUT);         // Entrada del sensor de presencia

  digitalWrite(Abrir, LOW);     // Se inicializa apagada a apertura de la ventana 
  digitalWrite(Cerrar, LOW);    // Se inicializa apagado el cirre de la ventana
  digitalWrite(Venti, LOW);     // Se inicializa apagado el ventilador 

  Serial.begin (115200);
  Serial.println("El sensor de gas se esta pre-calentando");
  delay(5000); // Espera a que el sensor se caliente durante 5 segundos

  timeLast = millis ();         // Inicia el control de tiempo

}// Fin de void setup*****************************************************************


void loop() {    //VOID LOOP**********************************************************///////////////////

  timeNow = millis();                       // Control de tiempo para esperas no bloqueantes
  if (timeNow - timeLast > wait) {          // Manda un mensaje por MQTT cada cinco segundos
    timeLast = timeNow;                     // Actualización de seguimiento de tiempo

    presencia();                            // Funcion de presencia
    COdos();                                // Funcion deteccion de CO2
  }// fin del if (timeNow - timeLast > wait)

}// Fin de void loop*****************************************************************///////////////////




// Funcioes del usuario
//----------------------- PRESENCIA----------------------------
void presencia() {                    // Esta funcion realiza el sensado de presencia
  ValorPIR = digitalRead(PIR);        // Lectura del Sensor PIR que se guarda en ValorPIR
  if (ValorPIR == HIGH) {             // Pregunta si esta en alta
    Serial.print(" | PRESENCIA | ");
    digitalWrite(LedInt, 0);          // de ser asi lo enciede
  } else {                            // si esta en bajo
    digitalWrite(LedInt, 1);          // Pemanece apagado (Lógica inversa) 
    Serial.print(" | AUSENCIA | ");   
  }
} 


//----------------------- CO2---------------------------

void COdos() {                       // Esta funcion realiza el sensado de presencia
  ValorCO2 = digitalRead(CO2);       // Lectura de la entrada digital (1=no hay / 0=Si hay)
  ValorCO2=!ValorCO2;                // Se niega ya que el sensor entrega la respuesta negada (0=No hay / 1=Si hay)
  Serial.print(" Detección de CO2 (1 hay / 0 no hay): ");
  Serial.print(ValorCO2);

  if (ValorCO2 == 1)                                    // Si ValorCO==1 entonces hay de CO2  
  {
    Serial.print("  ¡Se ha detectado CO2!  ");
    digitalWrite(Venti, HIGH);                         // Enciende Ventilador
    if (EdoVen == 0) {                                 // Si la ventana está cerrada
      Open();                                          // Abre la Ventana
    }
    delay(1000);
  }
  else {                                               // Si ValorCO==0 entonces NO hay de CO2                                          
    Serial.print("");
    digitalWrite (Venti, LOW);                         //  Se apaga el Ventilardor 
    if (EdoVen == 1) {                                 // Si la ventana esta abierta
      Close();                                         // Cierra Ventana
    }
    delay(1000);
  }

  Serial.println("");
}


//------------------------- Abrir Ventana  -----------------------------
void Open() {                           // Esta funcion abre la ventana
  digitalWrite(Abrir, HIGH);            // Se abre la ventana
  digitalWrite(Cerrar, LOW);            // Garantiza que no se va a cerrar 
  Serial.print(" º ABRIENDO º");
  delay(9000);                          // Se espera a que el sensor de completamente cerrado se libere

  do {                                  // Este ciclo sucede hata que se termine de abrir la ventana
    digitalWrite(Abrir, HIGH);
    digitalWrite(Cerrar, LOW);
    Serial.println(" º ABRIENDO º");
    ValorSv = digitalRead(Sv);         // Lectura de la EXOR
  } while (ValorSv);                   // Cuando los dos sensores de la ventana estan libres la EXOR indica que la ventana esta completamente abierta

  EdoVen = 1;                          // Estado de ventana indica abierta = 1
  digitalWrite(Abrir, LOW);            // Se apaga la apertura de la ventana
  digitalWrite(Cerrar, LOW);           // Se garantiza que no se cierre 
  Serial.print(" º DESACTIVADO º");
}

//--------------------------- Cerrar Ventana  -------------------------------
void Close() {                           // Esta funcion cierra la ventana
  digitalWrite(Abrir, LOW);              // Garantiza que no se va a abrir
  digitalWrite(Cerrar, HIGH);            // Se cierra la ventana
  Serial.print(" º CERRANDO º");
       ValorSv = digitalRead(Sv);        // Lectura de la compueta EXOR
       Serial.println(ValorSv);
  delay(5000);                           // Se espera a que el sensor de completamente abierta se obstruya 

  do {                                   // Este ciclo sucede hata que se termine de cerrar la ventana
    digitalWrite(Abrir, LOW);            // Se garantiza que no se abra la Ventana
    digitalWrite(Cerrar, HIGH);          // Se cierra la Ventana
    Serial.println(" º CERRANDO º");
    ValorSv = digitalRead(Sv);           // Lectura de la compuerta EXOR
    if (!ValorSv) {                      // Una ves que el sensor de completamente cerrado se activa continua cerrando
      Serial.print("En espera");
      delay(6500);                       //Tiempo para que termine de cerrar la ventana (el sensor esta antes del cierre total)
    }
  } while (ValorSv);                     // Termina el ciclo de cierre de ventana 

  EdoVen = 0;                            // Estado de ventana cerrado = 0
  digitalWrite(Abrir, LOW);              // Permanece apagodo la apertura de la ventana
  digitalWrite(Cerrar, LOW);             // Se apaga el cierre de la ventana
  Serial.print(" º DESACTIVADO º");

}
