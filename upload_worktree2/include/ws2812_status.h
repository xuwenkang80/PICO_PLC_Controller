#pragma once

#include <cstdint>

#include "hardware/pio.h"

namespace plc {

struct Rgb {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

class Ws2812Status {
public:
    void init(PIO pio, uint pin);
    bool ready() const { return initialized_; }
    void set(Rgb color);
    void off() { set({0, 0, 0}); }

private:
    PIO pio_ = pio0;
    uint sm_ = 0;
    bool initialized_ = false;
};

}  // namespace plc
