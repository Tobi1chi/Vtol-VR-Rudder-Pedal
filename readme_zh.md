# VTOL VR 脚舵项目说明（中文）

## 串口命令（`vtol_pedal_refactored`）

波特率：`115200`

当前固件使用单行命令式 CLI（不再使用旧版两步输入）。

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
