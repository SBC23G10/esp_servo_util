#include <cstdlib>
#include <ctime>
#include <thread>
#include <atomic>

#include <unistd.h>
#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"
#include "esp_log.h"

#include "servo_ctrl/servo_util.hpp"

static char tag[] = "Servo_comm_test";

// For a full documented source please refer to 'all_feature_test.c'

// Basic test with 3 servo_comm

extern "C"
void app_main()
{
	/*
	 * srand(time(NULL)) NOTE
	 * Take into account that deterministic behaviour can
	 * occur if the time keeps always the same on every reboot
	 */

    srand(time(NULL)); 
    int bits = 15;
	int minValue = 500;
	uint32_t duty = (1<<bits) * minValue / 20000;

	// Same timer for all servos

	servo_comm_ledc_channel_prepare(duty, 15, 50, LEDC_CHANNEL_0, LEDC_TIMER_0, 16);
	servo_comm_ledc_channel_prepare(duty, 15, 50, LEDC_CHANNEL_1, LEDC_TIMER_0, 17);
	servo_comm_ledc_channel_prepare(duty, 15, 50, LEDC_CHANNEL_2, LEDC_TIMER_0, 4);

	std::atomic<float> target_a = 0;
	std::atomic<float> target_b = 0;
	std::atomic<float> target_c = 0;

	Servo_comm servo_a(0, 180, 500, 3500 - 500, 500, 20, 500, 0, bits, LEDC_CHANNEL_0, &target_a);
	Servo_comm servo_b(1, 180, 1750, 3500 - 500, 500, 20, 500, 0, bits, LEDC_CHANNEL_1, &target_b);
	Servo_comm servo_c(2, 180, 2500, 3500 - 500, 500, 20, 500, 0, bits, LEDC_CHANNEL_2, &target_c);

	auto servo_bind = servo_thread_init({&servo_a, &servo_b, &servo_c/* , ... */});
	
	/*
	 * -------------------------------------------------------------------------------------------
	 * ------ All Done - fully concurrent servo control should be working. Now some testing ------
	 * -------------------------------------------------------------------------------------------
	 */

	for (int i = 0; i < 10; i++) {
		target_a.store((float)(rand() % 18000) / 100);
		target_b.store((float)(rand() % 18000) / 100);
		target_c.store((float)(rand() % 18000) / 100);
		ESP_LOGI(tag, "\n(Readings) { \n\t%s\n\t%s\n\t%s\n}", servo_a.info().c_str(),
														servo_b.info().c_str(),
														servo_c.info().c_str());
		usleep(3200000);
	}
	turn_off_servo_control(servo_bind);
	// Can be easily restored keeping last duties (useful for sleep-awake usages that keep up memory)
	restore_servo_control(servo_bind);

	turn_off_servo_control(servo_bind);
}