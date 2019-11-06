#pragma once

#include "inttypes.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_clk.h"
#include "time.h"
#include "freertos/FreeRTOS.h"
#include "esp_avrc_api.h"

//DEFINITIONS
#define NOP() asm volatile("nop")

typedef enum
{
    //BUTTONS
    PSB_STATE_IDLE = 0x000,
    PSB_STATE_SELECT = 0x0001,
    PSB_STATE_START = 0x0008,
    PSB_STATE_L2 = 0x0100,
    PSB_STATE_R2 = 0x0200,
    PSB_STATE_L1 = 0x0400,
    PSB_STATE_R1 = 0x0800,
    PSB_STATE_GREEN = 0x1000,
    PSB_STATE_RED = 0x2000,
    PSB_STATE_BLUE = 0x4000,
    PSB_STATE_PINK = 0x8000,
    //LEFT KEY PADS/STICKS
    PSB_STATE_PAD_UP = 0x0010,
    PSB_STATE_PAD_RIGHT = 0x0020,
    PSB_STATE_PAD_DOWN = 0x0040,
    PSB_STATE_PAD_LEFT = 0x0080,
    PSB_STATE_LEFT_STICK_LEFT = 0x0010,
    PSB_STATE_LEFT_STICK_DOWN = 0x0020,
    PSB_STATE_LEFT_STICK_RIGHT = 0x0040,
    PSB_STATE_LEFT_STICK_UP = 0x0080,
    //RIGHT KEY PADS/STICKS
    PSB_STATE_TRIANGLE = 0x1000,
    PSB_STATE_CIRCLE = 0x2000,
    PSB_STATE_CROSS = 0x4000,
    PSB_STATE_SQUARE = 0x8000,
    PSB_STATE_RIGHT_STICK_LEFT = 0x1000,
    PSB_STATE_RIGHT_STICK_DOWN = 0x2000,
    PSB_STATE_RIGHT_STICK_RIGHT = 0x4000,
    PSB_STATE_RIGHT_STICK_UP = 0x8000
} PSB_STATE;

typedef enum
{
    PSB_MODE_DIGITAL = 0x41,
    PSB_MODE_ANALOG = 0x73,
    PSB_MODE_CONFIG = 0xF,
    PSB_MODE_MAX
} PSB_MODE;

//Initialize component
void ps2lib_init(uint8_t dataPin, uint8_t cmdPin, uint8_t clkPin, uint8_t attPin, uint8_t ackPin, uint8_t clk_delay);
//Set controller to work with analog mode
void ps2lib_config_for_analog();
//Update controller value
uint32_t ps2lib_read();
//Check if any buttons changed since last read
bool ps2lib_buttons_changed();
//Check if a specific button changed since last read
bool ps2lib_isbutton_changed(PSB_STATE bt);
//Check if a specific button is pressed
bool ps2lib_button(PSB_STATE bt);
//Get the pressure value from the controller ranging from [255,0]
uint8_t ps2lib_getAnalog(uint8_t idx);
//Get pressure value from controller ranging from [+1.00,-1.00]
float ps2lib_getAnalogf(uint8_t idx);
//Get controller current state mode
PSB_MODE ps2lib_getMode();