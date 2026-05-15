# Pico PLC HMI 工具说明

## 已确认的 Pico 端原理

- `controller_demo` 通过 USB CDC 打印状态，CMake 中 `pico_enable_stdio_usb(controller_demo 1)` 已启用，UART stdio 未启用。
- 8 路输入 `IO_IN1..IO_IN8` 对应 Pico `GP2..GP9`，高电平有效，软件去抖时间为 8 ms。
- 8 路输出 `IO_OUT1..IO_OUT8` 对应 Pico `GP15, GP14, GP13, GP12, GP11, GP10, GP16, GP17`，高电平有效，`PicoPlcBoard::set_outputs(mask)` 按 bit0..bit7 写 OUT1..OUT8。
- TB6600 步进模式使用 `OUT1=PUL`、`OUT2=DIR`、`OUT3=ENA`。
- `RUN/STOP` 适配自锁急停常开触点：触点闭合时强制停止，松开后不会自动恢复运行。
- 固件每 1 秒打印一行状态，例如：

```text
run=1 mode=STEPPER in=0x00 out=0x02 step=1 dir=CW pps=200 ena=1 estop=0
```

因此 `pico_plc_hmi.html` 可以直接解析当前 demo 的日志来监控 IN/OUT 与步进状态。

## HTML 上位机文件

- 文件：`tools/pico_plc_hmi.html`
- 运行方式：用新版 Chrome 或 Edge 打开该 HTML，点击“连接”，选择 Pico 的 USB 串口。
- 解析兼容：
  - 文本：`run=1 mode=STEPPER in=0x00 out=0x02 step=1 dir=CW pps=200 ena=1 estop=0`
  - JSON：`{"run":1,"mode":"STEPPER","in":0,"out":2,"step":1,"dir":"CW","pps":200,"ena":1}`

## 上位机发送协议

```text
GET
OUT 0xNN
OUT n ON
OUT n OFF
RUN 1
RUN 0
MODE MANUAL
MODE STEPPER
STEP START
STEP STOP
STEP DIR CW
STEP DIR CCW
STEP SPEED 200
STEP EN ON
STEP EN OFF
```

`STEP SPEED` 的单位为 pulse/s，固件限制范围是 1..2000。执行步进命令后，Pico 会返回统一状态行。

当前固件已把 TB6600 `ENA` 按低电平有效处理。如果驱动板 ENA 需要高电平有效，在固件中把 `kStepperEnableActiveHigh` 改为 `true` 后重新编译。

`STEPPER` 模式下 OUT1-OUT3 被步进模块占用；上位机的辅助输出按钮只控制 OUT4-OUT8，可与步进电机同时运行。
