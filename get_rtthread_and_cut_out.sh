#!/bin/bash
set -euo pipefail

# =============================================================================
# RT-Thread 精简移植脚本
# 用途：从官方 RT-Thread 源码包中提取最小化的 Cortex-M4 运行环境
# =============================================================================

RT_THREAD_VERSION="5.2.2"
RT_THREAD_DIR="rt-thread-${RT_THREAD_VERSION}"

# 下载源码
wget -O ${RT_THREAD_DIR}.zip https://codeload.github.com/RT-Thread/rt-thread/zip/refs/tags/v${RT_THREAD_VERSION}

# -----------------------------------------------------------------------------
# 解压源码包
# -----------------------------------------------------------------------------
unzip "${RT_THREAD_DIR}.zip"
cd "${RT_THREAD_DIR}"

# -----------------------------------------------------------------------------
# 清理根目录：删除不需要的配置文件、文档和示例
# -----------------------------------------------------------------------------
rm -rf \
    .vscode .github .gitee .devcontainer .git .hooks \
    bsp documentation examples \
    .clang-format .gitattributes .gitignore \
    ChangeLog.md Kconfig.utestcases README*

# -----------------------------------------------------------------------------
# 精简 libcpu：仅保留 ARM Cortex-M4 架构支持
# -----------------------------------------------------------------------------
pushd libcpu > /dev/null
ls | grep -Ev '(arm|SConscript|Kconfig)' | xargs rm -rf

pushd arm > /dev/null
ls | grep -Ev '(common|cortex-m4|SConscript|Kconfig)' | xargs rm -rf
popd > /dev/null

popd > /dev/null

# -----------------------------------------------------------------------------
# 精简 components：仅保留核心组件
#   - drivers: 设备驱动框架
#   - finsh:   命令行 Shell
#   - libc:    C 库支持
#   - utilities: 工具组件 (ulog)
# -----------------------------------------------------------------------------
pushd components > /dev/null
ls | grep -Ev '(drivers|finsh|libc|SConscript|utilities|Kconfig)' | xargs rm -rf

# libc: 仅保留编译器适配和 C++ 支持
pushd libc > /dev/null
ls | grep -Ev '(compilers|cplusplus|SConscript|Kconfig)' | xargs rm -rf
popd > /dev/null

# utilities: 仅保留 ulog 日志组件
pushd utilities > /dev/null
ls | grep -Ev '(ulog|SConscript|Kconfig)' | xargs rm -rf
popd > /dev/null

# drivers: 删除不需要的驱动子系统
pushd drivers > /dev/null
rm -rf \
    ata audio block clk dma hwcrypto led mailbox reset thermal usb \
    graphic iio mfd mtd nvme ofw pci phy phye pic regulator \
    scsi smp_call touch virtio wlan
popd > /dev/null

popd > /dev/null

# -----------------------------------------------------------------------------
# 清理所有 utest 测试目录
# -----------------------------------------------------------------------------
find . -depth -type d -name "utest" -exec rm -rf {} \;

echo "RT-Thread ${RT_THREAD_VERSION} 精简完成！"
