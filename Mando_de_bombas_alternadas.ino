
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "EmonLib.h"             // Include Emon Library

#define CURRENT_CAL 38

#define termico1  8                     //protecciones
#define termico2  9

#define alarma_visual  12              //Sobrecorriente
#define alarma_sonora  13              //Sobrecorriente

#define bomba1  6                       //salida a reles
#define bomba2  7

#define sel_bomba 4                      //elige la bomba en modo mantenimiento; 1=bomba 1;0=bomba2

#define marcha  3                       //Pulsador NA
#define paro  2                         //pulsador NA
#define modo  5                         //Selector de 2 posiciones 0=mantenimiento; 1=mormal

#define presion  10                       //Preostato
#define presion2 11

boolean Encendido = false;              //Bandera de sistema false = apagado, true = encendido

boolean contando = false;               //Bandera para contar tiempo
int contador = 0;                       //contador de accionamientos de la bomba

unsigned long inicio = 0;               //Para contar tiempo
unsigned long presente = 0;
unsigned long tiempo_max = 30000;       //Tiempo a contar en milisegundos. Modificar a conveniencia

unsigned int max_act = 3;               //Limite de actuaciones. Modificar a conveniencia

boolean aux = false;                    //Variable de prop general

int alarma = 0;

unsigned int contador2 = 0;

int bomba = 6;                  //Variable para seleccionar la bomba

float currentDraw=0;

LiquidCrystal_I2C lcd(0x27, 20, 4);     //inicializo comunicacion I2C para controlar el display 16x2

EnergyMonitor emon1;             // Create an instance

void setup() {
  Serial.begin(9600);                   //Inicializo com serial
  emon1.current(1, CURRENT_CAL);       // Current: input pin, calibration.
  
  
  Wire.begin();
  
                                        // initialize the LCD, 
  lcd.begin();
  lcd.backlight();                       // Turn on the blacklight and print a message.
  lcd.clear();
  lcd.setCursor(2,0);
  lcd.print("MANDO AUTOMATICO");
  lcd.setCursor(5,1);
  lcd.print("DE 2 BOMBAS");
  lcd.setCursor(15,3);
  lcd.print("-FCA-");

  pinMode(marcha, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(marcha), encender, RISING);   //Interrupcion para el encendido del sistema pull up

  pinMode(paro, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(paro), apagar, FALLING);      //interrupcion para el apagado del sistema  pull up

  pinMode(modo, INPUT_PULLUP);            //Entrada pull up
  pinMode(presion, INPUT_PULLUP);         //entrada pull up
  pinMode(sel_bomba, INPUT_PULLUP);            //Entrada pull up

  pinMode(bomba1, OUTPUT);                //salidas a rele
  pinMode(bomba2, OUTPUT);

  digitalWrite(bomba1, HIGH);
  digitalWrite(bomba2, HIGH);

  pinMode(termico1, INPUT_PULLUP);                //enradas de termicos
  pinMode(termico2, INPUT_PULLUP);

  pinMode(alarma_visual, OUTPUT);
  pinMode(alarma_sonora, OUTPUT);

  digitalWrite(alarma_visual, HIGH);
  digitalWrite(alarma_sonora, HIGH);
   
  delay(5000);
}

void loop() {
    
                                            //Para generar un retardo de tiempo_max y verificar el numero max_act
  if(contando==false){
    inicio=millis();                        //tiempo inicial
    presente = millis(); 
    contando = true;
  } else {
    presente = millis();                     //lectura constante de tiempo 
    if((presente-inicio)>=0 && (presente-inicio)<=tiempo_max){    //verifico delta de tiempo 
      if(contador > max_act){               //verifico numero de accionamientos
        if(contador2==0){
        
        Encendido = true;
        contador = 0;
        contador2 =1;
        alarma = 1;
        delay(10000);
        } else{
        Encendido = true;
        bomba = bomba2;
        contador2=0;
        contador = 0;
        alarma = 2;
        delay(10000);
        }
      }
    } else {
      contando = false;
      contador = 0;
    }
  }
  
  if (Encendido == false){               //Sistema apagado
    
    digitalWrite(bomba1, HIGH);
    digitalWrite(bomba2, HIGH);
  
  } else {                               //Sistema encendido
    if (digitalRead(modo) == LOW){                   //Modo manual

      if(digitalRead(sel_bomba)==HIGH && digitalRead(termico1)==LOW){
        digitalWrite(bomba1, LOW);
        digitalWrite(bomba2, HIGH);
      } else if(digitalRead(sel_bomba)==LOW && digitalRead(termico2)==LOW){
        digitalWrite(bomba1, HIGH);
        digitalWrite(bomba2, LOW);
      } else {
        digitalWrite(bomba1, HIGH);
        digitalWrite(bomba2, HIGH);
      }
      
      alarma = 0;
      digitalWrite(alarma_visual, HIGH);
      digitalWrite(alarma_sonora, HIGH);
      
    } else {                            //Modo automatico

//      emon1.calcVI(20,2000);         // Calculate all. No.of half wavelengths (crossings), time-out
//      currentDraw = emon1.Irms;             //extract Irms into Variable

//      if(currentDraw>13.5){
//        bomba = bomba2;
//         digitalWrite(alarma_visual, LOW);   
//        //digitalWrite(alarma_sonora, LOW); 
//      }
 
      if (digitalRead(presion) == LOW){              //Presion alta
        
        digitalWrite(bomba, HIGH);
        delay(10);

        if(aux == true){                //actualizo el contador de activaciones
          aux = false;
          contador++;
          
        }
        
      } else {                          //Presion baja
        digitalWrite(bomba, LOW);
        aux = true;
      }
    }
  }

if(alarma!= 0){
  digitalWrite(alarma_visual, LOW);
  digitalWrite(alarma_sonora, LOW);
}


imprimir();
Serial.println(bomba);
delay(300);   
}// loop end


void encender() {
  Encendido = true;
}



void apagar() {
  
  delay(50);


  if(digitalRead(termico1)==HIGH && digitalRead(termico2)==HIGH){               //Ningun termico activado
    Encendido = false;
  } else if(digitalRead(termico1)==HIGH && digitalRead(termico2)==LOW && digitalRead(modo) == HIGH){          //termico 1 activado
    bomba = bomba2;
    digitalWrite(bomba1, HIGH);
  } else if(digitalRead(termico1)==LOW && digitalRead(termico2)==HIGH && digitalRead(modo) == HIGH){         //termico 2 activado
    bomba = bomba1;
    digitalWrite(bomba2, HIGH);
  } else {  
    Encendido = false;
  }
}



void imprimir(){
  lcd.clear();
  switch (alarma){
    case 0:
      lcd.setCursor(0,0);
      lcd.print("   CENTRO JAMBATU   ");

      if(digitalRead(modo)== LOW){
        lcd.setCursor(0,1);
        lcd.print("Modo:  Mantenimiento");
      } else {
        lcd.setCursor(0,1);
        lcd.print("  Modo: Automatico  ");
      }

      lcd.setCursor(0,2);
      lcd.print("  Bomba 1  Bomba 2  ");
      
      if(digitalRead(bomba1)==HIGH){
        lcd.setCursor(3,3);
        lcd.print("OFF");
      } else{
        lcd.setCursor(3,3);
        lcd.print("ON");
      }

      if(digitalRead(bomba2)==HIGH){
        lcd.setCursor(13,3);
        lcd.print("OFF");
      } else{
        lcd.setCursor(13,3);
        lcd.print("ON");
      }
    break;
    
    case 1:
      lcd.setCursor(0,0);
      lcd.print("   CENTRO JAMBATU   ");

      lcd.setCursor(0,1);
      lcd.print("Alarma 1: Sobrecarga");

      lcd.setCursor(0,2);
      lcd.print("  Bomba 1  Bomba 2  ");
      
      if(digitalRead(bomba1)==HIGH){
        lcd.setCursor(3,3);
        lcd.print("OFF");
      } else{
        lcd.setCursor(3,3);
        lcd.print("ON");
      }

      if(digitalRead(bomba2)==HIGH){
        lcd.setCursor(13,3);
        lcd.print("OFF");
      } else{
        lcd.setCursor(13,3);
        lcd.print("ON");
      }
    break;
    
    case 2:
        lcd.setCursor(0,0);
      lcd.print("   CENTRO JAMBATU   ");

      lcd.setCursor(0,1);
      lcd.print("Sobrecarga Fuerte B1");

      lcd.setCursor(0,2);
      lcd.print("  Bomba 1  Bomba 2  ");
      
      if(digitalRead(bomba1)==HIGH){
        lcd.setCursor(3,3);
        lcd.print("OFF");
      } else{
        lcd.setCursor(3,3);
        lcd.print("ON");
      }

      if(digitalRead(bomba2)==HIGH){
        lcd.setCursor(13,3);
        lcd.print("OFF");
      } else{
        lcd.setCursor(13,3);
        lcd.print("ON");
      }
    break;  
    break;        
  } 
}
