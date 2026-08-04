---
name: cw32-flash
description: CW32 烧录指南（pyocd）。Use when the user asks to flash/program/烧录 a compiled CW32 hex to a CW32L010/L011/L012 (or CW32F030) board via pyocd + CMSIS-DAP, when they ask to download/burn/烧录 the firmware, when needing the exact pyocd flash/reset commands, getting the debug/flash target name (PYOCD_TARGET), checking pyocd.yaml / local DFP packs, or restoring the CMake flash targets. Covers the flash workflow: run setup-toolchain.ps1 first (auto-download toolchain + DFP), then pyocd flash -t <target> <file>.hex. Front-load keywords: 烧录, flash, pyocd, CMSIS-DAP, SWD, DFP, hex, PYOCD_TARGET, pyocd.yaml, pyocd flash, cw32l010f8, cw32l011k8, cw32l012c8, cw32f030c8.
license: MIT
metadata:
  tool: pyocd
  debug_interface: CMSIS-DAP
  supported_chips: cw32l010|cw32l011|cw32l012|cw32f030
---

# CW32 烧录（pyocd）：toolchain 引导 → flash → 复位

本 skill 覆盖把编译好的 CW32 hex 通过 pyocd 烧录到目标板（CMSIS-DAP/SWD），以及烧录前的环境准备。配合 `cw32-framework`（建工程/编译产 hex）与 `cw32l010`/`cw32l011`/`cw32l012` 芯片 skill 使用。

自动烧录的 CMake 目标默认关闭（`CW32_ENABLE_FLASH` OFF）：烧录由 skill 用 pyocd 直调完成。**应用每次烧录都遵循 L0xx 那一套相同的流程。**

## 0. 环境准备（每次烧录前先跑，幂等）

先运行环境下载脚本，检测并补齐 pyocd 与本地 DFP pack（已存在则跳过，不重复下载）：

```powershell
powershell -ExecutionPolicy Bypass -File setup-toolchain.ps1
```

- 脚本位置：
  - cw32-dev 仓库根：`reference/setup-toolchain.ps1`（本 skill 附带副本）。
  - 由 `create-project.ps1` 生成的**完全独立工程**：`<project>/setup-toolchain.ps1`（生成时自动复制）。
- 作用：装 pyocd（`pip install pyocd`）+ 校验 `tools/pyocd/` 下 4 个本地 DFP `.pack`。
- 若 pyocd 已全局可用可跳过：`setup-toolchain.ps1 -SkipPyocd`。

## 1. 确认 flash target（PYOCD_TARGET）

pyocd 的 `-t` 是烧录目标名，按芯片固定：

| 芯片 | target（-t） |
|---|---|
| CW32L010 | `cw32l010f8` |
| CW32L011 | `cw32l011k8` |
| CW32L012 | `cw32l012c8` |
| CW32F030 | `cw32f030c8` |

- 独立工程的 `CMakeLists.txt` 里已把 `PYOCD_TARGET` 与芯片关联（用 `create-project.ps1` 生成的工程无需手填）。
- 校验命令：`pyocd list --targets` 应包含上述 target（来自本地 DFP）。target 不存在 = 缺相应 DFP pack。

## 2. pyocd.yaml（DFP 指向，缺一不可）

pyocd 必须在能解析 CW32 target 的目录下运行。两种方式：

- **cw32-dev 根目录**：仓库自带 `pyocd.yaml`，指向 `tools/pyocd/` 下 4 个 DFP；在根目录运行签名即生效。
- **独立工程**：根目录自带 `pyocd.yaml`（生成时复制）。

若在别处运行且报 "unable to find the target"，把工作目录切到含 `pyocd.yaml` 的根目录，或显式 `pyocd flash --pack <dfp.pack> ...`。

## 3. 烧录

```powershell
# 烧录（hex 路径 = build/<app>/<app>.hex，或独立工程的 build/<name>.hex）
pyocd flash -t ${PYOCD_TARGET} build/<app>/<app>.hex

# 需要复位目标时
pyocd reset -t ${PYOCD_TARGET}
```

- 调试器：CMSIS-DAP（板载 DAPLink/WCH-LinkE 等）。未接调试器时 `pyocd flash` 会等待/报找不到探针。
- 反复烧录不需每次跑 setup-toolchain；只有首次或换了环境/缺组件时才需要。

## 4. 恢复 CMake flash 目标（可选）

默认不生成 `flash`/`flash_reset` 目标。若需要（比如 CI 或命令行一键烧录），配置时加：

```powershell
cmake -B build/<app> -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake `
  -DCW32_ENABLE_FLASH=ON -DPYOCD_TARGET=cw32l012c8 ...
```

之后 `cmake --build build/<app> --target flash` 或 `--target flash_reset` 可用（内部仍是调 pyocd）。

## 参考文件

- `reference/setup-toolchain.ps1`：pyocd + DFP 引导下载脚本（幂等）。
- `reference/pyocd.yaml`：DFP pack 指向示例。
- cw32-dev 仓库根：`pyocd.yaml`、`tools/pyocd/*.pack`、`cmake/cw32.cmake`（`CW32_ENABLE_FLASH` 开关实现）。