#pragma once

#include "inttypes.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "math.h"
#include "esp_err.h"

#define MOTOR1_EN1_PIN 0
#define MOTOR1_EN2_PIN 0
#define MOTOR1_PWM_PIN 0
#define MOTOR2_EN1_PIN 0
#define MOTOR2_EN2_PIN 0
#define MOTOR2_PWM_PIN 0

#define MOTOR_PWM_FREQ 5000
#define MOTOR_PWM_RESOL 7

#define MOTOR_USE_ABS_STOP //wheter to use or not abs like stop
#ifdef MOTOR_USE_ABS_STOP
#define MOTOR_ABS_STOP_FADE_TIME_MS 200
#endif

#define MOTOR_USE_ACCEL //wheter to use or not acceleration
#ifdef MOTOR_USE_ACCEL
#define MOTOR_ACCEL_FADE_TIME_MS 100
#endif

enum
{
    MOTOR1_CH = 0,
    MOTOR2_CH,
    MOTOR_CH_MAX
};

typedef struct
{
    uint8_t en1;
    uint8_t en2;
    union {
        ledc_channel_config_t channel;
    };
} motor_t;

motor_t *motors;

void motor_init()
{
    // Config PWM Timer
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = MOTOR_PWM_RESOL,
        .freq_hz = MOTOR_PWM_FREQ,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .timer_num = LEDC_TIMER_0};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Config Motors
    motors = malloc(sizeof(motor_t) * 2);
    motor_t m[2] = {
        {.en1 = MOTOR1_EN1_PIN,
         .en2 = MOTOR1_EN2_PIN,
         .channel = {.channel = LEDC_CHANNEL_0,
                     .duty = 0,
                     .gpio_num = MOTOR1_PWM_PIN,
                     .speed_mode = LEDC_HIGH_SPEED_MODE,
                     .timer_sel = LEDC_TIMER_0,
                     .hpoint = 0}},
        {.en1 = MOTOR2_EN1_PIN,
         .en2 = MOTOR2_EN2_PIN,
         .channel = {
             .channel = LEDC_CHANNEL_0,
             .duty = 0,
             .gpio_num = MOTOR2_PWM_PIN,
             .speed_mode = LEDC_HIGH_SPEED_MODE,
             .timer_sel = LEDC_TIMER_0,
             .hpoint = 0}},
    };
    xthal_memcpy(motors, m, sizeof(motor_t) * 2);

    // Config Motor Relay Pin Direction
    for (int i = 0; i < MOTOR_CH_MAX; i++)
    {
        gpio_set_direction((motors + i)->en1, GPIO_MODE_OUTPUT);
        gpio_set_direction((motors + i)->en2, GPIO_MODE_OUTPUT);
        gpio_set_level((motors + i)->en1, 0);
        gpio_set_level((motors + i)->en2, 0);
    }

    // Enable PWM
    for (int i = 0; i < MOTOR_CH_MAX; i++)
    {
        ESP_ERROR_CHECK(ledc_channel_config(&(motors + i)->channel));
    }

#ifdef MOTOR_USE_ABS_STOP
    ESP_ERROR_CHECK(ledc_fade_func_install(0));
#endif

    ESP_LOGI(__func__, "all ok");
}

static void motor_set_dir(uint8_t idx, float vel)
{

    if (idx >= MOTOR_CH_MAX)
    {
        ESP_LOGE(__func__, "invalid motor idx: %d", idx);
        return;
    }
    motor_t motor = *(motors + idx);
    uint32_t target_duty = (uint32_t)(powf(2, MOTOR_PWM_RESOL) * fabsf(vel));
    // ESP_LOGI(__func__, "target duty = %d", target_duty);
    if (vel > 0)
    {
        //forwards
        gpio_set_direction(motor.en1, 1);
        gpio_set_direction(motor.en2, 0);
#ifdef MOTOR_USE_ACCEL
        ledc_set_fade_time_and_start(motor.channel.speed_mode, motor.channel.channel, target_duty, MOTOR_ABS_STOP_FADE_TIME_MS, LEDC_FADE_NO_WAIT);

#elif
        ledc_set_duty_and_update(motor.channel.speed_mode, motor.channel.channel, target_duty, 0);
#endif
    }
    else if (vel < 0)
    {
        //backwards
        gpio_set_direction(motor.en1, 0);
        gpio_set_direction(motor.en2, 1);

#ifdef MOTOR_USE_ACCEL
        ledc_set_fade_time_and_start(motor.channel.speed_mode, motor.channel.channel, target_duty, MOTOR_ABS_STOP_FADE_TIME_MS, LEDC_FADE_NO_WAIT);
#elif
        ledc_set_duty_and_update(motor.channel.speed_mode, motor.channel.channel, target_duty, 0);
#endif
    }
    else
    {
        //stop
#ifdef MOTOR_USE_ABS_STOP
        ledc_set_fade_time_and_start(motor.channel.speed_mode, motor.channel.channel, 0, MOTOR_ABS_STOP_FADE_TIME_MS, LEDC_FADE_NO_WAIT);
#elif
        gpio_set_direction(motor.en1, 1);
        gpio_set_direction(motor.en2, 1);
        ledc_set_duty_and_update(motor.channel.speed_mode, motor.channel.channel, 0, 0);
#endif
    }
}

typedef struct motor_dir_t
{
    float left;
    float right;
} motor_dir_t;

motor_dir_t motor_get_direction(float leftJoyStick, float RightJoyStick)
{
    motor_dir_t dir = {0};
    if (leftJoyStick == 0 && RightJoyStick != 0)
    {
        dir.left = -RightJoyStick;
        dir.right = RightJoyStick;
    }
    else if (RightJoyStick == 0)
    {
        dir.left = leftJoyStick;
        dir.right = leftJoyStick;
    }
    else if (RightJoyStick > 0)
    {
        if (leftJoyStick > 0)
        {
            dir.left = leftJoyStick - (2 * fabs(RightJoyStick));
        }
        else
        {
            dir.left = leftJoyStick + (2 * fabs(RightJoyStick));
        }
        dir.right = leftJoyStick;
    }
    else if (RightJoyStick < 0)
    {
        if (leftJoyStick > 0)
        {
            dir.right = leftJoyStick - (2 * fabs(RightJoyStick));
        }
        else
        {
            dir.right = leftJoyStick + (2 * fabs(RightJoyStick));
        }
        dir.left = leftJoyStick;
    }
    return dir;
}

void motor_go(float leftJoyStick, float RightJoyStick)
{
    motor_dir_t dir = motor_get_direction(leftJoyStick, RightJoyStick);
    motor_set_dir(0, dir.left);
    motor_set_dir(1, dir.right);
}