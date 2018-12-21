#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
//#include "esp_system.h"
#include <SoftwareSerial.h>

#define SerialPort Serial
#define SW_UART_TX 5
#define SW_UART_RX 4

#define CMD_SIZE  12

SoftwareSerial softSerial(SW_UART_RX, SW_UART_TX);

/** MOPPY モータードライバ制御
  */

//#define LF_MOTER_PIN 2
//#define LB_MOTER_PIN 5
//#define RF_MOTER_PIN 21
//#define RB_MOTER_PIN 22

const char ssid[] = "ESP32_MOBILITY"; // SSID
const char password[] = "esp32_con";  // password
const int WifiPort = 8000;      // ポート番号

const IPAddress HostIP(192, 168, 11, 1);       // IPアドレス
const IPAddress ClientIP(192, 168, 11, 2);       // IPアドレス
const IPAddress subnet(255, 255, 255, 0); // サブネットマスク
const IPAddress gateway(192,168, 11, 0);
const IPAddress dns(192, 168, 11, 0);
bool wifi_connect;

WiFiUDP Udp;
uint8_t WiFibuff[CMD_SIZE];
int motor_inst, motor_state ;
String cmd_msg, vel_msg;
int vel_linear=0, vel_argular=0;
int running_mode; //0: 事前通知モード、1:事前予告なし(通常)モード


void setup() 
{
  //PIN設定
  Serial.begin(9600);
  softSerial.begin(9600);
  motor_inst = 0;
  running_mode = 0;
  
  // Wifi設定 (SSID&パス設定)
  wifi_connect = false;
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  delay(100);
  WiFi.softAPConfig(HostIP, HostIP, subnet); // IPアドレス設定

  Serial.print("AP IP address: ");
  IPAddress myAddress = WiFi.softAPIP();
  Serial.println(myAddress);

  Udp.begin(WifiPort);  // UDP通信開始
  Serial.println("Starting UDP");
  
  Serial.print("Local port: ");
  Serial.println(WifiPort);

}

void loop() {

loop_start:

  int packet_size = Udp.parsePacket();

  if (packet_size > 0) {
    //Serial.print(packet_size);

    //Serial.println("Serial Received");   
    rcvWiFi();    
    if(WiFibuff[0] == 0x2){
          String s_buf = String((char*)WiFibuff);
          //Serial.print(s_buf.substring(1,8));
          //cmd_msg = (s_buf.substring(1,4));
          //vel_msg = (s_buf.substring(4.8));
          //running_mode = (s_buf.substring(8,9)).toInt();
          //Serial.println(s_buf);
          Serial.write(WiFibuff, CMD_SIZE);
          softSerial.write(WiFibuff, CMD_SIZE);
          
          // MVC (Mobility Direction Control)の場合
          //int motor_inst_tmp = motor_inst;
          
          //if(cmd_msg.equals("MDC")){
          //  if((vel_msg.substring(0,2)).equals("-9")) motor_inst = -4;
          //  if((vel_msg.substring(0,2)).equals("+0")) motor_inst =  0;
          //  if((vel_msg.substring(0,2)).equals("+9")) motor_inst =  4;

           // if((vel_msg.substring(2,4)).equals("-9")) motor_inst -= 1;
           // if((vel_msg.substring(2,4)).equals("+9")) motor_inst += 1;
          // }

          // レバー操作のチャタ防止対策 2回連続で同じコマンドの場合のみ有効にする
          //if(motor_inst_tmp != motor_inst && motor_inst != 0 && running_mode == 0 ){
          //   delay(100);
          //   goto loop_start;
          //}
          
          //Serial.print("Motor Instructiont:"); 
          //Serial.println(motor_inst);      
     }
  }

  // Motor制御
//        switch(motor_inst){
//          case  -5: //右後ろ           
//            break;
//          case -4:  //後ろ  
//            break;
//          case  -3: //左後ろ
//            break;
//          case  -1: //右
//            break;
//          case  1:  //左 
//            break;
//          case  3:  //右前
//            break;
//          case  4:  //前
//            break;
//          case  5:  //左前
//            break;
//          case  0:  //なし
//            break;
//        }
//  delay(10);
//  motor_state = motor_inst;
}

void rcvWiFi() {
  Udp.read(WiFibuff, CMD_SIZE);
  //Serial.print(WiFibuff);
  Udp.flush();
}
