/*########################################################################
// pinagem (so far)
//######################################################################## 
# A0 - LM35
#
# dig 2 - buzzer
# dig 3 - sensor cadeado aberto
#dig 4 - sensor de tensão
# dig 5 - reset RFID
# dig 7 - cooler
# dig 9 - servo
# dig 10 - LED Vermelho
# dig 11 - LED Amarelo
# dig 12 - LED Verde 
# dig 13 - relé 
#
# dig 20 - SDA Display
# dig 21 - SCL Display
#
# dig 22 - botão hora
# dig 24 - botão liga/desliga
# dig 26 - botão temperatura
#
# dig 50 - MISO RFID
# dig 51 - MOSI RFID
# dig 52 - SCK RFID
# dig 53 - SDA RFID
#
*/

/*########################################################################
// Mapeamento da EEPROM
//######################################################################## 
#
# 0 ~ 20 - cadastro de usuários 
#
# 50 - estado da manutenção
# 51 - hora inicio manutenção atual
# 52 - min inicio manutenção atual
# 53 - dia inicio manutenção atual
# 54 - mes inicio manutenção atual
# 55

*/


//########################################################################
// bibliotecas
//########################################################################
#include <SPI.h> /* necessário para o uso do RFID */
#include <MFRC522.h> /* necessário para o uso do RFID */
#include <Servo.h>  /* necessário para o uso do servo motor */
#include <Wire.h> /* necessária para o módulo  RTC */
#include <LiquidCrystal_I2C.h> /* necessário para o uso do display LCD*/
#include <EEPROM.h> /* necessário para o uso da memória eeprom*/


//########################################################################
// define a lista de usuários
//########################################################################
boolean usuario0;
boolean usuario1;
boolean usuario2;
boolean usuario3;
boolean usuario4;
boolean usuarios[6] = {usuario0, usuario1, usuario2, usuario3, usuario4};

String nome0 = "Ramos";
String nome1 = "Tavora";
String nome2 = "Luiz";
String nome3 = "Marcos";
String nome4 = "Ricco";
String nomes[6] = {nome0, nome1, nome2, nome3, nome4};


//========================================================================
// configura o RC522
//========================================================================
#define SS_PIN 53
#define RST_PIN 5

// Definicoes pino modulo RC522
MFRC522 mfrc522(SS_PIN, RST_PIN); 


//========================================================================
// define os delays da função millis
//========================================================================
/*  AVISO DE BUG: 
 *  provavelmente após um determinado número de horas, a função "millis" irá estourar
 *  a memória reservada para seu funcionamento, resetando assim sua contagem, porém,
 *  isso não resetará as variáveis abaixo.
*/
unsigned long delay1 = 0; // delay RFID + Serial + usuários
unsigned long delay2 = 0; // delay da mostra de temperatura na serial
unsigned long delay3 = 0; // delay da mostra usuários no LCD 

byte k = 0; /* necessário para mostrar os usuários no LCD */

//========================================================================
// configura o servo motor
//========================================================================
Servo servo_principal;          /* define o nome do servo*/
int servo_pr_abre = 0;          /* graus em que o servo deve estar para abrir o cadeado */
int servo_pr_fecha = 180;       /* graus em que o servo deve estar para abrir o fechar */

//========================================================================
// Configura o LCD
//========================================================================
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
/* A string abaixo serve para eliminar o glitch do LCD  */
String last_screen; 

//========================================================================
// Configura o LM35
//========================================================================
/* Define o pino do sensor como sendo a entrada A0 */
#define lm35 A0
/* Define o pino do cooler como sendo o pino digital 7 */
#define Cooler 7
/* Declara a variável temperatura como ponto flutuante */
float temperatura = 0;
/* Define o pino do botão liga/desliga */
#define Botao_temp 26

//========================================================================
// Configura o sensor de cadeado aberto.
//========================================================================
/* #define sensor_cadeado_aberto 3; */  // acredito que o problema com essa variável era o uso do ; no final. teste isso depois.
int estado_sensor;
bool manutencao;

//========================================================================
// Configura o uso da EEPROM
//========================================================================
/* define o espaço da EEPROM como 500 bytes, embora eu não entenda o motivo disso */
#define espacoEEPROM = 500; // testar se é possível apagar esta variável que parece inutil.


//========================================================================
// Configura o uso do RTC
//========================================================================
/* RTC no endereco 0x68 */
#define DS3231_ADDRESS 0x68
byte zero = 0x00;
#define Exibe_Hora 22
bool estado_Exibe_Hora;
byte since_hora;
byte since_min;
byte since_dia;
byte since_mes;
byte since_ano; // será usado apenas para gerar o gráfico

//========================================================================
// Configura o uso dos gráficos via Wi-Fi
//========================================================================
unsigned long inicio_manutencao;
unsigned long fim_manutencao;
int duracao_manutencao;
byte dia1, mes1, ano1;
byte dia2, mes2, ano2;
byte dia3, mes3, ano3;
byte dia4, mes4, ano4;
byte dia5, mes5, ano5;
 

//========================================================================
// Outras Configurações
//========================================================================
/* Define o pino do Buzzer */
#define Buzzer 2
/* Define o pino do relé principal */
#define SensorTensao 4
/* Define o pino do LED Vermelho */
#define LEDVermelho 10
/* Define o pino do LED Amarelo */
#define LEDAmarelo 11
/* Define o pino do LED Verde */
#define LEDVerde 12
/* Define o pino do relé principal */
#define Relay 13
/* Define o pino do botão liga/desliga */
#define Botao_lig_desl 24
bool lig_des;

//########################################################################
// void setup
//########################################################################
void setup() {
  /* Declara o pino do sensor de abertura como entrada com pull up interno */
  pinMode(3, INPUT_PULLUP);
  /* Declara o pino do sensor lm35 como entrada */
  pinMode(lm35, INPUT);
  /* Declara o pino do buzzer como saída */
  pinMode(Buzzer, OUTPUT);
  /* Declara o pino do relé como saída */
  pinMode(Relay, OUTPUT);
  /* Declara o pino dos LEDs  como saída */
  pinMode(LEDVermelho, OUTPUT);
  pinMode(LEDAmarelo, OUTPUT);
  pinMode(LEDVerde, OUTPUT);
  digitalWrite(LEDVerde, HIGH);
  /* Declara o pino do botão de hora como entrada com pull up interno */
  pinMode(Exibe_Hora, INPUT_PULLUP);
  /* Declara o pino do cooler como saída */
  pinMode(Cooler, OUTPUT);
  /* Declara o pino do sensor de tensão como entrada */
  pinMode(SensorTensao, INPUT_PULLUP);
  /* Declara o pino do botão de hora como entrada com pull up interno */
  pinMode(Botao_lig_desl, INPUT_PULLUP);
  lig_des = 0;
  /* Declara o pino do botão de hora como entrada com pull up interno */
  pinMode(Botao_temp, INPUT_PULLUP);
  
  /* lê o estado dos usuários na memória EEPROM */
  usuarios[0] = EEPROM.read(0);
  usuarios[1] = EEPROM.read(1);
  usuarios[2] = EEPROM.read(2);
  usuarios[3] = EEPROM.read(3);
  usuarios[4] = EEPROM.read(4);
  
  /* lê o estado da manutenção na memória EEPROM */
  manutencao = EEPROM.read(50);

  /* lê a hora de inicio de manutenção na memória EEPROM */
  since_hora = EEPROM.read(51);
  since_min = EEPROM.read(52);
  since_dia = EEPROM.read(53);
  since_mes = EEPROM.read(54);
  since_ano = EEPROM.read(55);

  /* inicia a serial */
  Serial.begin(9600);
  
  /* Inicia  SPI bus */
  SPI.begin();
  // Inicia MFRC522
  mfrc522.PCD_Init(); 

  /* configura o servo */
  servo_principal.attach(9);

  /* Mensagens iniciais no serial monitor */
  Serial.println("Aproxime o seu cartao do leitor...");
  Serial.println();

  /* Mensagens iniciais no LCD */
  delay(2000);
  lcd.begin(16,2);                                 
  lcd.clear();                                     
  lcd.setCursor(0,0);                              
  lcd.print("SISTEMA DE LOTO");                         
  lcd.setCursor(4,2);  
  lcd.print("by: 3LM");
  delay(1500);
}

//########################################################################
// funções
//########################################################################
//========================================================================
// função: verificação de temperatura
//========================================================================
void verificar_temperatura(){
  verificar_temperatura:
  if ((millis() - delay2) >= 3000){
    float leitura = analogRead(lm35);                /* Lê e armazena  o valor da entrada analógica em leitura*/
    temperatura = (5*leitura) / (10.23);             /* Cálcula o valor da temperatura*/
    Serial.print("\n");                              /* debug. pode ser removido ou comentado*/
    Serial.print(temperatura);                       /* debug. pode ser removido ou comentado*/
    Serial.print("ºC");                              /* debug. pode ser removido ou comentado*/
    delay2 = millis();
    if (digitalRead(Botao_temp) == 0){
      lcd.clear();                                     
      lcd.setCursor(1,0);
      lcd.print("temp. interna:");
      lcd.setCursor(5,1);
      lcd.print(temperatura);
      lcd.print(" ");
      lcd.write(B11011111);
      lcd.print("C");
      last_screen = "temperatura";
      delay(3000);
    }
    if (temperatura > 50){                           /* Executa o alarme de alta temperatura quando acima do valor indicado*/
      digitalWrite(LEDVerde , LOW);
      digitalWrite(Buzzer , HIGH);
      digitalWrite(LEDVermelho , HIGH);
      digitalWrite(Cooler , HIGH);
      delay(500);
      digitalWrite(Buzzer , LOW);
      lcd.clear();                                     
      lcd.setCursor(1,0);
      lcd.print("    Aviso!");
      lcd.setCursor(0,2); 
      lcd.print("Alta Temperatura");
      delay (1500);
      last_screen = "alta temperatura";              /* possível melhoria no futuro: eliminar o glitch da tela de manutenção */
      goto verificar_temperatura;                    // não funciona saporra
    } else {
      digitalWrite(LEDVerde, HIGH);
      digitalWrite(LEDVermelho, LOW);
      digitalWrite(Cooler , LOW);
    }
  }
}

//========================================================================
// função: verificador de abertura forçada
//========================================================================
void verificador_de_abertura_forcada(){
  //lcd.setCursor(0,1); 
  //lcd.print("@");
  if (estado_sensor == 1){
    lcd.clear();
    lcd.setCursor(0,0); 
    lcd.print("ABERTURA FORCADA");
    lcd.setCursor(0,1); 
    lcd.print("   DETECTADA");
    delay(1000);
    while (true){
       /* loop infinito :) */
       digitalWrite(Buzzer , HIGH);
       digitalWrite(LEDVermelho , HIGH);
       digitalWrite(LEDVerde , LOW);
       delay(500);
       digitalWrite(Buzzer , LOW);
       delay(500);
    }
  }
}


//========================================================================
// função: verificador de lista vazia
//========================================================================
void verificador_de_lista_vazia() {
  ////////////////////////////////////////////////////////////////////////
  // lista vazia (cadeado aberto)
  ////////////////////////////////////////////////////////////////////////
   if (usuarios[0] == 0 && usuarios[1] == 0 && usuarios[2] == 0 && usuarios[3] == 0 && usuarios[4] == 0){
    if (last_screen != "equipamento liberado"){           /* este "if" serve para eliminar o glitch do LCD  */
      Serial.println("");
      Serial.println("equipamento liberado");
      delay(50);
      lcd.clear();
      lcd.setCursor(2,0); 
      lcd.print("EQUIPAMENTO");
      if (lig_des == 1){
        lcd.setCursor(4,1); 
        lcd.print("LIGADO");  
      } else{
        lcd.setCursor(3,1); 
        lcd.print("DESLIGADO");  
      }
      if (manutencao == true){
        /* inserir aqui o código para criar um novo log de tempo de manutenção*/
        Serial.print(dia1); Serial.print(mes1); Serial.print(ano1); Serial.print("\n");
        Serial.print(dia2); Serial.print(mes2); Serial.print(ano2); Serial.print("\n");
        Serial.print(dia3); Serial.print(mes3); Serial.print(ano3); Serial.print("\n");
        Serial.print(dia4); Serial.print(mes4); Serial.print(ano4); Serial.print("\n");
        Serial.print(dia5); Serial.print(mes5); Serial.print(ano5); Serial.print("\n");
        Serial.print("\n");               // linha de debug
        Serial.print(inicio_manutencao);  // linha de debug
        Serial.print("\n");               // linha de debug
        fim_manutencao = millis();
        Serial.print(fim_manutencao);     // linha de debug
        duracao_manutencao = (inicio_manutencao - fim_manutencao) / 60000;
        Serial.print(duracao_manutencao);
      }
      manutencao = false;
      EEPROM.write(50, manutencao);
      servo_principal.write(servo_pr_abre);
      //digitalWrite(Relay, HIGH);                        /* apagar esta linha e a de baixo após testes de debug  */
      //lig_des = 1;
      last_screen = "equipamento liberado";               /* esta string serve para eliminar o glitch do LCD  */
      delay(1000);
    }
   }
  ////////////////////////////////////////////////////////////////////////
  // lista não vazia (cadeado fechado)
  ////////////////////////////////////////////////////////////////////////
   else {
    servo_principal.write(servo_pr_fecha);
    digitalWrite(Relay, LOW);
    digitalWrite(LEDAmarelo, LOW);
    lig_des = 0;
    if (last_screen != "em manutencao"){              /* este "if" serve para eliminar o glitch do LCD  */
      lcd.clear();
      if (manutencao == false){
        inicio_manutencao = millis();
      }
      manutencao = true;
      EEPROM.write(50, manutencao);
      if (last_screen == "estado usuario"){
        /*provavelmente eu deveria mover as linhas a seguir para o if manutencao == false. teste para verificar isto*/
        EEPROM.write(51, since_hora);
        EEPROM.write(52, since_min);
        EEPROM.write(53, since_dia);
        EEPROM.write(54, since_mes);
      }
    } 
    since();
    if (k < 5){                                       /* este é um "laço for" sem usar um "laço for" */
      lcd.setCursor(0,0);
      lcd.print(" EM MANUTENCAO");
      last_screen = "em manutencao";                  /* esta string serve para eliminar o glitch do LCD  */
      if ((millis() - delay3) >= 1000){
        if (usuarios[k] == 1){                        /* printa os nomes no LCD */
          lcd.setCursor(0,1);
          lcd.print(nomes[k]);
          lcd.print("         ");                     /* limpa os espaços não usados no LCD */
          delay3 = millis();
        }
        k++;
      } 
    }else {
        k = 0;
      }
   }
}

//========================================================================
// função: aciona o buzzer por 100ms
//========================================================================
void buzzer_100() {
 digitalWrite(Buzzer , HIGH);
 delay(100);
 digitalWrite(Buzzer , LOW); 
 delay(100);
}
//========================================================================
// função: print usuario ativado
//========================================================================
void print_ativado() {
  last_screen = "estado usuario";
  lcd.clear();
  lcd.setCursor(4,0); 
  lcd.print("USUARIO");
  lcd.setCursor(4,1); 
  lcd.print("ATIVADO");
  buzzer_100();
  delay(2900);  
}

//========================================================================
// função: print usuario desativado
//========================================================================
void print_desativado() {
  last_screen = "estado usuario";
  lcd.clear();
  lcd.setCursor(4,0); 
  lcd.print("USUARIO");
  lcd.setCursor(3,1); 
  lcd.print("DESATIVADO");
  buzzer_100();
  buzzer_100();
  delay(2700);  
}

//========================================================================
// função: RTC
//========================================================================
void RTC(){
  estado_Exibe_Hora = digitalRead(Exibe_Hora);
  if (manutencao == 0){
    /* Lê os valores (data e hora) do modulo DS3231 */
    Wire.beginTransmission(DS3231_ADDRESS);
    Wire.write(zero);
    Wire.endTransmission();
    Wire.requestFrom(DS3231_ADDRESS, 7);
    int segundos = ConverteparaDecimal(Wire.read());
    int minutos = ConverteparaDecimal(Wire.read());
    int horas = ConverteparaDecimal(Wire.read() & 0b111111);
    int diadasemana = ConverteparaDecimal(Wire.read()); 
    int diadomes = ConverteparaDecimal(Wire.read());
    int mes = ConverteparaDecimal(Wire.read());
    int ano = ConverteparaDecimal(Wire.read());
    since_hora = horas;
    since_min = minutos;
    since_dia = diadomes;
    since_mes = mes;
    since_ano = ano;
  
    /* Mostra os dados no display */
    if (estado_Exibe_Hora == 0){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("    ");
    /* Acrescenta o 0 (zero) se a hora for menor do que 10 */
    if (horas <10)
      lcd.print("0");
    lcd.print(horas);
    //since_hora = horas;
    lcd.print(":");
    /* Acrescenta o 0 (zero) se minutos for menor do que 10 */
    if (minutos < 10)
       lcd.print("0");
    lcd.print(minutos);
    //since_min = minutos;
     lcd.print(":");
     /* Acrescenta o 0 (zero) se segundos for menor do que 10 */
    if (segundos < 10)
       lcd.print("0");
    lcd.print(segundos);
    lcd.setCursor(2,1);
    /* Mostra o dia da semana */
    switch(diadasemana)
      {
        case 0:lcd.print("Dom");
        break;
        case 1:lcd.print("Seg");
        break;
        case 2:lcd.print("Ter");
        break;
        case 3:lcd.print("Quar");
        break;
        case 4:lcd.print("Qui");
        break;
        case 5:lcd.print("Sex");
        break;
        case 6:lcd.print("Sab");
      }
      lcd.setCursor(6,1);
      /* Acrescenta o 0 (zero) se dia do mes for menor do que 10 */
      if (diadomes < 10)
        lcd.print("0");
      lcd.print(diadomes);
      //since_dia = diadomes;
      lcd.print("/");
      /* Acrescenta o 0 (zero) se mes for menor do que 10 */
      if (mes < 10)
        lcd.print("0");
      lcd.print(mes);
      //since_mes = mes;
      lcd.print("/");
      lcd.print(ano);
      Serial.print("\n");
      Serial.print(since_hora);
      Serial.print(":");
      Serial.print(since_min);
      last_screen = "RTC";
      delay(2000);
    }
  }
}

byte ConverteParaBCD(byte val)
{ 
  /*Converte o número de decimal para BCD */
  return ( (val/10*16) + (val%10) );
}

byte ConverteparaDecimal(byte val)  
{ 
  /*Converte de BCD para decimal */
  return ( (val/16*10) + (val%16) );
}  

//========================================================================
// função: Since
//========================================================================
void since(){
 if (manutencao == 1 && estado_Exibe_Hora == 0){ 
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Inicio Manut.:");
  lcd.setCursor(0,1);
  if (since_hora < 10){
      lcd.print("0");
  }
  lcd.print(since_hora);
  lcd.print(":");
  if (since_min < 10){
    lcd.print("0");
  }
  lcd.print(since_min);
  lcd.print("hs. ");
  if (since_dia < 10){
    lcd.print("0");
  }
  lcd.print(since_dia);
  lcd.print("/");
  if (since_mes < 10){
    lcd.print("0");
  }
  lcd.print(since_mes);
  last_screen = "since";
  delay(5000);
 }
}


//========================================================================
// função: verificar tensão
//========================================================================
void Tensao(){
  if (manutencao == 1 && digitalRead(SensorTensao) == 1){
    lcd.clear();
    lcd.setCursor(0,0); 
    lcd.print("FALHA CONTATOR");
    lcd.setCursor(0,1); 
    lcd.print("   DETECTADA");
    delay(1000);
    while (true){
       /* loop infinito :) */
       digitalWrite(Buzzer , HIGH);
       digitalWrite(LEDVerde , LOW);
       digitalWrite(LEDVermelho , HIGH);
       delay(500);
       digitalWrite(Buzzer , LOW);
       delay(500);
    }
  }
}

//========================================================================
// função: reset timer
//========================================================================
void ResetTimer() {
  if (millis() > 4294961295){
    delay1 = 0;
    delay2 = 0;
    delay3 = 0; 
  }
}


//========================================================================
// função: liga/desliga manual
//========================================================================
void Liga_Desliga() {  
  if (manutencao == 0 && digitalRead(Botao_lig_desl) == 0){
    if(lig_des == 0){
      buzzer_100();
      digitalWrite(Relay, HIGH);
      digitalWrite(LEDAmarelo, HIGH);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Ligando...");
      lig_des = 1;
      last_screen = "ligado";
      delay(2500);
    }else{
      buzzer_100();
      buzzer_100();
      digitalWrite(Relay, LOW);
      digitalWrite(LEDAmarelo, LOW);
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Desligando...");
      lig_des = 0;
      last_screen = "desligado";
      delay(2500);
    }
  }
  if (manutencao == 1 && digitalRead(Botao_lig_desl) == 0){
    digitalWrite(Buzzer, HIGH);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Impossivel ligar");
    lcd.setCursor(0,1);
    lcd.print("em manutencao");
    last_screen = "impossivel";
    delay(2000);
    digitalWrite(Buzzer, LOW);
    delay(1000);
  }
}

//########################################################################
// void loop
//########################################################################
void loop() {

  estado_sensor = digitalRead(3);
  if (estado_sensor == 1){                           /* verifica se o cadeado está fechado antes de iniciar  */
    if (last_screen != "cadeado aberto"){            /* este "if" serve para eliminar o glitch do LCD */
    lcd.clear();
    lcd.setCursor(0,0); 
    lcd.print("FECHE A PORTA");
    lcd.setCursor(0,1); 
    lcd.print("  PARA INICIAR");
    last_screen = "cadeado aberto";                   /* esta string serve para eliminar o glitch do LCD  */
    }
    else{
      
    }
  }
  else{
    verificador_de_lista_vazia();
  }

  verificar_temperatura();
  RTC();
  ResetTimer();
  Liga_Desliga();
  Tensao();                                              
    
  
  if (manutencao == true){                            /* verificar se este if pode ser apagado */
    verificador_de_abertura_forcada();                /* || */
  }                                                   /* || */
 
  //========================================================================
  // coisas do RFID que eu não sei como funciona
  //========================================================================

  /* Aguarda a aproximacao do cartao   */
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  /* Seleciona um dos cartoes  */
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }
  
  String conteudo= "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    /* NÃO APAGAR AS LINHAS ABAIXO, ELAS SERVEM PARA FORMAR A FORMATAÇÃO DO ID */
    conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  /* NÃO APAGAR A LINHA ABAIXO, ELA SERVE PARA FORMAR A FORMATAÇÃO DO ID */
  conteudo.toUpperCase();


  //########################################################################
  // CARTÕES
  // -----------------------------------------------------------------------
  // ATENÇÃO: NÃO ALTERAR O TEMPO DO "DELAY", NEM MESMO PENSE EM TROCÁ-LO POR "MILLIS"!!!!
  // CASO O FAÇA, TESTE O SISTEMA VÁRIAS VEZES. NÃO SE ESQUEÇA DA NOSSA EPÍGRAFE.
  //########################################################################
  
  /* verifica se o cadeado está fechado antes de validar os usuários */
  if (estado_sensor == 0){
    //////////////////////
    // tag "Master"
    //////////////////////
    if (conteudo.substring(1) == "D9 60 53 A3"){ 
        usuarios[0] = 0;
        usuarios[1] = 0;
        usuarios[2] = 0;
        usuarios[3] = 0;
        usuarios[4] = 0;
        EEPROM.write(0, usuarios[0]);
        EEPROM.write(1, usuarios[1]);
        EEPROM.write(2, usuarios[2]);
        EEPROM.write(3, usuarios[3]);
        EEPROM.write(4, usuarios[4]);
        last_screen = "estado usuario";
        lcd.clear();
        lcd.setCursor(2,0); 
        lcd.print("CHAVE MESTRE");
        lcd.setCursor(0,1); 
        lcd.print("limpando memoria");
        buzzer_100();
        buzzer_100();
        buzzer_100();
        buzzer_100();
        delay(3000);
        }
        
    //////////////////////
    // tag dos peão
    //////////////////////
  
    if (conteudo.substring(1) == "97 44 0B 62"){     /*|Ramos| */
      usuarios[0] = !usuarios[0];
      EEPROM.write(0, usuarios[0]);
      if (usuarios[0] == true){
        print_ativado();
      } else{
        print_desativado();
        }
      }
      
    if (conteudo.substring(1) == "23 35 C3 EE"){    /* |Tavora| */
      usuarios[1] = !usuarios[1];
      EEPROM.write(1, usuarios[1]);
      if (usuarios[1] == true){
        print_ativado();
      } else{
        print_desativado();
        }
      }
      
    if (conteudo.substring(1) == "9C A0 F3 89"){    /* |Luiz| */
      usuarios[2] = !usuarios[2];
      EEPROM.write(2, usuarios[2]);
      if (usuarios[2] == true){
        print_ativado();
      } else{
        print_desativado();
        }
      }
  
    if (conteudo.substring(1) == "EE 84 33 3F"){    /* |Marcos| */
      usuarios[3] = !usuarios[3];
      EEPROM.write(3, usuarios[3]);
      if (usuarios[3] == true){
        print_ativado();
      } else{
        print_desativado();
        }
      }
  
     if (conteudo.substring(1) == "72 D5 CD 82"){   /* |Ricco| */
      usuarios[4] = !usuarios[4];
      EEPROM.write(4, usuarios[4]);
      if (usuarios[4] == true){
        print_ativado();
      } else{
        print_desativado();
        }
      }
 }
}
