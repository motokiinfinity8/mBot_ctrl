#include <Arduino.h>
#include <Wire.h>
#include <SoftwareSerial.h>

#include <MeMCore.h>

#define M_SPEED_L 6
#define M_DIR_L 7
#define M_SPEED_R 5
#define M_DIR_R 4
#define SW_UART_TX 11
#define SW_UART_RX 12

#define CMD_SIZE  12
#define STEP_PWM_1MS   80  //1秒ごとに増加するPWM値

SoftwareSerial softSerial(SW_UART_RX, SW_UART_TX);

double angle_rad = PI/180.0;
double angle_deg = 180.0/PI;

//char s_buf[30];   // 文字列格納用
String recv_buffer;//
int i = 0;        // 文字数のカウンタ
bool inst_flg = 0;

String cmd_msg, vel_msg;
int vel_linear=0, vel_argular=0;
int motor_inst, motor_state ;
int running_mode; //0: 事前通知モード、1:事前予告なし(通常)モード
int cmd_start_time = 0;


MeBuzzer buzzer;
MeRGBLed rgbled_7(7, 7==7?2:4);
MeDCMotor motor_9(9);
MeDCMotor motor_10(10);
int Speed = 0;

void setup(){
  Serial.begin(19200);
  softSerial.begin(9600);
  motor_inst = 0;
  motor_state = -3;
  running_mode = 0;
  recv_buffer = "";
}

void loop(){
loop_start: 
  String cmdmsg = "";

  // Serial受信(内部バッファへのコピー)
  rcv_buf_update();
  
  // Serialバッファからコマンド取得
  cmdmsg = getCmdmsg();
  if(cmdmsg != ""){
    //Serial.print(cmdmsg);
    cmd_msg = (cmdmsg.substring(0,3));
    vel_msg = (cmdmsg.substring(3.7));
    running_mode = (cmdmsg.substring(7,8)).toInt();
    Serial.print(cmd_msg);
    Serial.print(" ");
    Serial.print(vel_msg);
    Serial.print(" ");
  
    // MDC (Mobility Direction Control)の場合
    int motor_inst_tmp = motor_inst;
          
    if(cmd_msg.equals("MDC")){
        if((vel_msg.substring(0,2)).equals("-9")) motor_inst = -4;
        if((vel_msg.substring(0,2)).equals("+0")) motor_inst =  0;
        if((vel_msg.substring(0,2)).equals("+9")) motor_inst =  4;

        if((vel_msg.substring(2,4)).equals("-9")) motor_inst -= 1;
        if((vel_msg.substring(2,4)).equals("+9")) motor_inst += 1;
    }

    // レバー操作のチャタ防止対策 2回連続で同じコマンドの場合のみ有効にする
    if(motor_inst_tmp != motor_inst && motor_inst != 0 && running_mode == 0 ){
         delay(100);
         goto loop_start;
    }

    Serial.print("Motor Instructiont:"); 
    Serial.println(motor_inst);   
  }

  //モータ制御
  if(Time_Mesure(cmd_start_time) < 1000){
    Speed = 96;
  }else{
    Speed = (int)(96 + STEP_PWM_1MS * (float)(Time_Mesure(cmd_start_time)-1000) / 1000);
  }

  if(running_mode ==2){
    Speed = 96;
  }

  if(Speed > 255) Speed = 255;

  Serial.print("  ");
  Serial.println(Time_Mesure(cmd_start_time));
  Serial.print("  ");
  Serial.println(Speed);

  // Motor制御命令に変更があった場合に停止 & 音源再生
  if((motor_inst != motor_state) && motor_inst != 0 && running_mode == 0){
    Serial.println("Motor Instruction input!"); 
    // Motor停止
    motor_9.run(0);
    motor_10.run(0);
     switch(motor_inst){
          case  -5: //右後ろ
            rgbled_7.setColor(1,255,0,0); //赤色
            rgbled_7.setColor(2,0,0,0); //無色
            rgbled_7.show(); 
            buzzer.tone(392, 500);   //G4音階を500ms
            break;
          case -4:  //後ろ
            rgbled_7.setColor(0,255,0,0); //赤色
            rgbled_7.show();
            buzzer.tone(349, 500);   //F4音階を500ms
            break;
          case  -3: //左後ろ
            rgbled_7.setColor(1,0,0,0); //無色
            rgbled_7.setColor(2,255,0,0); //赤色
            rgbled_7.show(); 
            buzzer.tone(330, 500);   //E4音階を500ms
            break;
          case  -1: //右 
           rgbled_7.setColor(1,0,255,255); //水色
           rgbled_7.setColor(2,0,0,0); //無色
           rgbled_7.show();
           buzzer.tone(659, 500);   //E5音階を500ms 
           break;
          case  1:  //左
           rgbled_7.setColor(1,0,0,0); //無色
           rgbled_7.setColor(2,0,255,0); //緑色
           rgbled_7.show();  
           buzzer.tone(440, 500);   //A4音階を500ms  
           break;
          case  3:  //右前 
           rgbled_7.setColor(1,255,255,0); //黄色
           rgbled_7.setColor(2,0,0,0); //無色
           rgbled_7.show(); 
           buzzer.tone(587, 500);   //D5音階を500ms
           break;
          case  4:  //前
           rgbled_7.setColor(0,255,255,0); //黄色
           rgbled_7.show();  
           buzzer.tone(523, 500);   //C5音階を500ms 
           break;
          case  5:  //左前
           rgbled_7.setColor(1,0,0,0); //無色
           rgbled_7.setColor(2,255,255,0); //黄色
           rgbled_7.show(); 
           buzzer.tone(494, 500);   //B4音階を500ms 
           break;
        }  
  }


  
  int leftSpeed, rightSpeed;
  switch(motor_inst){
      case  -5: //右後ろ
        leftSpeed = -1*Speed;
        rightSpeed = -1*Speed/2;
        rgbled_7.setColor(1,255,0,0); //赤色
        rgbled_7.setColor(2,0,0,0); //無色
        rgbled_7.show();       
        break;
      case -4:  //後ろ
        leftSpeed = -1*Speed;
        rightSpeed = -1*Speed;
        rgbled_7.setColor(0,255,0,0); //赤色
        rgbled_7.show();  
        break;
      case  -3: //左後ろ 
        leftSpeed = -1*Speed/2;
        rightSpeed = -1*Speed;
        rgbled_7.setColor(1,0,0,0); //無色
        rgbled_7.setColor(2,255,0,0); //赤色
        rgbled_7.show();  
        break;
      case  -1: //右
        leftSpeed = Speed;
        rightSpeed = -1*Speed;
        rgbled_7.setColor(1,0,255,255); //水色
        rgbled_7.setColor(2,0,0,0); //無色
        rgbled_7.show();  
        break;
      case  1:  //左
        leftSpeed = -1*Speed;
        rightSpeed = Speed;
        rgbled_7.setColor(1,0,0,0); //無色
        rgbled_7.setColor(2,0,255,0); //緑色
        rgbled_7.show();  
       break;
      case  3:  //右前
        leftSpeed = Speed;
        rightSpeed = Speed/2;
        rgbled_7.setColor(1,255,255,0); //黄色
        rgbled_7.setColor(2,0,0,0); //無色
        rgbled_7.show();  
        break;
     case  4:  //前
        leftSpeed = Speed;
        rightSpeed = Speed;
        rgbled_7.setColor(0,255,255,0); //黄色
        rgbled_7.show();  
        break;
      case  5:  //左前
        leftSpeed = Speed/2;
        rightSpeed = Speed;
        rgbled_7.setColor(1,0,0,0); //無色
        rgbled_7.setColor(2,255,255,0); //黄色
        rgbled_7.show();  
        break;
      case  0:  //なし
        leftSpeed = 0;
        rightSpeed = 0;
        rgbled_7.setColor(0,0,0,0); //無色
        rgbled_7.show();  
        break;
    }
    motor_9.run((9)==M1?-(leftSpeed):(leftSpeed));
    motor_10.run((10)==M1?-(rightSpeed):(rightSpeed));



  //情報更新
  delay(10);
  if(running_mode != 1){
    if(motor_inst != motor_state || motor_inst ==0 ) cmd_start_time = millis();  // 命令情報が変わった場合、時間リセット
  }else{
    if(abs(motor_inst-motor_state) >1 || motor_inst ==0 ) cmd_start_time = millis();  // 命令情報が前後で変わった場合、時間リセット
  }
  motor_state = motor_inst; //状態更新
    
}

//受信用内部バッファの更新
void rcv_buf_update(){
  int size = softSerial.available();
  char c;
  int i;

  for(i=0; i<size; i++){
    c = softSerial.read();
    recv_buffer += c;
  }
}

//内部バッファからコマンド命令の検索・抽出
String getCmdmsg(){
  String result = "";
  bool incmd = false;
  int start_pos, end_pos = 0;
  int i;
  
  for( i=0; i<recv_buffer.length(); i++){
    if(incmd == false){
      // STX捜索
      if(recv_buffer.charAt(i) == 0x2){
        incmd = true;
        start_pos = i;
      }
    }else{
      // ETX捜索
      if(recv_buffer.charAt(i) == 0x3){
        incmd = false;
        end_pos = i;
        // メッセージ抽出 & メッセージより前のフレーム破棄
        result = recv_buffer.substring( start_pos+1, end_pos);
        recv_buffer = recv_buffer.substring(end_pos+1, recv_buffer.length());
        break;
      }
    }
  }
  return result;
}

int Time_Mesure( int st_time ){
  return millis() - st_time;
}
