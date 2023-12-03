#include "servo_util.hpp"

/*
 *	Servo-util control over pwm USING ledc-lib
 *  Simple abstraction layer that provides fully
 *  concurrent and atomic
 *  multi-servo operation control with ease
 * 
 *  by SBC23G10
 *
 */

Servo_comm::Servo_comm():
        	id('0'),
		ROT_MAX_DEG(180),
		SWEEP_PERIOD_MS(1500),

		DUTY_RANGE_US(2500 - 500),
		DUTY_UPD_PERIOD_MS(20),
		STEPS(1500 / 20),
		REST_PERIOD_MS(1000),
		REVERSED(0),

		duty_current((1<<15) * 500 / 20000),
		rot_current(0.0f),
        	conn(LEDC_CHANNEL_0),
		dummy_target(std::atomic<float>(0.0f)),
		end(0)
{
	_target_ = &dummy_target;
}

Servo_comm::Servo_comm(
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
		std::atomic<float>* target
		) :
        	id(id),
		ROT_MAX_DEG(ROT_MAX_DEG),
		SWEEP_PERIOD_MS(SWEEP_PERIOD_MS),

		DUTY_RANGE_US(DUTY_RANGE_US),
		DUTY_UPD_PERIOD_MS(DUTY_UPD_PERIOD_MS),
		STEPS(SWEEP_PERIOD_MS / DUTY_UPD_PERIOD_MS),
		REST_PERIOD_MS(REST_PERIOD_MS),
		REVERSED(REVERSED),

		duty_current((1<<bits) * initial_duty / (DUTY_UPD_PERIOD_MS * 1000)),
		rot_current(0.0f),
		conn(conn),
		dummy_target(std::atomic<float>(0.0f)),
		_target_(target),
		end(0)
{
	if (target == NULL)
		_target_ = &dummy_target;
}
void Servo_comm::end_now()
{
	end = 1;
}
Servo_comm::~Servo_comm()
{
	this->end_now();
}

/* 
 * This is necessary at the end of every calling function
 * (if one of its declarations has been addressed)
 * to avoid pointing to a non-managed address
 * 
 * Can only be changed when servo is in canceled state
 */

void Servo_comm::unset_target()
{
	if (this->end)
		_target_ = &(this->dummy_target);
}

/* 
 * Again, this is necessary after every new instance or
 * source update (if target address not set)
 * to read directly from that managed addressed value
 * 
 * Can only be changed when servo is in canceled state
 */

void Servo_comm::set_target(std::atomic<float>* target)
{
	if (this->end && target != NULL)
		_target_ = target;
}

std::string Servo_comm::info()
{
    return std::string(
        "Servo_comm_"+ std::to_string((int)id)
        + ": " "Sweep period (ms) = "
        + std::to_string(SWEEP_PERIOD_MS)
        + ", Target rotation = "
        + std::to_string(_target_->load())
        + ", Clockwise = "
        + (REVERSED ? "true" : "false")
		+ ", Status = "
        + (end ? "canceled " : "active ")
        );
}

void Servo_comm::task()
{
	std::atomic<float>& target = *_target_;
	float rot_relat;
	uint32_t duty_step_change;
	uint32_t sleep_period_us = DUTY_UPD_PERIOD_MS * 1000;
	uint32_t rest_period_us = REST_PERIOD_MS * 1000;
	float target_rc;
	uint32_t nop = 0;
	
	// Loop awaiting for end == 1 to exit

	for (; !end ; usleep(rest_period_us)) {
		
		/*
		 * If target pointer reference has been unset (address equals dummy one)
		 * skip THIS iteration
		 */
		if (&target == &dummy_target)
			continue;

		/*
		 * Avoid race conditions (rc) eval with one single copy of target value at time
		 *
		 * First of all, it IS an atomic variable and last but not least:
		 * Using a lock can be an option but pointless if the value is ONLY UPDATED
		 * by other threads since this thread will ONLY READ within period, in this specific case
		 * we can take this as an advantage just because it will ignore values that are updated
		 * even in a shorter time span than the minimum rotation period saving energy by avoiding
		 * in progress servo rotation interrupt and the mechanical impact that it implies
		 * 
		 * Basically with the variable being atomic itself is totally enough and safe for this approach
		 */

		target_rc = target.load(); if(nop){}
		target_rc = REVERSED ? 180.0f-target_rc:target_rc;
		if (fabs(target_rc - rot_current) < 5e-1 || target_rc > 180.0f)
			continue;

		rot_relat = (float)(target_rc - rot_current) / ROT_MAX_DEG;
		duty_step_change = round(DUTY_RANGE_US * rot_relat) / STEPS;
		for (int i=0; i<STEPS; i++) {
			duty_current += duty_step_change;
			ledc_set_duty(LEDC_HIGH_SPEED_MODE, conn, duty_current);
			ledc_update_duty(LEDC_HIGH_SPEED_MODE, conn);
			usleep(sleep_period_us);
		}
		rot_current = target_rc;
	}
}
void servo_comm_ledc_channel_prepare(
		uint32_t duty,
		uint32_t bits,
		uint32_t freq,
		ledc_channel_t channel,
		ledc_timer_t timer,
		int gpio)
{
	ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution    = LEDC_TIMER_15_BIT,
        .timer_num  = timer,
        .freq_hz    = freq
        };
	ledc_timer_config(&timer_conf);

	ledc_channel_config_t ledc_conf = {
        .gpio_num   = gpio,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel    = channel,
		// disable fade interrupt
        .intr_type  = LEDC_INTR_DISABLE, 
        .timer_sel  = timer,
        .duty       = duty
        };
	ledc_channel_config(&ledc_conf);
}

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
	servo_thread_init(std::initializer_list<Servo_comm*> list)
{
	std::vector<std::tuple<Servo_comm&, std::thread>> servo_thread_tup;
	for (auto s_t_ : list)
		servo_thread_tup.push_back({*s_t_, std::thread(std::ref(*s_t_))});
	return servo_thread_tup;
}
void turn_off_servo_control(std::vector<std::tuple<Servo_comm&, std::thread>>& servo_bind)
{
	// auto for-each will fail ):
	for (int i=0; i<servo_bind.size(); i++) {
		std::get<0>(servo_bind.at(i)).end_now();
		if (std::get<1>(servo_bind.at(i)).joinable())
			std::get<1>(servo_bind.at(i)).join();
	}
	// you can add "return bool " <-> "return joined_count == servo_bind.size();"
}

void restore_servo_control(std::vector<std::tuple<Servo_comm&, std::thread>>& servo_bind)
{
	for (int i=0; i<servo_bind.size(); i++)
		std::get<1>(servo_bind.at(i)) = std::thread(std::ref(std::get<0>(servo_bind.at(i))));
}
