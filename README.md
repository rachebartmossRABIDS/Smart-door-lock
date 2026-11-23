# 智能门锁 (Smart Door Lock)

基于 **STM32F103C8** 的智能门锁固件，支持 **键盘密码**、**AS608 指纹** 与
**机智云 (Gizwits) App 远程** 三种开锁方式，配备 OLED 交互界面、步进电机锁体、
蜂鸣器报警，密码保存于片内 Flash。

> 本项目为学习 / 毕业设计用途，硬件与云端配置需自行准备。

## 功能特性

- **密码开锁**：4×4 矩阵键盘输入，密码存于 STM32 内部 Flash，掉电不丢失
- **指纹开锁**：AS608 指纹模块，支持录入 / 匹配 / 删除
- **远程开锁**：通过机智云 WiFi 模块与手机 App 远程开锁、查看门锁状态
- **报警功能**：多次错误触发蜂鸣器报警，并向 App 上报错误次数
- **OLED 界面**：主菜单 / 功能菜单，中文提示
- **锁体控制**：步进电机驱动开 / 关锁

机智云数据点（App 侧可读写）：

| 数据点 | 类型 | 说明 |
|--------|------|------|
| `Lock_Control` | bool | App 远程开 / 关锁指令 |
| `Lock_State`   | bool | 当前门锁状态上报 |
| `Alarm_Clear`  | bool | App 清除报警 |
| `Alarm_Count`  | uint32 | 报警 / 错误次数上报 |

## 硬件清单

| 模块 | 型号 / 说明 | 连接 |
|------|-------------|------|
| 主控 | STM32F103C8 (STM32F103C8T6, "Blue Pill") | — |
| 指纹模块 | AS608 | USART2 @ 57600 |
| WiFi 模块 | 机智云兼容模组 (GAgent) | USART3 (PB10 TX / PB11 RX) |
| 显示 | OLED (I2C/SPI) | 见 `Hardware/OLED.h` |
| 键盘 | 4×4 矩阵键盘 | 见 `Hardware/MatrixKey.h` |
| 锁体 | 步进电机 | 见 `Hardware/Stepper.h` |
| 报警 | 有源蜂鸣器 | 见 `Hardware/Buzzer.h` |

> 具体引脚定义请查看 `Hardware/` 下各驱动源文件。

## 目录结构

```
智能门锁/
├── User/            主程序 main.c、中断服务、STM32 配置
├── Hardware/        外设驱动：AS608 指纹、OLED、矩阵键盘、步进电机、蜂鸣器、串口
├── System/          系统级：延时、定时器、Flash 读写、密码存储
├── Gizwits/         机智云物联网协议与产品数据点
├── Utils/           机智云工具：数据点处理、MD5、环形缓冲、通用函数
├── Library/         ST 标准外设库 (STM32F10x StdPeriph)
├── Start/           启动文件、CMSIS 内核、系统时钟
├── Project.uvprojx  Keil MDK 工程文件
├── Project.uvoptx   Keil 工程选项
└── keilkill.bat     清理编译产物脚本
```

> `Objects/`、`Listings/`、`DebugConfig/` 为 Keil 编译 / 调试自动生成，
> 已通过 `.gitignore` 排除，不纳入版本库。

## 编译与烧录

1. 安装 **Keil MDK-ARM**（含 STM32F10x 器件支持包）。
2. 双击打开 `Project.uvprojx`。
3. **填入机智云凭据**（见下节），否则无法联网。
4. 选择目标器件 `STM32F103C8`，编译 (F7)。
5. 通过 ST-Link / J-Link 下载 (F8) 到开发板。

如需清理编译产物，可运行 `keilkill.bat`。

## 机智云配置（重要）

为安全起见，公开仓库中的 `Gizwits/gizwits_protocol.h` 已将产品凭据**脱敏为占位符**：

```c
#define PRODUCT_KEY    "YOUR_PRODUCT_KEY_HERE"      // 第 41 行
#define PRODUCT_SECRET "YOUR_PRODUCT_SECRET_HERE"   // 第 46 行
```

编译 / 烧录前，请替换为你自己在 [机智云开发者中心](https://www.gizwits.com)
创建产品后获得的 `Product Key` 与 `Product Secret`，并在云端定义与上表一致的
数据点，同时使用机智云提供的 WiFi 模组固件。

> ⚠️ 请勿把真实的 `PRODUCT_SECRET` 提交到公开仓库，以免被他人盗用接入你的产品。

## 说明

- 源码中部分中文注释采用 GBK 编码（Keil 默认），在 GitHub 网页上可能显示为乱码；
  用 Keil 或支持 GBK 的编辑器打开即可正常显示，不影响编译。
- 本仓库不包含编译生成的 `.hex` / `.axf` 固件，请自行编译。

## 许可证

本项目基于 [MIT License](LICENSE) 开源。

其中 `Library/`（STMicroelectronics 标准外设库）与 `Gizwits/`、`Utils/`
（机智云 SDK）等第三方代码，版权归各自作者所有，遵循其原始许可条款。
