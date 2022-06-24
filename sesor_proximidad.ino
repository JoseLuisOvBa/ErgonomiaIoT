
/*
 * Proma de sensores de presencia para la ventana y establecer si esta completamente abierta o completamente cerrada
 * Por: J. David Garcia Sarmiento
 * Fecha: 24 de junio 2022
 * Esto programa es parte del programa principal del proyecto capstone de Monitoreo de Ventilacion
 */

 // Bibliotecas

// Constantes 
const int LED=13;            //Led de prueba 
const int S1=6;              //Sensor de presencia 1 indicador de movimento de la ventana, y si esta abierto o cerrada la ventana
const int S2=2;               //Sensor de presencia 1 indicador de movimento de la ventana, y si esta abierto o cerrada la ventana
  
// Variables
int val1;                   //Variable de dato digital del sensor1 (1 o 0)
int val2;                  //Variable de dato digital del sensor1 (1 o 0)

// Definición de objetos


// Condiciones iniciales - Se ejecuta sólo una vez al energizar
void setup(){              // Inicio de void setup ()
   
  Serial.begin (9600);    //comunicacion con el monitor serial
  
  pinMode(LED,OUTPUT);    //Parametros iniciales, establece el LED como salida
  pinMode(S1,INPUT);      //Indicador de entrada de dato sensor1
  pinMode(S2, INPUT);     //Indicador de entrada de dato sensor2
}// Fin de void setup

// Cuerpo del programa - Se ejecuta constamente
void loop(){              // Inicio de void loop
  val1=digitalRead(S1);   //Almacena el dato registrado por el sensor1
  val2=digitalRead(S2);   //Almacena el dato registrado por el sensor2

  Serial.print(val1);     //Muestra el valor en el monitor serial almanecado en la variable val1
  Serial.print(val2);     //Muestra el valor en el monitor serial almanecado en la variable val2
  
  if  (!val1 && !val2){   // S1 y S2 = 1 led se apaga y se indica que la ventana esta cerrada
    digitalWrite(LED,LOW);  //Manda la orden al LED que se apague
    Serial.println();       //Salto de linea
    Serial.print("COMPLETAMENTE CERRADO");    //Muestra este mensaje en el monitor serial 
    }   
    
     else {               // S1 y S2 diferente de 1 led se prende y se indica que la ventana esta abierta
      digitalWrite(LED,HIGH);     //Manda la orden al LED que se apague
      Serial.println();           //Salto de linea
      Serial.print("COMPLETAMENTE ABIERTO"); //Muestra este mensaje en el monitor serial 
     }
}// Fin de void loop

// Funcioes del usuario
