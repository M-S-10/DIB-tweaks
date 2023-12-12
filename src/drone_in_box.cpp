/*
Takes in a pwm signal at gpio 33 and based on the signal closes or opens the centering portion of the box.
Gives commands to the stepper motors accordingly for opening/closing the centering.

Author: Ankush Goyal
*/


#include <AccelStepper.h>

bool logging = false;

// motors for the opening and closing of centering
AccelStepper x_step_motor(AccelStepper::DRIVER, 17,15);
AccelStepper y_step_motor(AccelStepper::DRIVER, 16,14);
bool x_stop_motor_flag = false;
bool y_stop_motor_flag = false;

//pwm signal received from pixhawk
const byte pixhawk_input_pin = 13;
volatile unsigned long pulse_change_start_time = 0;
volatile unsigned long time_difference;

//this variable is set true to close/open the centering
volatile bool initiate_closing_sequence_flag = false;
volatile bool initiate_opening_sequence_flag = false;

//limit switches
const byte swx1 = 21;
const byte swx2 = 22;
const byte swy1 = 23;
const byte swy2 = 25;

//ISR function for pwm signal from pixhawk
// void IRAM_ATTR ISR_func_pwm() {
//   /*
// 	Here we are measuring the signal high time and low time. 
// 	The signal received is square wave of 50 Hz i.e. 20ms time period. 
// 	The duty cycle is always less than 10%, that is 2ms.
// 	if it is between 5% to 7.5%, we initiate closing sequence
// 	and if it is between 7.5% to 10%, we initiate opening sequence.
// 	We only consider the high time of the square wave by not checking if 
// 	the time difference is above 2.5ms (as we know duty cycle < 10%)
//   */
//   unsigned long pulse_change_current_time = micros();
//   time_difference = pulse_change_current_time - pulse_change_start_time;
//   if (time_difference < 2500 && time_difference > 800) {
//     if (time_difference < 1500) {
//       initiate_closing_sequence_flag = true;
//     }
//     else {
//       initiate_opening_sequence_flag = true;
//     }
//   }
//   pulse_change_start_time = micros();
// }

void IRAM_ATTR ISR_func_pwm() {
  if (digitalRead(swx2) != HIGH || digitalRead(swy2) != HIGH) {
    initiate_closing_sequence_flag = true;
  }
  else if (digitalRead(swx1) != HIGH && digitalRead(swy1) != HIGH) {
    initiate_opening_sequence_flag = true;
  }
}


// this is the limit switch ISR function. swx1 is for detecting opening of x_step_motor.
// Whenever the switch is pressed, it means that the x_step_motors have fully opened the x centering part.
// And so, we stop the motors by putting it's stop_motor_flag as True
void IRAM_ATTR ISR_func_swx1() {
  x_stop_motor_flag = true;
}

// this is the limit switch ISR function. swx1 is for detecting closing of x_step_motor
void IRAM_ATTR ISR_func_swx2() {
  x_stop_motor_flag = true;
}

// this is the limit switch ISR function. swx1 is for detecting opening of y_step_motor
void IRAM_ATTR ISR_func_swy1() {
  y_stop_motor_flag = true;
}

// this is the limit switch ISR function. swx1 is for detecting closing of y_step_motor
void IRAM_ATTR ISR_func_swy2() {
  y_stop_motor_flag = true;
}


// the step motor will run indefinitely until it's stop_motor_flag is True
// ISR functions for limit switches changes stop_motor_flag
// First we close the x_centering and after that, the y_centering
void initiate_closing_sequence() {
  if (logging) {
    Serial.println("Initiating x closing sequence");
    Serial.println(time_difference);
  }
  x_step_motor.setSpeed(1200);
  // attachInterrupt(digitalPinToInterrupt(swx2), ISR_func_swx2, HIGH);
  delay(50);
  while (1) {//while (x_stop_motor_flag == false) {
    int i = 0;
    if (digitalRead(swx2) == HIGH) {
      for (i=0;i<5;i++) {
        if (digitalRead(swx2) == LOW) {
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
  // detachInterrupt(digitalPinToInterrupt(swx2));
  x_stop_motor_flag = false;


  if (logging) {
    Serial.println("Initiating y closing sequence");
    Serial.println(time_difference);
  }
  y_step_motor.setSpeed(1200);
  // attachInterrupt(digitalPinToInterrupt(swy2), ISR_func_swy2, HIGH);
  delay(50);
  while(1) {//while (y_stop_motor_flag == false) {
    int i = 0;
    if (digitalRead(swy2) == HIGH) {
      for (i=0;i<5;i++) {
        if (digitalRead(swy2) == LOW) {
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
  // detachInterrupt(digitalPinToInterrupt(swy2));  
  y_stop_motor_flag = false;
}


// the step motor will run indefinitely until it's stop_motor_flag is True
// ISR functions for limit switches changes stop_motor_flag
// First we open the x_centering and after that, the y_centering
void initiate_opening_sequence() {
  bool x_open = false;
  bool y_open = false;
    
  x_step_motor.setSpeed(-1200);
  y_step_motor.setSpeed(-1200);

  while ((!x_open) || (!y_open)) {

    if (digitalRead(swx1) == LOW && (!x_open)) {
      if (logging) {
        Serial.println("Initiating x opening sequence");
        Serial.println(time_difference);
      }
      // while(1) {//while (x_stop_motor_flag == false) {
      int i = 0;
      if (digitalRead(swx1) == HIGH) {
        for (i=0;i<5;i++) {
          if (digitalRead(swx1) == LOW) {
            break;
          }
          delay(1);
        }
      }
      if (i != 5) {
        x_step_motor.runSpeed();;
      }
      else {
        x_open = true;
        x_step_motor.setSpeed(0);
      }
      
      // }
      // detachInterrupt(digitalPinToInterrupt(swx1));
      x_stop_motor_flag = false;
    }

    if (digitalRead(swy1) == LOW && (!y_open)) {
      if (logging) {
        Serial.println("Initiating y opening sequence");
        Serial.println(time_difference);
      }
      // while (1) {//while (y_stop_motor_flag == false) {
      int i = 0;
      if (digitalRead(swy1) == HIGH) {
        for (i=0;i<5;i++) {
          if (digitalRead(swy1) == LOW) {
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
      // detachInterrupt(digitalPinToInterrupt(swy1));
      y_stop_motor_flag = false;
    }
  }
  x_step_motor.setSpeed(0);
  y_step_motor.setSpeed(0);
}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); 

  // Set up the stepper motor pins
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);

  if (logging) {
    Serial.println("before");
  }

  // need to make changes in the set motor speed function. The rpm speed of the motor is not correct.
  // Initialize the stepper motor
  x_step_motor.setMaxSpeed(1800.0);
  x_step_motor.setAcceleration(200.0);
  y_step_motor.setMaxSpeed(1800.0);
  y_step_motor.setAcceleration(200.0);

  if (logging) {
    Serial.println("after");
  }

  pinMode(pixhawk_input_pin, INPUT);
  pinMode(36, INPUT);
  pinMode(swx1, INPUT);
  pinMode(swx2, INPUT);
  pinMode(swy1, INPUT);
  pinMode(swy2, INPUT);

  delay(100);

  // attachInterrupt(digitalPinToInterrupt(pixhawk_input_pin), ISR_func_pwm, CHANGE);
  // attachInterrupt(digitalPinToInterrupt(pixhawk_input_pin), ISR_func_pwm, LOW); // Just for testing right now, using a push switch to open/close the box

}

void loop() {
  // constantly sensing for the pwm signal from pixhawk. If proper singal of less than 10% duty cycle is received,
  // we take necessary step for closing/opening of the centering.
  // detaching the interrupt while closing to ignore all the signals during operation of the motor.
  if (digitalRead(36) == LOW) {
    int i;
    for (i=0;i<500;i++) {
      if (digitalRead(36) == LOW) {
        delay(1);
      }
      else {
        break;
      }
    }
    if (i == 500) {
      if (logging) {
        Serial.println("pixhawk signal high");
      }
      while (digitalRead(36) == LOW) {
        ;
      }
      if (digitalRead(swx2) != HIGH || digitalRead(swy2) != HIGH) {
        initiate_closing_sequence_flag = true;
      }
      else if (digitalRead(swx1) != HIGH && digitalRead(swy1) != HIGH) {
        initiate_opening_sequence_flag = true;
      }
    }
  }
  if (initiate_closing_sequence_flag) {
    // detachInterrupt(digitalPinToInterrupt(pixhawk_input_pin));
    initiate_closing_sequence_flag = false;
    initiate_closing_sequence();
    // attachInterrupt(digitalPinToInterrupt(pixhawk_input_pin), ISR_func_pwm, CHANGE);
    // attachInterrupt(digitalPinToInterrupt(pixhawk_input_pin), ISR_func_pwm, LOW); // Just for testing right now, using a push switch to open/close the box
  }
  else if (initiate_opening_sequence_flag) {
    // detachInterrupt(digitalPinToInterrupt(pixhawk_input_pin));
    initiate_opening_sequence_flag = false;
    initiate_opening_sequence();
    // attachInterrupt(digitalPinToInterrupt(pixhawk_input_pin), ISR_func_pwm, CHANGE);
    // attachInterrupt(digitalPinToInterrupt(pixhawk_input_pin), ISR_func_pwm, LOW); // Just for testing right now, using a push switch to open/close the box
  }
}
