#ifndef CHASSIS_HPP_
#define CHASSIS_HPP_

#include<math.h>
#include "Arduino.h"
#include "EMMC42V5.hpp"

HardwareSerial motor_ser(1);
EMMC42V5 motor_frount_left(&motor_ser, 1);
EMMC42V5 motor_frount_right(&motor_ser, 2);
EMMC42V5 motor_back_left(&motor_ser, 3);
EMMC42V5 motor_back_right(&motor_ser, 4);

#define CHASSIS_LONG 75
#define CHASSIS_WHITE 102
#define PI 3.1415926
#define WHEEL_RADIUS 37.5

/*
底盘初始化
*/
void Chassis_Enable(bool _i){
  int acd=motor_frount_left.enable(_i);
    Serial.println(acd);
  motor_frount_right.enable(_i);
  motor_back_left.enable(_i);
  motor_back_right.enable(_i);

  motor_frount_left.angle_reset();
  motor_frount_right.angle_reset();  
  motor_back_left.angle_reset();
  motor_back_right.angle_reset();    
}

float fand_max(float a, float b, float c, float d) {
    float max1, max2;
    a = fabs(a);
    b = fabs(b);
    c = fabs(c);
    d = fabs(d);

    a > b ? max1 = a : max1 = b;
    c > d ? max2 = c : max2 = d;
    return (max1 > max2) ? max1 : max2;   
}
/*
增量式位置模式控制
平移单位是mm
角度单位是度
*/
void Chassis_Incremental_Position(float position_x, float position_y, float position_z) {
  Chassis_Enable(true);
  float _pulse_x, _pulse_y, _pulse_z;
  float _motor_frount_left_pulse, _motor_frount_right_pulse, _motor_back_left_pulse, _motor_back_right_pulse;

  int _speed = 250;
  int _acc = 100;

  float _wait_time = 0.0,_p = 0.0;
  _pulse_x = position_x * 3200.0 / (PI * WHEEL_RADIUS * 2.0);
  _pulse_y = -(position_y * 3200.0 / (PI * WHEEL_RADIUS * 2.0));
  // USBSerial.print("pulse_z:");
  // USBSerial.println(position_z);

  _pulse_z = -((3200.0*position_z*sqrt(CHASSIS_LONG*CHASSIS_LONG+CHASSIS_WHITE*CHASSIS_WHITE)*sqrt(2.0))/(360*WHEEL_RADIUS*2));//sqrt
  // USBSerial.println(_pulse_z);
  _motor_frount_left_pulse = -(-_pulse_y - _pulse_x - _pulse_z );
  _motor_frount_right_pulse = -(-_pulse_y + _pulse_x - _pulse_z );
  _motor_back_left_pulse = -_pulse_y + _pulse_x + _pulse_z ;
  _motor_back_right_pulse = -_pulse_y - _pulse_x + _pulse_z ;

  motor_frount_left.pulse_control(_motor_frount_left_pulse, _speed,_acc,0,1);
  motor_frount_right.pulse_control(_motor_frount_right_pulse, _speed,_acc,0,1);
  motor_back_left.pulse_control(_motor_back_left_pulse , _speed,_acc,0,1);
  motor_back_right.pulse_control(_motor_back_right_pulse, _speed,_acc,0,1);
  motor_frount_left.sync();


  _p = fand_max(_motor_frount_left_pulse, _motor_frount_right_pulse, _motor_back_left_pulse, _motor_back_right_pulse);


  delay(abs(_p)/3200/_speed*60*1000+(256-_acc)*0.05*_speed);
  int64_t X1_pusle=motor_frount_left.read_input_pulses();
  Serial.println(X1_pusle);

  // USBSerial.println("end");
  //t = p/132)*0.2*0.22
  //2-(p/13200)
}

#endif
