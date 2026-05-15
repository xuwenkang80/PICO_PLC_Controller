# Pico PLC Controller

这个工程是按目录中的 `原理图.pdf` 创建的 Raspberry Pi Pico 类 PLC 控制程序。

## 引脚映射

| 功能 | Pico GPIO |
| --- | --- |
| IO_IN1..IO_IN8 | GP2..GP9 |
| IO_OUT1..IO_OUT8 | GP15, GP14, GP13, GP12, GP11, GP10, GP16, GP17 |
| I2C SDA / SCL | GP18 / GP19 |
| CONFIRM / MENU / RUN-STOP | GP20 / GP21 / GP22 |
| WS2812 | GP26 |

输入按光耦输出为高电平有效处理，输出按 MOSFET 栅极高电平有效处理，三个按键按低电平有效处理。

## Demo 行为

`controller_demo` 提供四种模式：

- `MIRROR`: 输出 1..8 跟随输入 1..8。
- `LATCH`: 按 `CONFIRM` 时把当前输入锁存到输出。
- `CHASE`: 8 路输出依次跑马灯。
- `STEPPER`: 用 OUT1/OUT2/OUT3 控制 TB6600 步进电机驱动器。

`RUN/STOP` 切换运行和停止；普通模式停止时所有输出关闭，`STEPPER` 模式停止时停止脉冲并保留 DIR/ENA 状态。`MENU` 切换模式。OLED 如果是常见 SSD1306 `0x3C` 地址会显示运行状态；没有接屏时程序仍会继续运行。WS2812 用红色表示停止，绿色/黄色/蓝色/紫色表示不同运行模式。

## TB6600 控制

步进模式默认映射：

| PLC 输出 | Pico GPIO | TB6600 信号 |
| --- | --- | --- |
| OUT1 | GP15 | PUL |
| OUT2 | GP14 | DIR |
| OUT3 | GP13 | ENA |

USB CDC 串口支持以下 ASCII 命令：

```text
GET
STEP START
STEP STOP
STEP DIR CW
STEP DIR CCW
STEP SPEED 200
STEP EN ON
STEP EN OFF
```

速度单位是 pulse/s，范围为 1..2000。`STEP STOP` 会停止 OUT1 脉冲，OUT3 使能保持当前状态；如需释放电机再发送 `STEP EN OFF`。

如果你的 TB6600 端子把 ENA 当作低电平有效或“脱机”信号使用，可在 `demo/controller_demo.cpp` 中把 `kStepperEnableActiveHigh` 改为 `false`。

## 构建

```powershell
cmake -S . -B build -G "Ninja" -DPICO_SDK_PATH="C:\path\to\pico-sdk"
cmake --build build
```

生成的 UF2 在 `build/demo/controller_demo.uf2` 或 `build/controller_demo.uf2`，具体位置取决于 CMake 生成器和 SDK 版本。
