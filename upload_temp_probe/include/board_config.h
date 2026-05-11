#pragma once

#include <array>
#include <cstdint>

#include "hardware/i2c.h"
#include "pico/types.h"

namespace plc::board {

constexpr uint kInputCount = 8;
constexpr uint kOutputCount = 8;

// Mapping recovered from 原理图.pdf:
// inputs: IO_IN1..8, outputs: IO_OUT1..8.
inline constexpr std::array<uint, kInputCount> kInputPins = {
    2, 3, 4, 5, 6, 7, 8, 9,
};

inline constexpr std::array<uint, kOutputCount> kOutputPins = {
    10, 11, 12, 13, 14, 15, 16, 17,
};

constexpr bool kInputsActiveHigh = true;
constexpr bool kOutputsActiveHigh = true;

constexpr uint kButtonConfirmPin = 20;
constexpr uint kButtonMenuPin = 21;
constexpr uint kButtonRunStopPin = 22;
constexpr bool kButtonsActiveLow = true;

constexpr uint kDisplaySdaPin = 18;
constexpr uint kDisplaySclPin = 19;
constexpr uint kDisplayBaudrate = 400'000;
constexpr uint8_t kDisplayI2cAddress = 0x3c;
inline i2c_inst_t *const kDisplayI2c = i2c1;

constexpr uint kWs2812Pin = 26;
constexpr uint kWs2812Count = 1;

constexpr uint kInputDebounceMs = 8;
constexpr uint kButtonDebounceMs = 25;

}  // namespace plc::board
