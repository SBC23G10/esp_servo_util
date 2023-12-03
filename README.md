| Supported Targets | ESP32 | ESP32-C2 | ESP32-C3 | ESP32-C6 | ESP32-H2 | ESP32-S2 | ESP32-S3 |
| ----------------- | ----- | -------- | -------- | -------- | -------- | -------- | -------- |

# Servo Playground

### Hardware Required

* A USB cable for Power supply and programming
* Any common servo

See [Development Boards](https://www.espressif.com/en/products/devkits) for more information about it.

### Build and Flash

Run `idf.py -p PORT flash monitor` to build, flash and monitor the project.

(To exit the serial monitor, type ``Ctrl-]``.)

See the [Getting Started Guide](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for full steps to configure and use ESP-IDF to build projects.

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