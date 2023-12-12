/*
Library for running stepper motor with TB6560 Stepper motor driver.
Tested on ESP32 DOIT board only.

Author: Ankush Goyal
*/


#include <Tworks_Stepper.h>


// we need two pins for running the stepper motor, the direction pin and the step pin.
Tworks_Stepper::Tworks_Stepper(byte dir_pin_input, byte step_pin_input) {
	dir_pin = dir_pin_input;
	step_pin = step_pin_input;
}

// setting mode for the pin
bool Tworks_Stepper::begin() {
	pinMode(dir_pin, OUTPUT);
	pinMode(step_pin, OUTPUT);
	return true;
}

// true for clockwise and false for anticlockwise
bool Tworks_Stepper::set_spinning_direction(bool dir){
	if (dir) {
		digitalWrite(dir_pin, HIGH);
	}
	else {
		digitalWrite(dir_pin, LOW);
	}
	return true;
}

// set speed in rpm. 
// TODO: Need to configure motor speed.
bool Tworks_Stepper::set_motor_speed(double speed) {
	// for 1 rpm, num steps = steps_per_revolution.
	// In 1 step, there is a delay of 2*d
	// 2*d*speed*steps_per_revolution = 1 minute
	// d (in us) = (60*1000*1000) / (2*speed*steps_per_revolution)
	double total_steps = double(speed)*steps_per_revolution;
	Serial.println(total_steps);
	double delay_in_ms = double(60*1000) / (2*total_steps);
	Serial.println(delay_in_ms);
	delay_between_motor_steps = long(delay_in_ms * 1000);
	Serial.println(delay_between_motor_steps);
	Serial.println("");
	return true;
}

// run motor indefinitely till the stop_motor_flag is true.
// delay variable calculated in the set_motor_speed function.
bool Tworks_Stepper::run_motor() {
	while (!stop_motor_flag) {
		// Serial.println("Running motor");
		// delayMicroseconds(100000);
		digitalWrite(step_pin, HIGH);
		delayMicroseconds(delay_between_motor_steps);
		digitalWrite(step_pin, LOW);
		delayMicroseconds(delay_between_motor_steps);
	}
	return true;
}