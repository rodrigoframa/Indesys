
#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h> 
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  

// Inicializa o display no endereco 0x27
LiquidCrystal_I2C lcd(0x3f,2,1,0,4,5,6,7,3, POSITIVE);

Servo microservo9g;

#define SS_PIN 10
#define RST_PIN 9
// Definicoes pino modulo RC522
MFRC522 mfrc522(SS_PIN, RST_PIN); 

// Leds indicadores acesso liberado ou negado
int led_liberado = 5;
int led_negado = 6;
int pinBuzzer = 7;
int pinSensorPIR = 8;
int valorSensorPIR = 0;
int botao = 4;
boolean estado = false;

char st[20];

void setup() 
{
  lcd.begin (16,2);
  pinMode(led_liberado, OUTPUT);
  pinMode(led_negado, OUTPUT);
  pinMode(botao, INPUT);
  pinMode(pinSensorPIR,INPUT);
  // Define que o servo esta ligado a porta digital 3
  microservo9g.attach(3);
  // Move o servo para a posicao inicial (cancela fechada)
  microservo9g.write(90);
  // Inicia a serial
  Serial.begin(9600);
  // Inicia  SPI bus
  SPI.begin();
  // Inicia MFRC522
  mfrc522.PCD_Init(); 
}

void mensagemInicio(){    
  lcd.setCursor(0,0);
  lcd.print("Bem Vindo ao Lab");
  lcd.setCursor(0,1);
  lcd.print("Use seu Cartao  ");
  }

void limpaLCD(){
  lcd.setCursor(0,0);
  lcd.print("                ");
  lcd.setCursor(0,1);
  lcd.print("                ");
  }

void ligarAlarme() {
  //Ligando o led
  digitalWrite(led_negado, HIGH);
  lcd.setCursor(0,0);
  lcd.print("!!!!!ALERTA!!!!!");
  lcd.setCursor(0,1);
  lcd.print("INTRUSO DETECTADO");
  //Ligando o buzzer com uma frequencia de 1500 hz.
  tone(pinBuzzer,1500);
   
  delay(5000); //tempo que o led fica acesso e o buzzer toca
  limpaLCD();
  desligarAlarme();
}
 
void desligarAlarme() {
  //Desligando o led
  digitalWrite(led_negado, LOW);   
  //Desligando o buzzer
  noTone(pinBuzzer);
}

void msgInicioIntrusao(){
  lcd.setCursor(0,0);
  lcd.print("Sistema Intrusao");
  lcd.setCursor(0,1);
  lcd.print("ATIVADO");
}

void acionarDeteccao(){ 
  estado = true;
  lcd.flush();
  msgInicioIntrusao();
  
  //Lendo o valor do sensor PIR. Este sensor pode assumir 2 valores
  //1 quando detecta algum movimento e 0 quando não detecta.
  valorSensorPIR = digitalRead(pinSensorPIR);
      
  //Verificando se ocorreu detecção de movimentos
  if (valorSensorPIR == 1) {
    ligarAlarme();
  } else {
    desligarAlarme();
  }
}

void loop() 
{
  
  if(digitalRead(botao) == HIGH){
    if(estado){
      estado = false;
    }else{
      estado = true;
    }
  }
  if(estado){
    acionarDeteccao();
  }else{    
  mensagemInicio();
  
  // Aguarda a aproximacao do cartao  
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    return;
  }
  // Seleciona um dos cartoes
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }
  // Mostra UID na serial
//  Serial.print("UID da tag :");
  String conteudo= "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
     
  }

  conteudo.toUpperCase();
  conteudo = conteudo+"\n";
  Serial.print(conteudo.substring(1));
  // Testa se o cartao1 foi lido
//  if (conteudo.substring(1) == "45 5A E5 D5")
//  {
  while (Serial.available() < 0){
    
  }
  String resultado = Serial.readString();
  if (!resultado.equals("null")){
    // Levanta a cancela e acende o led verde
    microservo9g.write(-90);
    digitalWrite(led_liberado, HIGH);
    limpaLCD();
    lcd.setCursor(0,0);
    lcd.print(resultado);
    lcd.setCursor(0,1);
    lcd.print("Acesso liberado");
    delay(3000);
    microservo9g.write(90);
    mensagemInicio();
    digitalWrite(led_liberado, LOW);
  }else{
    limpaLCD();
    lcd.setCursor(0,0);
    lcd.print("Nao Registrado  ");
    lcd.setCursor(0,1);
    lcd.print("Acesso Negado   ");
    // Pisca o led vermelho
    for (int i= 1; i<5 ; i++)
    {
      digitalWrite(led_negado, HIGH);
      delay(200);
      digitalWrite(led_negado, LOW);
      delay(200);
    }
  }
  
  Serial.flush();
  delay(1000); 
  mensagemInicio();
  }
}

