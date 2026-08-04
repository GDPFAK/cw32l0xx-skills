# setup-toolchain.ps1
# CW32 开发框架工具链引导安装脚本。
# 用途：确保 tools/ 下具备 arm-none-eabi-gcc / CMake / Ninja / pyocd(+DFP)。
# 行为（幂等）：
#   1. 检测每个组件；已存在则跳过（不重复下载）。
#   2. 缺失时优先从 -CopyFrom 目录复制（本机已有工具链时最快）。
#   3. 否则从官方源下载 zip 并解压到 tools/。
# 用法：
#   powershell -ExecutionPolicy Bypass -File setup-toolchain.ps1
#   powershell -ExecutionPolicy Bypass -File setup-toolchain.ps1 -CopyFrom "D:\somewhere\tools"
#   powershell -ExecutionPolicy Bypass -File setup-toolchain.ps1 -SkipPyocd   # 已全局装 pyocd 时
param(
    [string]$ToolsRoot = (Join-Path $PSScriptRoot "tools"),
    [string]$CopyFrom = "",            # 已有工具链目录，复制模式
    [switch]$SkipPyocd,                # 跳过 pyocd 检查
    [switch]$Force                     # 重新下载覆盖已存在组件
)

$ErrorActionPreference = "Stop"

# ---- 组件定义：检测路径 / 复制源子目录 / 下载 zip URL / zip 内顶层目录 ----
$Components = @(
    @{
        Name       = "arm-none-eabi-gcc 13.3.1"
        CheckPath  = "gcc\xpack-arm-none-eabi-gcc-13.3.1-1.1\bin\arm-none-eabi-gcc.exe"
        CopySubDir = "gcc"
        Url        = "https://github.com/xpack-dev-tools/arm-none-eabi-gcc-xpack/releases/download/v13.3.1-1.1/xpack-arm-none-eabi-gcc-13.3.1-1.1-win32-x64.zip"
        ZipTop     = "xpack-arm-none-eabi-gcc-13.3.1-1.1"
        DestSubDir = "gcc"
    },
    @{
        Name       = "CMake 3.30.5"
        CheckPath  = "cmake-3.30.5-windows-x86_64\bin\cmake.exe"
        CopySubDir = "cmake-3.30.5-windows-x86_64"
        Url        = "https://github.com/Kitware/CMake/releases/download/v3.30.5/cmake-3.30.5-windows-x86_64.zip"
        ZipTop     = "cmake-3.30.5-windows-x86_64"
        DestSubDir = ""
    },
    @{
        Name       = "Ninja 1.12.1"
        CheckPath  = "ninja\ninja.exe"
        CopySubDir = "ninja"
        Url        = "https://github.com/ninja-build/ninja/releases/download/v1.12.1/ninja-win.zip"
        ZipTop     = ""              # ninja-win.zip 顶层就是 ninja.exe
        DestSubDir = "ninja"
    }
)

function Test-Component([hashtable]$c) {
    Test-Path -LiteralPath (Join-Path $ToolsRoot $c.CheckPath)
}

function Copy-Component([hashtable]$c) {
    $src = Join-Path $CopyFrom $c.CheckPath
    if (-not (Test-Path -LiteralPath $src)) {
        Write-Host "  [copy] source missing: $src"
        return $false
    }
    $dst = Join-Path $ToolsRoot $c.CopySubDir
    New-Item -ItemType Directory -Path $dst -Force | Out-Null
    Copy-Item -LiteralPath $src -Destination $dst -Recurse -Force
    Write-Host "  [copy] $($c.Name) <- $src"
    return $true
}

function Download-Component([hashtable]$c) {
    Write-Host "  [down] $($c.Name): $($c.Url)"
    $dlDir = Join-Path $ToolsRoot ".downloads"
    New-Item -ItemType Directory -Path $dlDir -Force | Out-Null
    $zip = Join-Path $dlDir ([System.IO.Path]::GetFileName($c.Url))
    & curl.exe -L --fail --retry 3 --connect-timeout 30 -o $zip $c.Url
    if ($LASTEXITCODE -ne 0) {
        throw "download failed: $($c.Url) (exit=$LASTEXITCODE). 若 GitHub 不可达，请先下载 zip 到 $dlDir 后重试，或改用 -CopyFrom。"
    }
    $extractTo = Join-Path $ToolsRoot $c.DestSubDir
    New-Item -ItemType Directory -Path $extractTo -Force | Out-Null
    Expand-Archive -LiteralPath $zip -DestinationPath $extractTo -Force
    # ninja-win.zip 顶层即 ninja.exe；gcc/cmake zip 带顶层目录
    if ($c.ZipTop) {
        $src = Join-Path $extractTo $c.ZipTop
        if (-not (Test-Path -LiteralPath $src)) {
            throw "zip 内未找到顶层目录: $($c.ZipTop)"
        }
    }
    Write-Host "  [down] $($c.Name): OK"
}

Write-Host "Toolchain root: $ToolsRoot"
if ($CopyFrom) {
    Write-Host "Copy mode from: $CopyFrom"
}

$fail = $false
foreach ($c in $Components) {
    if (-not $Force -and (Test-Component $c)) {
        Write-Host "[ok] $($c.Name): already present, skip"
        continue
    }
    if ($Force) { Write-Host "[--] $($c.Name): force reinstall" }
    else        { Write-Host "[!!] $($c.Name): missing" }

    $done = $false
    if ($CopyFrom) { $done = Copy-Component $c }
    if (-not $done) {
        try { Download-Component $c } catch { Write-Host "  ERROR: $_"; $fail = $true }
    }
}

# ---- pyocd (Python 包 + 本地 DFP) ----
if (-not $SkipPyocd) {
    $pyocd = Get-Command pyocd -ErrorAction SilentlyContinue
    if ($pyocd) {
        Write-Host "[ok] pyocd: $(& $pyocd.Source --version 2>$null)"
    } else {
        Write-Host "[!!] pyocd: not found"
        $py = Get-Command python -ErrorAction SilentlyContinue
        if ($py) {
            Write-Host "  [pip] installing pyocd via: $($py.Source)"
            & $py.Source -m pip install --upgrade pyocd
        } else {
            Write-Host "  ERROR: 未找到 python。请安装 Python 后运行: python -m pip install pyocd"
            $fail = $true
        }
    }
} else {
    Write-Host "[--] pyocd: skipped (-SkipPyocd)"
}

# ---- DFP pack（CW32 烧录目标必需）----
$dfpDir = Join-Path $ToolsRoot "pyocd"
if (Test-Path -LiteralPath $dfpDir) {
    $packs = Get-ChildItem -LiteralPath $dfpDir -Filter "*.pack" -ErrorAction SilentlyContinue
    Write-Host "[ok] DFP packs: $($packs.Count) files in tools/pyocd/"
    if ($packs.Count -eq 0) {
        Write-Host "  WARN: tools/pyocd/ 无 .pack 文件。CW32 target 需从武汉芯源官网下载 DFP 放入该目录。"
    }
} else {
    Write-Host "[!!] DFP: tools/pyocd/ 不存在。CW32 烧录 target 需本地 DFP（见 README）。"
}

Write-Host ""
if ($fail) {
    Write-Host "完成（部分组件失败，见上方 ERROR）。"
    exit 1
} else {
    Write-Host "工具链就绪。构建时在命令前添加 PATH:"
    Write-Host "  `$env:PATH = '$ToolsRoot\gcc\xpack-arm-none-eabi-gcc-13.3.1-1.1\bin;$ToolsRoot\ninja;$ToolsRoot\cmake-3.30.5-windows-x86_64\bin;$env:PATH'"
}
