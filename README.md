# 正点原子 ATK-DMF407 + simpleFOC + RT-Thread #
移植simpleFOC + RT-Thread到正点原子 ATK-DMF407板子上。该项目在linux环境下开发，git clone该仓库后，使用`git submodule update --init --recursive`克隆子模块。

## 移植思路
* 方案一、移植rtduino，simpleFOC是基于Arduino平台的，移植rtduino后，simpleFOC可以运行。
* 方案二、直接移植simpleFOC，去掉Arduino平台，减少抽象层数，提高运行效率。
