# ESP32-C3 Pinout Selection Guide

目标：把 ESP32-C3 的 GPIO 选择规则整理成适合工程设计、也适合 AI 做 pin assignment 推理的参考文档。

当前仓库内 `vtol_pedal_esp32c3` 工程的直接相关结论：

- 固件模式上，当前构建是 `BLE only`。
- 不要为该工程规划 `USB HID gamepad` 路线；ESP32-C3 的片上 USB 在这里按 `USB Serial/JTAG` 约束处理，不按可自定义 HID 设备处理。
- 当前默认引脚定义是：
  - `GPIO4` -> `PIN_RUDDER_L`
  - `GPIO5` -> `PIN_RUDDER_R`
  - `GPIO0` -> `PIN_BUTTON`
- 对这组默认值的工程判断：
  - `GPIO4/GPIO5` 合理，属于低风险 ADC 选择
  - `GPIO0` 可用但不是按钮默认首选；如果没有板级约束，优先改到 `GPIO1/2/3/10`

---

## 1. 设计时的核心原则

ESP32-C3 具备 `GPIO Matrix`，因此多数数字外设并不严格绑定到固定引脚。

这意味着工程上做 pinout 选择时，优先级通常不是“某外设必须接某脚”，而是：

1. 先避开会影响启动的 strap / boot 相关引脚
2. 再决定是否保留 USB 引脚
3. 再决定是否保留默认 UART0 下载口
4. 最后把 SPI / I2C / PWM / 普通数字 IO 映射到安全 GPIO

简化判断：

- `最高优先级保留`：GPIO8、GPIO9
- `通常建议保留`：GPIO18、GPIO19、GPIO20、GPIO21
- `优先分配普通外设`：GPIO1、GPIO2、GPIO3、GPIO4、GPIO5、GPIO6、GPIO7、GPIO10
- `可用但需要额外确认`：GPIO0

---

## 2. 快速结论

### 2.1 最安全的普通 GPIO

优先给通用数字外设使用：

```text
GPIO1
GPIO2
GPIO3
GPIO4
GPIO5
GPIO6
GPIO7
GPIO10
```

适合：

- LED
- 按键
- 中断输入
- PWM
- SPI
- I2C
- 普通控制信号

### 2.2 尽量避免优先占用的 GPIO

```text
GPIO0
GPIO8
GPIO9
GPIO18
GPIO19
GPIO20
GPIO21
```

原因：

- `GPIO0`：带 ADC，且原文中按 boot 相关注意脚处理，使用前要再次确认启动电平影响
- `GPIO8 / GPIO9`：boot / strap 相关
- `GPIO18 / GPIO19`：USB Serial / JTAG
- `GPIO20 / GPIO21`：默认 UART0 下载 / 日志

对当前工程直接落地时，AI 应额外遵守：

- `PIN_RUDDER_L/PIN_RUDDER_R` 优先继续使用 `GPIO4/GPIO5`，除非板子 ADC 引出受限
- `PIN_BUTTON` 不要默认继续放在 `GPIO0`，除非明确知道上电电平不会被外设干扰
- 不要建议把 `GPIO18/GPIO19` 复用成与 USB HID 相关的功能，因为当前工程并不走该路线

---

## 3. GPIO 分组视图

### 3.1 Group A: 推荐优先分配

这些脚最适合作为默认资源池：

| GPIO | 典型用途 | 风险等级 | 说明 |
| --- | --- | --- | --- |
| GPIO1 | ADC / 普通 IO | Low | 安全，适合复用 |
| GPIO2 | ADC / SPI MISO | Low | 安全，常用于 SPI |
| GPIO3 | ADC / 普通 IO | Low | 安全 |
| GPIO4 | ADC / I2C SDA | Low | 安全，常用于 I2C |
| GPIO5 | ADC / I2C SCL | Low | 安全，常用于 I2C |
| GPIO6 | SPI SCLK / 普通 IO | Low | 安全 |
| GPIO7 | SPI MOSI / 普通 IO | Low | 安全 |
| GPIO10 | SPI CS / 普通 IO | Low | 安全 |

### 3.2 Group B: 可用但要先判断系统需求

| GPIO | 主要冲突 | 风险等级 | 说明 |
| --- | --- | --- | --- |
| GPIO0 | Boot / ADC 相关约束 | Medium | 可用，但不适合作为默认优先脚 |
| GPIO18 | USB_DM | Medium/High | 仅在不保留 USB 时复用 |
| GPIO19 | USB_DP | Medium/High | 仅在不保留 USB 时复用 |
| GPIO20 | UART0_RX | Medium | 仅在不依赖默认下载串口时复用 |
| GPIO21 | UART0_TX | Medium | 仅在不依赖默认下载串口时复用 |

### 3.3 Group C: 尽量避免

| GPIO | 原因 | 风险等级 | 建议 |
| --- | --- | --- | --- |
| GPIO8 | Strap / boot 相关 | High | 不接会强拉高/拉低的外设 |
| GPIO9 | Boot 模式相关 | High | 一般保留给 BOOT 按钮 |

---

## 4. Capability Matrix

这是给 AI 做 pin 选择最有用的一张表：先看是否冲突，再看是否满足能力需求。

| GPIO | ADC | Boot/Strap Sensitive | USB | Default UART | 推荐作为普通 IO | 备注 |
| --- | --- | --- | --- | --- | --- | --- |
| GPIO0 | Yes | Yes/Needs review | No | No | No | 可做 ADC，但不建议默认占用 |
| GPIO1 | Yes | No | No | No | Yes | 安全 |
| GPIO2 | Yes | No | No | No | Yes | 安全 |
| GPIO3 | Yes | No | No | No | Yes | 安全 |
| GPIO4 | Yes | No | No | No | Yes | 常作为 I2C SDA |
| GPIO5 | Yes | No | No | No | Yes | 常作为 I2C SCL |
| GPIO6 | No | No | No | No | Yes | 常作为 SPI SCLK |
| GPIO7 | No | No | No | No | Yes | 常作为 SPI MOSI |
| GPIO8 | No | Yes | No | No | No | strap 相关 |
| GPIO9 | No | Yes | No | No | No | boot 相关，常接 BOOT |
| GPIO10 | No | No | No | No | Yes | 常作为 SPI CS |
| GPIO18 | No | No | USB_DM | No | Conditional | 保留 USB 时不要占用 |
| GPIO19 | No | No | USB_DP | No | Conditional | 保留 USB 时不要占用 |
| GPIO20 | No | No | No | UART0_RX | Conditional | 默认下载 / 日志口 |
| GPIO21 | No | No | No | UART0_TX | Conditional | 默认下载 / 日志口 |

---

## 5. 外设推荐分配

注意：由于 GPIO Matrix，大多数数字外设都能换脚。下面是工程上“低风险、可维护”的默认推荐，而不是唯一合法答案。

### 5.1 I2C

推荐默认：

```text
SDA -> GPIO4
SCL -> GPIO5
```

原因：

- 都在安全 GPIO 池中
- 习惯上容易记忆
- 对多数板级设计比较稳定

### 5.2 SPI

推荐默认：

```text
MOSI -> GPIO7
MISO -> GPIO2
SCLK -> GPIO6
CS   -> GPIO10
```

原因：

- 全部位于低风险 GPIO 池
- 避开 USB / UART / boot 敏感脚

### 5.3 UART

默认下载 / 调试串口：

```text
TX -> GPIO21
RX -> GPIO20
```

规则：

- 如果板子要保留最省心的烧录 / 日志方案，就保留这两个脚
- 如果产品化后不再依赖默认串口，可重新映射，但要确保下载和调试流程已替代

### 5.4 USB

内置 USB Serial / JTAG：

```text
GPIO18 -> USB_DM
GPIO19 -> USB_DP
```

规则：

- 如果要 USB 下载、USB 调试、USB 虚拟串口，就不要复用这两脚
- 如果项目完全不用 USB，这两脚可回收，但应明确记录设计假设

### 5.5 PWM

PWM 可映射到任意适合的数字 GPIO，优先顺序建议：

```text
第一选择: GPIO1 / GPIO2 / GPIO3 / GPIO4 / GPIO5 / GPIO6 / GPIO7 / GPIO10
第二选择: GPIO18 / GPIO19 / GPIO20 / GPIO21
避免选择: GPIO0 / GPIO8 / GPIO9
```

---

## 6. ADC 使用建议

ADC 可用引脚：

| GPIO | ADC Channel |
| --- | --- |
| GPIO0 | ADC1_CH0 |
| GPIO1 | ADC1_CH1 |
| GPIO2 | ADC1_CH2 |
| GPIO3 | ADC1_CH3 |
| GPIO4 | ADC1_CH4 |
| GPIO5 | ADC1_CH5 |

已知工程注意点：

- 分辨率按原文记为 `12-bit`
- 输入范围按原文记为 `0-3.3V`
- Wi-Fi 工作时 ADC 精度可能受影响

工程建议：

- 如果既要 ADC 又要低风险布局，优先从 `GPIO1~GPIO5` 选
- `GPIO0` 虽然支持 ADC，但不要作为默认首选
- 对当前仓库的舵轴输入，`GPIO4 + GPIO5` 是合理默认值

---

## 7. Boot / Strap 规则

这是 AI 做自动 pin 规划时必须优先满足的约束。

### 7.1 强约束

- 不要把 `GPIO8`、`GPIO9` 接到会在上电时强驱动电平的外设
- `GPIO9` 通常保留给 `BOOT button`
- `GPIO9 = LOW` 会进入下载模式
- `GPIO9 = HIGH` 为正常启动

### 7.2 弱约束

- `GPIO0` 也不建议连接会在上电时干扰电平判断的外设
- 对 `GPIO0`、`GPIO8`、`GPIO9`，默认策略应为“能不用就不用”
- 因此如果当前工程的按钮功能后续要固化到硬件设计，AI 应优先建议将按钮从 `GPIO0` 迁移到普通安全 GPIO

---

## 8. 推荐的板级资源分配模板

适合大多数“需要 USB、串口、I2C、SPI” 的通用开发板或控制板。

```text
USB_DM   -> GPIO18
USB_DP   -> GPIO19
UART0_RX -> GPIO20
UART0_TX -> GPIO21
I2C_SDA  -> GPIO4
I2C_SCL  -> GPIO5
SPI_MOSI -> GPIO7
SPI_MISO -> GPIO2
SPI_SCLK -> GPIO6
SPI_CS   -> GPIO10

Free low-risk GPIO:
GPIO1
GPIO3
```

这个模板的特点：

- 保留 USB
- 保留默认 UART0
- 避开 boot 敏感脚
- 给常见外设预留了稳定布局

---

## 9. AI Pin Assignment Rules

如果让 AI 自动给 ESP32-C3 分配引脚，可以直接按下面规则执行。

### 9.1 选择顺序

1. 默认禁止分配 `GPIO8`、`GPIO9`
2. 默认不优先分配 `GPIO0`
3. 若 `requires_usb = true`，禁止分配 `GPIO18`、`GPIO19`
4. 若 `requires_uart0_download = true`，保留 `GPIO20`、`GPIO21`
5. SPI / I2C / PWM / 普通数字输出优先从 `GPIO1、2、3、4、5、6、7、10` 中选
6. ADC 优先从 `GPIO1、2、3、4、5` 中选，尽量最后才考虑 `GPIO0`

### 9.2 推荐优先级表

```text
Priority A  : GPIO1 GPIO2 GPIO3 GPIO4 GPIO5 GPIO6 GPIO7 GPIO10
Priority B  : GPIO18 GPIO19 GPIO20 GPIO21
Priority C  : GPIO0
Do not use  : GPIO8 GPIO9
```

### 9.3 简化伪代码

```text
if pin in {GPIO8, GPIO9}:
    reject

if pin == GPIO0:
    use_only_when_necessary

if need_usb and pin in {GPIO18, GPIO19}:
    reject

if need_default_uart0 and pin in {GPIO20, GPIO21}:
    reject

prefer pin in {GPIO1, GPIO2, GPIO3, GPIO4, GPIO5, GPIO6, GPIO7, GPIO10}
```

---

## 10. 最终结论

如果目标是“尽量不踩坑”的 ESP32-C3 pinout：

- 普通外设默认从 `GPIO1、2、3、4、5、6、7、10` 里选
- USB 固定看作 `GPIO18 / GPIO19`
- 默认串口固定看作 `GPIO20 / GPIO21`
- `GPIO8 / GPIO9` 视为保留脚
- `GPIO0` 视为谨慎使用脚

对 AI 来说，最重要的不是背所有功能，而是先遵守这些资源冲突规则。
