/*
Library for running stepper motor with TB6560 Stepper motor driver.
Tested on ESP32 DOIT board only.

Author: Ankush Goyal
*/



#include <Arduino.h>


class Tworks_Stepper {
	public:
		Tworks_Stepper(byte dir_pin_input, byte step_pin_input);
		bool begin();
		bool set_spinning_direction(bool dir);
		bool set_motor_speed(double speed);
		bool run_motor(void);
		bool stop_motor_flag = false;
	private:
		byte dir_pin;
		byte step_pin;
		uint8_t speed;
		long delay = 1000;
		double steps_per_revolution = 200;
		long delay_between_motor_steps;
};