#include "ws2812_status.h"

#include "hardware/clocks.h"
#include "ws2812.pio.h"

namespace plc {
namespace {

uint32_t pack_grb(Rgb color) {
    return (static_cast<uint32_t>(color.g) << 24) |
           (static_cast<uint32_t>(color.r) << 16) |
           (static_cast<uint32_t>(color.b) << 8);
}

}  // namespace

void Ws2812Status::init(PIO pio, uint pin) {
    pio_ = pio;
    sm_ = pio_claim_unused_sm(pio_, true);
    const uint offset = pio_add_program(pio_, &ws2812_program);
    ws2812_program_init(pio_, sm_, offset, pin, 800000, false);
    initialized_ = true;
    off();
}

void Ws2812Status::set(Rgb color) {
    if (!initialized_) {
        return;
    }
    pio_sm_put_blocking(pio_, sm_, pack_grb(color));
}

}  // namespace plc
