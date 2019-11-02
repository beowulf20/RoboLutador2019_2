#define PIN_ACK 32
#define PIN_CLK 33
#define PIN_CMD 26
#define PIN_DAT 27
#define PIN_ATT 25

#include "esp_log.h"
#include "ps2lib.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "motor.h"
#include "driver/uart.h"
#include "math.h"

void control_update_task()
{

    for (;;)
    {
        ps2lib_read();
        vTaskDelay(250 / portTICK_RATE_MS);
    }
}

void handle_controls_task()
{
    bool isUp = false;
    bool isDown = false;
    bool isLeft = false;
    bool isRight = false;
    float turnValue = 0; //right horizontal
    // float a1 = 0; //right vertical
    // float a2 =0; //left horizontal
    float speedValue = 0; //left vertical
    for (;;)
    {
        PSB_MODE mode = ps2lib_getMode();
        if (mode == PSB_MODE_ANALOG)
        {
            turnValue = ps2lib_getAnalogf(0);
            speedValue = ps2lib_getAnalogf(3);
        }
        else if (mode == PSB_MODE_DIGITAL)
        {
            isUp = ps2lib_button(PSB_STATE_PAD_UP);
            isDown = ps2lib_button(PSB_STATE_PAD_DOWN);
            isLeft = ps2lib_button(PSB_STATE_PAD_LEFT);
            isRight = ps2lib_button(PSB_STATE_PAD_RIGHT);
            if (isUp)
                speedValue = 1;
            else if (isDown)
                speedValue = -1;
            else
                speedValue = 0;
            if (isLeft)
                turnValue = 1;
            else if (isRight)
                turnValue = -1;
            else
                turnValue = 0;
        }
        motor_go(speedValue, turnValue);
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void app_main()
{
    ps2lib_init(PIN_DAT, PIN_CMD, PIN_CLK, PIN_ATT, PIN_ACK, 30);
    ps2lib_config_for_analog();
    motor_init();
    xTaskCreatePinnedToCore(control_update_task, "ctrl update", 8196, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(handle_controls_task, "handle ctrl", 8196, NULL, 2, NULL, 1);
}
