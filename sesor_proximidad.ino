

const int LED=13;
const int S1=6;
const int S2=2;
int val1;
int val2;

void setup(){

  Serial.begin (9600);
  
  pinMode(LED,OUTPUT);
  pinMode(S1,INPUT);
  pinMode(S2, INPUT);
}

void loop(){
  val1=digitalRead(S1);
  val2=digitalRead(S2);

  Serial.print(val1);
  Serial.print(val2);
  
  if  (!val1 && !val2){ // S1 y S2 = 1 led se apaga y se indica que la ventana esta cerrada
    digitalWrite(LED,LOW);
    Serial.println();
    Serial.print("COMPLETAMENTE CERRADO");
    }
    
     else {               // S1 y S2 diferente de 1 led se prende y se indica que la ventana esta abierta
      digitalWrite(LED,HIGH);
      Serial.println();
      Serial.print("COMPLETAMENTE ABIERTO");
     }
}
