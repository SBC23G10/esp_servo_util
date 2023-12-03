#include <cstdlib>
#include <ctime>
#include <thread>
#include <atomic>

#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "servo_ctrl/servo_util.hpp"
static char tag[] = "All_feature_servo_comm_test";

extern "C"
void app_main()
{
	/*
	 * srand(time(NULL)) NOTE
	 * Take into account that deterministic behaviour will
	 * occur if the time keeps always the same on every reboot
	 */
    srand(time(NULL));
	int i;
	// bit resolution the higher the best precision/more time implied
    int bits = 15;
	// duty minimun value in uSecs
	int minValue = 500;

	uint32_t duty = (1<<bits) * minValue / 20000;
	
	// Values that will be referenced by every servo_comm
	
	std::atomic<float> target_a = 0;
	std::atomic<float> target_b = 0;
	std::atomic<float> target_reset = 0;

	// Prepare pwm (using ledc-lib) for each target servo

	// In this case using same timer
	servo_comm_ledc_channel_prepare(duty, /*bits*/15, /*freq*/50, LEDC_CHANNEL_0, LEDC_TIMER_0, /*GPIO N*/16);
	servo_comm_ledc_channel_prepare(duty, 15, 50, LEDC_CHANNEL_1, LEDC_TIMER_0, 17);
	servo_comm_ledc_channel_prepare(duty, 15, 50, LEDC_CHANNEL_2, LEDC_TIMER_0, 4);

	/* 
	 * Simply create a servo communication like this, you can bind the referenced value here (in constructor)
	 * or later Using Servo_comm::set_target(& ref_value );
	 * you must unbind (at the end of ...) the referenced value if its existence is attached to that function call
	 * otherwise it will be referencing some junk value
	 */

	Servo_comm a(
			0,			// Set id (up to 256) <char>
			180,		// Axis max in degrees
			1750,		// Fixed rotation period for every target in ms
			3500 - 500, // Duty range in uSecs
			500,		// Init duty uSecs
			20,			// freq^-1 in ms
			500,		// Rest period between target change in ms
			0,			// 1 - Clockwise / 0 - CounterClockwise
			bits,		// Resolution bits
            LEDC_CHANNEL_0,
			&target_a	// Target referenced value
			);

	Servo_comm b(1,180,500,3500 - 500,500,20,500,0,bits,LEDC_CHANNEL_1,&target_b);

	// Used to reset internal init (0) a 180 servo by making it sweep from 0 to 180 and vice versa

	Servo_comm reset('R',180,500,3500 - 500,500,20,500,0,bits,LEDC_CHANNEL_2,&target_reset);

	/* 
	 * Now create a thread and bind for every servo (functor call),
	 * it will return a vector of servo_comm-thread tuples,
	 * to keep the relationship during thread destruction and creation
	 */
	
	auto servo_bind = servo_thread_init({&a, &b, &reset});

	/*
	 * -------------------------------------------------------------------------------------------
	 * ------ All Done - fully concurrent servo control should be working. Now some testing ------
	 * -------------------------------------------------------------------------------------------
	 */


	// This will simulate a general purpose servo operation for 10 iterations

	for(i = 0; i < 10; i++) {
		target_a.store((float)(rand() % 18000) / 100);
		target_b.store((float)(rand() % 18000) / 100);
		target_reset.store((int)target_reset.load() == 180 ? 0.0f:180.0f);

		/* 
		 * Nothing to do here, every thread is supposed to update to the referenced value changes.
		 * No race conditions are possible since std::atomic variables outside the change period are ignored
		 * and not used for recalc operations, even those within period are read once
		 * and copied to a local var to keep integrity of the original value
		 */

		ESP_LOGI(tag, "\n(Readings) { \n\t%s\n\t%s\n\t%s\n}", a.info().c_str(),
														b.info().c_str(),
														reset.info().c_str());
		usleep(3200000);
	}

	/*
	 * This will simulate an enter-sleep preprotocol valid for sleeps that keep up memory
	 * Note that this function takes a reference to our vector of servo-thread tuples
	 */
	turn_off_servo_control(servo_bind);

	// Proof that even updating values for 10 iterations no servo will be updated (canceled-status)
	for(i = 0; i < 10; i++) {
		target_a.store((float)(rand() % 18000) / 100);
		target_b.store((float)(rand() % 18000) / 100);
		
		ESP_LOGI(tag, "\n(Readings) { \n\t%s\n\t%s\n\t%s\n}", a.info().c_str(),
														b.info().c_str(),
														reset.info().c_str());
		usleep(3200000);
	}
	
	/*
	 * This will simulate a wake-up post protocol for re-enabling servo control
	 * referencing again our vector of servo-thread tuples
	 */
	restore_servo_control(servo_bind);

	for(i = 0; i < 10; i++) {
		target_a.store((float)(rand() % 18000) / 100);
		target_b.store((float)(rand() % 18000) / 100);
		
		ESP_LOGI(tag, "\n(Readings) { \n\t%s\n\t%s\n\t%s\n}", a.info().c_str(),
														b.info().c_str(),
														reset.info().c_str());
		usleep(3200000);
	}


	turn_off_servo_control(servo_bind);
	// ...
}