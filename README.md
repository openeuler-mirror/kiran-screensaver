# kinar-screensaver
## 编译
- 安装编译依赖
`sudo yum install qt5-qtbase-devel qt5-qtx11extras-devel qt5-linguist kiran-log-qt5-devel kiranwidgets-qt5-devel gsettings-qt-devel libxcb-devel`
- 在源码根目录下创建**build**目录
  `mkdir build`
- 生成**Makefile**
  `cmake3 -DCMAKE_INSTALL_PREFIX=/usr  ..`
- 编译
  `make -j4`

## 安装
`make install`

## 卸载
`make uninstall`

### 运行
重启服务或终端执行kiran-screensaver即可(目前需关闭mate-screensaver)
`$ /usr/bin/kiran-screensaver`