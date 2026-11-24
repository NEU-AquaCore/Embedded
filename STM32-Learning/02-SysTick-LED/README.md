# 02：SysTick 毫秒计时与非阻塞闪灯

这是 `01-LED-Blink` 的下一关。保留原实验不动，仍使用 VS Code、ARM GCC、CMake、Ninja 和寄存器编程，不依赖 HAL 或标准外设库。

## 你会看到什么

- 适用于常见 PC13 低电平点亮的板载 LED；请以板子原理图为准。
- 上电后先灭灯，约 500ms 后点亮，再过约 500ms 熄灭，完整周期约 1 秒。
- 使用内部 HSI，标称 8MHz。SysTick 以 HCLK 为时钟，重装值为 `7999`，标称每 1ms 中断一次。HSI 有误差，这不是精密时钟校准实验。
- 主循环不调用阻塞延时，之后可以加入按键扫描和串口处理。

## 程序是怎样工作的

1. `startup/startup.s` 设置初始栈、复制 `.data`、清零 `.bss`，随后调用 `main()`。
2. 向量表第 15 项接到 `SysTick_Handler`，而不是默认死循环。
3. `clock_init()` 主动选择 HSI，并设置 AHB 不分频。
4. `systick_init()` 启用 1ms 中断。
5. `SysTick_Handler()` 只增加 `g_milliseconds`。
6. 主循环用 `now - last_toggle >= LED_INTERVAL_MS` 判断是否到时，再翻转 LED。

`g_milliseconds` 使用 `volatile`，让主循环每次读取中断更新后的值。时间判断使用无符号减法，可以跨越一次 32 位计数器回绕；程序需要在计数器转满一圈（约 49.7 天）前检查。

**非阻塞不等于省电：** 当前主循环仍不断检查时间，只是不再卡在延时函数里。低功耗可以留到后续实验。

## ST-Link 接线

断电接线，确认后再上电：

| ST-Link 信号 | 开发板信号 |
|---|---|
| SWDIO | PA13 / SWDIO |
| SWCLK | PA14 / SWCLK |
| GND | GND |
| VTref（若调试器提供） | 板子的 3.3V，作为电压参考 |

开发板需要合适的电源。常见小型 ST-Link 的 `3.3V` 引脚可能是电源输出，不要把它当成 VTref 输入；按你的调试器说明接线，并避免与板子的另一电源输出直接并联。保持 BOOT0 = 0。本配置使用软件系统复位，不要求连接 NRST。

## 在 VS Code 中构建与调试

以整个 `D:\Embedded` 文件夹作为工作区：

1. `Ctrl+Shift+B`：运行默认任务 `02: Build`，先配置再编译。
2. 接好 ST-Link 后，打开“运行和调试”，选择 `02: SysTick LED (ST-Link)`。
3. **按 F5 会自动构建、下载固件并复位，会替换板上当前程序。**
4. 初次停在 `main()` 是正常的；按 F5 继续运行，LED 才会开始闪烁。
5. 想观察变量，继续运行一会后按暂停，再在“监视”中添加：
   - `g_milliseconds`：累计运行毫秒数。
   - `g_led_is_on`：当前逻辑亮灭状态。
6. 可以在 `led_toggle()` 内打断点。不要一直停在每毫秒执行一次的中断里，否则程序几乎一直暂停。

暂停调试时，SysTick 也会暂停；调试期间的时间不是完整的墙上时间。如果自动配置后提示选择 Kit，选择对应 ARM 工具链，不要选桌面版 GCC。

工作区已配置本机实际的 CMake、ARM 工具链、OpenOCD 路径。工具搬位置后需更新 `.vscode/settings.json`，OpenOCD 的脚本目录需更新 `.vscode/launch.json` 中的 `searchDir`。要重新回到实验 01，调整工作区的 CMake 源目录、构建目录和 toolchain 路径。

## PowerShell 手动构建

在工作区根目录执行：

```powershell
cmake -S STM32-Learning/02-SysTick-LED -B STM32-Learning/02-SysTick-LED/build -G Ninja -DCMAKE_TOOLCHAIN_FILE=D:/Embedded/STM32-Learning/02-SysTick-LED/cmake/arm-none-eabi.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build STM32-Learning/02-SysTick-LED/build
```

产物在本实验的 `build/`：`STM32_SysTick_LED.elf`、`.hex`、`.bin` 和 `.map`。这份链接脚本按 F103C8 官方容量使用 64KB Flash、20KB RAM，并为栈保留至少 1KB 空间，不使用堆。

## 计时逻辑测试（不需要开发板）

先完成上面的构建，再在工作区根目录执行：

```powershell
gcc -std=c11 -Wall -Wextra -Werror -I STM32-Learning/02-SysTick-LED/src STM32-Learning/02-SysTick-LED/tests/test_timing.c -o STM32-Learning/02-SysTick-LED/build/test_timing.exe
& ./STM32-Learning/02-SysTick-LED/build/test_timing.exe
```

这里的 `gcc` 是本机 MSYS2 的桌面编译器，不是 ARM 编译器。预期输出：

```text
PASS: 10 timing checks, including uint32_t wraparound.
```

这些测试验证时间判断的阈值和回绕情况，不替代板上时钟、中断和 LED 的实测。

## 自己动手改一处

把 `src/main.c` 中的 `LED_INTERVAL_MS` 从 `500U` 改为 `100U`，重新构建、下载。观察闪烁速度变化，再解释：为什么改这里不会改变 SysTick 的 1ms 中断周期？

## 本次板上验证（2026-11-10）

- ST-Link V2，固件 V2J29S7，SWD 已连通；芯片报告设备 ID `0x20036410`，Flash 为 64KB。
- 原固件已完整备份到 `backups/firmware-before-systick-20261110-204133.bin`，大小 65536 字节，旁边的 `.sha256` 文件保存校验值。备份目录已忽略，不会误提交到 Git；请勿随意删除。
- 新固件烧录后通过 OpenOCD 校验。
- 在 `led_toggle()` 入口用硬件断点采样：毫秒计数为 `500 / 1000 / 1500`，对应 PC13 为 `1 / 0 / 1`。这些是翻转前的状态，说明前两次调用确实改变了引脚输出。
- 已核对 SysTick 重装值 `7999`、控制位 `0b111`、HSI 与 AHB 不分频、PC13 推挽输出，以及向量表地址 `0x08000000`。
- GDB 已实测连接、停止在 `main()`、读取 `g_milliseconds`，随后复位并脱离，恢复独立运行。VS Code 的 F5 配置已准备好，但未自动操作编辑器 UI。

### 当前工具链的已知提示

本机 ARM GDB 可能提示 `could not convert ... from the host encoding (CP1252) to UTF-32`，并在终端中把中文源码注释显示成乱码。本次连接、断点和变量读取均通过；当前 GDB 不支持用 `set host-charset UTF-8` 修正，暂未为此更换或安装工具链。

ST-Link 请求 1000kHz 时自动采用 950kHz，是它支持的实际档位，不影响本实验。烧录时提示补齐到 Flash 页末尾是正常的页擦除行为，原固件已在烧录前备份。
