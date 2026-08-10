# kiran-screensaver

## Build

- Install dependencies:
`sudo yum install qt5-qtbase-devel qt5-qtx11extras-devel qt5-linguist kiran-log-qt5-devel kiranwidgets-qt5-devel gsettings-qt-devel libxcb-devel`
- Create the `build` directory in the root directory of the source code:
  `mkdir build`
- Generate the makefile:
  `cmake3 -DCMAKE_INSTALL_PREFIX=/usr  ..`
- Compile
  `make -j4`.

## Installation

`make install`

## Uninstallation

`make uninstall`

### Run

Restart your system service, or run the following command directly from the terminal (Note: `mate-screensaver` must be disabled first):
`$ /usr/bin/kiran-screensaver`
