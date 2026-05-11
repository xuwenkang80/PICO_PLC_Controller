#include <cctype>
#include <cstdio>
#include <cstdlib>

#include "board_config.h"
#include "hardware/gpio.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "plc_board.h"
#include "ssd1306.h"
#include "ws2812_status.h"

namespace {

enum class DemoMode : uint8_t {
    Mirror = 0,
    Latch = 1,
    Chase = 2,
    Manual = 3,
};

const char *mode_name(DemoMode mode) {
    switch (mode) {
    case DemoMode::Mirror:
        return "MIRROR";
    case DemoMode::Latch:
        return "LATCH";
    case DemoMode::Chase:
        return "CHASE";
    case DemoMode::Manual:
        return "MANUAL";
    }
    return "MIRROR";
}

DemoMode next_mode(DemoMode mode) {
    switch (mode) {
    case DemoMode::Mirror:
        return DemoMode::Latch;
    case DemoMode::Latch:
        return DemoMode::Chase;
    case DemoMode::Chase:
    case DemoMode::Manual:
        return DemoMode::Mirror;
    }
    return DemoMode::Mirror;
}

plc::Rgb status_color(bool running, DemoMode mode, uint32_t now_ms) {
    const bool bright = ((now_ms / 350) % 2) == 0;
    const uint8_t low = bright ? 18 : 4;

    if (!running) {
        return {low, 0, 0};
    }

    switch (mode) {
    case DemoMode::Mirror:
        return {0, 20, 0};
    case DemoMode::Latch:
        return {18, 14, 0};
    case DemoMode::Chase:
        return {0, 0, 22};
    case DemoMode::Manual:
        return {0, 16, 18};
    }
    return {0, 20, 0};
}

bool equals_ignore_case(const char *left, const char *right) {
    while (*left != '\0' && *right != '\0') {
        if (std::toupper(static_cast<unsigned char>(*left)) !=
            std::toupper(static_cast<unsigned char>(*right))) {
            return false;
        }
        ++left;
        ++right;
    }
    return *left == '\0' && *right == '\0';
}

bool parse_u8(const char *text, uint8_t &value) {
    char *end = nullptr;
    const auto parsed = std::strtoul(text, &end, 0);
    if (end == text || *end != '\0' || parsed > 0xff) {
        return false;
    }
    value = static_cast<uint8_t>(parsed);
    return true;
}

bool parse_bool_arg(const char *text, bool &value) {
    if (equals_ignore_case(text, "1") || equals_ignore_case(text, "ON") ||
        equals_ignore_case(text, "TRUE")) {
        value = true;
        return true;
    }

    if (equals_ignore_case(text, "0") || equals_ignore_case(text, "OFF") ||
        equals_ignore_case(text, "FALSE")) {
        value = false;
        return true;
    }

    return false;
}

bool parse_mode_arg(const char *text, DemoMode &mode) {
    if (equals_ignore_case(text, "MIRROR")) {
        mode = DemoMode::Mirror;
        return true;
    }
    if (equals_ignore_case(text, "LATCH")) {
        mode = DemoMode::Latch;
        return true;
    }
    if (equals_ignore_case(text, "CHASE")) {
        mode = DemoMode::Chase;
        return true;
    }
    if (equals_ignore_case(text, "MANUAL")) {
        mode = DemoMode::Manual;
        return true;
    }
    return false;
}

void print_status(bool running, DemoMode mode, const plc::PicoPlcBoard &board) {
    printf("run=%u mode=%s in=0x%02X out=0x%02X\n",
           running ? 1 : 0,
           mode_name(mode),
           board.inputs(),
           board.outputs());
}

void process_command_line(char *line,
                          bool &running,
                          DemoMode &mode,
                          uint8_t &manual_outputs,
                          bool &status_requested) {
    char verb[12]{};
    char arg1[16]{};
    char arg2[16]{};
    const int argc = std::sscanf(line, "%11s %15s %15s", verb, arg1, arg2);

    if (argc <= 0) {
        return;
    }

    if (equals_ignore_case(verb, "GET")) {
        status_requested = true;
        return;
    }

    if (equals_ignore_case(verb, "RUN") && argc >= 2) {
        if (parse_bool_arg(arg1, running)) {
            status_requested = true;
            return;
        }
    }

    if (equals_ignore_case(verb, "MODE") && argc >= 2) {
        if (parse_mode_arg(arg1, mode)) {
            status_requested = true;
            return;
        }
    }

    if (equals_ignore_case(verb, "OUT") && argc >= 2) {
        uint8_t value = 0;
        if (argc == 2 && parse_u8(arg1, value)) {
            manual_outputs = value;
            mode = DemoMode::Manual;
            running = true;
            status_requested = true;
            return;
        }

        if (argc >= 3 && parse_u8(arg1, value) && value >= 1 && value <= plc::board::kOutputCount) {
            bool enabled = false;
            if (parse_bool_arg(arg2, enabled)) {
                const uint8_t bit = static_cast<uint8_t>(1u << (value - 1));
                if (enabled) {
                    manual_outputs |= bit;
                } else {
                    manual_outputs &= static_cast<uint8_t>(~bit);
                }
                mode = DemoMode::Manual;
                running = true;
                status_requested = true;
                return;
            }
        }
    }

    printf("err=bad_command cmd=%s\n", line);
}

void poll_serial_commands(bool &running,
                          DemoMode &mode,
                          uint8_t &manual_outputs,
                          bool &status_requested) {
    static char line[48]{};
    static size_t length = 0;

    while (true) {
        const int ch = getchar_timeout_us(0);
        if (ch == PICO_ERROR_TIMEOUT) {
            break;
        }

        if (ch == '\r' || ch == '\n') {
            if (length > 0) {
                line[length] = '\0';
                process_command_line(line, running, mode, manual_outputs, status_requested);
                length = 0;
            }
            continue;
        }

        if (length < sizeof(line) - 1) {
            line[length++] = static_cast<char>(ch);
        } else {
            length = 0;
            printf("err=line_too_long\n");
        }
    }
}

void update_display(plc::Ssd1306 &display, bool running, DemoMode mode, uint8_t inputs, uint8_t outputs) {
    if (!display.available()) {
        return;
    }

    char line[22]{};
    display.clear();

    std::snprintf(line, sizeof(line), "RUN:%s MODE:%s", running ? "ON" : "OFF", mode_name(mode));
    display.draw_text(0, 0, line);

    std::snprintf(line, sizeof(line), "IN:%02X OUT:%02X", inputs, outputs);
    display.draw_text(0, 2, line);

    display.draw_text(0, 4, "MENU MODE");
    display.draw_text(0, 5, "RUN/STOP TOGGLE");
    display.draw_text(0, 6, "CONFIRM LATCH");
    display.display();
}

void init_display_bus() {
    i2c_init(plc::board::kDisplayI2c, plc::board::kDisplayBaudrate);
    gpio_set_function(plc::board::kDisplaySdaPin, GPIO_FUNC_I2C);
    gpio_set_function(plc::board::kDisplaySclPin, GPIO_FUNC_I2C);
    gpio_pull_up(plc::board::kDisplaySdaPin);
    gpio_pull_up(plc::board::kDisplaySclPin);
}

}  // namespace

int main() {
    stdio_init_all();

    plc::PicoPlcBoard board;
    board.init();

    init_display_bus();
    plc::Ssd1306 display;
    const bool display_ready = display.begin(plc::board::kDisplayI2c, plc::board::kDisplayI2cAddress);

    plc::Ws2812Status status;
    status.init(pio0, plc::board::kWs2812Pin);

    bool running = false;
    DemoMode mode = DemoMode::Mirror;
    uint8_t latched_outputs = 0;
    uint8_t manual_outputs = 0;
    uint8_t chase_mask = 0x01;
    uint32_t last_chase_ms = 0;
    uint32_t last_display_ms = 0;
    uint32_t last_log_ms = 0;

    printf("Pico PLC controller demo started. Display: %s\n", display_ready ? "ready" : "not found");

    while (true) {
        const uint32_t now_ms = to_ms_since_boot(get_absolute_time());
        board.update(now_ms);
        bool status_requested = false;
        poll_serial_commands(running, mode, manual_outputs, status_requested);

        if (board.button_pressed(plc::Button::RunStop)) {
            running = !running;
        }

        if (board.button_pressed(plc::Button::Menu)) {
            mode = next_mode(mode);
        }

        if (board.button_pressed(plc::Button::Confirm)) {
            latched_outputs = board.inputs();
        }

        uint8_t output_mask = 0;
        if (running) {
            switch (mode) {
            case DemoMode::Mirror:
                output_mask = board.inputs();
                break;
            case DemoMode::Latch:
                output_mask = latched_outputs;
                break;
            case DemoMode::Chase:
                if (now_ms - last_chase_ms >= 250) {
                    last_chase_ms = now_ms;
                    chase_mask = static_cast<uint8_t>((chase_mask << 1) | (chase_mask >> 7));
                }
                output_mask = chase_mask;
                break;
            case DemoMode::Manual:
                output_mask = manual_outputs;
                break;
            }
        }
        board.set_outputs(output_mask);

        if (status_requested) {
            print_status(running, mode, board);
        }

        status.set(status_color(running, mode, now_ms));

        if (now_ms - last_display_ms >= 200) {
            last_display_ms = now_ms;
            update_display(display, running, mode, board.inputs(), board.outputs());
        }

        if (now_ms - last_log_ms >= 1000) {
            last_log_ms = now_ms;
            print_status(running, mode, board);
        }

        sleep_ms(5);
    }
}
