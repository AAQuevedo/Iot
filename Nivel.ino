#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal.h>

WiFiClient esp32Client;
PubSubClient mqttClient(esp32Client);
LiquidCrystal lcd (22,23,5,18,19,21);
const char* ssid     = "QUEVEDO";
const char* password = "Toby$2021#";

char *server = "broker.emqx.io";
int port = 1883;

int ledpin= 26;
int Sensor1=33;
int Sensor2 =32;
int t1in = 12;
int t2in = 14;
int t1out = 27;
int t2out = 26;

int var = 0;
int ledval = 0;
int s1 = 0;
int s2 = 0;

long limite = 4000;

char datos[40];
String resultS = "";

void wifiInit() {
    Serial.print("Conectándose a ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
        delay(500);
    }
    Serial.println("");
    Serial.println("Conectado a WiFi");
    Serial.println("Dirección IP: ");
    Serial.println(WiFi.localIP());
  }

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");

  char payload_string[length + 1];
  
  int resultI;

  memcpy(payload_string, payload, length);
  payload_string[length] = '\0';
  resultI = atoi(payload_string);
  
  var = resultI;

  resultS = "";
  
  for (int i=0;i<length;i++) {
    resultS= resultS + (char)payload[i];
  }
  Serial.println();
}



void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Intentando conectarse MQTT...");

    if (mqttClient.connect("arduinoClient")) {
      Serial.println("Conectado");

      mqttClient.subscribe("Entrada/lvl");
      
    } else {
      Serial.print("Fallo, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" intentar de nuevo en 5 segundos");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
  pinMode(ledpin,OUTPUT);
  pinMode(t1in,OUTPUT);
  pinMode(t2in,OUTPUT);
  pinMode(t1out,OUTPUT);
  pinMode(t2out,OUTPUT);
  lcd.begin(16,2);
  Serial.begin(115200);
  delay(10);
  wifiInit();
  mqttClient.setServer(server, port);
  mqttClient.setCallback(callback);
}

void loop()
{
   if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();

  lcd.setCursor(0,0);
  //lcd.print("BIENVENIDO A");
  lcd.setCursor(0,2);
  //lcd.print("TANQUE");

  Serial.print("Limite: ");
  Serial.println(resultS);
  limite = resultS.toInt();

  if(var == 0)
  {
  digitalWrite(ledpin,LOW);
  }
  else if (var == 1)
  {
  //digitalWrite(ledpin,HIGH);
  }

  s1 = analogRead(Sensor1);
  Serial.print("Sensor1: ");
  Serial.println(s1);

  s2 = analogRead(Sensor2);
  Serial.print("Sensor2: ");
  Serial.println(s2);

  if(s1+s2 > limite){
    digitalWrite(t1in, LOW);
    digitalWrite(t2in, LOW);
    digitalWrite(t1out, LOW);
    digitalWrite(t2out, LOW);
    lcd.setCursor(0,0);
    lcd.print("TANQUE   ");
    lcd.setCursor(0,2);
    lcd.print("LLENO    ");
  }else{
    digitalWrite(t1in, HIGH);
    digitalWrite(t2in, HIGH);
    digitalWrite(t1out, LOW);
    digitalWrite(t2out, LOW);
    lcd.setCursor(0,0);
    lcd.print("             ");
    lcd.setCursor(0,2);
    lcd.print("             ");
  }

  if(s1+s2 < 1000){
    digitalWrite(t1in, HIGH);
    digitalWrite(t2in, HIGH);
    digitalWrite(t1out, LOW);
    digitalWrite(t2out, LOW);
    lcd.setCursor(0,0);
    lcd.print("TANQUE   ");
    lcd.setCursor(0,2);
    lcd.print("VACIO    ");
  }

  

  sprintf(datos, "Valor Sensor 1: %d ", Sensor1);
  mqttClient.publish("Salida/lvl",datos);
  sprintf(datos, "Valor Sensor 2: %d ", Sensor2);
  mqttClient.publish("Salida/lvl",datos);
  Serial.println();
  delay(5000);
}
