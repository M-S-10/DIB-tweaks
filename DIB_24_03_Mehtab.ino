
#include <AccelStepper.h>


const int override_key_manual_pin = 8;
const int override_key_auto_pin = 9;
const int man_centering_opening_pin = 4;
const int man_centering_closing_pin = 5;
const int man_door_opening_pin = 6;
const int man_door_closing_pin = 7;
const int PWM_Input_Pin = 18;
const int emergency_pin=3;

//limit switches
const byte swxo = 43;
const byte swxc = 42;
const byte swyo = 41;
const byte swyc = 40;
const byte swd1o = 38;
const byte swd1c = 39;
const byte swd2o = 36;
const byte swd2c =37;

const byte x_step = 44;//44
const byte x_dir =  47;//47
const byte y_step = 45;//45
const byte y_dir =  48;//48
const byte d1_step = 46;
const byte d1_dir = 35;
const byte d2_step = 2;
const byte d2_dir = 34;

//enable pins
const byte d1_ena = 29;
const byte d2_ena = 33;
const byte y_ena = 13;
const byte x_ena = 12;

bool manual_mode_flag = false;
bool auto_mode_flag = false;
bool logging =  false;

//flags
volatile bool initiate_closing_sequence_flag = false;
volatile bool initiate_opening_sequence_flag = false;
volatile bool initiate_centering_closing_sequence_flag = false;//not in use yet
volatile bool initiate_centerting_opening_sequence_flag = false;//not in use yet

volatile bool centering_opening_sequence_flag =false;
volatile bool centering_closing_sequence_flag =false;
volatile bool door_opening_sequence_flag=false;
volatile bool door_closing_sequence_flag =false;

volatile bool centering_closing_finish =false; 
volatile bool centering_opening_finish =false;
volatile bool door_opening_finish=false;
volatile bool door_closing_finish=false;

volatile unsigned long pulse_change_start_time = 0;
volatile unsigned long time_difference = 0;



AccelStepper x_step_motor(AccelStepper::DRIVER, x_step,x_dir);
AccelStepper y_step_motor(AccelStepper::DRIVER, y_step,y_dir);
AccelStepper d1_step_motor(AccelStepper::DRIVER, d1_step,d1_dir);
AccelStepper d2_step_motor(AccelStepper::DRIVER, d2_step,d2_dir);
bool x_stop_motor_flag = false;
bool y_stop_motor_flag = false;

bool del_d2=false;


void setup()
{
   Serial.begin(9600);
   Serial.println("start");
 // attachInterrupt(digitalPinToInterrupt(pwmInputPin), ISR_func_pwm, CHANGE);
  attachInterrupt(digitalPinToInterrupt(emergency_pin), ISR_PWM_Emergency, HIGH);

pinMode(override_key_manual_pin,INPUT);
pinMode(override_key_auto_pin,INPUT);
pinMode(man_centering_opening_pin,INPUT);
pinMode(man_centering_closing_pin,INPUT);
pinMode(man_door_opening_pin,INPUT);
pinMode(man_door_closing_pin,INPUT);



//inputs:
//limit switches
pinMode(swxo,INPUT);
pinMode(swxc,INPUT);
pinMode(swyo,INPUT);
pinMode(swyc,INPUT);
pinMode(swd1o,INPUT);
pinMode(swd1c,INPUT);
pinMode(swd2o,INPUT);
pinMode(swd2c,INPUT);

//driver enable pins
pinMode (d1_ena,OUTPUT);
pinMode (d2_ena,OUTPUT);
digitalWrite(d1_ena, LOW);
digitalWrite(d2_ena, LOW);
pinMode (x_ena, OUTPUT);
pinMode (y_ena, OUTPUT);
digitalWrite(x_ena, LOW);
digitalWrite(y_ena, LOW);


pinMode(x_step, OUTPUT);
pinMode(x_dir, OUTPUT);
pinMode(y_step, OUTPUT);
pinMode(y_dir, OUTPUT);
pinMode(d1_step, OUTPUT);
pinMode(d1_dir, OUTPUT);
pinMode(d2_step, OUTPUT);
pinMode(d2_dir, OUTPUT);

  x_step_motor.setMaxSpeed(1800.0);
  x_step_motor.setAcceleration(200.0);
  y_step_motor.setMaxSpeed(1800.0);
  y_step_motor.setAcceleration(200.0);
  d1_step_motor.setMaxSpeed(5000.0);
  d1_step_motor.setAcceleration(200.0);
  d2_step_motor.setMaxSpeed(5000.0);
  d2_step_motor.setAcceleration(200.0);
delay(100);
}



void loop() {
  // constantly sensing for the pwm signal from pixhawk. If proper singal of less than 10% duty cycle is received,
  // we take necessary step for closing/opening of the centering.
  // detaching the interrupt while closing to ignore all the signals during operation of the motor.

//override_key_auto_pin and override_key_manual_pin are active low signals
if (digitalRead(override_key_auto_pin) == HIGH)
{
    if (logging)
    {
        Serial.println("auto mode");
        delay(10);
    }
//add debounce if required
    //detach manual interrupt pin
    manual_mode_flag = false;
    auto_mode_flag = true;
}

else if(digitalRead(override_key_manual_pin) == HIGH){
    //attach manual inteerupt pin

        if (logging)
    {
        Serial.println("manual mode");
        delay(10);
    }
    manual_mode_flag = true;
    auto_mode_flag = false;
}

else
{
    manual_mode_flag = false;
    auto_mode_flag = false;
   centering_closing_finish = false;
   centering_opening_finish =false;
   door_opening_finish = false;
     door_closing_finish =false;



}




if (manual_mode_flag)
{



    if(digitalRead(man_centering_opening_pin) == HIGH)
    {
        
        if (logging)
    {
        Serial.println("manual cenetring opening initiated");
        delay(10);
    }
    if(centering_opening_finish == false)
    {
        digitalWrite(x_ena, LOW);
        digitalWrite(y_ena, LOW);
        centering_opening_sequence_flag= true;
        initiate_centering_opening_sequence();
    }
    }

    else if (digitalRead(man_centering_closing_pin) == HIGH)
    {
        if (logging)
        {
            Serial.println("manual cenetring closing initiated");
            delay(10);
        }
        if (centering_closing_finish == false)
        {
            digitalWrite(x_ena, LOW);
            digitalWrite(y_ena, LOW);
            centering_closing_sequence_flag= true;
            initiate_centering_closing_sequence();    
        }    
    }

    else{
            centering_closing_finish=false;
            centering_opening_finish =false;
            digitalWrite(x_ena, HIGH);
            digitalWrite(y_ena, HIGH);
        }

    if (digitalRead(man_door_opening_pin) == HIGH)
    {
              if (logging)
        {
            Serial.println("manual door opening initiated");
        }
          if(door_opening_finish == false)
          {
            digitalWrite(d1_ena, LOW);
            digitalWrite(d2_ena, LOW);
            door_opening_sequence_flag=true;
           // Serial.print("door_opening_finish ");
           // Serial.println(door_opening_finish);
            initiate_door_opening();
          }
    }

    else if (digitalRead(man_door_closing_pin) == HIGH)
    {
          if (logging)
    {
        Serial.println("manual door closing initiated");
        delay(10);
    }
    if(door_closing_finish==false)
    {
        digitalWrite(d1_ena, LOW);
        digitalWrite(d2_ena, LOW);
        door_closing_sequence_flag =true;
        initiate_door_closing();
    }
    }

    else{
                 if(logging)
        {
            Serial.println("idle_manual_mode");
        }
          //Serial.print("door_opening_finish ");
         // Serial.println(door_opening_finish);
        door_opening_finish = false;
        door_closing_finish =false;
        digitalWrite(d1_ena, HIGH);
        digitalWrite(d2_ena, HIGH);
    }
    
}

else if (auto_mode_flag)
{
//the open and close seq flags are set via an interrupt function
    if (initiate_opening_sequence_flag)
    {
        initiate_opening_sequence_flag=false;
        centering_closing_sequence_flag=true;
        //detach interrupt
        initiate_door_opening();
        delay(50);
        initiate_centering_opening_sequence();
        //attach interrupt
    }

    else if(initiate_closing_sequence_flag)
    {
        initiate_closing_sequence_flag=false;
        centering_closing_sequence_flag=true;
        //detach interrupt
        initiate_centering_closing_sequence();
        delay(50);
        initiate_door_closing(); 
        //attach interrupt
    }

    else{
        if(logging)
        {
            Serial.println("idle_auto_mode");
        }

    }
}


  

}


void ISR_PWM_Emergency()
{
    //reset all flags
    flag_initial_states();
    //motor speeds to zero
//    while(digitalRead(emergency_pin))
//    {
//        if(logging)
//        {
//            Serial.println("in emgergency");
//            delay(100);
//        }
//    }
}


void initiate_door_closing()
{
  del_d2 =false;
bool d1_close = false;
  bool d2_close = false;
    
  d1_step_motor.setSpeed(-2500);
  d2_step_motor.setSpeed(-3500);
//digitalWrite(d1_ena,HIGH);
//digitalWrite(d2_ena,HIGH);
  while (door_closing_sequence_flag == true) {
            if(digitalRead(man_door_closing_pin) == LOW)
    {
      break;
    }
 //   if (digitalRead(swd1c) == LOW && (!d1_close)) {
    if  (!d1_close) {
      if (logging) {
        Serial.println("Initiating d1 closing sequence");
        Serial.println(digitalRead(swd1c));
      }
      // while(1) {//while (x_stop_motor_flag == false) {
      int i = 0;
      if (digitalRead(swd1c) == LOW) {
        for (i=0;i<5;i++) {
          if (digitalRead(swd1c) ==HIGH) {
            break;
          }
          delay(1);
        }
      }
      if (i != 5) {
        d1_step_motor.runSpeed();
      }
      else {
        d1_close = true;
        d1_step_motor.setSpeed(0);
      }
      
      // }
      // detachInterrupt(digitalPinToInterrupt(swxo));
    }

    if (!d2_close) {
      if (logging) {
        Serial.println("Initiating d2 closing sequence");
        Serial.println(time_difference);
      }
      // while (1) {//while (y_stop_motor_flag == false) {
      int i = 0;
      if (digitalRead(swd2c) == LOW) {
        for (i=0;i<5;i++) {
          if (digitalRead(swd2c) == HIGH) {
            break;
          }
          delay(1);
        }
      }

      if (i != 5) {
        d2_step_motor.runSpeed();
      }
      else {
        d2_close = true;
        d2_step_motor.setSpeed(0);
      }
        
      // }
      // detachInterrupt(digitalPinToInterrupt(swyo));
    }
    if(d1_close == true)
    {
      if(d2_close == true)
      {
        door_closing_sequence_flag=false;
        door_closing_finish =true;
      }
    }
   /// delay(10);
  }
  
  d1_step_motor.setSpeed(0);
  d1_step_motor.setSpeed(0);
  door_closing_sequence_flag=false;
  door_closing_finish =true;
  digitalWrite(d1_ena, HIGH);
  digitalWrite(d2_ena, HIGH);

  }
  




void initiate_door_opening()
{
   bool d1_open = false;
  bool d2_open = false;
  //digitalWrite(d1_ena,HIGH);
//digitalWrite(d2_ena,HIGH);
  d1_step_motor.setSpeed(3000);//200
  d2_step_motor.setSpeed(2500);

  

  while (door_opening_sequence_flag == true) {
            if(digitalRead(man_door_opening_pin) == LOW)
    {
      break;
    }
//the limit switch is active high and considreded to be low.
 //   if (digitalRead(swxo) == LOW && (!x_open)) {
     if  (!d1_open) {
      if (logging) {
        Serial.println("Initiating d1 opening sequence");
        Serial.println(time_difference);
      }
      int i = 0;
      if (digitalRead(swd1o) == LOW) {
        for (i=0;i<5;i++) {
          if (digitalRead(swd1o) == HIGH) {
            break;
          }
          delay(1);
        }
      }
      if (i != 5) {
        d1_step_motor.runSpeed();
      }
      else {
        d1_open = true;
        d1_step_motor.setSpeed(0);
      }
      
      // }
      // detachInterrupt(digitalPinToInterrupt(swxo));
    }

    if (!d2_open) {
      // if(!del_d2){
      //   delay(1000);
      // }
      del_d2=true;
      if (logging) {
        Serial.println("Initiating d2 opening sequence");
        Serial.println(time_difference);
      }
      // while (1) {//while (y_stop_motor_flag == false) {
      int i = 0;
      if (digitalRead(swd2o) == LOW) {
        for (i=0;i<5;i++) {
          if (digitalRead(swd2o) == HIGH) {
            break;
          }
          delay(1);
        }
      }

      if (i != 5) {
        d2_step_motor.runSpeed();
      }
      else {
        d2_open = true;
        d2_step_motor.setSpeed(0);
      }
        
      // }
      // detachInterrupt(digitalPinToInterrupt(swyo));
    }
    if(d1_open)
    {
      if(d2_open)
      {
        door_opening_sequence_flag=false;
        door_opening_finish =true;
      }
    }
    ////delay(10);
  }
  d1_step_motor.setSpeed(0);
  d1_step_motor.setSpeed(0);
  door_opening_sequence_flag=false;
  door_opening_finish =true;
  digitalWrite(d1_ena, HIGH);
  digitalWrite(d2_ena, HIGH);
}

void initiate_centering_opening_sequence()
{
  bool x_open = false;
  bool y_open = false;
    
  x_step_motor.setSpeed(-800);
  y_step_motor.setSpeed(-800);

  while (centering_opening_sequence_flag == true) {
//        if(digitalRead(man_centering_opening_pin) == LOW)
//    {
//      break;
//    }
//the limit switch is active high and considreded to be low.
 //   if (digitalRead(swxo) == LOW && (!x_open)) {
    if  (!x_open) {
      if (logging) {
        Serial.println("Initiating x opening sequence");
        Serial.println(time_difference);
      }
      // while(1) {//while (x_stop_motor_flag == false) {
      int i = 0;
      if (digitalRead(swxo) == HIGH) {
        for (i=0;i<5;i++) {
          if (digitalRead(swxo) == LOW) {
            break;
          }
          delay(1);
        }
      }
      if (i != 5) {
        x_step_motor.runSpeed();
      }
      else {
        x_open = true;
        x_step_motor.setSpeed(0);
      }
      
      // }
      // detachInterrupt(digitalPinToInterrupt(swxo));
      x_stop_motor_flag = false;
    }

    if (!y_open) {
      if (logging) {
        Serial.println("Initiating y opening sequence");
        Serial.println(time_difference);
      }
      // while (1) {//while (y_stop_motor_flag == false) {
      int i = 0;
      if (digitalRead(swyo) == HIGH) {
        for (i=0;i<5;i++) {
          if (digitalRead(swyo) == LOW) {
            break;
          }
          delay(1);
        }
      }

      if (i != 5) {
        y_step_motor.runSpeed();
      }
      else {
        y_open = true;
        y_step_motor.setSpeed(0);
      }
        
      // }
      // detachInterrupt(digitalPinToInterrupt(swyo));
      y_stop_motor_flag = false;
    }
    if(x_open)
    {
      if(y_open){
    
        centering_opening_sequence_flag=false;
        centering_opening_finish =true;
      
    }}

  }
  x_step_motor.setSpeed(0);
  y_step_motor.setSpeed(0);
  centering_opening_sequence_flag=false;
  centering_opening_finish =true;
  digitalWrite(x_ena, HIGH);
  digitalWrite(y_ena, HIGH);
}

void initiate_centering_closing_sequence()
{
if (logging) {
    Serial.println("Initiating x closing sequence");
    Serial.println(time_difference);
  }
  x_step_motor.setSpeed(800);
  delay(50);
  while (centering_closing_sequence_flag == true) {//while (x_stop_motor_flag == false) {
//  if(digitalRead(man_centering_closing_pin) == LOW)
//    {
//      break;
//    }
    
    if(logging)
    {
      Serial.println("in centering closing loop");
    }
    int i = 0;
    if (digitalRead(swxc) == HIGH) {
      for (i=0;i<5;i++) {
        if (digitalRead(swxc) == LOW) {
          break;
        }
        delay(1);
      }
    }
    if (i == 5) {
      break;
    }
    x_step_motor.runSpeed();
  }
  x_step_motor.setSpeed(0);
  // detachInterrupt(digitalPinToInterrupt(swxc));
  x_stop_motor_flag = false;


  if (logging) {
    Serial.println("Initiating y closing sequence");
    Serial.println(time_difference);
  }
  y_step_motor.setSpeed(800);
  // attachInterrupt(digitalPinToInterrupt(swyc), ISR_func_swy2, HIGH);
  delay(50);
  while(centering_closing_sequence_flag == true) {//while (y_stop_motor_flag == false) {
//
//            if(digitalRead(man_centering_opening_pin) == LOW)
//    {
//      break;
//    }
    int i = 0;
    if (digitalRead(swyc) == HIGH) {
      for (i=0;i<5;i++) {
        if (digitalRead(swyc) == LOW) {
          break;
        }
        delay(1);
      }
    }
    if (i == 5) {
      break;
    }
    y_step_motor.runSpeed();

    
  }
  y_step_motor.setSpeed(0);
  // detachInterrupt(digitalPinToInterrupt(swyc));  
  y_stop_motor_flag = false;
  centering_closing_sequence_flag=false;
  centering_closing_finish=true;
  digitalWrite(x_ena, HIGH);
  digitalWrite(y_ena, HIGH);
}

void ISR_func_pwm() {
  unsigned long pulse_change_current_time = micros();
  time_difference = pulse_change_current_time - pulse_change_start_time;

  if (time_difference < 2500 && time_difference > 800) {
    if (time_difference < 1500) {
      initiate_closing_sequence_flag = true;
    } 
    else {
      initiate_opening_sequence_flag = true;
    }
  }
  pulse_change_start_time = micros();
}

void flag_initial_states()
{
    centering_opening_sequence_flag=false;
    centering_closing_sequence_flag=false;
    door_opening_sequence_flag=false;
    door_closing_sequence_flag =false;
    centering_closing_finish=false;
}
