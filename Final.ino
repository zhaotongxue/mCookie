//A lot of include............
#include "./audio.h"
#include <SoftwareSerial.h>
#include <Wire.h>
#include <EEPROM.h>
#include "U8glib.h"
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);
#include <ESP8266.h>
#ifdef ESP32
#error "Error"
#endif
#if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1284P__) || defined (__AVR_ATmega644P__) || defined(__AVR_ATmega128RFA1__)
#define EspSerial Serial1
#define UARTSPEED  115200
#endif
#if defined (__AVR_ATmega168__) || defined (__AVR_ATmega328__) || defined (__AVR_ATmega328P__)
#define EspSerial mySerial
#define UARTSPEED  9600
#endif
#define SSID        F("62552757")
#define PASSWORD    F("62552757")
#define HOST_NAME   F("api.heclouds.com")
#define HOST_PORT   (80)
//The pattern of http request text Cost much of my time ಥ_ಥ
//Yes,Http message...
static const byte  GETDATA[]  PROGMEM = {
	"GET https://api.heclouds.com/devices/22834365/datapoints?datastream_id=id,status,vol&limit=1 HTTP/1.1\r\napi-key: zXUxQaupAumEyFfR8rEX7l9c9Yg=\r\nHost: api.heclouds.com\r\nConnection: close\r\n\r\n"
};
ESP8266 wifi(&EspSerial);
//define two variables...
int rollCount=0,rollRoomMembers=0;
//define another three variables...
bool volChange=false,statusChange=false,idChange=false;
int music_status=0,temp_music_status=0;
int music_vol=10,temp_music_vol=10;
int music_num_MAX=9;
int current_music=1,temp_current_music=1;
char* names[]={"BUPT Theme Song-AiYue ","Glad You Come-Boyce Avenue  ","Penumatic Tokyo-Env ","Luv Letter-DJ ","Only My Railgun-None ","Sugar-Kidz Bop Kids ","Unity-The Fat Rat ","Star Sky-Two Steps From Hell ","Schnappi-Joy Gruttmann "};
//I'm lazy,don't know the method of calc the length of char* 
//int name_length[]={22,27,19,14,21,20,18,29,23};
bool isConnected=false;
bool bottomBar=true;
bool canPlay=false;
void drawTitle(){
	//Roll The Name of songs~~
	//Point is one thing that unsafe.
	//Array in C++ is not as easy as it is in java...
	u8g.setPrintPos(4,48);
	/*
		for(int i=0;i<name_length[current_music];i++){
			if(rollCount+i<name_length[current_music]){
				u8g.print(*(names[current_music]+rollCount+i));
			}else{
				u8g.print(*(names[current_music]+rollCount+i-name_length[current_music]));
			}
		}
		*/
	for(int i=0;i<strlen(names[current_music]);i++){
		if(rollCount+i<strlen(names[current_music])){
			u8g.print(*(names[current_music]+rollCount+i));
		}
		else
		{
			u8g.print(*(names[current_music]+rollCount+i-strlen(names[current_music])));
		}

	}
		u8g.print("...");
}
void drawNotConnected(){
	//it will draw like this:✡ (NO BORDER!!)
	u8g.drawTriangle(64,4,32,48,96,48);
	u8g.drawTriangle(64,60,32,16,96,16);
}

bool networkHandle() {
	//do something with net work ,include handle response message.
	canPlay=true;
	uint8_t buffer[415]={0};
	uint32_t len = wifi.recv(buffer, sizeof(buffer), 2000);
	if (len > 0) {
		for (uint32_t i = 0; i < len; i++) {
			Serial.print((char)buffer[i]);
		}
	}
	//the ram of the device is too limited,so i enhered the length of response message,at specific index,there are nessicity value.
//  272，273 vol
//344 id
//414 status
	temp_music_vol=((int)buffer[272]-48)*10+((int)buffer[273]-48)-10;
	temp_current_music=(int)buffer[344]-48;
	temp_music_status=(int)buffer[414]-48;
	Serial.println(temp_music_vol);
	Serial.println(temp_current_music);
	Serial.println(temp_music_status);  
	wifi.releaseTCP();
}
void mp3Handle(){
	//Handle play things.
	if(canPlay){
	if(current_music!=temp_current_music){
		idChange=true;
		current_music=temp_current_music;
	}
	if(music_vol!=temp_music_vol){
		volChange=true;
		music_vol=temp_music_vol;
	}
	if(music_status!=temp_music_status){
		statusChange=true;
		music_status=temp_music_status;
	}
	if(statusChange){
		if(music_status==1){
			audio_play();
		}else{
			audio_pause();
		}
	}
	if(volChange){
		audio_vol(music_vol);
	}
	if(idChange){
		audio_choose(current_music+1);
		audio_play();
	}
	volChange=false;
	idChange=false;
	statusChange=false;
}
}
void setup(void)
{
Serial.begin(115200);
//I don't konw....
  while (!Serial); // wait for Leonardo enumeration, others continue immediately
  Serial.println(F("WIFI start"));
  delay(100);
  WifiInit(EspSerial, UARTSPEED);
  wifi.setOprToStationSoftAP();
  if (wifi.joinAP(SSID, PASSWORD)) {
  	Serial.println(F("WIFI Connect!"));
 	isConnected=true;
  } else {
  	isConnected=false;
  	Serial.println(F("NO WIFI"));
  }
  //what's MUX??
  wifi.disableMUX();
  //initialize  audio module 
  audio_init(DEVICE_TF,MODE_One_END,music_vol);  
}
void drawPlay(){
	u8g.drawBox(4,4,8,16);
	u8g.drawBox(20,4,8,16);
	u8g.setPrintPos(32,16);
	u8g.print("Playing");
}
void drawPause(){
	u8g.drawTriangle(4,4,30,8,4,16);
	u8g.setPrintPos(32,16);
	u8g.print("Paused");
}

void drawVol(){
	u8g.drawLine(4,35,4+30*4,35);
	u8g.drawLine(4+30*4,35,4+30*4,35-30*0.6);
	u8g.drawLine(4,35,4+30*4,35-30*0.6);
	u8g.drawTriangle(4,35,4+music_vol*4,35,4+music_vol*4,35-music_vol*0.6);
}
void drawBottom(){
		u8g.setPrintPos(4, 16 * 4);
		switch(rollRoomMembers){
			case 0:
			u8g.print("****ROOM 324****");
			break;
			case 1:
			u8g.print("Zhao: 2017210672");
			break;
			case 2:
			u8g.print("Ran:   2017210676");
			break;
			case 3:
			u8g.print("Cheng:2017210679");
			break;
	}
}
void drawAll(){
		u8g.setFont(u8g_font_7x13);
		if(music_status==0){
			drawPause();
		}else{
			drawPlay();
		}
		drawVol();
		drawTitle();
		drawBottom();
}
void loop(void)
{
	if(isConnected){
		if (wifi.createTCP(HOST_NAME, HOST_PORT)) {
			Serial.print(F("TCP\n"));
			isConnected=true;
		} else {
			Serial.print(F("No TCP\n"));
			isConnected=true;
		}
		wifi.sendFromFlash(GETDATA, sizeof(GETDATA)); 
		networkHandle();
		mp3Handle();
		u8g.firstPage();
		do {
			drawAll();
		} while (u8g.nextPage());
	}
	else
	{
		u8g.firstPage();
		do{
			drawNotConnected();
		}while(u8g.nextPage());
		delay(5000);
		setup();
	}
	rollCount++;
	rollCount=rollCount%strlen(names[current_music]);
	//show ata lower speed than the name of song.
	if(bottomBar){
		bottomBar=false;
		rollRoomMembers=rollRoomMembers+1;
		rollRoomMembers=rollRoomMembers%4;
	}else{
		bottomBar=true;
	}
}

