#include "ssd1306.h"

#include <cstring>

namespace plc {
namespace {

constexpr uint8_t kControlCommand = 0x00;
constexpr uint8_t kControlData = 0x40;

void glyph_for(char c, uint8_t out[5]) {
    switch (c) {
    case '0': { uint8_t g[5] = {0x3e, 0x51, 0x49, 0x45, 0x3e}; std::memcpy(out, g, 5); return; }
    case '1': { uint8_t g[5] = {0x00, 0x42, 0x7f, 0x40, 0x00}; std::memcpy(out, g, 5); return; }
    case '2': { uint8_t g[5] = {0x42, 0x61, 0x51, 0x49, 0x46}; std::memcpy(out, g, 5); return; }
    case '3': { uint8_t g[5] = {0x21, 0x41, 0x45, 0x4b, 0x31}; std::memcpy(out, g, 5); return; }
    case '4': { uint8_t g[5] = {0x18, 0x14, 0x12, 0x7f, 0x10}; std::memcpy(out, g, 5); return; }
    case '5': { uint8_t g[5] = {0x27, 0x45, 0x45, 0x45, 0x39}; std::memcpy(out, g, 5); return; }
    case '6': { uint8_t g[5] = {0x3c, 0x4a, 0x49, 0x49, 0x30}; std::memcpy(out, g, 5); return; }
    case '7': { uint8_t g[5] = {0x01, 0x71, 0x09, 0x05, 0x03}; std::memcpy(out, g, 5); return; }
    case '8': { uint8_t g[5] = {0x36, 0x49, 0x49, 0x49, 0x36}; std::memcpy(out, g, 5); return; }
    case '9': { uint8_t g[5] = {0x06, 0x49, 0x49, 0x29, 0x1e}; std::memcpy(out, g, 5); return; }
    case 'A': { uint8_t g[5] = {0x7e, 0x11, 0x11, 0x11, 0x7e}; std::memcpy(out, g, 5); return; }
    case 'C': { uint8_t g[5] = {0x3e, 0x41, 0x41, 0x41, 0x22}; std::memcpy(out, g, 5); return; }
    case 'D': { uint8_t g[5] = {0x7f, 0x41, 0x41, 0x22, 0x1c}; std::memcpy(out, g, 5); return; }
    case 'E': { uint8_t g[5] = {0x7f, 0x49, 0x49, 0x49, 0x41}; std::memcpy(out, g, 5); return; }
    case 'F': { uint8_t g[5] = {0x7f, 0x09, 0x09, 0x09, 0x01}; std::memcpy(out, g, 5); return; }
    case 'G': { uint8_t g[5] = {0x3e, 0x41, 0x49, 0x49, 0x7a}; std::memcpy(out, g, 5); return; }
    case 'H': { uint8_t g[5] = {0x7f, 0x08, 0x08, 0x08, 0x7f}; std::memcpy(out, g, 5); return; }
    case 'I': { uint8_t g[5] = {0x00, 0x41, 0x7f, 0x41, 0x00}; std::memcpy(out, g, 5); return; }
    case 'L': { uint8_t g[5] = {0x7f, 0x40, 0x40, 0x40, 0x40}; std::memcpy(out, g, 5); return; }
    case 'M': { uint8_t g[5] = {0x7f, 0x02, 0x0c, 0x02, 0x7f}; std::memcpy(out, g, 5); return; }
    case 'N': { uint8_t g[5] = {0x7f, 0x04, 0x08, 0x10, 0x7f}; std::memcpy(out, g, 5); return; }
    case 'O': { uint8_t g[5] = {0x3e, 0x41, 0x41, 0x41, 0x3e}; std::memcpy(out, g, 5); return; }
    case 'P': { uint8_t g[5] = {0x7f, 0x09, 0x09, 0x09, 0x06}; std::memcpy(out, g, 5); return; }
    case 'R': { uint8_t g[5] = {0x7f, 0x09, 0x19, 0x29, 0x46}; std::memcpy(out, g, 5); return; }
    case 'S': { uint8_t g[5] = {0x46, 0x49, 0x49, 0x49, 0x31}; std::memcpy(out, g, 5); return; }
    case 'T': { uint8_t g[5] = {0x01, 0x01, 0x7f, 0x01, 0x01}; std::memcpy(out, g, 5); return; }
    case 'U': { uint8_t g[5] = {0x3f, 0x40, 0x40, 0x40, 0x3f}; std::memcpy(out, g, 5); return; }
    case 'V': { uint8_t g[5] = {0x1f, 0x20, 0x40, 0x20, 0x1f}; std::memcpy(out, g, 5); return; }
    case 'Y': { uint8_t g[5] = {0x07, 0x08, 0x70, 0x08, 0x07}; std::memcpy(out, g, 5); return; }
    case ':': { uint8_t g[5] = {0x00, 0x36, 0x36, 0x00, 0x00}; std::memcpy(out, g, 5); return; }
    case '/': { uint8_t g[5] = {0x20, 0x10, 0x08, 0x04, 0x02}; std::memcpy(out, g, 5); return; }
    case '-': { uint8_t g[5] = {0x08, 0x08, 0x08, 0x08, 0x08}; std::memcpy(out, g, 5); return; }
    default:
        std::memset(out, 0, 5);
        return;
    }
}

}  // namespace

bool Ssd1306::begin(i2c_inst_t *i2c, uint8_t address) {
    i2c_ = i2c;
    address_ = address;
    ready_ = true;

    const uint8_t init_commands[] = {
        0xae, 0x20, 0x00, 0xb0, 0xc8, 0x00, 0x10, 0x40,
        0x81, 0x7f, 0xa1, 0xa6, 0xa8, 0x3f, 0xa4, 0xd3,
        0x00, 0xd5, 0x80, 0xd9, 0xf1, 0xda, 0x12, 0xdb,
        0x40, 0x8d, 0x14, 0xaf,
    };

    for (uint8_t value : init_commands) {
        if (!command(value)) {
            ready_ = false;
            return false;
        }
    }

    clear();
    display();
    return true;
}

void Ssd1306::clear() {
    std::memset(buffer_, 0, sizeof(buffer_));
}

void Ssd1306::draw_text(int x, int page, const char *text) {
    if (!text || page < 0 || page >= kPages) {
        return;
    }

    while (*text && x < kWidth) {
        draw_char(x, page, *text++);
        x += 6;
    }
}

void Ssd1306::display() {
    if (!ready_) {
        return;
    }

    for (int page = 0; page < kPages; ++page) {
        command(static_cast<uint8_t>(0xb0 + page));
        command(0x00);
        command(0x10);
        data(&buffer_[page * kWidth], kWidth);
    }
}

bool Ssd1306::command(uint8_t value) {
    const uint8_t payload[] = {kControlCommand, value};
    return i2c_write_blocking(i2c_, address_, payload, sizeof(payload), false) == sizeof(payload);
}

bool Ssd1306::data(const uint8_t *values, int length) {
    uint8_t payload[kWidth + 1]{};
    payload[0] = kControlData;
    std::memcpy(&payload[1], values, static_cast<size_t>(length));
    return i2c_write_blocking(i2c_, address_, payload, length + 1, false) == length + 1;
}

void Ssd1306::draw_char(int x, int page, char c) {
    if (x < 0 || x + 4 >= kWidth || page < 0 || page >= kPages) {
        return;
    }

    uint8_t glyph[5]{};
    glyph_for(c, glyph);

    uint8_t *row = &buffer_[page * kWidth + x];
    for (int i = 0; i < 5; ++i) {
        row[i] = glyph[i];
    }
    if (x + 5 < kWidth) {
        row[5] = 0;
    }
}

}  // namespace plc
