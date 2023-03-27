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

usage: listbmcpin list ASPEED ast2600/2620 pin config...
  options:
    -m --multipin                    List PIN config.
    -g --gpiopin                     List GPIO PINr config.
    -h --help                        Output usage message and exit.
    -v --version                     Output the version number and exit
