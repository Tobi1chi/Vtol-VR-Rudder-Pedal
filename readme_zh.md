# VTOL VR 脚舵项目说明（中文）

## 串口命令（`vtol_pedal_refactored`）

波特率：`115200`

当前固件使用单行命令式 CLI（不再使用旧版两步输入）。

不同目标板会裁剪可用模式：
- `vtol_pedal_refactored`（ESP32 原生 USB 目标）：支持 `mode hid|ble`
- `vtol_pedal_esp32c3`：仅支持 `mode ble`，`mode hid` 会显示禁用提示
- `vtol_pedal_rp2040`：仅支持 `mode hid`，`mode ble` 会显示禁用提示

### 命令列表

- `help`
- `status`
- `mode hid|ble`
- `debug on|off`
- `test on|off`
- `set curve 0|1`
- `set filter 0|1`
- `set min_l <1..4095>`
- `set min_r <1..4095>`
- `set max_l <1..4095>`
- `set max_r <1..4095>`
- `set erange <1..4095>`
- `save`（`vtol_pedal_refactored` 下为即时持久化提示命令）
- `factory_reset`

### 返回格式

- 成功：`OK: <message>`
- 失败：`ERR: <reason>`
- 状态：`STATUS: key=value`

### 输入规则

- 命令大小写不敏感。
- 支持前后空格与多空格。
- 数字解析严格（例如 `12abc` 会被拒绝）。
- 非法输入不会写入持久化配置。

## RP2040 版本（`vtol_pedal_rp2040`）

RP2040 目录复用了 ESP32 重构版本的整体结构：
- 输入采样与校准映射
- 贝塞尔曲线与低通滤波
- 串口命令控制

### RP2040 特性说明

- 使用 `Joystick` 库输出 USB HID。
- 目前仅支持 `mode hid`。
- 启动时会提示 `BLE disabled`，`status` 会显示 `BLE=DISABLED`。
- 输入 `mode ble` 时，CLI 会明确提示 RP2040 构建不支持 BLE。
- 设置持久化采用 EEPROM：
  - `save`：将当前设置写入 EEPROM
  - `factory_reset`：恢复默认并写入 EEPROM
- 支持互动式串口输入：
  - `mode`、`debug`、`test`、`set`、`set <key>` 可进入下一步提示
  - `cancel` 退出当前互动输入状态

### RP2040 UF2 烧录方式

1. 按住开发板上的 `BOOT` 键不放。
2. 按住 `BOOT` 的同时将设备插入 USB。
3. 松开 `BOOT`，此时会出现一个 U 盘设备。
4. 将编译得到的 `.uf2` 文件直接拷贝到该设备中。
5. 开发板会自动重启并运行新固件。

## ESP32-C3 版本（`vtol_pedal_esp32c3`）

ESP32-C3 目录复用了重构版的大部分输入/滤波/串口 CLI 逻辑，但输出能力与原 ESP32 工程不同。

### ESP32-C3 特性说明

- 当前仅支持 `mode ble`。
- 启动时会提示 `USB HID disabled / BLE only`。
- `status` 会显示 `HID=DISABLED`。
- 输入 `mode hid` 时，CLI 会明确提示：ESP32-C3 片上 USB 是 `USB Serial/JTAG`，不能作为原生 USB HID gamepad 使用。
- 当前默认引脚定义中：
  - `GPIO4` / `GPIO5` 用作两个 ADC 舵轴输入，属于低风险选择
  - `GPIO0` 作为按钮脚可用但不是优选，若做正式硬件建议改到 `GPIO1/2/3/10` 这类更稳的普通 IO
