# Pico PLC HMI 工具说明

## 已确认的 Pico 端原理

- `controller_demo` 通过 USB CDC 打印状态，CMake 中 `pico_enable_stdio_usb(controller_demo 1)` 已启用，UART stdio 未启用。
- 8 路输入 `IO_IN1..IO_IN8` 对应 Pico `GP2..GP9`，高电平有效，软件去抖时间为 8 ms。
- 8 路输出 `IO_OUT1..IO_OUT8` 对应 Pico `GP10..GP17`，高电平有效，`PicoPlcBoard::set_outputs(mask)` 按 bit0..bit7 写 OUT1..OUT8。
- 当前 demo 每 1 秒打印一行：

```text
run=1 mode=MIRROR in=0x03 out=0x03
```

因此 `pico_plc_hmi.html` 可以直接解析当前 demo 的日志来监控 IN/OUT。

## 当前固件限制

当前 `demo/controller_demo.cpp` 没有读取 USB 串口输入，也没有解析上位机命令；并且主循环会按本地模式持续刷新输出。所以 HTML 页面可以发送控制命令，但 Pico 端需要增加命令解析后，OUT 控制才会真正生效。

## HTML 上位机文件

- 文件：`tools/pico_plc_hmi.html`
- 运行方式：用新版 Chrome 或 Edge 打开该 HTML，点击“连接”，选择 Pico 的 USB 串口。
- 解析兼容：
  - 当前 demo 文本：`run=1 mode=MIRROR in=0x03 out=0x03`
  - JSON 文本：`{"run":1,"mode":"MANUAL","in":3,"out":3}`

## 页面发送的建议协议

```text
GET
OUT 0xNN
OUT n ON
OUT n OFF
RUN 1
RUN 0
MODE MANUAL
```

建议 Pico 端执行命令后仍返回统一状态行：

```text
run=1 mode=MANUAL in=0x03 out=0xA5
```

