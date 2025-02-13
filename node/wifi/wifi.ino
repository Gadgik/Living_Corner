#include <FastBot.h> 
#include <Wire.h>

#define TRANS_ID 8
#define WIFI_SSID "*******"
#define WIFI_PASS "******************"
#define BOT_TOKEN "*************"
#define CHAT_ID "******"
// переменная, в которую запишем ID сообщения   
// с меню для дальнейшего редактирования 
int32_t menuID = 0; 
byte depth = 0; 
bool state_water = false;
FastBot bot(BOT_TOKEN);

void connectWiFi() { 
  delay(2000); 
  WiFi.begin(WIFI_SSID, WIFI_PASS); 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(500); 
    if (millis() > 15000) ESP.restart(); 
  } 
  Serial.println("Connected");
}  

#define DHTPIN D0
#define auto_water_pin D3
#define auto_felt_pin D4
#define drnk D7
#define LED_PIN_f D5
#define LED_PIN_h D6
#define water_sens A0

#define LED_NUM_f 9
#define LED_NUM_h 8
const unsigned long INTERVAL_send = 60*1000*5;
unsigned long  waitTime_send = 0;
#define felt_time 800
#define water_time 3000
//#include "FastLED.h"
//CRGB leds_h[LED_NUM_h]; 
//CRGB leds_f[LED_NUM_f];

//#include "DHT.h"  
//#define DHTTYPE DHT11 
//DHT dht(DHTPIN, DHTTYPE); 

bool auto_felt = false;
bool auto_water = true;
bool led_ham = false;
bool led_aqua = false;
float air_temp = 0;
float air_humid = 0;
float humid_1 = 0;
float humid_2 = 0;
float turb = 0;

FB_Time t;

FB_Time felt_date;

String times_hams[3] = {"-","-","-"}; 
String times_fish[3] = {"-","-","-"}; 

void setup() {
  Serial.begin(115200);
  Wire.begin(); 

  pinMode(DHTPIN,INPUT);
  pinMode(auto_water_pin,OUTPUT);
  pinMode(auto_felt_pin,OUTPUT);
  pinMode(drnk,OUTPUT);
  pinMode(LED_PIN_f,OUTPUT);
  pinMode(LED_PIN_h,OUTPUT);
  pinMode(water_sens,INPUT);
  
  connectWiFi();   
  bot.setChatID(CHAT_ID); 
  bot.attach(newMsg); 
  bot.setTextMode(FB_MARKDOWN);
  //FastLED.addLeds<WS2812, LED_PIN_f>(leds_f,LED_NUM_f);
  //FastLED.addLeds<WS2812, LED_PIN_h>(leds_h,LED_NUM_h);
  //FastLED.setBrightness(140);
  //FastLED.clear();
}

void sending(String str, String ID){ 
  bot.deleteMessage(menuID); 
  bot.sendMessage(str,ID); 
  String menu = F("cage \t garden \n aqua \t info "); 
  bot.inlineMenu("*Выберете команду*", menu); 
  depth = 0;
  menuID = bot.lastBotMsg(); 
} 

void transmit(String text) {
  Wire.beginTransmission(TRANS_ID);
  Wire.write(text.c_str());          
  Wire.endTransmission();             
}

void felting(){
  transmit("felt plant 1");
  bot.sendMessage("1",CHAT_ID);
  delay(5000);
  delay(1000);
  bot.sendMessage("on",CHAT_ID);
  digitalWrite(auto_felt_pin, HIGH);
  delay(felt_time);
  digitalWrite(auto_felt_pin, LOW);
  bot.sendMessage("off",CHAT_ID);
  delay(1000);
  delay(10000);
  bot.sendMessage("2",CHAT_ID);
  transmit("felt plant 2");
  delay(6000);
  delay(1000);
  bot.sendMessage("3",CHAT_ID);
  digitalWrite(auto_felt_pin, HIGH);
  delay(felt_time);
  digitalWrite(auto_felt_pin, LOW);
  bot.sendMessage("4",CHAT_ID);
  delay(1000);
  delay(6000);
}

void newMsg(FB_msg& msg) {
  
    if (msg.text == "/start") {  
      bot.sendMessage("Hello!",msg.chatID); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
      depth = 0;
    }
// ЦВЕТОЧКИ ------------------------------------------------------
    if (msg.data == "garden") { 
      String menu = F("auto water \n fertilizer \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 1;
    } 
    if (msg.data == "auto water") { 
      String menu = F("auto on \n auto off \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 3;
    } 
    if (msg.data == "auto on" and depth == 3) { 
      auto_water = true;
      sending("Автополив включен",msg.chatID); 
    } 
    if (msg.data == "auto off" and depth == 3) { 
      auto_water = false;
      sending("Автополив выключен",msg.chatID); 
    } 
    if (msg.data == "fertilizer") { 
      String menu = F("auto on \t auto off \n interval \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 4;
    } 
    if (msg.data == "auto on" and depth == 4) { 
      auto_felt = true;
      felting();
      FB_Time felt_day = bot.getTime(3); 
      sending("Удобрение включено",msg.chatID); 
    } 
    if (msg.data == "auto off" and depth == 4) { 
      auto_felt = false;
      sending("Удобрение выключено",msg.chatID); 
    } 
    if (msg.data == "interval" and depth == 4) { 
    } 
// ХОМЯК ---------------------------------------------------------------   
    if (msg.data == "cage") { 
      String menu = F("led \t feed now \n time \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 1; 
    } 
    if (msg.data == "led" and depth == 1) { 
      String menu = F("ON \n OFF"); 
      bot.editMenu(menuID, menu); 
      depth = 2; 
    } 
    if (msg.data == "ON" and depth == 2) { 
      led_ham = true;
      sending("Освещение клетки включено",msg.chatID);
    } 
    if (msg.data == "OFF" and depth == 2) { 
      led_ham = false;
      sending("Освещение клетки выключено",msg.chatID);
    } 
    if (msg.data == "feed now" and depth == 1) {  
      transmit("feed hamster");
      sending("Хомячок покормлен",msg.chatID);
    }
    if (msg.data == "time" and depth == 1) {
      String menu = F("list \n add time \n del time \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 7; 
    }
    if ( msg.data == "add time" and depth == 7){ 
      if(times_hams[0]=="-" or times_hams[1]=="-" or times_hams[2]=="-" ){
        bot.sendMessage("Введите время",msg.chatID); 
        bot.attach(newTime_cage); 
      }else{
        bot.sendMessage("Нет места , удалите что-то",msg.chatID); 
      }
    } 
    if ( msg.data == "del time" and depth == 7){ 
      if(times_hams[0] != "-" or times_hams[1] != "-" or times_hams[2] != "-" ){
        bot.sendMessage("Выберете что удалить",msg.chatID); 
        String t = "1: "; 
        t += times_hams[0]; 
        bot.sendMessage(t,msg.chatID); 
        t = "2: "; 
        t += times_hams[1]; 
        bot.sendMessage(t,msg.chatID); 
        t = "3: "; 
        t += times_hams[2]; 
        bot.sendMessage(t,msg.chatID); 
        bot.attach(delTime_cage); 
      }else{
        bot.sendMessage("Нечего удалять",msg.chatID); 
      } 
    } 
    if(msg.data == "list" and depth == 7){
      bot.sendMessage("Время кормления хомяка",msg.chatID);
      bot.deleteMessage(menuID); 
      String t = "1: "; 
      t += times_hams[0]; 
      bot.sendMessage(t,msg.chatID); 
      t = "2: "; 
      t += times_hams[1]; 
      bot.sendMessage(t,msg.chatID); 
      t = "3: "; 
      t += times_hams[2];  
      sending(t,msg.chatID); 
    }

    
// АКВАС ---------------------------------------------------------------   
    if (msg.data == "aqua") { 
      String menu = F("led \t feed now \n time \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 2;
    } 
    if (msg.data == "led" and depth == 2) { 
      String menu = F("ON \n OFF"); 
      bot.editMenu(menuID, menu); 
      depth = 5; 
    } 
    if (msg.data == "ON" and depth == 5) { 
      led_aqua = true;
      sending("Освещение аквариума включено",msg.chatID);
    } 
    if (msg.data == "OFF" and depth == 5) { 
      led_aqua = false;
      sending("Освещение аквариума выключено",msg.chatID);
    } 
    if (msg.data == "feed now" and depth == 2) {  
      String menu = F("1 \n 2"); 
      bot.editMenu(menuID, menu); 
      depth = 8; 
    }
    if(msg.data == "1" and depth == 8){
      transmit("feed fish 1");
      sending("Рыбка 1 покормлена",msg.chatID);
    }
    if(msg.data == "2" and depth == 8){
      transmit("feed fish 2");
      sending("Рыбка 2 покормлена",msg.chatID);
    }
    if (msg.data == "time" and depth == 2) {
      String menu = F("list \n add time \n del time \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 8; 
    }
    if ( msg.data == "add time" and depth == 8){ 
      if(times_fish[0]=="-" or times_fish[1]=="-" or times_fish[2]=="-" ){
        bot.sendMessage("Введите время",msg.chatID); 
        bot.attach(newTime_fish); 
      }else{
        bot.sendMessage("Нет места , удалите что-то",msg.chatID); 
      }
    } 
    if ( msg.data == "del time" and depth == 8){ 
      if(times_fish[0] != "-" or times_fish[1] != "-" or times_fish[2] != "-" ){
        bot.sendMessage("Выберете что удалить",msg.chatID); 
        String t = "1: "; 
        t += times_fish[0]; 
        bot.sendMessage(t,msg.chatID); 
        t = "2: "; 
        t += times_fish[1]; 
        bot.sendMessage(t,msg.chatID); 
        t = "3: "; 
        t += times_fish[2]; 
        bot.sendMessage(t,msg.chatID); 
        bot.attach(delTime_cage); 
      }else{
        bot.sendMessage("Нечего удалять",msg.chatID); 
      } 
    } 
    if(msg.data == "list" and depth == 8){
      bot.sendMessage("Время кормления рыб",msg.chatID);
      bot.deleteMessage(menuID); 
      String t = "1: "; 
      t += times_fish[0]; 
      bot.sendMessage(t,msg.chatID); 
      t = "2: "; 
      t += times_fish[1]; 
      bot.sendMessage(t,msg.chatID); 
      t = "3: "; 
      t += times_fish[2];  
      sending(t,msg.chatID); 
    }

// ИНФОРМАЦИЯ ---------------------------------------------------------------   
    if (msg.data == "info") { 
      String menu = F("air info \t soil info \n water info \t Back \n test"); 
      bot.editMenu(menuID, menu); 
      depth = 1; 
    }
    if (msg.data == "soil info") { 
      String ham = "*Влажность почвы 1 :*"; 
      ham+=("%d",humid_1); 
      ham+="*%*"; 
      bot.sendMessage(ham,msg.chatID); 
      ham = "*Влажность почвы 2 :*"; 
      ham+=("%d",humid_2); 
      ham+="*%*"; 
      sending(ham,msg.chatID); 
    } 
    if (msg.data == "air info") { 
      String t = "*Температура воздуха:*"; 
      t+=("%d",air_temp); 
      t+="*°С*";  
      bot.sendMessage(t,msg.chatID); 
      String h = "*Влажность воздуха:*"; 
      h+=("%d",air_humid); 
      h+="*%*"; 
      sending(h,msg.chatID);
    }
    if (msg.data == "water info") { 
      if(state_water == 0){
        sending("Вода чистая",msg.chatID); 
      }else{
        sending("Вода мутная",msg.chatID); 
      }
    }
    if (msg.data == "test") { 
      sending("Тест устройства начат",msg.chatID); 
    } 
// НАЗАД
    if (msg.data == "Back" && (depth == 1 || depth == 2)) { 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.editMenu(menuID, menu); 
      depth = 0; 
    } 
    if ((msg.data == "Back" && depth == 4)||(msg.data == "Back" && depth == 3)) { 
      String menu = F("auto water \n fertilizer \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 1;
    } 
    if (msg.data == "Back" && depth == 7) { 
      String menu = F("led \t feed now \n time \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 1; 
    }
    if (msg.data == "Back" && depth == 8) { 
      String menu = F("led \t feed now \n time \n Back"); 
      bot.editMenu(menuID, menu); 
      depth = 2;
    }
    
}

void newTime_cage(FB_msg& msg){ 
  if (msg.text == "cansel") { 
    bot.attach(newMsg); 
  }else{ 
    if (times_hams[0] == "-"){ 
      bot.deleteMessage(menuID); 
      times_hams[0] = msg.text; 
      bot.sendMessage("Время добавлено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    }else if (times_hams[1] == "-"){ 
      bot.deleteMessage(menuID); 
      times_hams[1] = msg.text; 
      bot.sendMessage("Время добавлено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    }else if (times_hams[2] == "-"){ 
      bot.deleteMessage(menuID); 
      times_hams[2] = msg.text;   
      bot.sendMessage("Время добавлено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    }else{ 
      bot.deleteMessage(menuID); 
      bot.sendMessage("Ошибка!",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    } 
     
  } 
} 

void newTime_fish(FB_msg& msg){ 
  if (msg.text == "cansel") { 
    bot.attach(newMsg); 
  }else{ 
    if (times_fish[0] == "-"){ 
      bot.deleteMessage(menuID); 
      times_fish[0] = msg.text; 
      bot.sendMessage("Время добавлено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    }else if (times_fish[1] == "-"){ 
      bot.deleteMessage(menuID); 
      times_fish[1] = msg.text; 
      bot.sendMessage("Время добавлено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    }else if (times_fish[2] == "-"){ 
      bot.deleteMessage(menuID); 
      times_fish[2] = msg.text; 
      bot.sendMessage("Время добавлено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    }else{ 
      bot.deleteMessage(menuID); 
      bot.sendMessage("Ошибка!",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    } 
     
  } 
} 

void delTime_cage(FB_msg& msg){ 
  if (msg.text == "cansel") { 
    bot.attach(newMsg); 
  }else{ 
    if(msg.text == "1"){ 
      bot.deleteMessage(menuID); 
      times_hams[0] ="-"; 
      bot.sendMessage("Время удалено",msg.chatID); 
      bot.attach(newMsg); 
      
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    } 
    else if(msg.text == "2"){ 
      bot.deleteMessage(menuID); 
      times_hams[1] ="-"; 
      bot.sendMessage("Время удалено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    } 
    else if(msg.text == "3"){ 
      bot.deleteMessage(menuID); 
      times_hams[2] ="-"; 
      bot.sendMessage("Время удалено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    } 
    else{ 
      bot.deleteMessage(menuID); 
      bot.sendMessage("Ошибка!",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info ");   
      bot.inlineMenu("*Выберете команду*", menu);
      menuID = bot.lastBotMsg(); 
    } 
  } 
} 

void delTime_fish(FB_msg& msg){ 
  if (msg.text == "cansel") { 
    bot.attach(newMsg); 
  }else{ 
    if(msg.text == "1"){ 
      bot.deleteMessage(menuID); 
      times_fish[0] ="-"; 
      bot.sendMessage("Время удалено",msg.chatID); 
      bot.attach(newMsg); 
      
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    } 
    else if(msg.text == "2"){ 
      bot.deleteMessage(menuID); 
      times_fish[1] ="-"; 
      bot.sendMessage("Время удалено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    } 
    else if(msg.text == "3"){ 
      bot.deleteMessage(menuID); 
      times_fish[2] ="-"; 
      bot.sendMessage("Время удалено",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info "); 
      bot.inlineMenu("*Выберете команду*", menu); 
      menuID = bot.lastBotMsg(); 
    } 
    else{ 
      bot.deleteMessage(menuID); 
      bot.sendMessage("Ошибка!",msg.chatID); 
      bot.attach(newMsg); 
      String menu = F("cage \t garden \n aqua \t info ");   
      bot.inlineMenu("*Выберете команду*", menu);
      menuID = bot.lastBotMsg(); 
    } 
  } 
} 

void loop() {
  bot.tick();
  if((auto_felt == true) && (t.dayWeek == felt_date.dayWeek) && (t.day != felt_date.day)){
    felting();
    felt_date = t;
  }
  state_water = digitalRead(water_sens);
  if(state_water == 1 and ( millis() - waitTime_send > INTERVAL_send)){
    bot.sendMessage("Вода стала грязной",CHAT_ID);
    waitTime_send = millis();
  }
  t = bot.getTime(3); 
  for(int i = 0;i<3;i++){ 
    if(times_hams[i]==t.timeString()){ 
      transmit("feed hamster");
      bot.sendMessage("Хомячок покормлен",CHAT_ID);
    }
    if(times_fish[i]==t.timeString()){ 
      transmit("feed fish 1");
      bot.sendMessage("Рыбка 1 покормлена",CHAT_ID);
      delay(15000);
      transmit("feed fish 2");
      bot.sendMessage("Рыбка 2 покормлена",CHAT_ID);
    } 
  } 
  //if(led_aqua == 1){
  //  for(int i = 0; i < LED_NUM_f ; i++) {
  //    leds_f[i] = CRGB::White;
  //    FastLED.show();
  //  }
  //}else{
  //  for(int i = 0; i < LED_NUM_f ; i++) {
  //    leds_f[i] = CRGB::Black;
  //    FastLED.show();
  //  }
  //}
  //if(led_ham == 1){
    // for(int i = 0; i < LED_NUM_h ; i++) {
    //    leds_h[i] = CRGB::White;
    //    FastLED.show();
    //  }
  //}else{
  //  for(int i = 0; i < LED_NUM_h ; i++) {
  //    leds_h[i] = CRGB::Black;
  //    FastLED.show();
  //  }
  //}
} 
