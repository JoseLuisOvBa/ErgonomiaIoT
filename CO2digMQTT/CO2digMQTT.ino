
/*Monitoreo de CO2, Presencia y ventilación
   Envio de datos de los sensores MQ135 (CO2) y Pir Hcsr501 (presencia) por MQTT
   por: Jose Luis Oviedo Barriga, David Garcia Sarmiento, Erica 
   Fecha: 4 de julio de 2022

   Este programa lee los sensores MQ135 (CO2) y Pir Hcsr501 (presencia) por MQTT

              CO2                        PRESENCIA
   ESP32      MQ135PinAtras      ESP32      Pir Hcsr501   ESP32      Ventilador
   I012 ------    1 Datos        GND ------ 1 GND         I015 ------1 Datos
   GND -------    3 GND          I013 ------2 Datos
   VCC -------    4 Vin          VCC -------3 Vin

   Asignacion de pines DISPONIBLES ESP32CAM
   IO4    Abrir     En 1 Abrir ventana
   IO2    Cerrar    En 1 Cerrar ventana
   IO14   Sv        Sensor de ventana compeltamente Abierta
   IO15   Venti     Rele Ventilador
   IO12   CO2       Sensor CO2
   IO13   PIR       Sensor Presencia
   IO33   LedInt     LEDinterno (Acuse de Presencia)

*/

// Bibliotecas************************************************************************
#include <WiFi.h>  // Biblioteca para el control de WiFi
#include <PubSubClient.h> //Biblioteca para conexion MQTT

//Datos de WiFi**********************************************************************

const char* ssid = "HUAWEI Y8s";                  // Nombre de la red WiFi
const char* password = "40cff5815000";          // Contraseña de la red Wifi
const char* mqtt_server = "192.168.43.44";   // Red local (ifconfig en terminal) -> broker MQTT
IPAddress server(192,168,43,44);              // Red local (ifconfig en terminal) -> broker MQTT
/*
const char* ssid = "OviRab";                  // Nombre de la red WiFi
const char* password = "99121976";            // Contraseña de la red Wifi
const char* mqtt_server = "192.168.1.104";   // Red local (ifconfig en terminal) -> broker MQTT
IPAddress server(192,168,1,104);              // Red local (ifconfig en terminal) -> broker MQTT
*/

// Objetos***************************************************************************

WiFiClient espClient; // Este objeto maneja los datos de conexion WiFi
PubSubClient client(espClient); // Este objeto maneja los datos de conexion al broker


// Constantes*************************************************************************
int ValorCO2 = 0;   // variable para guardar el valor analógico del sensor
int ValorPIR = 0;     // variable para guardar el valor digital del sensor de presencia
int ValorSv = 0;     // variable para guardar el valor digital del sensor ventana activa
int EdoVen = 0;     //Condicion inicial Ventana cerrada  EdoVen=1 Abierta // EdoVen=0 Cerrada

// Variables**************************************************************************
long timeNow, timeLast; // Variables de control de tiempo no bloqueante
int data = 0; // Contador
int wait = 5000;  // Indica la espera cada 5 segundos para envío de mensajes MQTT

// Definición de objetos**************************************************************

#define LedInt 33
#define PIR 13
#define Sv 14
#define Venti 15
#define CO2 12
#define Abrir 4
#define Cerrar 2

// Condiciones iniciales - Se ejecuta sólo una vez al energizar***********************

void setup() {
  Serial.begin (115200);

  pinMode(Abrir, OUTPUT);      //Salida del ESP32 de apertura de ventana y entrada 1 de puente H
  pinMode(Cerrar, OUTPUT);     //Salida del ESP32 de cierre de ventana y entrada 2 de puente H
  pinMode(Venti, OUTPUT);     //Salida de encendido de ventiladaro
  pinMode(LedInt, OUTPUT);     //Salida al LED interno del ESP32
  pinMode(Sv, INPUT);         //Entrada del sensor ventana abierta
  pinMode(PIR, INPUT);         //Entrada del sensor de presencia
  pinMode(CO2, INPUT);         //Entrada del sensor de CO2
  
  digitalWrite(Abrir, LOW);
  digitalWrite(Cerrar, LOW);
  digitalWrite(Venti, LOW);
  digitalWrite(LedInt, HIGH);

    Serial.println();
    Serial.println();
    Serial.print("Conectar a ");
    Serial.println(ssid);
 
   WiFi.begin(ssid, password); // Esta es la función que realiz la conexión a WiFi
  

   while (WiFi.status() != WL_CONNECTED) { // Este bucle espera a que se realice la conexión
      digitalWrite (LedInt, HIGH);
      delay(500); //dado que es de suma importancia esperar a la conexión, debe usarse espera bloqueante
      digitalWrite (LedInt, LOW);
      Serial.print(".");  // Indicador de progreso
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
    client.setServer(server, 1883); // Conectarse a la IP del broker en el puerto indicado
    client.setCallback(callback); // Activar función de CallBack, permite recibir mensajes MQTT y ejecutar funciones a partir de ellos
    delay(1500);  // Esta espera es preventiva, espera a la conexión para no perder información

  Serial.println("El sensor de gas se esta pre-calentando");
  delay(5000); // Espera a que el sensor se caliente durante 20 segundos


  timeLast = millis (); // Inicia el control de tiempo

}// Fin de void setup*****************************************************************

void loop() {    //VOID LOOP**********************************************************///////////////////
  
  if (!client.connected()) {   // Si NO hay conexión al broker ...
    reconnect();               // Ejecuta el intento de reconexión
  }   
   
  client.loop();               // Es para mantener la comunicación con el broker

  timeNow = millis(); // Control de tiempo para esperas no bloqueantes
  if (timeNow - timeLast > wait) { // Manda un mensaje por MQTT cada cinco segundos
    timeLast = timeNow; // Actualización de seguimiento de tiempo



//----------------------- PRESENCIA----------------------------

  ValorPIR = digitalRead(PIR);   //Lectura del Sensor PIR que se guarda en ValorPIR

        char dataString[8]; // Define una arreglo de caracteres para enviarlos por MQTT, especifica la longitud del mensaje en 8 caracteres
        dtostrf(ValorPIR, 1, 2, dataString);  // Esta es una función nativa de leguaje AVR que convierte un arreglo de caracteres en una variable String
        Serial.print("Presencia: "); // Se imprime en monitor solo para poder visualizar que el evento sucede
        Serial.println(dataString);
        client.publish("sic/capston16/presencia", dataString); // Esta es la función que envía los datos por MQTT, especifica el tema y el valor


//if-else comentado por que se raliza desde el callback
/*  
  if (ValorPIR == HIGH) {          //Pregunta si esta en alta
//   Serial.print(" | PRESENCIA | ");
    digitalWrite(LedInt, 0);       //de ser asi lo enciede
  } else {                      // si esta en bajo
    digitalWrite(LedInt, 1);    //Pemanece apagado
//    Serial.print(" | AUSENCIA | ");
  }*/  //if-else comentado

//----------------------- CO2---------------------------

 ValorCO2 = digitalRead(CO2); // lectura de la entrada digital
  
  Serial.print("CO2 (0)si / (1)no:  ");   //prueba
  Serial.println(ValorCO2);  //prueba
  delay(500);                                   // wait 100ms for next reading

   
        dtostrf(ValorCO2, 1, 2, dataString);
        Serial.print("CO2 str: "); // Se imprime en monitor solo para poder visualizar que el evento sucede
        Serial.println(dataString);
        client.publish("sic/capston16/CO2", dataString);
        

  

  if (ValorCO2 == 0)  // Limite 540ppm de CO2
  {
    Serial.print("  ¡Se ha detectado CO2!  ");
    digitalWrite(Venti, HIGH); //Enciende Ventilador
    if (EdoVen == 0) {
      Open(); //Abre Ventana
      EdoVen==1;
    }
    delay(1000);
  }
  else {
    Serial.print("");
    digitalWrite (Venti, LOW);
    if (EdoVen == 1) {
      Close(); //Cierra Ventana
      EdoVen==0;
    }
    delay(1000);
  }

  Serial.println("");

        delay (100);
        dtostrf(EdoVen, 1, 2, dataString);
        Serial.print("Ventana(1-Abierta/0-Cerrada): "); // Se imprime en monitor solo para poder visualizar que el evento sucede
        Serial.println(dataString);
        client.publish("sic/capston16/EdoVen", dataString);


  }// fin del if (timeNow - timeLast > wait)

}// Fin de void loop*****************************************************************///////////////////






// Funcioes del usuario
//------------------------- Abrir Ventana  -----------------------------
void Open() {                           //Esta funcion abre la ventana
  digitalWrite(Abrir, HIGH);
  digitalWrite(Cerrar, LOW);
  Serial.print(" º ABRIENDO º");
  delay(9000);

  do {
    digitalWrite(Abrir, HIGH);
    digitalWrite(Cerrar, LOW);
    Serial.println(" º ABRIENDO º");
    ValorSv = digitalRead(Sv);
  } while (ValorSv);

  EdoVen = 1;
  digitalWrite(Abrir, LOW);
  digitalWrite(Cerrar, LOW);
  Serial.print(" º DESACTIVADO º");
}

//--------------------------- Cerrar Ventana  -------------------------------
void Close() {                           //Esta funcion cierra la ventana
  digitalWrite(Abrir, LOW);
  digitalWrite(Cerrar, HIGH);
  Serial.print(" º CERRANDO º");
       ValorSv = digitalRead(Sv);
       Serial.println(ValorSv);
  delay(5000);

  do {
    digitalWrite(Abrir, LOW);
    digitalWrite(Cerrar, HIGH);
    Serial.println(" º CERRANDO º");
    ValorSv = digitalRead(Sv);
    if (!ValorSv) {
      Serial.print("En espera");
      delay(6500); //Tiempo para que termine de cerrar la ventana (el sensor esta antes del cierre total)
    }
  } while (ValorSv);

  EdoVen = 0;
  digitalWrite(Abrir, LOW);
  digitalWrite(Cerrar, LOW);
  Serial.print(" º DESACTIVADO º");

}




//--------------------------- callback --------------------------- 
// Esta función permite tomar acciones en caso de que se reciba un mensaje correspondiente a un tema al cual se hará una suscripción
void callback(char* topic, byte* message, unsigned int length) {

    // Indicar por serial que llegó un mensaje
    Serial.print("Llegó un mensaje en el tema: ");
    Serial.print(topic);

    // Concatenar los mensajes recibidos para conformarlos como una varialbe String
    String messageTemp; // Se declara la variable en la cual se generará el mensaje completo  
    for (int i = 0; i < length; i++) {  // Se imprime y concatena el mensaje
      Serial.print((char)message[i]);
       messageTemp += (char)message[i];
    }

    // Se comprueba que el mensaje se haya concatenado correctamente
    Serial.println();
    Serial.print ("Mensaje concatenado en una sola variable: ");
    Serial.println (messageTemp);

    // En esta parte puedes agregar las funciones que requieras para actuar segun lo necesites al recibir un mensaje MQTT
    // Ejemplo, en caso de recibir el mensaje true - false, se cambiará el estado del led soldado en la placa.
    // El ESP323CAM está suscrito al tema esp/output
    if (String(topic) == "sic/capston16/presencia") {  // En caso de recibirse mensaje en el tema esp32/output
      if(messageTemp == "true"){
        Serial.println("Led encendido");
        digitalWrite(LedInt, LOW); //enciende led interno
      }// fin del if (String(topic) == "sic/capston16/presencia"")
      else if(messageTemp == "false"){
        Serial.println("Led apagado");
        digitalWrite(LedInt, HIGH); //apaga led interno
        }// fin del else if(messageTemp == "false")
    }// fin del if (String(topic) == "sic/capston16/presencia")
}// fin del void callback*************************************************************

//--------------------------- Función para reconectarse  --------------------------- 

void reconnect() {
  // Bucle hasta lograr conexión
  while (!client.connected()) { // Pregunta si hay conexión
    Serial.print("Tratando de contectarse...");
    // Intentar reconexión
    if (client.connect("ESP32CAMClient")) { //Pregunta por el resultado del intento de conexión
      Serial.println("Conectado");
      client.subscribe("sic/capston16/presencia"); // Esta función realiza la suscripción al tema
    }// fin del  if (client.connect("ESP32CAMClient"))
    else {  //en caso de que la conexión no se logre
      Serial.print("Conexion fallida, Error rc=");
      Serial.print(client.state()); // Muestra el codigo de error
      Serial.println(" Volviendo a intentar en 5 segundos");
      // Espera de 5 segundos bloqueante
      delay(5000);
      Serial.println (client.connected ()); // Muestra estatus de conexión
    }// fin del else
  }// fin del bucle while (!client.connected())
}// fin de void reconnect(
