```
# listbmcpin - list ASPEED ast2600/2620 pin configuration

* Version: 0.1
* Author: yung-sheng.huang <yung-sheng.huang@fii-na.com>

This package has the listbmcpin tool used to get ASPEED ast2600/2620 pin
configuration.


# Build

```
$ gcc -Wall listbmcpin.c -o listbmcpin
```

# Usage
listbmcpin -h

usage: listbmcpin list ASPEED ast2600/2620 pin config...
  options:
    -m --multipin                    List PIN config.
    -g --gpiopin                     List GPIO PIN config.
    -h --help                        Output usage message and exit.
    -v --version                     Output the version number and exit
```

```
bb file example:
recipes-support\bmc-tools\bmc-tools.bb

SUMMARY = "Set of tools for BMC"
HOMEPAGE = "https://github.com/Yung-sheng/bmc-tools"
LICENSE = "CLOSED"

SRC_URI = "git://github.com/Yung-sheng/bmc-tools.git;protocol=https;branch=${BRANCH}"

PR = "r1"
PV = "1.0+git${SRCPV}"
# Tag for v00.01.00
SRCREV = "b0751a80ea3bbf5891b698c586737c811aee1d21"
BRANCH = "main"

S = "${WORKDIR}/git"

inherit meson
```
```
OBMC_IMAGE_EXTRA_INSTALL:append:heiji = " entity-manager"
OBMC_IMAGE_EXTRA_INSTALL:append:heiji = " dbus-sensors"
OBMC_IMAGE_EXTRA_INSTALL:append:heiji = " heiji-init"
OBMC_IMAGE_EXTRA_INSTALL:append:heiji = " pciutils"
OBMC_IMAGE_EXTRA_INSTALL:append:heiji = " x86-power-control"
OBMC_IMAGE_EXTRA_INSTALL:append:heiji = " aspeed-svf"
OBMC_IMAGE_EXTRA_INSTALL:append:heiji = " heiji-fw"
OBMC_IMAGE_EXTRA_INSTALL:append:heiji = " bmc-tools"
```
