//Connecting some libraries
#include <Adafruit_NeoPixel.h>
#include <LiquidCrystal.h>
#include <TroykaDHT.h>

DHT dht(0, DHT21); //Connecting DHT21 temperature and humidity sensor

LiquidCrystal lcd(12, 11, 5, 4, 3, 2); //Connecting 20x4 LCD display

#define PIXEL_PIN    6   //The pin of LED strip
#define PIXEL_COUNT 12   //Count of Leds in the strip. You need to edit this field

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, NEO_RGB + NEO_KHZ800); //Cnnecting LED strip
//Naming pins
#define S1    7
#define S2    8
#define S3    10
#define S4    13
#define BUZZER    9
//Setting variables
String input="No connection";
String estimated="Ip: no data";
String state="";
bool led_blink=false;
uint32_t blink_color;
bool last_blink=false;

void setup() {
  lcd.begin(20, 4); //beginning lcd display
  strip.begin(); //setting up led strip
  strip.show();
  Serial.begin(9600); //beginning serial
  //Pinmode section
  pinMode(S1,INPUT);
  pinMode(S2,INPUT);
  pinMode(S3,INPUT);
  pinMode(S4,INPUT);
  pinMode(BUZZER,INPUT);
  dht.begin(); //beggining dht21 sensor
  lcd.setCursor(5, 0); //printing a hello banner on lcd display
  lcd.print("ralex2304"); //Author's name
  lcd.setCursor(3, 1);
  lcd.print("Anycubic 4MAX");
  lcd.setCursor(5, 2);
  lcd.print("Octoprint");
  lcd.setCursor(3, 3);
  lcd.print("Status monitor");
  delay(1500); //waiting for 1.5 seconds
  String i="";
  lcd.clear(); //clearing the display
  while(!Serial){ //waiting for serial
    i+=".";
    if(i=="...."){
      i="";
    }
    lcd.setCursor(5, 0);
    lcd.print("ralex2304");
    lcd.setCursor(2, 3);
    lcd.print("Waiting serial"+i+"   ");
    if(i==""){
      colorAll(Col(0, 0, 0));
    } else if(i=="."){
      colorAll(Col(0, 255, 0));
    }else if(i==".."){
      colorAll(Col(255, 255, 0));
    }else if(i=="..."){
      colorAll(Col(255, 0, 0));
    }
    delay(1000);
  }
  //Reporting about successful serial connection
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("ralex2304");
  lcd.setCursor(2, 2);
  lcd.print("Serial connected");
  lcd.setCursor(4, 3);
  lcd.print("Waiting data");
  colorAll(Col(0, 0, 0));
  delay(1000);
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("Status:");
  int len=0;
  state=input;
  if(input.length()+(int((20-input.length())/2))*2==19){
    len=int((20-input.length())/2)+1;
  }else{
    len=int((20-input.length())/2);
  }
  for(int i=int((20-input.length())/2);i!=0;i--){
    //Serial.println(i);
    input=" "+input;
  }
  for(int i=len;i!=0;i--){
    //Serial.println(i);
    input=input+" ";
  }
  lcd.setCursor(0, 1);
  lcd.print(input);
  lcd.setCursor(0, 3);
  lcd.print(estimated);
}
//Main loop
void loop() {
  input="";
  if(Serial.available()){
    input=Serial.readStringUntil('\n');  
  }
  if(input=="status"){
    while(!Serial.available()){}
    String input="";
    if(Serial.available()){
      input=Serial.readStringUntil('\n');
    }
    lcd.setCursor(6, 0);
    lcd.print("Status:");
    int len=0;
    state=input;
    if(input.length()+(int((20-input.length())/2))*2==19){
      len=int((20-input.length())/2)+1;
    }else{
      len=int((20-input.length())/2);
    }
    for(int i=int((20-input.length())/2);i!=0;i--){
      input=" "+input;
    }
    for(int i=len;i!=0;i--){
      input=input+" ";
    }
    lcd.setCursor(0, 1);
    lcd.print(input);
    input="";
  } else if(input=="finished"){
    tone(BUZZER,2000,3000);
    for(int i=0;i<5;i++){
      colorAll(Col(0,255,0));
      delay(300);
      colorAll(Col(0,0,0));
      delay(300);
    }
  } else if(input=="estimated"){
    while(!Serial.available()){}
    String input="";
    if(Serial.available()){
      input=Serial.readStringUntil('\n');
    }
    estimated=input;
    lcd.setCursor(0, 3);
    for(int i=20-input.length();i!=0;i--){
      input+=" ";
    }
    lcd.print(input);
    input="";
  }
  led_blink=false;
  //Checking the printer state
  if(state=="No connection"){
    led_blink=true;
    blink_color=Col(255,0,0);
  } else if(state=="Printer is not ok"){
    colorAll(Col(255,0,0));
  } else if(state=="Operational"){
    colorAll(Col(0,255,0));
  }
  else if(state=="offline" || state.indexOf("error")>0){
    led_blink=true;
    blink_color=Col(255,0,0);
  } else{
    if(state=="Paused" || state=="Pausing" || state=="Resuming"){
      led_blink=true;
      blink_color=Col(255,255,0);
    } else{
      colorAll(Col(255,255,0));
    }
  }
  if(led_blink){
    blinking();
  }
  dht.read(); //Reading data from temperature and humidity sensor
  switch(dht.getState()) {
    //All is ok
    case DHT_OK:
      lcd.setCursor(0,2);
      lcd.print("Temp:"+String(dht.getTemperatureC()).substring(0,4)+"C Hum:"+String(dht.getHumidity()).substring(0,4)+"%");
      break;
    //Chechsum error
    case DHT_ERROR_CHECKSUM:
      lcd.setCursor(0,2);
      lcd.print("Temp sensor checksum");
      break;
    //Timeout error
    case DHT_ERROR_TIMEOUT:
      lcd.setCursor(0,2);
      lcd.print("Temp sensor timeout ");
      break;
    //No reply error
    case DHT_ERROR_NO_REPLY:
      lcd.setCursor(0,2);
      lcd.print("Temp sensor not conn");
      break;
  }
  delay(1000);
  if(!digitalRead(S2)){ //detecting a click on a shutdown button
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(" Are you sure you   ");
    lcd.setCursor(0,1);
    lcd.print(" want to shutdown?  ");
    lcd.setCursor(0,3);
    lcd.print("            Yes  No ");
    while(digitalRead(S4)&&digitalRead(S3)){}
    if(!digitalRead(S4)){
      Serial.write("shutdown\n");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("   Shutting down    ");
      lcd.setCursor(0,1);
      lcd.print("    Please wait     ");
      state="Shutting down";
      delay(1000);
    }
    lcd.clear();
    lcd.setCursor(6, 0);
    lcd.print("Status:");
    int len=0;
    input=state;
    if(input.length()+(int((20-input.length())/2))*2==19){
      len=int((20-input.length())/2)+1;
    }else{
      len=int((20-input.length())/2);
    }
    for(int i=int((20-input.length())/2);i!=0;i--){
      //Serial.println(i);
      input=" "+input;
    }
    for(int i=len;i!=0;i--){
      //Serial.println(i);
      input=input+" ";
    }
    lcd.setCursor(0, 1);
    lcd.print(input);
    input="";
    lcd.setCursor(0, 3);
    lcd.print(estimated);
  }
  if(!digitalRead(S1)){ //detecting a click on a cancel button
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("  Are you sure you  ");
    lcd.setCursor(0,1);
    lcd.print("  want to cancel?   ");
    lcd.setCursor(0,3);
    lcd.print("            Yes  No ");
    while(digitalRead(S4)&&digitalRead(S3)){}
    if(!digitalRead(S4)){
      Serial.write("cancel\n");
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("    Cancelling      ");
      lcd.setCursor(0,1);
      lcd.print("    Please wait     ");
      state="Cancelling";
      delay(1000);
    }
    lcd.clear();
    lcd.setCursor(6, 0);
    lcd.print("Status:");
    int len=0;
    input=state;
    if(input.length()+(int((20-input.length())/2))*2==19){
      len=int((20-input.length())/2)+1;
    }else{
      len=int((20-input.length())/2);
    }
    for(int i=int((20-input.length())/2);i!=0;i--){
      //Serial.println(i);
      input=" "+input;
    }
    for(int i=len;i!=0;i--){
      //Serial.println(i);
      input=input+" ";
    }
    lcd.setCursor(0, 1);
    lcd.print(input);
    input="";
    lcd.setCursor(0, 3);
    lcd.print(estimated);
  }
  if(!digitalRead(S4)){ //detecting a click on a resume button
    state="Resuming";
    input=state;
    int len;
    if(input.length()+(int((20-input.length())/2))*2==19){
      len=int((20-input.length())/2)+1;
    }else{
      len=int((20-input.length())/2);
    }
    for(int i=int((20-input.length())/2);i!=0;i--){
      //Serial.println(i);
      input=" "+input;
    }
    for(int i=len;i!=0;i--){
      //Serial.println(i);
      input=input+" ";
    }
    lcd.setCursor(0, 1);
    lcd.print(input);
    input="";
    Serial.write("resume\n");
    while(!digitalRead(S4)){}
  }
  if(!digitalRead(S3)){ //detecting a click on a pause button
    state="Pausing";
    input=state;
    int len;
    if(input.length()+(int((20-input.length())/2))*2==19){
      len=int((20-input.length())/2)+1;
    }else{
      len=int((20-input.length())/2);
    }
    for(int i=int((20-input.length())/2);i!=0;i--){
      //Serial.println(i);
      input=" "+input;
    }
    for(int i=len;i!=0;i--){
      //Serial.println(i);
      input=input+" ";
    }
    lcd.setCursor(0, 1);
    lcd.print(input);
    input="";
    Serial.write("pause\n");
    while(!digitalRead(S3)){}
  }
}
//Led strip blinking function
void blinking(){
  if(last_blink){
    colorAll(Col(0,0,0));
  } else{
    colorAll(blink_color);
  }
  last_blink=!last_blink;
}
//Led strip show function
void colorAll(uint32_t c)
{
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    //Fill current pixel
    strip.setPixelColor(i, c);
  }
  strip.show();
}
//Led strip color generator
uint32_t Col(int r, int g, int b){
  return strip.Color(r,g,b);
}
