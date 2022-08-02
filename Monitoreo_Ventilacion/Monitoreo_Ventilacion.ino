
/*Monitoreo de CO2, Presencia y ventilación
   Envio de datos de los sensores MQ135 (CO2) y Pir Hcsr501 (presencia) por MQTT
   por: Jose Luis Oviedo Barriga, David Garcia Sarmiento, Erika Barojas Payan 
   Fecha: 15 de julio de 2022

   Este programa realiza lo siguiente:
   
   1.- Controla la buena calidad del aire al evitar la alta concentración de CO2 dentro de un espacio cerrado monitoreando 
       este comportamiento por MQTT. 
       Para lograrlo se monitore la concentración de dicho gas y al superar el nivel establecido se enciende un ventilador y
       se abre una ventana. Cuando los niveles de CO2 retornan a los niveles adecuados, el ventilador se apaga y la ventana se cierra. 
       La ventana cuenta con dos sensores en sus extremos y su señal llega a una compuerta EXOR, dicha salida en el programa es 
       denominada como SV.
   2.- Monitorea la presencia de personas. 
       Se cuenta con un sensor de presencia PIR el cual detecta la presencia, una vez que detecta la presencias manda una señal 
       a través de MQTT y desde MQTT se enciende una alama visual (el LED interno de ESP32).  
       También se puede apagar y encender este LED de forma manual desde MQTT. 


           CO2                        PRESENCIA                  Relevador 
   ESP32            MQ135        ESP32      PIR Hcsr501      ESP32      Rele de Ventilador
   I012 ------    1 Datos        GND ------ 1 GND            I015 ------1 Datos
   GND -------    3 GND          I013 ------2 Datos
   VCC -------    4 Vin          VCC -------3 Vin


   Asignacion de pines DISPONIBLES ESP32CAM
   
   IO4    Abrir     En 1 para Abrir ventana
   IO2    Cerrar    En 1 para Cerrar ventana
   IO14   Sv        Salida de EXOR (En 0 si ventana compeltamente Abierta/Cerrada)
   IO15   Venti     Rele Ventilador
   IO12   CO2       Sensor CO2
   IO13   PIR       Sensor Presencia
   IO33   LedInt    LEDinterno (Acuse de Presencia desde MQTT)

*/

// Bibliotecas************************************************************************
#include <WiFi.h>          // Biblioteca para el control de WiFi
#include <PubSubClient.h>  //Biblioteca para conexion MQTT

//Datos de WiFi**********************************************************************

const char* ssid = "HUAWEI Y8s";              // Nombre de la red WiFi
const char* password = "40cff5815000";        // Contraseña de la red Wifi
const char* mqtt_server = "192.168.43.44";    // Red local (ifconfig en terminal) -> broker MQTT
IPAddress server(192,168,43,44);              // Red local (ifconfig en terminal) -> broker MQTT


// Objetos***************************************************************************

WiFiClient espClient;            // Este objeto maneja los datos de conexion WiFi
PubSubClient client(espClient);  // Este objeto maneja los datos de conexion al broker


// Constantes*************************************************************************

int ValorCO2 = 0;    // Variable para guardar el valor digital del sensor
int ValorPIR = 0;    // Variable para guardar el valor digital del sensor de presencia
int ValorSv = 0;     // Variable para guardar el valor digital del sensor ventana activa
int EdoVen = 0;      // Condicion inicial Ventana cerrada.  EdoVen=1 Abierta // EdoVen=0 Cerrada
int EdoVenti=0;      // Estado del ventilador. EdoVenti=0 Apagador // EdoVenti=1 Encendido

// Variables**************************************************************************

long timeNow, timeLast;    // Variables de control de tiempo no bloqueante
int data = 0;              // Contador
int wait = 5000;           // Indica la espera cada 5 segundos para envío de mensajes MQTT

// Definición de objetos**************************************************************

#define LedInt 33    // LEDinterno (Acuse de Presencia desde MQTT)
#define PIR 13       // Sensor Presencia
#define Sv 14        // Salida de EXOR (En 0 si ventana compeltamente Abierta/Cerrada)
#define Venti 15     // Rele Ventilador
#define CO2 12       // Sensor CO2
#define Abrir 4      // En 1 para Abrir ventana
#define Cerrar 2     // En 1 para Cerrar ventana

// Condiciones iniciales - Se ejecuta sólo una vez al energizar***********************

void setup() {
  Serial.begin (115200);
  // WiFi.mode(WIFI_STA);
  
  pinMode(Abrir, OUTPUT);      // Salida del ESP32 de apertura de ventana y entrada 1 de puente H
  pinMode(Cerrar, OUTPUT);     // Salida del ESP32 de cierre de ventana y entrada 2 de puente H
  pinMode(Venti, OUTPUT);      // Salida de encendido de ventiladaro
  pinMode(LedInt, OUTPUT);     // Salida al LED interno del ESP32
  pinMode(Sv, INPUT);          // Entrada de la EXOR (en 0 si Ventana completamente Abiera/Cerrada)
  pinMode(PIR, INPUT);         // Entrada del sensor de presencia
  pinMode(CO2, INPUT);         // Entrada del sensor de CO2
  
  digitalWrite(Abrir, LOW);
  digitalWrite(Cerrar, LOW);
  digitalWrite(Venti, LOW);
  digitalWrite(LedInt, HIGH);

    Serial.println();
    Serial.println();
    Serial.print("Conectar a ");
    Serial.println(ssid);
 
 
   WiFi.begin(ssid, password); // Esta es la función que realiz la conexión a WiFi
  

   while (WiFi.status() != WL_CONNECTED) {          // Este bucle espera a que se realice la conexión
      digitalWrite (LedInt, HIGH);
      delay(500);                                   //dado que es de suma importancia esperar a la conexión, debe usarse espera bloqueante
      digitalWrite (LedInt, LOW);
      Serial.print(".");                            // Indicador de progreso
      delay (5);
    }
   
    // Cuando se haya logrado la conexión, el programa avanzará, por lo tanto, puede informarse lo siguiente
    Serial.println();
    Serial.println("WiFi conectado");
    Serial.println("Direccion IP: ");
    Serial.println(WiFi.localIP());

    // Si se logro la conexión, encender led
    if (WiFi.status () > 0){
    digitalWrite (LedInt, LOW);
    }
  
    delay (1000); // Esta espera es solo una formalidad antes de iniciar la comunicación con el broker

    // Conexión con el broker MQTT
    client.setServer(server, 1883);                  // Conectarse a la IP del broker en el puerto indicado
    client.setCallback(callback);                    // Activar función de CallBack, permite recibir mensajes MQTT y ejecutar funciones a partir de ellos
    delay(1500);                                     // Esta espera es preventiva, espera a la conexión para no perder información

  Serial.println("El sensor de gas se esta pre-calentando");
  delay(5000);                                      // Espera a que el sensor se caliente


  timeLast = millis ();                              // Inicia el control de tiempo

}// Fin de void setup*****************************************************************

void loop() { //VOID LOOP**********************************************************///////////////////
 
  if (!client.connected()) {        // Si NO hay conexión al broker ...
    reconnect();                    // Ejecuta el intento de reconexión
  }   
   
  client.loop();                    // Es para mantener la comunicación con el broker

  timeNow = millis();               // Control de tiempo para esperas no bloqueantes
  if (timeNow - timeLast > wait) {  // Manda un mensaje por MQTT cada cinco segundos
    timeLast = timeNow;             // Actualización de seguimiento de tiempo



//----------------------- PRESENCIA----------------------------

  ValorPIR = digitalRead(PIR);                 // Lectura del Sensor PIR que se guarda en ValorPIR

        char dataString[8];                    // Define un arreglo de caracteres para enviarlos por MQTT, especifica la longitud del mensaje en 8 caracteres
        dtostrf(ValorPIR, 1, 2, dataString);   // Esta es una función nativa de leguaje AVR que convierte un arreglo de caracteres en una variable String
        Serial.print("Presencia ---> ");       // Se imprime en monitor solo para poder visualizar que el evento sucede
        Serial.println(dataString);
        client.publish("sic/capston16/presencia", dataString); // Envía los datos por MQTT, especifica el tema y el valor


//----------------------- CO2---------------------------

 ValorCO2 = digitalRead(CO2);                            // Lectura CO2 en la entrada digital ValorCO2=0 si hay gas // ValorCO2=1 si NO hay gas
 
 delay(100);                                             // Sepera 100ms 
   
        dtostrf(!ValorCO2, 1, 2, dataString);            // Se convierte el dato de int str y se niega el valor de ValorCO2 para !ValorCO2=1 si hay gas // !ValorCO2=0 si NO hay gas
        Serial.print("CO2 ---> ");                       // Se imprime en monitor solo para poder visualizar que el evento sucede
        Serial.println(dataString);
        client.publish("sic/capston16/CO2", dataString); // Envía los datos por MQTT, especifica el tema y el valor
        

  

  if (ValorCO2 == 0)                                      // Si ValorCO==0 entonces hay de CO2
  {
    digitalWrite(Venti, HIGH);                            // Enciende Ventilador
    EdoVenti=1;                                           // Estado de Ventilador encendido = 1
        
        delay (100);
        dtostrf(EdoVenti, 1, 2, dataString);                  //Se convierte el dato de int str y se niega el valor de ValorCO2 para !ValorCO2=1 si hay gas // !ValorCO2=0 si NO hay gas
        Serial.print("Estado de ventilador: ---> ");          // Se imprime en monitor solo para poder visualizar que el evento sucede
        Serial.println(dataString);
        client.publish("sic/capston16/EdoVenti", dataString); // Envía los datos por MQTT, especifica el tema y el valor del estado del Ventilador
    
    if (EdoVen == 0) {        // Si la ventana esta cerrada 
      Open();                 // Abre Ventana
      EdoVen==1;              // Cambia el estado de la ventana a abierta = 1  
    }
    delay(1000);
  }
  else {                                                     // Si el livel de gas es el adecuado
   digitalWrite (Venti, LOW);                                // Se apaga el ventilador
    EdoVenti=0;                                              // Estado de ventilador apagado = 0
        delay (100);
        dtostrf(EdoVenti, 1, 2, dataString);                 //Se convierte el dato de int str
        Serial.print("Estado de ventilador: ---> ");         // Se imprime en monitor solo para poder visualizar que el evento sucede
        Serial.println(dataString);
        client.publish("sic/capston16/EdoVenti", dataString);// Envía los datos por MQTT, especifica el tema y el valor del estado del Ventilador
        
    if (EdoVen == 1) {                                       // Si la ventana esta abierta
      Close();                                               // Cierra Ventana               
      EdoVen==0;                                             // Estado de ventana cerrada = 0
    }
    delay(1000);
  }

  Serial.println("");

        delay (100);
        dtostrf(EdoVen, 1, 2, dataString);                  //Se convierte el dato de int str
        Serial.print("Ventana(1-Abierta/0-Cerrada) ---> "); // Se imprime en monitor solo para poder visualizar que el evento sucede
        Serial.println(dataString);
        client.publish("sic/capston16/EdoVen", dataString); // Envía los datos por MQTT, especifica el tema y el valor del estado de la ventana





  }// fin del if (timeNow - timeLast > wait)

}// Fin de void loop*****************************************************************///////////////////






// Funcioes del usuario
//------------------------- Abrir Ventana  -----------------------------
void Open() {                           // Esta funcion abre la ventana
  digitalWrite(Abrir, HIGH);            // Se abre la ventana
  digitalWrite(Cerrar, LOW);            // Garantiza que no se va a cerrar 
  Serial.print(" ABRIENDO >>");
  delay(9000);                          // Se espera a que el sensor de completamente cerrado se libere

  do {                                  // Este ciclo sucede hata que se termine de abrir la ventana
    digitalWrite(Abrir, HIGH);        
    digitalWrite(Cerrar, LOW);
    Serial.println(" ABRIENDO >>");
    ValorSv = digitalRead(Sv);         // Lectura de la EXOR
  } while (ValorSv);                   // Cuando los dos sensores de la ventana estan libres la exor indica que la ventana esta completamente abierta

  EdoVen = 1;                          // Estado de ventana indica abierta = 1
  digitalWrite(Abrir, LOW);            // Se apaga la apertura de la ventana
  digitalWrite(Cerrar, LOW);           // Se garantiza que no se cierre 
  Serial.print("< Abierto >");
}

//--------------------------- Cerrar Ventana  -------------------------------
void Close() {                           //Esta funcion cierra la ventana
  digitalWrite(Abrir, LOW);              // Garantiza que no se va a abrir
  digitalWrite(Cerrar, HIGH);            // Se cierra la ventana
  Serial.print("<< CERRANDO ");
       ValorSv = digitalRead(Sv);
       Serial.println(ValorSv);
  delay(5000);                           // Se espera a que el sensor de completamente abierta se obstruya 

  do {                                   // Este ciclo sucede hata que se termine de cerrar la ventana
    digitalWrite(Abrir, LOW);        
    digitalWrite(Cerrar, HIGH);
    Serial.println("<< CERRANDO");
    ValorSv = digitalRead(Sv);           // Lectura de la compuerta EXOR
    if (!ValorSv) {                      // Una ves que el sensor de completamente cerrado se activa continua cerrando
      Serial.print("En espera");
      delay(6500);                       //Tiempo para que termine de cerrar la ventana (el sensor esta antes del cierre total)
    }
  } while (ValorSv);                     // Termina el ciclo de cierre de ventana 

  EdoVen = 0;                            // Estado de ventana cerrado = 0
  digitalWrite(Abrir, LOW);              // Permanece apagodo la apertura de la ventana
  digitalWrite(Cerrar, LOW);             // Se apaga el cierre de la ventana
  Serial.print("> Cerrado <");

}




//--------------------------- callback --------------------------- 
// Esta función permite tomar acciones en caso de que se reciba un mensaje correspondiente a un tema al cual se hará una suscripción
void callback(char* topic, byte* message, unsigned int length) {

    // Indicar por serial que llegó un mensaje
    Serial.print("Llegó un mensaje en el tema: ");
    Serial.print(topic);

    // Concatenar los mensajes recibidos para conformarlos como una varialbe String
    String messageTemp;                        // Se declara la variable en la cual se generará el mensaje completo  
    for (int i = 0; i < length; i++) {         // Se imprime y concatena el mensaje
      Serial.print((char)message[i]);
       messageTemp += (char)message[i];
    }

    // Se comprueba que el mensaje se haya concatenado correctamente
    Serial.println();
    Serial.print ("Mensaje concatenado en una sola variable: ");
    Serial.println (messageTemp);

    // En esta parte puedes agregar las funciones que requieras para actuar segun lo necesites al recibir un mensaje MQTT
    // Ejemplo, en caso de recibir el mensaje true - false, se cambiará el estado del led soldado en la placa.
    // El ESP323CAM está suscrito al tema sic/capston16/led
 
    
    
    if (String(topic) == "sic/capston16/led") {                 // En caso de recibirse mensaje en el tema sic/capston16/led           CALLBACK LED
      if(messageTemp == "true"){
        Serial.println("Led encendido");
        digitalWrite(LedInt, LOW);                              // Enciende led interno
      }// fin del if (String(topic) == "sic/capston16/led")
      else if(messageTemp == "false"){
        Serial.println("Led apagado");
        digitalWrite(LedInt, HIGH);                             // Apaga led interno
        }
    }

  
}// fin del void callback*************************************************************

//--------------------------- Función para reconectarse  --------------------------- 

void reconnect() {
  // Bucle hasta lograr conexión
  while (!client.connected()) {                        // Pregunta si hay conexión
    Serial.print("Tratando de contectarse...");
    // Intentar reconexión
   //  WiFi.mode(WIFI_STA);
    if (client.connect("ESP32CAMClient")) {            //Pregunta por el resultado del intento de conexión
      Serial.println("Conectado");
            client.subscribe("sic/capston16/led"); 
              } // Fin del  if (client.connect("ESP32CAMClient"))
    else {                                             // En caso de que la conexión no se logre
      Serial.print("Conexion fallida, Error rc=");
      Serial.print(client.state());                    // Muestra el codigo de error
      Serial.println(" Volviendo a intentar en 5 segundos");
      // Espera de 5 segundos bloqueante
      delay(5000);
      Serial.println (client.connected ());            // Muestra estatus de conexión
    }// fin del else
  }// fin del bucle while (!client.connected())
}// fin de void reconnect(
