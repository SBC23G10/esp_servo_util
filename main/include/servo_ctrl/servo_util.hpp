#ifndef __SERVO_UTIL__
#define __SERVO_UTIL__

/*
 *  Servo-util control over pwm USING ledc-lib
 *  Simple abstraction layer that provides fully
 *  concurrent and atomic
 *  multi-servo operation control with ease
 * 
 *  by SBC23G10
 *
 */

#include <iostream>
#include <atomic>
#include <vector>
#include <thread>
#include <cmath>
#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "driver/ledc.h"

class Servo_comm {
	public:

	/* Functor simply calls main task() */

	void operator ()()
	{
		end = 0;
		task();
		/*
		 * Cancel-thread related code can be implemented,
		 * Anyways no memory need to be freed since no
		 * dynamic allocation has been involved
		 * 
		 * Only call join after successful turn_off_servo_control() call
		 */
	}

	Servo_comm();
	Servo_comm(
			char id,
			uint32_t ROT_MAX_DEG,
			uint32_t SWEEP_PERIOD_MS,

			uint32_t DUTY_RANGE_US,
			uint32_t initial_duty,
			uint32_t DUTY_UPD_PERIOD_MS,
			uint32_t REST_PERIOD_MS,
			char REVERSED,
			
			char bits,
			ledc_channel_t conn,

			/* 
			 * It can be hacky to reach float precision, in this case with some inner
			 * duty_range multiplying and rounding this can be reached
			 */

			std::atomic<float>* target
			);
	void end_now();
	~Servo_comm();
	/*
	 * This is necessary at the end of every calling function
	 * (if one of its declarations has been addressed)
	 * to avoid pointing to a non-managed address
	 */

	void unset_target();

	/*
	 * Again, this is necessary after every new instance (if target address not set)
	 * to read directly from that managed addressed value
	 */

	void set_target(std::atomic<float>* target);

	std::string info();

	private:

	const char id;
	const uint32_t ROT_MAX_DEG;
	const uint32_t SWEEP_PERIOD_MS;

	const uint32_t DUTY_RANGE_US;
	const uint32_t DUTY_UPD_PERIOD_MS;
	const uint32_t STEPS;
	
	const uint32_t REST_PERIOD_MS;
	const char REVERSED;

	uint32_t duty_current;
	float rot_current;
    	ledc_channel_t conn;
	
	// Just to keep semantics dummy_target also as an atomic var

	std::atomic<float> dummy_target;
	std::atomic<float>* _target_;
	char end;
	
	void task();
};

void servo_comm_ledc_channel_prepare(
		uint32_t duty,
		uint32_t bits,
		uint32_t freq,
		ledc_channel_t channel,
		ledc_timer_t timer,
		int gpio);
/*
 * std::vector<std::tuple<Servo_comm&, std::thread>>    tf!!??
 *
 * Well, it allows keeping a bunch of servo_comm binded with its handler thread
 * so, when you want to do some turn on / off stuff you will NOT
 * have to mess with tracking where its thread was declared or passing as
 * a reference to a join dedicated call function (:
 * 
 * Threads are initialized with a reference ( std::ref(functor) ) so it keeps alive with original
 * servo class instance and all related referenced stuff
 *
 * 
 * The correct order of servo-thread initializing is
 *
 *    [[ previous servo_comm_ledc_channel_prepare(...) call as well servo_comm declaration ]]
 *  
 *->  auto s_bind = servo_thread_init({servo0, servo1, servoN, ...}) -- Now all servo_commm will be working as we expect
 *  
 *    [[ your_etc_code ]]
 *  
 *    if(my_servo_stop_condition) -- some sleep or whatever that causes servo to be stopped
 *->  		turn_off_servo_control(s_bind);
 *  
 *    [[ now awake or simply called by another trigger ]]
 *->  restore_servo_control(s_bind)
 *   
 * 
 * If you want to simply stop until reboot do NOT call restore_servo_control(s_bind)
 * actually turn_off_servo_control joins every needed thread and thats basically all
 * since there is NO unhandled dynamic memory dealloc involved by servo instances
 * 
 */

std::vector<std::tuple<Servo_comm&, std::thread>>
	servo_thread_init(std::initializer_list<Servo_comm*> list);
void turn_off_servo_control(std::vector<std::tuple<Servo_comm&, std::thread>>& servo_bind);
void restore_servo_control(std::vector<std::tuple<Servo_comm&, std::thread>>& servo_bind);

#endif /* !__SERVO_UTIL */
