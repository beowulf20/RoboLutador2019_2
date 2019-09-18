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

void control_update_task()
{
    for (;;)
    {
        ps2lib_read();
        vTaskDelay(50 / portTICK_RATE_MS);
    }
}

// void handle_controls_task()
// {
//     bool isUp = false;
//     bool isDown = false;
//     bool isLeft = false;
//     bool isRight = false;
//     uint32_t raw_data = 0;
//     for (;;)
//     {
//         isUp = ps2lib_button(PSB_STATE_PAD_UP);
//         isDown = ps2lib_button(PSB_STATE_PAD_DOWN);
//         isLeft = ps2lib_button(PSB_STATE_PAD_LEFT);
//         isRight = ps2lib_button(PSB_STATE_PAD_RIGHT);
//         // raw_data = ps2lib_read();
//         ESP_LOGI(__func__, "UDLR = %d%d%d%d | analog 1 2 3 4 = %d %d %d %d | mode = %02X",
//                  isUp, isDown, isLeft, isRight,
//                  ps2lib_getAnalog(0), ps2lib_getAnalog(1), ps2lib_getAnalog(2), ps2lib_getAnalog(3),
//                  ps2lib_getMode());
//         // ESP_LOGI(__func__, "<<%d,%d,%d,%d,%d,%d,%d,%d,%d>>",
//         //          isUp, isLeft, isDown, isRight,
//         //          ps2lib_getAnalog(0), ps2lib_getAnalog(1), ps2lib_getAnalog(2), ps2lib_getAnalog(3),
//         //          ps2lib_getMode());
//         vTaskDelay(300 / portTICK_RATE_MS);
//     }
// }

void handle_controls_task()
{
    bool isUp = false;
    bool isDown = false;
    bool isLeft = false;
    bool isRight = false;
    bool LastisUp = false;
    bool LastisDown = false;
    bool LastisLeft = false;
    bool LastisRight = false;
    uint32_t raw_data = 0;
    for (;;)
    {
        isUp = ps2lib_button(PSB_STATE_PAD_UP);
        isDown = ps2lib_button(PSB_STATE_PAD_DOWN);
        isLeft = ps2lib_button(PSB_STATE_PAD_LEFT);
        isRight = ps2lib_button(PSB_STATE_PAD_RIGHT);
        // raw_data = ps2lib_read();

        // if (isUp != LastisUp)
        // {
        //     ESP_LOGI(__func__, "Up pressed/released!");
        // }
        // if (isDown != LastisDown)
        // {
        //     ESP_LOGI(__func__, "Down pressed/released!");
        // }
        // if (isLeft != LastisLeft)
        // {
        //     ESP_LOGI(__func__, "Left pressed/released!");
        // }
        // if (isRight != LastisRight)
        // {
        //     ESP_LOGI(__func__, "Right pressed/released!");
        // }
        ESP_LOGI(__func__, "UDLR = %d%d%d%d | analog 1 2 3 4 = %d %d %d %d | mode = %02X",
                 isUp, isDown, isLeft, isRight,
                 ps2lib_getAnalog(0), ps2lib_getAnalog(1), ps2lib_getAnalog(2), ps2lib_getAnalog(3),
                 ps2lib_getMode());
        // ESP_LOGI(__func__, "<<%d,%d,%d,%d,%d,%d,%d,%d,%d>>",
        //          isUp, isLeft, isDown, isRight,
        //          ps2lib_getAnalog(0), ps2lib_getAnalog(1), ps2lib_getAnalog(2), ps2lib_getAnalog(3),
        //          ps2lib_getMode());
        vTaskDelay(50 / portTICK_RATE_MS);
        LastisUp = isUp;
        LastisDown = isDown;
        LastisLeft = isLeft;
        LastisRight = isRight;
    }
}

void app_main()
{
    ps2lib_init(PIN_DAT, PIN_CMD, PIN_CLK, PIN_ATT, PIN_ACK, 30);
    ps2lib_config_for_analog();
    xTaskCreatePinnedToCore(control_update_task, "ctrl update", 8196, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(handle_controls_task, "handle ctrl", 8196, NULL, 2, NULL, 1);
}
