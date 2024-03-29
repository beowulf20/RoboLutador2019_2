#include "ps2lib.h"

unsigned long IRAM_ATTR micros()
{
    return (unsigned long)(esp_timer_get_time());
}
void IRAM_ATTR delayMicroseconds(uint32_t us)
{
    uint32_t m = micros();
    if (us)
    {
        uint32_t e = (m + us);
        if (m > e)
        { //overflow
            while (micros() > e)
            {
                NOP();
            }
        }
        while (micros() < e)
        {
            NOP();
        }
    }
}
//========== VARIABLES ===========//
uint8_t _dataPin = 0, _cmdPin = 0, _attPin = 0, _clkPin = 0, _clk_delay = 0, _dataIn = 0, _ackPin = 0;
bool _temp;
uint8_t _dataIn, _data1, _data2;
uint8_t _analog1, _analog2, _analog3, _analog4;
uint32_t _dataOut;
uint32_t _lastButtons;
esp_timer_handle_t ps2lib_timer_handle;
PSB_MODE ps2lib_controller_mode = PSB_MODE_DIGITAL;

uint8_t ps2lib_shift(uint8_t dataOut)
{
    _dataIn = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        if (dataOut & (1 << i))
        {
            gpio_set_level((gpio_num_t)_cmdPin, 1);
        }
        else
        {
            gpio_set_level((gpio_num_t)_cmdPin, 0);
        }
        gpio_set_level((gpio_num_t)_clkPin, 0);
        delayMicroseconds(_clk_delay);
        if (gpio_get_level((gpio_num_t)_dataPin))
        {
            _dataIn |= (1 << i);
        }
        gpio_set_level((gpio_num_t)_clkPin, 1);
        delayMicroseconds(_clk_delay);
    }
    return _dataIn;
}

void ps2lib_init(uint8_t dataPin, uint8_t cmdPin, uint8_t clkPin, uint8_t attPin, uint8_t ackPin, uint8_t clk_delay)
{
    _dataPin = dataPin;
    _cmdPin = cmdPin;
    _attPin = attPin;
    _clkPin = clkPin;
    _clk_delay = clk_delay;
    _ackPin = ackPin;
    //////// GPIO INIT
    // DATA
    gpio_set_direction((gpio_num_t)dataPin, GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)dataPin, GPIO_PULLUP_ONLY);
    gpio_pullup_en((gpio_num_t)dataPin);
    // COMMAND
    gpio_set_direction((gpio_num_t)cmdPin, GPIO_MODE_OUTPUT);
    // CLOCK
    gpio_set_direction((gpio_num_t)clkPin, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)clkPin, 1);
    // ATTENTION/CS
    gpio_set_direction((gpio_num_t)attPin, GPIO_MODE_OUTPUT);    
    gpio_set_level((gpio_num_t)attPin, 1);
    // ACKNOWLEDGE
    gpio_set_direction((gpio_num_t)ackPin, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode((gpio_num_t)ackPin, GPIO_PULLUP_ONLY);
    gpio_pullup_en((gpio_num_t)ackPin);
    gpio_set_level((gpio_num_t)ackPin, 1);
    //////////////////

    //////// TIMER INIT
    // esp_timer_create(&_timer_args, &ps2lib_timer_handle);
    ///////////////////
}

void ps2lib_config_for_analog()
{
    gpio_set_level((gpio_num_t)_attPin, 0);
    //GO INTO CONFIGURATION MODE
    ps2lib_shift(0x01);
    ps2lib_shift(0x43);
    ps2lib_shift(0x00);
    ps2lib_shift(0x01);
    ps2lib_shift(0x00);
    // ENABLE ANALOG MODE
    ps2lib_shift(0x01);
    ps2lib_shift(0x44);
    ps2lib_shift(0x00);
    ps2lib_shift(0x01);
    ps2lib_shift(0x03);
    ps2lib_shift(0x00);
    ps2lib_shift(0x00);
    ps2lib_shift(0x00);
    ps2lib_shift(0x00);
    // CONFIG CONTROLLER TO RETURN ALL PRESSURE VALUES
    ps2lib_shift(0x01);
    ps2lib_shift(0x4f);
    ps2lib_shift(0x00);
    ps2lib_shift(0xff);
    ps2lib_shift(0xff);
    ps2lib_shift(0x03);
    ps2lib_shift(0x00);
    ps2lib_shift(0x00);
    ps2lib_shift(0x00);
    // EXIT CONFIG MODE
    ps2lib_shift(0x01);
    ps2lib_shift(0x43);
    ps2lib_shift(0x00);
    ps2lib_shift(0x00);
    ps2lib_shift(0x5a);
    ps2lib_shift(0x5a);
    ps2lib_shift(0x5a);
    ps2lib_shift(0x5a);
    ps2lib_shift(0x5a);
    gpio_set_level((gpio_num_t)_attPin, 1);
}
uint32_t ps2lib_read()
{
    gpio_set_level((gpio_num_t)_attPin, 0);
    // POLL VALUES
    ps2lib_shift(0x01);
    ps2lib_controller_mode = ps2lib_shift(0x42);
    ps2lib_shift(0x00);

    //DIGITAL VALUES
    _data1 = ~ps2lib_shift(0xFF);
    _data2 = ~ps2lib_shift(0xFF);

    // ANALOG VALUES
    _analog1 = ~ps2lib_shift(0x00);
    _analog2 = ~ps2lib_shift(0x00);
    _analog3 = ~ps2lib_shift(0x00);
    _analog4 = ~ps2lib_shift(0x00);

    gpio_set_level((gpio_num_t)_attPin, 1);

    _lastButtons = _dataOut;
    _dataOut = (_data2 << 8) | _data1;

    return _dataOut;
}

bool ps2lib_buttons_changed()
{
    return ((_lastButtons ^ _dataOut) > 0);
}

bool ps2lib_isbutton_changed(PSB_STATE bt)
{
    return (((_lastButtons ^ _dataOut) & bt) > 0);
}

bool ps2lib_button(PSB_STATE bt)
{
    return ((_dataOut & bt) > 0);
}

uint8_t ps2lib_getAnalog(uint8_t idx)
{
    switch (idx)
    {
    case 0:
        return _analog1;
    case 1:
        return _analog2;
    case 2:
        return _analog3;
    case 3:
        return _analog4;
    default:
        ESP_LOGE(__func__, "invalid analog index");
        return 0;
    }
}

float ps2lib_getAnalogf(uint8_t idx)
{
    float a = ps2lib_getAnalog(idx) - 127;
    if (a > 0)
    {
        a /= 128.0;
    }
    else if (a < 0)
    {
        a /= 127.0;
    }
    return a;
}

PSB_MODE ps2lib_getMode()
{
    return ps2lib_controller_mode;
}
