#include "plc_board.h"

#include <cstddef>

#include "hardware/gpio.h"

namespace plc {
namespace {

bool normalize_level(bool electrical_level, bool active_high) {
    return active_high ? electrical_level : !electrical_level;
}

void write_output_pin(uint pin, bool enabled) {
    gpio_put(pin, normalize_level(enabled, board::kOutputsActiveHigh));
}

}  // namespace

void PicoPlcBoard::DebouncedPin::init(uint gpio, bool active_high_value, uint32_t debounce) {
    pin = gpio;
    active_high = active_high_value;
    debounce_ms = debounce;
    raw_active = normalize_level(gpio_get(pin), active_high);
    stable_active = raw_active;
    pressed_edge = false;
    changed_at_ms = to_ms_since_boot(get_absolute_time());
}

void PicoPlcBoard::DebouncedPin::update(uint32_t now_ms) {
    pressed_edge = false;

    const bool active = normalize_level(gpio_get(pin), active_high);
    if (active != raw_active) {
        raw_active = active;
        changed_at_ms = now_ms;
    }

    if (raw_active != stable_active && now_ms - changed_at_ms >= debounce_ms) {
        const bool was_active = stable_active;
        stable_active = raw_active;
        pressed_edge = !was_active && stable_active;
    }
}

void PicoPlcBoard::init() {
    for (uint pin : board::kInputPins) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
    }

    for (uint pin : board::kOutputPins) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_OUT);
        write_output_pin(pin, false);
    }

    constexpr std::array<uint, 3> button_pins = {
        board::kButtonConfirmPin,
        board::kButtonMenuPin,
        board::kButtonRunStopPin,
    };

    for (uint pin : button_pins) {
        gpio_init(pin);
        gpio_set_dir(pin, GPIO_IN);
        if constexpr (board::kButtonsActiveLow) {
            gpio_pull_up(pin);
        } else {
            gpio_pull_down(pin);
        }
    }

    for (size_t i = 0; i < inputs_.size(); ++i) {
        inputs_[i].init(board::kInputPins[i], board::kInputsActiveHigh, board::kInputDebounceMs);
    }

    for (size_t i = 0; i < buttons_.size(); ++i) {
        buttons_[i].init(button_pins[i], !board::kButtonsActiveLow, board::kButtonDebounceMs);
    }

    output_mask_ = 0;
    update(to_ms_since_boot(get_absolute_time()));
}

void PicoPlcBoard::update(uint32_t now_ms) {
    input_mask_ = 0;
    for (size_t i = 0; i < inputs_.size(); ++i) {
        inputs_[i].update(now_ms);
        if (inputs_[i].stable_active) {
            input_mask_ |= static_cast<uint8_t>(1u << i);
        }
    }

    for (auto &button : buttons_) {
        button.update(now_ms);
    }
}

bool PicoPlcBoard::input(uint index) const {
    if (index >= board::kInputCount) {
        return false;
    }
    return (input_mask_ & (1u << index)) != 0;
}

void PicoPlcBoard::set_outputs(uint8_t mask) {
    output_mask_ = mask;
    for (size_t i = 0; i < board::kOutputPins.size(); ++i) {
        write_output_pin(board::kOutputPins[i], (mask & (1u << i)) != 0);
    }
}

void PicoPlcBoard::set_output(uint index, bool enabled) {
    if (index >= board::kOutputCount) {
        return;
    }

    uint8_t mask = output_mask_;
    if (enabled) {
        mask |= static_cast<uint8_t>(1u << index);
    } else {
        mask &= static_cast<uint8_t>(~(1u << index));
    }
    set_outputs(mask);
}

bool PicoPlcBoard::button_down(Button button) const {
    const auto index = static_cast<size_t>(button);
    if (index >= buttons_.size()) {
        return false;
    }
    return buttons_[index].stable_active;
}

bool PicoPlcBoard::button_pressed(Button button) const {
    const auto index = static_cast<size_t>(button);
    if (index >= buttons_.size()) {
        return false;
    }
    return buttons_[index].pressed_edge;
}

}  // namespace plc
