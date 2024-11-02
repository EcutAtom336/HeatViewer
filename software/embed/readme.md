# 手持热成像 - HeatViewer 软件部分

## 子模块（位于 hal）

- buzzer

对应用提供预设的 buzzer 音频。

- hw_init

硬件初始化相关。

- laser

激光指示相关。

- led

led 相关。

- power_manage

电源管理相关。

- sd_card_ex

sd 卡拓展模块，用于检测 sd 卡是否存在。

- sd_card

gd32 官方提供的 sd card 驱动库。

- sw_init

软件初始化相关。

## 应用模块（位于 app）

- app

主应用程序。

- mlx90640_handler

mlx90640 数据处理程序。

- on_board_led

开发板 led 闪烁。

## 其他软件

- 响应式按键管理库 [MultiButton](https://github.com/0x1abin/MultiButton)

- 轻量级 log 库 [EasyLogger](https://github.com/armink/EasyLogger.git)

- 轻量级 rtos [Freertos](https://github.com/FreeRTOS/FreeRTOS.git)

- ARM Cortex-M 系列 MCU 错误追踪库 [CmBacktrace](https://github.com/armink/CmBacktrace?tab=readme-ov-file)

- 通用 FAT 文件系统模块 [FatFs](https://github.com/abbrev/fatfs)

- MLX90640 官方驱动库 [MLX90640Driver](https://github.com/melexis/mlx90640-library)
