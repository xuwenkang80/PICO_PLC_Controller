#pragma once

#include <array>
#include <cstdint>

#include "board_config.h"
#include "pico/stdlib.h"

namespace plc {

enum class Button : uint8_t {
    Confirm = 0,
    Menu = 1,
    RunStop = 2,
};

class PicoPlcBoard {
public:
    void init();
    void update(uint32_t now_ms);

    uint8_t inputs() const { return input_mask_; }
    bool input(uint index) const;

    void set_outputs(uint8_t mask);
    void set_output(uint index, bool enabled);
    uint8_t outputs() const { return output_mask_; }
    void all_outputs_off() { set_outputs(0); }

    bool button_down(Button button) const;
    bool button_pressed(Button button) const;

private:
    struct DebouncedPin {
        uint pin = 0;
        bool active_high = true;
        uint32_t debounce_ms = 0;
        bool raw_active = false;
        bool stable_active = false;
        bool pressed_edge = false;
        uint32_t changed_at_ms = 0;

        void init(uint gpio, bool active_high_value, uint32_t debounce);
        void update(uint32_t now_ms);
    };

    std::array<DebouncedPin, board::kInputCount> inputs_{};
    std::array<DebouncedPin, 3> buttons_{};
    uint8_t input_mask_ = 0;
    uint8_t output_mask_ = 0;
};

}  // namespace plc
