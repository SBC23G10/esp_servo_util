| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- |

# ESP Servo Util Playground

Servo-util control over pwm USING ledc-lib
Simple abstraction layer that provides fully concurrent and atomic multi-servo operation control with ease

### Hardware Required

* Any common servo

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

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
