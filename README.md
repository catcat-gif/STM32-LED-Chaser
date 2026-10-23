# STM32 流水灯（四灯 + 按键换向）

STM32F103C8T6 最小系统板（蓝药丸）+ 面包板，4 个 LED 做流水灯；按一下按键，流水方向立刻反向。
Keil MDK-ARM 5 + STM32CubeMX 生成，**HAL 库**。

## 一、功能

| 功能 | 说明 |
| --- | --- |
| 基础要求 | 4 个 LED 依次点亮，形成流水效果，间隔 200 ms（`STEP_MS` 可改） |
| 进阶要求 | 按 PA0 按键切换流水方向（正向 ↔ 反向），按键 20 ms 消抖，按下立即生效 |
| 实现方式 | 用 `HAL_GetTick()` 做非阻塞定时，按键扫描不会被 `HAL_Delay()` 卡住 |

## 二、硬件接线

| 器件 | 一端 | 另一端 |
| --- | --- | --- |
| 蓝灯 | 长脚(正极) → 限流电阻 → **PB12** | 短脚(负极) → 面包板负极轨 |
| 黄灯 | 长脚(正极) → 限流电阻 → **PB13** | 短脚(负极) → 面包板负极轨 |
| 绿灯 | 长脚(正极) → 限流电阻 → **PB14** | 短脚(负极) → 面包板负极轨 |
| 红灯 | 长脚(正极) → 限流电阻 → **PB15** | 短脚(负极) → 面包板负极轨 |
| 按键 | 一脚 → **PA0** | 另一脚 → GND |
| 电源 | 板子 **GND** → 面包板蓝色负极轨 | 板子 3.3V 不用接灯 |

要点：

- 每个 LED **必须**串一个限流电阻（220 Ω ~ 1 kΩ），电阻两只脚要跨在面包板不同的横排上，否则等于短路。
- LED 是**高电平点亮**（长脚接 GPIO，短脚接 GND）。
- 板子 GND 和面包板负极轨之间必须有一根线，否则灯不亮。

## 三、引脚定义

| 宏 | 端口 | 引脚 | 作用 |
| --- | --- | --- | --- |
| `LED_BLUE_PIN` | GPIOB | PB12 | 蓝灯 |
| `LED_YELLOW_PIN` | GPIOB | PB13 | 黄灯 |
| `LED_GREEN_PIN` | GPIOB | PB14 | 绿灯 |
| `LED_RED_PIN` | GPIOB | PB15 | 红灯 |
| `BTN_PIN` | GPIOA | PA0 | 按键（上拉输入，按下为低） |

> 如果你的板子实际接的是另一组引脚，只需要改 `Core/Src/main.c` 顶部的 `WIRING` 宏（1 = PB12~PB15，0 = 旧图 PB8/PB9/PA1/PA2）。

## 四、编译与烧录

1. 用 Keil 打开 `Led/MDK-ARM/Led.uvprojx`。
2. `F7` 编译（0 Error 0 Warning）。
3. 用 ST-Link 连板子（SWDIO / SWCLK / GND / 3.3V），`F8` 下载。
4. 也可以直接烧 `firmware/Led.hex`（ST-Link Utility / STM32CubeProgrammer / FlyMcu 串口均可）。

改流水速度：改 `main.c` 里的 `STEP_MS`（默认 200 ms，越小越快）。

## 五、目录结构

```
stm32-led/
├─ Led/                     CubeMX 工程
│  ├─ Core/Src/main.c       全部业务逻辑在这里
│  ├─ Core/Inc/main.h
│  ├─ Drivers/              HAL 库 + CMSIS
│  └─ MDK-ARM/Led.uvprojx   Keil 工程文件
├─ firmware/Led.hex         编译好的固件（可直接烧录）
└─ README.md
```

## 六、仓库与演示

- **GitHub 仓库**：<https://github.com/catcat-gif/STM32-LED-Chaser>
- **演示视频**：`演示视频.mp4`（19 s，正向流水 → 按 PA0 换向 → 反向流水）

## 七、备注

- 系统时钟用的是内部 HSI 8 MHz，`HAL_Delay` / `HAL_GetTick` 计时准确，作业够用；想跑 72 MHz 可在 CubeMX 里开 PLL。
- 按键只用软件消抖（20 ms），没有加外部电容。
