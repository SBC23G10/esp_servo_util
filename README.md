| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- |

# ESP-32 IDF Servo Util Playground

### Servo-util control over pwm USING ledc-lib
Simple abstraction layer that provides fully concurrent and atomic multi-servo operation control with ease

### Hardware Required

* Any common servo

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

## Basic servo-util usage

```c++
extern "C"
void app_main()
{
    srand(time(NULL));
        // Set precision bits
        int bits = 15;

        // Min duty value in uSecs
	int minValue = 500;

        // Calculate duty value given precision and 20000 uSecs period (50Hz)
	uint32_t duty = (1<<bits) * minValue / 20000;

	// Same timer for all servos

	servo_comm_ledc_channel_prepare(duty, 15, 50, LEDC_CHANNEL_0, LEDC_TIMER_0, 16);
  servo_comm_ledc_channel_prepare(duty, 15, 50, LEDC_CHANNEL_0, LEDC_TIMER_0, 17);
        // Declare an "to be referenced" atomic value

	std::atomic<float> target_a = 0;
	std::atomic<float> target_b = 0;

        // Declare servo given readings reference value

	Servo_comm servo_a(0, 180, 500, 3500 - 500, 500, 20, 500, 0, bits, LEDC_CHANNEL_0, &target_a);
	Servo_comm servo_b(1, 180, 1750, 3500 - 500, 500, 20, 500, 0, bits, LEDC_CHANNEL_1, &target_b);

	auto servo_bind = servo_thread_init({&servo_a, &servo_b /*, ... */});

	// Test by simply assigning target_a random values between 0.00 - 179.99

        for (int i = 0; i < 10; i++) {
                target_a.store((float)(rand() % 18000) / 100);
		target_b.store((float)(rand() % 18000) / 100);
		usleep(3200000);
        }

	// Simply turn off servo communication functionality

	turn_off_servo_control(servo_bind);

	// Can be easily restored keeping last duties (useful for sleep-awake usages that keep up memory)

	restore_servo_control(servo_bind);

	turn_off_servo_control(servo_bind);
}
```

## Example Output

```text
(Readings) { 
        Servo_comm_0: Sweep period (ms) = 1750, Target rotation = 133.100006, Clockwise = false, Status = canceled 
        Servo_comm_1: Sweep period (ms) = 500, Target rotation = 38.619999, Clockwise = false, Status = canceled 
        Servo_comm_82: Sweep period (ms) = 500, Target rotation = 0.000000, Clockwise = false, Status = canceled 
}
I (66149) All_feature_servo_comm_test: 
(Readings) { 
        Servo_comm_0: Sweep period (ms) = 1750, Target rotation = 104.839996, Clockwise = false, Status = active 
        Servo_comm_1: Sweep period (ms) = 500, Target rotation = 33.630001, Clockwise = false, Status = active 
        Servo_comm_82: Sweep period (ms) = 500, Target rotation = 0.000000, Clockwise = false, Status = active 
}
```
## General testing

For general testing you should pick one of the examples given in main/example_main_srcs putting it inside the main directory ../

## License

The source code for the site is licensed under the MIT license, which you can find in the LICENSE file.
