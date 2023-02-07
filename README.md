# BUILDING...   建造中...

#### 一、编译环境：

##### 1. 配置 PkgConfig

windows:

1. 前往 http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/
2. 下载 [pkg-config_0.26-1_win32.zip](http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/pkg-config_0.26-1_win32.zip)
3. 添加 `bin/pkg-config.exe` 到 `${库目录}`
4. 下载 [gettext-runtime_0.18.1.1-2_win32.zip](http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/gettext-runtime_0.18.1.1-2_win32.zip)
5. 添加 `bin/intl.dll` 到 `${库目录}`
6. 前往 http://ftp.gnome.org/pub/gnome/binaries/win32/glib/2.28
7. 下载 [glib_2.28.8-1_win32.zip](http://ftp.acc.umu.se/pub/gnome/binaries/win32/glib/2.28/glib_2.28.8-1_win32.zip)
8. 添加 `bin/libglib-2.0-0.dll` 到 `${库目录}`

linux: `sudo apt-get install pkg-config`

##### 2. 配置 vulkan sdk

windows:

1. 前往 [LunarXchange (lunarg.com)](https://vulkan.lunarg.com/sdk/home)
2. 下载最新 vulkan SDK Installer 并安装
3. 安装完成后可运行安装目录下的 `Bin\vkcube.exe` 或在命令行运行 `vulkaninfo` 测试环境

linux:

- 可以类似 windows 在 [LunarXchange (lunarg.com)](https://vulkan.lunarg.com/sdk/home) 上获取 SDK 并安装
- 或者执行如下指令

```sh
#ubuntu16  vulkan版本1.126
wget -qO - http://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -
sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-1.1.126-xenial.list http://packages.lunarg.com/vulkan/1.1.126/lunarg-vulkan-1.1.126-xenial.list
sudo apt update
sudo apt install vulkan-sdk
```

##### 4. 配置 freeType2

windows:

- 前往 https://freetype.org/
- 下载合适版本的 freetype 并用目标编译器编译
- 将编译好的二进制文件和头文件分别放入编译器的 `bin&share、include` 目录

windows 且使用 mingw32-w64 编译的话可以在这里下载到编译好的库 https://packages.msys2.org/package/mingw-w64-x86_64-freetype

linux:

- 源码位置 `cd ${freetype_source_dir}`
- 目标位置 `./configure --prefix=/usr/local/freetype`
- 编译 `make`
- 安装编译结果 `make install`

#### 二、生成 rc

编辑项目根目录下的 `Config.cmake` 文件，将 `set(CC_RC_REGEN true)` 一行解除注释。重新生成 cmake 项目，若 `./gui/rc/` 目录下出现了 `gen` 文件夹，则可以初步判断生成成功。生成后请在 `Config.cmake` 中将上述行再度注释，否则会占用大量生成时间。
