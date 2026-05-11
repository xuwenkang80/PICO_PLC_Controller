# Pico PLC Controller

这个工程是按目录中的 `原理图.pdf` 创建的 Raspberry Pi Pico 类 PLC 控制程序。

## 引脚映射

| 功能 | Pico GPIO |
| --- | --- |
| IO_IN1..IO_IN8 | GP2..GP9 |
| IO_OUT1..IO_OUT8 | GP10..GP17 |
| I2C SDA / SCL | GP18 / GP19 |
| CONFIRM / MENU / RUN-STOP | GP20 / GP21 / GP22 |
| WS2812 | GP26 |

输入按光耦输出为高电平有效处理，输出按 MOSFET 栅极高电平有效处理，三个按键按低电平有效处理。

## Demo 行为

`controller_demo` 提供三种模式：

- `MIRROR`: 输出 1..8 跟随输入 1..8。
- `LATCH`: 按 `CONFIRM` 时把当前输入锁存到输出。
- `CHASE`: 8 路输出依次跑马灯。

`RUN/STOP` 切换运行和停止；停止时所有输出关闭。`MENU` 切换模式。OLED 如果是常见 SSD1306 `0x3C` 地址会显示运行状态；没有接屏时程序仍会继续运行。WS2812 用红色表示停止，绿色/黄色/蓝色表示不同运行模式。

## 构建

```powershell
cmake -S . -B build -G "Ninja" -DPICO_SDK_PATH="C:\path\to\pico-sdk"
cmake --build build
```

生成的 UF2 在 `build/demo/controller_demo.uf2` 或 `build/controller_demo.uf2`，具体位置取决于 CMake 生成器和 SDK 版本。
