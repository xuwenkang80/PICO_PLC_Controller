#pragma once

#include <cstdint>

#include "hardware/i2c.h"

namespace plc {

class Ssd1306 {
public:
    static constexpr int kWidth = 128;
    static constexpr int kHeight = 64;
    static constexpr int kPages = kHeight / 8;
    static constexpr int kBufferSize = kWidth * kPages;

    bool begin(i2c_inst_t *i2c, uint8_t address);
    bool available() const { return ready_; }

    void clear();
    void draw_text(int x, int page, const char *text);
    void display();

private:
    bool command(uint8_t value);
    bool data(const uint8_t *values, int length);
    void draw_char(int x, int page, char c);

    i2c_inst_t *i2c_ = nullptr;
    uint8_t address_ = 0;
    bool ready_ = false;
    uint8_t buffer_[kBufferSize]{};
};

}  // namespace plc
