# create-project.ps1
# CW32 standalone project generator (5-layer architecture).
# Copies the 5-layer template sources from a source repo (cw32-dev) and the
# vendor standard peripheral library into Drivers/, producing a fully
# independent project that does not depend on cw32-dev.
#
# Usage:
#   powershell -ExecutionPolicy Bypass -File create-project.ps1 `
#     -Name my_motor -Chip cw32l012 -OutDir D:\work `
#     -SourceRoot D:\ai-project\dev\cw32-dev -Template motor_control
param(
    [string]$Name = "",                          # project (= app) name, required
    [ValidateSet("cw32l010", "cw32l011", "cw32l012", "cw32f003", "cw32f030", "cw32l031")]
    [string]$Chip = "cw32l012",                  # target chip
    [string]$OutDir = (Join-Path $PSScriptRoot "projects"),  # output dir
    [string]$SourceRoot = "D:\ai-project\dev\cw32-dev",      # cw32-dev repo path
    [ValidateSet("motor_control", "power_supply", "blink")]
    [string]$Template = "motor_control",         # source template
    [string]$Board = "cw32l0xx_mini"             # board (BSP source)
)

$ErrorActionPreference = "Stop"

if (-not $Name) { throw "Missing required -Name (project name)" }
if (-not (Test-Path -LiteralPath $SourceRoot)) { throw "SourceRoot not found: $SourceRoot" }

# chip -> compile define
$defineMap = @{
    "cw32l010" = "CW32L010"; "cw32l011" = "CW32L011"; "cw32l012" = "CW32L012"
    "cw32f003" = "CW32F003"; "cw32f030" = "CW32F030"; "cw32l031" = "CW32L031"
}
$ChipDefine = $defineMap[$Chip]
$SdkTarget = "${Chip}_sdk"

# validate source dirs
$srcApp = Join-Path $SourceRoot "apps\$Template"
$srcSdk = Join-Path $SourceRoot "sdk\$Chip"
$srcBoardDir = Join-Path $SourceRoot "boards\$Board"
foreach ($p in @($srcApp, $srcSdk, $srcBoardDir)) {
    if (-not (Test-Path -LiteralPath $p)) { throw "Source dir not found: $p" }
}

$dest = Join-Path $OutDir $Name
New-Item -ItemType Directory -Path $dest -Force | Out-Null

# ---- 1) copy 5-layer template sources ----
foreach ($layer in @("App", "Core", "Device", "System", "BSP")) {
    $s = Join-Path $srcApp $layer
    $d = Join-Path $dest $layer
    if (Test-Path -LiteralPath $s) {
        Copy-Item -LiteralPath $s -Destination $d -Recurse -Force
    }
}

# ---- 2) copy vendor standard peripheral library into Drivers/ ----
New-Item -ItemType Directory -Path (Join-Path $dest "Drivers") -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $srcSdk "inc") -Destination (Join-Path $dest "Drivers\inc") -Recurse -Force
Copy-Item -LiteralPath (Join-Path $srcSdk "src") -Destination (Join-Path $dest "Drivers\src") -Recurse -Force

# ---- 3) startup file / linker script (referenced by cw32_app) ----
New-Item -ItemType Directory -Path (Join-Path $dest "startup") -Force | Out-Null
New-Item -ItemType Directory -Path (Join-Path $dest "lds") -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $SourceRoot "startup\startup_$Chip.s") -Destination (Join-Path $dest "startup\") -Force
Copy-Item -LiteralPath (Join-Path $SourceRoot "lds\$Chip.ld") -Destination (Join-Path $dest "lds\") -Force

# ---- 4) CMSIS headers (SDK dependency) ----
Copy-Item -LiteralPath (Join-Path $SourceRoot "cmsis") -Destination (Join-Path $dest "cmsis") -Recurse -Force

# ---- 5) board support package (board.c/h -> BSP/) ----
Copy-Item -LiteralPath (Join-Path $srcBoardDir "board.c") -Destination (Join-Path $dest "BSP\board.c") -Force
Copy-Item -LiteralPath (Join-Path $srcBoardDir "board.h") -Destination (Join-Path $dest "BSP\board.h") -Force

# ---- 6) env script / cmake toolchain / pyocd config ----
Copy-Item -LiteralPath (Join-Path $SourceRoot "setup-toolchain.ps1") -Destination (Join-Path $dest "setup-toolchain.ps1") -Force
Copy-Item -LiteralPath (Join-Path $SourceRoot "pyocd.yaml") -Destination (Join-Path $dest "pyocd.yaml") -Force
New-Item -ItemType Directory -Path (Join-Path $dest "cmake") -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $SourceRoot "cmake\toolchain-arm-none-eabi.cmake") -Destination (Join-Path $dest "cmake\") -Force
Copy-Item -LiteralPath (Join-Path $SourceRoot "cmake\cw32.cmake") -Destination (Join-Path $dest "cmake\") -Force

# ---- 7) collect 5-layer source files, generate top CMakeLists.txt ----
# board.c is built into board_standalone, exclude it from the app sources.
# Use forward slashes in CMake paths.
$appSrcs = @("App/main.c", "App/app_task.c")
foreach ($layer in @("App", "Core", "Device", "System", "BSP")) {
    $dir = Join-Path $dest $layer
    if (-not (Test-Path -LiteralPath $dir)) { continue }
    Get-ChildItem -LiteralPath $dir -Filter "*.c" -File | ForEach-Object {
        $rel = "$layer/$($_.Name)"
        if ($rel -eq "BSP/board.c") { return }
        if ($rel -notin $appSrcs) { $appSrcs += $rel }
    }
}
$srcList = ($appSrcs | ForEach-Object { "  $_`r" }) -join ""
$includeDirs = ("App", "Core", "Device", "System", "BSP" | ForEach-Object {
    "  `${CMAKE_CURRENT_SOURCE_DIR}/$_"
}) -join "`r`n"

$ledDefines = "BOARD_LED_GPIO=CW_GPIOB`r`n  BOARD_LED_PIN=GPIO_PIN_2"

$cmake = @"
cmake_minimum_required(VERSION 3.20)
project($Name C ASM)

# ============ chip / board / rtos (overridable) ============
set(CW32_CHIP  "$Chip"       CACHE STRING "CW32 chip")
set(CW32_BOARD "standalone"  CACHE STRING "Board target name")
set(CW32_RTOS  "none"        CACHE STRING "RTOS (none|freertos|rtthread)")
set(CW32_ENABLE_FLASH OFF    CACHE BOOL   "Generate flash / flash_reset targets")

include(cmake/cw32.cmake)

# ============ vendor standard peripheral library (Drivers/) ============
add_subdirectory(Drivers)

# ============ board support package (BSP: board.c + board wiring) ============
add_library(board_standalone STATIC BSP/board.c)
target_link_libraries(board_standalone PUBLIC ${SdkTarget})
target_include_directories(board_standalone PUBLIC BSP)
target_compile_definitions(board_standalone PUBLIC
  $ledDefines
)

# ============ five-layer application ============
cw32_app($Name
$srcList)
target_include_directories($Name PRIVATE
$includeDirs
)
"@
Set-Content -LiteralPath (Join-Path $dest "CMakeLists.txt") -Value $cmake -Encoding UTF8

# ---- 8) Drivers/CMakeLists.txt (vendor lib -> ${Chip}_sdk) ----
$drvCmake = @'
# Drivers/CMakeLists.txt - vendor standard peripheral library (__CHIP__)
set(TARGET __SDK_TARGET__)

file(GLOB SDK_SRCS src/*.c)
add_library(${TARGET} STATIC ${SDK_SRCS})

target_include_directories(${TARGET} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_SOURCE_DIR}/cmsis
)
target_compile_definitions(${TARGET} PUBLIC __CHIP_DEFINE__)

# suppress warnings from vendor code only
target_compile_options(${TARGET} PRIVATE
  -Wno-type-limits
  -Wno-old-style-declaration
  -Wno-unused-parameter
)
'@
$drvCmake = $drvCmake.Replace("__CHIP__", $Chip).Replace("__SDK_TARGET__", $SdkTarget).Replace("__CHIP_DEFINE__", $ChipDefine)
Set-Content -LiteralPath (Join-Path $dest "Drivers\CMakeLists.txt") -Value $drvCmake -Encoding UTF8

Write-Host "Project generated: $dest"
Write-Host ""
Write-Host "Next steps (run inside the project dir):"
Write-Host "  1) powershell -ExecutionPolicy Bypass -File setup-toolchain.ps1"
Write-Host "  2) cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-arm-none-eabi.cmake"
Write-Host "     ninja -C build"
Write-Host "  3) pyocd flash -t ${Chip}c8 build/$Name.hex"