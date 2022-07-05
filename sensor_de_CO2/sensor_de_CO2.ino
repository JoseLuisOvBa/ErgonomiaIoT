// Bibliotecas
#include <WiFi.h>           // para el control de WiFi
#include <PubSubClient.h>   // para conexión MQTT

// Constantes para objetos
#define MQ135pin (0) 

// Objetos
//DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;             // Este objeto maneja los datos de conexion WiFi
PubSubClient client(espClient);   // Este objeto maneja los datos de conexion al broker

// Constantes 
const int LED3 = 14;                          // Conexión WiFi
const char* ssid = "HUAWEI Y8s";          // Nombre de la red WiFi Cel de David
const char* password = "40cff5815000";        // Contraseña de la red Wifi Cel de David
const char* mqtt_server = "192.168.43.44";   // Red local (ifconfig en terminal) -> broker MQTT Con Cel de David
IPAddress server(192,168,43,44);             // Red local (ifconfig en terminal) -> broker MQTT Con Cel de David

// Variables
float co2value; //variable para guardar el valor analógico del sensor
int relayPin=8;
long timeNow5, timeLast5; // Vars de control de tiempo no-bloq en envío de mens MQTT
int wait5 = 5000;         // Espera cada 5 s para envío de mensajes MQTT





// ----------------- Función SETUP (Se ejecuta sólo una vez al energizar) ----------------- //
void setup(){
  Serial.begin(115200); // Inicializamos el puerto serial a 9600

 // Configuración de pines - Leds
   pinMode (LED3,OUTPUT);
   
  // Conexión a la red WiFi

  Serial.println();
  Serial.print("Conectar a ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);             // Esta es la función que realiza la conexión a WiFi
 
  while (WiFi.status() != WL_CONNECTED) { // Este bucle espera a que se realice la conexión a WiFi
    digitalWrite (LED3,LOW);              // Apagar LED de conexión WiFi
    delay(500);                             // Esperar 5 ms a la conexión (espera bloqueante)
    digitalWrite (LED3,HIGH);             // Encender LED de conexión WiFi
    Serial.print(".");                    // Indicador de progreso
    delay(500);                           // Esperar 0.5 s a la conexión (espera bloqueante)
  }
  
// Cuando se haya logrado la conexión:
  Serial.println();
  Serial.println("WiFi conectado");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());

  // Como se logró la conexión, encender LED de conexión
  if (WiFi.status () > 0){
    digitalWrite (LED3,HIGH);       // Encender LED de conexión WiFi
  }
  
  // Conexión al Broker

  delay(1000);                      // Espera 1 s antes de iniciar la comunicación con el broker
  client.setServer(server,1883);    // Conectarse a la IP del broker en el puerto indicado
//  client.setCallback(callback);   // Activ func CallBack, permite recibir mensjs MQTT y ejec funcs a partir de ellos
  delay(1500);                      // Esta espera es preventiva, espera a la conexión para no perder información


// PROGRAMA DEL SENSOR CO2
   Serial.println("El sensor de gas se esta pre-calentando");
  delay(5000); // Espera a que el sensor se caliente durante 20 segundos

  pinMode (relayPin, OUTPUT);

}// Fin de void setup


void reconnect() {
  while (!client.connected()) {                  // Mientras NO haya conexión ...
  Serial.print("Tratando de conectarse...");
  if (client.connect("ESP32CAMClient")) {      // Si se logró la conexión ...
  Serial.println("Conectado");
  client.subscribe("esp32/output");          // Esta función realiza la suscripción al tema (¿se necesita aquí? creo que no)
   }                                            
   else {                                       // Si no se logró la conexión ...
    Serial.print("Conexion fallida, Error rc=");
    Serial.print(client.state());              // Muestra el código de error
    Serial.println(" Volviendo a intentar en 5 segundos");
    delay(5000);                               // Espera de 5 segundos bloqueante
    Serial.println (client.connected ());      // Muestra estatus de conexión
  }                                            
 }                                              
}

void lecturaco2() {
    timeNow5 = millis(); // Control de tiempo para esperas no bloqueantes
    
    if (timeNow5 - timeLast5 > wait5) { 
      co2value = analogRead(MQ135pin); // lectura de la entrada analogica "A0""
      Serial.print("Valor detectado por el sensor: ");
      Serial.print(co2value);
      
      if(co2value > 100){
        Serial.print(" | Se ha detectado gas!");
        digitalWrite (relayPin, HIGH);
        delay (2000);
        }  
      
  Serial.println("");
  delay(2000); // espera por 2 segundos para la siguiente lectura
  digitalWrite (relayPin, LOW);
  
  //delay (2000);
  //delay (2000);

//--------------------------- publicación nivel de CO2 ------------------------ //
    char co2String[8];              // Arreglo de caracteres a enviar por MQTT (long del msj: 8 caracteres)

    dtostrf(co2value, 1, 2, co2String);    // Función nativa de leguaje AVR; convierte un arreglo de caracs en una var String

    Serial.print("CO2 PPM: ");
    Serial.print(co2String);
    
    client.publish("codigoIoT/SIC/venCO2", co2String);   // Envía Temperatura por MQTT, especifica el tema y el valor

    timeLast5 = timeNow5;            // Actualización de seguimiento de tiempo

  }

}
// ----------------- Función SETUP (Se ejecuta sólo una vez al energizar) ----------------- //

void loop(){

   if (!client.connected()) {   // Si NO hay conexión al broker ...
    reconnect();               // Ejecuta el intento de reconexión
  }                            

  client.loop();               // Es para mantener la comunicación con el broker
  lecturaco2 ();
}
  // ----------------------------------------SENSOR DE CO2 - PRUEBA 04/07/22-----------------------------
  



// -------------------------------- Función para Reconectarse -------------------------------- //
       
