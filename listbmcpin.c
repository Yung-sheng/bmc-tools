/*
 * listbmcpin.c: Simple program to get BMC pin config.
 *
 *  Copyright (C) 2023, yung-sheng.huang (yung-sheng.huang@fii-na.com)
 *
 */

// <time.h> requires a specific C source version (greater than)
#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <inttypes.h>
#include <time.h>
#include <getopt.h>

#define VERSION "listbmcpin version 0.1"

const char *sopts = "mgvh";
static const struct option lopts[] = {
	{"multipin",	no_argument,		NULL,	'm' },
	{"gpiopin",		no_argument,		NULL,	'g' },
	{"help",		no_argument,		NULL,	'h' },
	{"version",		no_argument,		NULL,	'v' },
	{0, 0, 0, 0}
};
static void print_usage(const char *name)
{
	fprintf(stderr, "usage: %s list ASPEED ast2600/2620 pin config...\n", name);
	fprintf(stderr, "  options:\n");
	fprintf(stderr, "    -m --multipin                    List PIN config.\n");
	fprintf(stderr, "    -g --gpiopin                     List GPIO PINr config.\n");
	fprintf(stderr, "    -h --help                        Output usage message and exit.\n");
	fprintf(stderr, "    -v --version                     Output the version number and exit\n");
}

int f_dbg = 0;
//SCU REG
#define ADDR_SCU400  0x1E6E2400
#define ADDR_SCU404  0x1E6E2404
#define ADDR_SCU410  0x1E6E2410
#define ADDR_SCU414  0x1E6E2414
#define ADDR_SCU418  0x1E6E2418
#define ADDR_SCU41C  0x1E6E241C
#define ADDR_SCU430  0x1E6E2430
#define ADDR_SCU434  0x1E6E2434
#define ADDR_SCU438  0x1E6E2438
#define ADDR_SCU450  0x1E6E2450
#define ADDR_SCU470  0x1E6E2470
#define ADDR_SCU4B0  0x1E6E24B0
#define ADDR_SCU4B4  0x1E6E24B4
#define ADDR_SCU4B8  0x1E6E24B8
#define ADDR_SCU4BC  0x1E6E24BC
#define ADDR_SCU4D4  0x1E6E24D4
#define ADDR_SCU4D8  0x1E6E24D8
#define ADDR_SCU500  0x1E6E2500
#define ADDR_SCU510  0x1E6E2510
#define ADDR_SCU690  0x1E6E2690
#define ADDR_SCU694  0x1E6E2694


uint64_t gRegVal_SCU400 = 0;
uint64_t gRegVal_SCU404 = 0;
uint64_t gRegVal_SCU410 = 0;
uint64_t gRegVal_SCU414 = 0;
uint64_t gRegVal_SCU418 = 0;
uint64_t gRegVal_SCU41C = 0;
uint64_t gRegVal_SCU430 = 0;
uint64_t gRegVal_SCU434 = 0;
uint64_t gRegVal_SCU438 = 0;
uint64_t gRegVal_SCU450 = 0;
uint64_t gRegVal_SCU470 = 0;
uint64_t gRegVal_SCU4B0 = 0;
uint64_t gRegVal_SCU4B4 = 0;
uint64_t gRegVal_SCU4B8 = 0;
uint64_t gRegVal_SCU4BC = 0;
uint64_t gRegVal_SCU4D4 = 0;
uint64_t gRegVal_SCU4D8 = 0;
uint64_t gRegVal_SCU500 = 0;
uint64_t gRegVal_SCU510 = 0;
uint64_t gRegVal_SCU690 = 0;
uint64_t gRegVal_SCU694 = 0;

//GPIO REG
#define ADDR_GPIO000_ABCD_DATA  0x1E780000
#define ADDR_GPIO004_ABCD_DIR   0x1E780004
#define ADDR_GPIO020_EFGH_DATA  0x1E780020
#define ADDR_GPIO024_EFGH_DIR   0x1E780024
#define ADDR_GPIO070_IJKL_DATA  0x1E780070
#define ADDR_GPIO074_IJKL_DIR   0x1E780074
#define ADDR_GPIO078_MNOP_DATA  0x1E780078
#define ADDR_GPIO07C_MNOP_DIR   0x1E78007C
#define ADDR_GPIO080_QRST_DATA  0x1E780080
#define ADDR_GPIO084_QRST_DIR   0x1E780084
#define ADDR_GPIO088_UVWX_DATA  0x1E780088
#define ADDR_GPIO08C_UVWX_DIR   0x1E78008C
#define ADDR_GPIO1E0_YZ_DATA    0x1E7801E0
#define ADDR_GPIO1E4_YZ_DIR     0x1E7801E4
#define ADDR_GPIO800_ABCD_DATA  0x1E780800
#define ADDR_GPIO804_ABCD_DIR   0x1E780804
#define ADDR_GPIO800_E_DATA     0x1E780820
#define ADDR_GPIO804_E_DIR      0x1E780824

uint64_t gRegVal_DATA_ABCD = 0;
uint64_t gRegVal_DIR_ABCD  = 0;
uint64_t gRegVal_DATA_EFGH = 0;
uint64_t gRegVal_DIR_EFGH  = 0;
uint64_t gRegVal_DATA_IJKL = 0;
uint64_t gRegVal_DIR_IJKL  = 0;
uint64_t gRegVal_DATA_MNOP = 0;
uint64_t gRegVal_DIR_MNOP  = 0;
uint64_t gRegVal_DATA_QRST = 0;
uint64_t gRegVal_DIR_QRST  = 0;
uint64_t gRegVal_DATA_UVWX = 0;
uint64_t gRegVal_DIR_UVWX  = 0;
uint64_t gRegVal_DATA_YZ   = 0;
uint64_t gRegVal_DIR_YZ    = 0;
uint64_t gRegVal_DATA_18ABCD   = 0;
uint64_t gRegVal_DIR_18ABCD    = 0;
uint64_t gRegVal_DATA_18E      = 0;
uint64_t gRegVal_DIR_18E       = 0;




#define CheckBit(val,pos) ((val) &   (1<<(pos)))

#define printerr(fmt, ...)               \
    do                                   \
{                                        \
    fprintf(stderr, fmt, ##__VA_ARGS__); \
    fflush(stderr);                      \
} while (0)


void pin_AF23toAF24() {
	printf("AF23: ");
	if(CheckBit(gRegVal_SCU438,16))
		printf("I3C1SCL\n");
	else
		printf("Hi-Z, Input\n");

	printf("AE24: ");
	if(CheckBit(gRegVal_SCU438,17))
		printf("I3C1SDA\n");
	else
		printf("Hi-Z, Input\n");

	printf("AF22: ");
	if(CheckBit(gRegVal_SCU438,18))
		printf("I3C2SCL\n");
	else
		printf("Hi-Z, Input\n");
		
	printf("AE22: ");
	if(CheckBit(gRegVal_SCU438,19))
		printf("I3C2SDA\n");
	else
		printf("Hi-Z, Input\n");


	printf("AF25: ");
	if(CheckBit(gRegVal_SCU438,20))
		printf("I3C3SCL\n");
	else if(CheckBit(gRegVal_SCU4D8,20))
		printf("FSI1CLK\n");
	else
		printf("Hi-Z, Input\n");

	printf("AE26: ");
	if(CheckBit(gRegVal_SCU438,21))
		printf("I3C3SDA\n");
	else if(CheckBit(gRegVal_SCU4D8,21))
		printf("FSI1DATA\n");
	else
		printf("Hi-Z, Input\n");
		
	printf("AE25: ");
	if(CheckBit(gRegVal_SCU438,22))
		printf("I3C4SCL\n");
	else if(CheckBit(gRegVal_SCU4D8,22))
		printf("FSI2CLK\n");
	else
		printf("Hi-Z, Input\n");
		
	printf("AF24: ");
	if(CheckBit(gRegVal_SCU438,23))
		printf("I3C4SDA\n");
	else if(CheckBit(gRegVal_SCU4D8,23))
		printf("FSI2DATA\n");
	else
		printf("Hi-Z, Input\n");
}

void pin_M24toK25() {
	printf(" M24: ");
	if(CheckBit(gRegVal_SCU410,0))
		printf("MDC3\n");
	else if(CheckBit(gRegVal_SCU4B0,0))
		printf("SCL11\n");
	else
		printf("GPIOA0\n");

	printf(" M25: ");
	if(CheckBit(gRegVal_SCU410,1))
		printf("MDIO3\n");
	else if(CheckBit(gRegVal_SCU4B0,1))
		printf("SDA11\n");
	else
		printf("GPIOA1\n");

	printf(" L26: ");
	if(CheckBit(gRegVal_SCU410,2))
		printf("MDC4\n");
	else if(CheckBit(gRegVal_SCU4B0,2))
		printf("SCL12\n");
	else
		printf("GPIOA2\n");

	printf(" K24: ");
	if(CheckBit(gRegVal_SCU410,3))
		printf("MDIO4\n");
	else if(CheckBit(gRegVal_SCU4B0,3))
		printf("SDA12\n");
	else
		printf("GPIOA3\n");


	printf(" K26: ");
	if(CheckBit(gRegVal_SCU410,4))
		printf("MACLINK1\n");
	else if(CheckBit(gRegVal_SCU4B0,4))
		printf("SCL13\n");
	else if(CheckBit(gRegVal_SCU690,4))
		printf("SGPS2CK\n");
	else
		printf("GPIOA4\n");

	printf(" L24: ");
	if(CheckBit(gRegVal_SCU410,5))
		printf("MACLINK2\n");
	else if(CheckBit(gRegVal_SCU4B0,5))
		printf("SDA13\n");
	else if(CheckBit(gRegVal_SCU690,5))
		printf("SGPS2LD\n");
	else
		printf("GPIOA5\n");

	printf(" L23: ");
	if(CheckBit(gRegVal_SCU410,6))
		printf("MACLINK3\n");
	else if(CheckBit(gRegVal_SCU4B0,6))
		printf("SCL14\n");
	else if(CheckBit(gRegVal_SCU690,6))
		printf("SGPS2I0\n");
	else
		printf("GPIOA6\n");

	printf(" K25: ");
	if(CheckBit(gRegVal_SCU410,7))
		printf("MACLINK4\n");
	else if(CheckBit(gRegVal_SCU4B0,7))
		printf("SDA14\n");
	else if(CheckBit(gRegVal_SCU690,7))
		printf("SGPS2I1\n");
	else
		printf("GPIOA7\n");
}

void pin_J26toJ24() {
	printf(" J26: ");
	if(CheckBit(gRegVal_SCU410,8))
		printf("SALT1\n");
	else
		printf("GPIOB0\n");

	printf(" K23: ");
	if(CheckBit(gRegVal_SCU410,9))
		printf("SALT2\n");
	else
		printf("GPIOB1\n");

	printf(" H26: ");
	if(CheckBit(gRegVal_SCU410,10))
		printf("SALT3\n");
	else
		printf("GPIOB2\n");

	printf(" J25: ");
	if(CheckBit(gRegVal_SCU410,11))
		printf("SALT4\n");
	else
		printf("GPIOB3\n");

	printf(" J23: ");
	if(CheckBit(gRegVal_SCU410,12))
		printf("MDC2\n");
	else
		printf("GPIOB4\n");

	printf(" G26: ");
	if(CheckBit(gRegVal_SCU410,13))
		printf("MDIO2\n");
	else
		printf("GPIOB5\n");

	printf(" H25: ");
	if(CheckBit(gRegVal_SCU410,14))
		printf("TXD4\n");
	else
		printf("GPIOB6\n");

	printf(" J24: ");
	if(CheckBit(gRegVal_SCU410,15))
		printf("RXD4\n");
	else
		printf("GPIOB7\n");
}

void pin_H24toG24() {
	printf(" H24: ");
	if(CheckBit(gRegVal_SCU410,16) &&
	   CheckBit(gRegVal_SCU510,0))
		printf("RGMII3TXCK\n");
	else if(CheckBit(gRegVal_SCU410,16) &&
	   		!CheckBit(gRegVal_SCU510,0))
		printf("RMII3RCLKO\n");
	else
		printf("GPIOC0\n");

	printf(" J22: ");
	if(CheckBit(gRegVal_SCU410,17) &&
	   CheckBit(gRegVal_SCU510,0))
		printf("RGMII3TXCTL\n");
	else if(CheckBit(gRegVal_SCU410,17) &&
	   		!CheckBit(gRegVal_SCU510,0))
		printf("RMII3TXEN\n");
	else
		printf("GPIOC1\n");

	printf(" H22: ");
	if(CheckBit(gRegVal_SCU410,18) &&
	   CheckBit(gRegVal_SCU510,0))
		printf("RGMII3TXD0\n");
	else if(CheckBit(gRegVal_SCU410,18) &&
	   		!CheckBit(gRegVal_SCU510,0))
		printf("RMII3TXD0\n");
	else
		printf("GPIOC2\n");

	printf(" H23: ");
	if(CheckBit(gRegVal_SCU410,19) &&
	   CheckBit(gRegVal_SCU510,0))
		printf("RGMII3TXD1\n");
	else if(CheckBit(gRegVal_SCU410,19) &&
	   		!CheckBit(gRegVal_SCU510,0))
		printf("RMII3TXD1\n");
	else
		printf("GPIOC3\n");

	printf(" G22: ");
	if(CheckBit(gRegVal_SCU410,20) &&
	   CheckBit(gRegVal_SCU510,0))
		printf("RGMII3TXD2\n");
	else
		printf("GPIOC4\n");

	printf(" F22: ");
	if(CheckBit(gRegVal_SCU410,21) &&
	   CheckBit(gRegVal_SCU510,0))
		printf("RGMII3TXD3\n");
	else
		printf("GPIOC5\n");

	printf(" G23: ");
	if(CheckBit(gRegVal_SCU410,22) &&
	   CheckBit(gRegVal_SCU510,0))
		printf("RGMII3RXCK\n");
	else if(CheckBit(gRegVal_SCU410,22) &&
	   		!CheckBit(gRegVal_SCU510,0))
		printf("RMII3RCLKI\n");
	else
		printf("GPIOC6\n");

	printf(" G24: ");
	if(CheckBit(gRegVal_SCU410,23) &&
	   CheckBit(gRegVal_SCU510,0))
		printf("RGMII3RXCTL\n");
	else
		printf("GPIOC7\n");
}

void pin_F23toE25() {
	printf(" F23: ");
	if(CheckBit(gRegVal_SCU410,24) &&
	   CheckBit(gRegVal_SCU510,0))
		printf("RGMII3RXD0\n");
	else if(CheckBit(gRegVal_SCU410,24) &&
	   		!CheckBit(gRegVal_SCU510,0))
		printf("RMII3RXD0\n");
	else
		printf("GPIOD0\n");

	printf(" F26: ");
	if(CheckBit(gRegVal_SCU410,25) &&
	   CheckBit(gRegVal_SCU510,0))
		printf("RGMII3RXD1\n");
	else if(CheckBit(gRegVal_SCU410,25) &&
	   		!CheckBit(gRegVal_SCU510,0))
		printf("RMII3RXD1\n");
	else
		printf("GPIOD1\n");

	printf(" F25: ");
	if(CheckBit(gRegVal_SCU410,26) &&
	   CheckBit(gRegVal_SCU510,0))
		printf("RGMII3RXD2\n");
	else if(CheckBit(gRegVal_SCU410,26) &&
	   		!CheckBit(gRegVal_SCU510,0))
		printf("RMII3CRSDV\n");
	else
		printf("GPIOD2\n");

	printf(" E26: ");
	if(CheckBit(gRegVal_SCU410,27) &&
	   CheckBit(gRegVal_SCU510,0))
		printf("RGMII3RXD3\n");
	else if(CheckBit(gRegVal_SCU410,27) &&
	   		!CheckBit(gRegVal_SCU510,0))
		printf("RMII3RXER\n");
	else
		printf("GPIOD3\n");


	printf(" F24: ");
	if(CheckBit(gRegVal_SCU410,28))
		printf("NCTS3\n");
	else if(CheckBit(gRegVal_SCU4B0,28) &&
			CheckBit(gRegVal_SCU510,1))
		printf("RGMII4TXCK\n");
	else if(CheckBit(gRegVal_SCU4B0,28) &&
			!CheckBit(gRegVal_SCU510,1))
		printf("RMII4RCLKO\n");
	else
		printf("GPIOD4\n");

	printf(" E23: ");
	if(CheckBit(gRegVal_SCU410,29))
		printf("NDCD3\n");
	else if(CheckBit(gRegVal_SCU4B0,29) &&
			CheckBit(gRegVal_SCU510,1))
		printf("RGMII4TXCTL\n");
	else if(CheckBit(gRegVal_SCU4B0,29) &&
			!CheckBit(gRegVal_SCU510,1))
		printf("RMII4TXEN\n");
	else
		printf("GPIOD5\n");

	printf(" E24: ");
	if(CheckBit(gRegVal_SCU410,30))
		printf("NDSR3\n");
	else if(CheckBit(gRegVal_SCU4B0,30) &&
			CheckBit(gRegVal_SCU510,1))
		printf("RGMII4TXD0\n");
	else if(CheckBit(gRegVal_SCU4B0,30) &&
			!CheckBit(gRegVal_SCU510,1))
		printf("RMII4TXD0\n");
	else
		printf("GPIOD6\n");

	printf(" E25: ");
	if(CheckBit(gRegVal_SCU410,31))
		printf("NRI3\n");
	else if(CheckBit(gRegVal_SCU4B0,31) &&
			CheckBit(gRegVal_SCU510,1))
		printf("RGMII4TXD1\n");
	else if(CheckBit(gRegVal_SCU4B0,31) &&
			!CheckBit(gRegVal_SCU510,1))
		printf("RMII4TXD1\n");
	else
		printf("GPIOD7\n");
}

void pin_D26toB24() {
	printf(" D26: ");
	if(CheckBit(gRegVal_SCU414,0) &&
	   !CheckBit(gRegVal_SCU470,16))
		printf("NDTR3\n");
	else if(CheckBit(gRegVal_SCU4B4,0) &&
			CheckBit(gRegVal_SCU510,1) &&
			!CheckBit(gRegVal_SCU470,16))
		printf("RGMII4TXD2\n");
	else
		printf("GPIOE0\n");

	printf(" D24: ");
	if(CheckBit(gRegVal_SCU414,1) &&
	   !CheckBit(gRegVal_SCU470,17))
		printf("NDTR3\n");
	else if(CheckBit(gRegVal_SCU4B4,1) &&
			CheckBit(gRegVal_SCU510,1) &&
			!CheckBit(gRegVal_SCU470,17))
		printf("RGMII4TXD3\n");
	else
		printf("GPIOE1\n");

	printf(" C25: ");
	if(CheckBit(gRegVal_SCU414,2) &&
	   !CheckBit(gRegVal_SCU470,18))
		printf("NCTS4\n");
	else if(CheckBit(gRegVal_SCU4B4,2) &&
			CheckBit(gRegVal_SCU510,1) &&
			!CheckBit(gRegVal_SCU470,18))
		printf("RGMII4RXCK\n");
	else if(CheckBit(gRegVal_SCU4B4,2) &&
			!CheckBit(gRegVal_SCU510,1) &&
			!CheckBit(gRegVal_SCU470,18))
		printf("RMII4RCLKI\n");
	else
		printf("GPIOE2\n");

	printf(" C26: ");
	if(CheckBit(gRegVal_SCU414,3) &&
	   !CheckBit(gRegVal_SCU470,19))
		printf("NDCD4\n");
	else if(CheckBit(gRegVal_SCU4B4,3) &&
			CheckBit(gRegVal_SCU510,1) &&
			!CheckBit(gRegVal_SCU470,19))
		printf("RGMII4RXCTL\n");
	else
		printf("GPIOE3\n");


	printf(" C24: ");
	if(CheckBit(gRegVal_SCU414,4) &&
	   !CheckBit(gRegVal_SCU470,20))
		printf("NDSR4\n");
	else if(CheckBit(gRegVal_SCU4B4,4) &&
			CheckBit(gRegVal_SCU510,1) &&
			!CheckBit(gRegVal_SCU470,20))
		printf("RGMII4RXD0\n");
	else if(CheckBit(gRegVal_SCU4B4,4) &&
			!CheckBit(gRegVal_SCU510,1) &&
			!CheckBit(gRegVal_SCU470,20))
		printf("RMII4RXD0\n");
	else
		printf("GPIOE4\n");

	printf(" B26: ");
	if(CheckBit(gRegVal_SCU414,5) &&
	   !CheckBit(gRegVal_SCU470,21))
		printf("NDSR4\n");
	else if(CheckBit(gRegVal_SCU4B4,5) &&
			CheckBit(gRegVal_SCU510,1) &&
			!CheckBit(gRegVal_SCU470,21))
		printf("RGMII4RXD1\n");
	else if(CheckBit(gRegVal_SCU4B4,5) &&
			!CheckBit(gRegVal_SCU510,1) &&
			!CheckBit(gRegVal_SCU470,21))
		printf("RMII4RXD1\n");
	else
		printf("GPIOE5\n");

	printf(" B25: ");
	if(CheckBit(gRegVal_SCU414,6) &&
	   !CheckBit(gRegVal_SCU470,22))
		printf("NDTR4\n");
	else if(CheckBit(gRegVal_SCU4B4,6) &&
			CheckBit(gRegVal_SCU510,1) &&
			!CheckBit(gRegVal_SCU470,22))
		printf("RGMII4RXD2\n");
	else if(CheckBit(gRegVal_SCU4B4,6) &&
			!CheckBit(gRegVal_SCU510,1) &&
			!CheckBit(gRegVal_SCU470,22))
		printf("RMII4CRSDV\n");
	else
		printf("GPIOE6\n");

	printf(" B24: ");
	if(CheckBit(gRegVal_SCU414,7) &&
	   !CheckBit(gRegVal_SCU470,23))
		printf("NRTS4\n");
	else if(CheckBit(gRegVal_SCU4B4,7) &&
			CheckBit(gRegVal_SCU510,1) &&
			!CheckBit(gRegVal_SCU470,23))
		printf("RGMII4RXD3\n");
	else if(CheckBit(gRegVal_SCU4B4,7) &&
			!CheckBit(gRegVal_SCU510,1) &&
			!CheckBit(gRegVal_SCU470,23))
		printf("RMII4RXER\n");
	else
		printf("GPIOE7\n");
}

void pin_D22toA23() {
	printf(" D22: ");
	if(CheckBit(gRegVal_SCU414,8))
		printf("SD1CLK\n");
	else if(CheckBit(gRegVal_SCU4B4,8))
		printf("PWM8\n");
	else
		printf("GPIOF0\n");

	printf(" E22: ");
	if(CheckBit(gRegVal_SCU414,9))
		printf("SD1CMD\n");
	else if(CheckBit(gRegVal_SCU4B4,9))
		printf("PWM9\n");
	else
		printf("GPIOF1\n");

	printf(" D23: ");
	if(CheckBit(gRegVal_SCU414,10))
		printf("SD1DAT0\n");
	else if(CheckBit(gRegVal_SCU4B4,10))
		printf("PWM10\n");
	else
		printf("GPIOF2\n");

	printf(" C23: ");
	if(CheckBit(gRegVal_SCU414,11))
		printf("SD1DAT1\n");
	else if(CheckBit(gRegVal_SCU4B4,11))
		printf("PWM11\n");
	else
		printf("GPIOF3\n");

	printf(" C22: ");
	if(CheckBit(gRegVal_SCU414,12))
		printf("SD1DAT2\n");
	else if(CheckBit(gRegVal_SCU4B4,12))
		printf("PWM12\n");
	else
		printf("GPIOF4\n");

	printf(" A25: ");
	if(CheckBit(gRegVal_SCU414,13))
		printf("SD1DAT3\n");
	else if(CheckBit(gRegVal_SCU4B4,13))
		printf("PWM13\n");
	else
		printf("GPIOF5\n");

	printf(" A24: ");
	if(CheckBit(gRegVal_SCU414,14))
		printf("SD1CD#\n");
	else if(CheckBit(gRegVal_SCU4B4,14))
		printf("PWM14\n");
	else
		printf("GPIOF6\n");

	printf(" A23: ");
	if(CheckBit(gRegVal_SCU414,15))
		printf("SD1WP#\n");
	else if(CheckBit(gRegVal_SCU4B4,15))
		printf("PWM15\n");
	else
		printf("GPIOF7\n");
}

void pin_E21toB21() {
	printf(" E21: ");
	if(CheckBit(gRegVal_SCU414,16))
		printf("TXD6\n");
	else if(CheckBit(gRegVal_SCU4B4,16) &&
	        CheckBit(gRegVal_SCU450,1))
		printf("SD2CLK\n");
	else if( CheckBit(gRegVal_SCU694,16) )
		printf("SALT9\n");
	else
		printf("GPIOG0\n");

	printf(" B22: ");
	if(CheckBit(gRegVal_SCU414,17))
		printf("RXD6\n");
	else if(CheckBit(gRegVal_SCU4B4,17) &&
	        CheckBit(gRegVal_SCU450,1))
		printf("SD2CMD\n");
	else if( CheckBit(gRegVal_SCU694,17) )
		printf("SALT10\n");
	else
		printf("GPIOG1\n");

	printf(" C21: ");
	if(CheckBit(gRegVal_SCU414,18))
		printf("TXD7\n");
	else if(CheckBit(gRegVal_SCU4B4,18) &&
	        CheckBit(gRegVal_SCU450,1))
		printf("SD2DAT0\n");
	else if( CheckBit(gRegVal_SCU694,18) )
		printf("SALT11\n");
	else
		printf("GPIOG2\n");

	printf(" A22: ");
	if(CheckBit(gRegVal_SCU414,19))
		printf("RXD7\n");
	else if(CheckBit(gRegVal_SCU4B4,19) &&
	        CheckBit(gRegVal_SCU450,1))
		printf("SD2DAT1\n");
	else if( CheckBit(gRegVal_SCU694,19) )
		printf("SALT12\n");
	else
		printf("GPIOG3\n");

	printf(" A21: ");
	if(CheckBit(gRegVal_SCU414,20))
		printf("TXD8\n");
	else if(CheckBit(gRegVal_SCU4B4,20) &&
	        CheckBit(gRegVal_SCU450,1))
		printf("SD2DAT2\n");
	else if( CheckBit(gRegVal_SCU694,20) )
		printf("SALT13\n");
	else
		printf("GPIOG4\n");

	printf(" E20: ");
	if(CheckBit(gRegVal_SCU414,21))
		printf("RXD8\n");
	else if(CheckBit(gRegVal_SCU4B4,21) &&
	        CheckBit(gRegVal_SCU450,1))
		printf("SD2DAT3\n");
	else if( CheckBit(gRegVal_SCU694,21) )
		printf("SALT14\n");
	else
		printf("GPIOG5\n");

	printf(" D21: ");
	if(CheckBit(gRegVal_SCU414,22))
		printf("TXD9\n");
	else if(CheckBit(gRegVal_SCU4B4,22) &&
	        CheckBit(gRegVal_SCU450,1))
		printf("SD2CD#\n");
	else if( CheckBit(gRegVal_SCU694,22) )
		printf("SALT15\n");
	else
		printf("GPIOG6\n");

	printf(" B21: ");
	if(CheckBit(gRegVal_SCU414,23))
		printf("RXD9\n");
	else if(CheckBit(gRegVal_SCU4B4,23) &&
	        CheckBit(gRegVal_SCU450,1))
		printf("SD2WP#\n");
	else if( CheckBit(gRegVal_SCU694,23) )
		printf("SALT16\n");
	else
		printf("GPIOG7\n");
}

void pin_A18toE18() {
	printf(" A18: ");
	if(CheckBit(gRegVal_SCU414,24))
		printf("SGPM1CK\n");
	else
		printf("GPIOH0\n");

	printf(" B18: ");
	if(CheckBit(gRegVal_SCU414,25))
		printf("SGPM1LD\n");
	else
		printf("GPIOH1\n");

	printf(" C18: ");
	if(CheckBit(gRegVal_SCU414,26))
		printf("SGPM1O\n");
	else
		printf("GPIOH2\n");

	printf(" A17: ");
	if(CheckBit(gRegVal_SCU414,27))
		printf("SGPM1I\n");
	else
		printf("GPIOH3\n");

	printf(" D18: ");
	if(CheckBit(gRegVal_SCU414,28))
		printf("SGPS1CK\n");
	else if( CheckBit(gRegVal_SCU4B4,28) )
		printf("SCL15\n");
	else
		printf("GPIOH4\n");

	printf(" B17: ");
	if(CheckBit(gRegVal_SCU414,29))
		printf("SGPS1LD\n");
	else if( CheckBit(gRegVal_SCU4B4,29) )
		printf("SDA15\n");
	else
		printf("GPIOH5\n");

	printf(" C17: ");
	if(CheckBit(gRegVal_SCU414,30))
		printf("SGPS1I0\n");
	else if( CheckBit(gRegVal_SCU4B4,30) )
		printf("SCL16\n");
	else
		printf("GPIOH6\n");

	printf(" E18: ");
	if(CheckBit(gRegVal_SCU414,31))
		printf("SGPS1I1\n");
	else if( CheckBit(gRegVal_SCU4B4,31) )
		printf("SDA16\n");
	else
		printf("GPIOH7\n");
}

void pin_D17toA15() {
	printf(" D17: ");
	if(CheckBit(gRegVal_SCU418,0))
		printf("MTRSTN2\n");
	else if( CheckBit(gRegVal_SCU4B8,0) )
		printf("TXD12\n");
	else
		printf("GPIOI0\n");

	printf(" A16: ");
	if(CheckBit(gRegVal_SCU418,1))
		printf("MTDI2\n");
	else if( CheckBit(gRegVal_SCU4B8,1) )
		printf("RXD12\n");
	else
		printf("GPIOI1\n");

	printf(" E17: ");
	if(CheckBit(gRegVal_SCU418,2))
		printf("MTCK2\n");
	else if( CheckBit(gRegVal_SCU4B8,2) )
		printf("TXD13\n");
	else
		printf("GPIOI2\n");

	printf(" D16: ");
	if(CheckBit(gRegVal_SCU418,3))
		printf("MTMS2\n");
	else if( CheckBit(gRegVal_SCU4B8,3) )
		printf("RXD13\n");
	else
		printf("GPIOI3\n");

	printf(" C16: ");
	if(CheckBit(gRegVal_SCU418,4))
		printf("MTDO2\n");
	else
		printf("GPIOI4\n");

	printf(" E16: ");
	if(CheckBit(gRegVal_SCU418,5) ||
	   CheckBit(gRegVal_SCU510,5))
		printf("SIOPBO#\n");
	else
		printf("GPIOI5\n");

	printf(" B16: ");
	if(CheckBit(gRegVal_SCU418,6) ||
	   CheckBit(gRegVal_SCU510,5))
		printf("SIOPBI#\n");
	else
		printf("GPIOI6\n");

	printf(" A15: ");
	if(CheckBit(gRegVal_SCU418,7) ||
	   CheckBit(gRegVal_SCU510,5))
		printf("SIOSCI#\n");
	else
		printf("GPIOI7\n");
}

void pin_B20toD19() {
	printf(" B20: ");
	if(CheckBit(gRegVal_SCU418,8))
		printf("HVI3C3SCL\n");
	else if( CheckBit(gRegVal_SCU4B8,8) )
		printf("SCL1\n");
	else
		printf("GPIOJ0\n");

	printf(" A20: ");
	if(CheckBit(gRegVal_SCU418,9))
		printf("HVI3C3SDA\n");
	else if( CheckBit(gRegVal_SCU4B8,9) )
		printf("SDA1\n");
	else
		printf("GPIOJ1\n");

	printf(" E19: ");
	if(CheckBit(gRegVal_SCU418,10))
		printf("HVI3C4SCL\n");
	else if( CheckBit(gRegVal_SCU4B8,10) )
		printf("SCL2\n");
	else
		printf("GPIOJ2\n");

	printf(" D20: ");
	if(CheckBit(gRegVal_SCU418,11))
		printf("HVI3C4SDA\n");
	else if( CheckBit(gRegVal_SCU4B8,11) )
		printf("SDA2\n");
	else
		printf("GPIOJ3\n");

	printf(" C19: ");
	if(CheckBit(gRegVal_SCU418,12))
		printf("HVI3C5SCL\n");
	else if( CheckBit(gRegVal_SCU4B8,12) )
		printf("SCL3\n");
	else
		printf("GPIOJ4\n");

	printf(" A19: ");
	if(CheckBit(gRegVal_SCU418,13))
		printf("HVI3C5SDA\n");
	else if( CheckBit(gRegVal_SCU4B8,13) )
		printf("SDA3\n");
	else
		printf("GPIOJ5\n");

	printf(" C20: ");
	if(CheckBit(gRegVal_SCU418,14))
		printf("HVI3C6SCL\n");
	else if( CheckBit(gRegVal_SCU4B8,14) )
		printf("SCL4\n");
	else
		printf("GPIOJ6\n");

	printf(" D19: ");
	if(CheckBit(gRegVal_SCU418,15))
		printf("HVI3C6SDA\n");
	else if( CheckBit(gRegVal_SCU4B8,15) )
		printf("SDA4\n");
	else
		printf("GPIOJ7\n");
}

void pin_A11toE12() {
	printf(" A11: ");
	if(CheckBit(gRegVal_SCU418,16))
		printf("SCL5\n");
	else
		printf("GPIOK0\n");

	printf(" C11: ");
	if(CheckBit(gRegVal_SCU418,17))
		printf("SDA5\n");
	else
		printf("GPIOK1\n");

	printf(" D12: ");
	if(CheckBit(gRegVal_SCU418,18))
		printf("SCL6\n");
	else
		printf("GPIOK2\n");

	printf(" E13: ");
	if(CheckBit(gRegVal_SCU418,19))
		printf("SDA6\n");
	else
		printf("GPIOK3\n");

	printf(" D11: ");
	if(CheckBit(gRegVal_SCU418,20))
		printf("SCL7\n");
	else
		printf("GPIOK4\n");

	printf(" E11: ");
	if(CheckBit(gRegVal_SCU418,21))
		printf("SDA7\n");
	else
		printf("GPIOK5\n");

	printf(" F13: ");
	if(CheckBit(gRegVal_SCU418,22))
		printf("SCL8\n");
	else
		printf("GPIOK6\n");

	printf(" E12: ");
	if(CheckBit(gRegVal_SCU418,23))
		printf("SDA8\n");
	else
		printf("GPIOK7\n");
}

void pin_D15toC14() {
	printf(" D15: ");
	if(CheckBit(gRegVal_SCU418,24))
		printf("SCL9\n");
	else
		printf("GPIOL0\n");

	printf(" A14: ");
	if(CheckBit(gRegVal_SCU418,25))
		printf("SDA9\n");
	else
		printf("GPIOL1\n");

	printf(" E15: ");
	if(CheckBit(gRegVal_SCU418,26))
		printf("SCL10\n");
	else
		printf("GPIOL2\n");

	printf(" A13: ");
	if(CheckBit(gRegVal_SCU418,27))
		printf("SDA10\n");
	else
		printf("GPIOL3\n");

	printf(" C15: ");
	if(CheckBit(gRegVal_SCU418,28))
		printf("TXD3\n");
	else
		printf("GPIOL4\n");

	printf(" F15: ");
	if(CheckBit(gRegVal_SCU418,29))
		printf("RXD3\n");
	else
		printf("GPIOL5\n");

	printf(" B14: ");
	if(CheckBit(gRegVal_SCU418,30))
		printf("VGAHS\n");
	else
		printf("GPIOL6\n");

	printf(" C14: ");
	if(CheckBit(gRegVal_SCU418,31))
		printf("VGAVS\n");
	else
		printf("GPIOL7\n");
}

void pin_D14toD13() {
	printf(" D14: ");
	if(CheckBit(gRegVal_SCU41C,0))
		printf("NCTS1\n");
	else
		printf("GPIOM0\n");

	printf(" B13: ");
	if(CheckBit(gRegVal_SCU41C,1))
		printf("NDCD1\n");
	else
		printf("GPIOM1\n");

	printf(" A12: ");
	if(CheckBit(gRegVal_SCU41C,2))
		printf("NDSR1\n");
	else
		printf("GPIOM2\n");

	printf(" E14: ");
	if(CheckBit(gRegVal_SCU41C,3))
		printf("NRI1\n");
	else
		printf("GPIOM3\n");

	printf(" B12: ");
	if(CheckBit(gRegVal_SCU41C,4))
		printf("NDTR1\n");
	else
		printf("GPIOM4\n");

	printf(" C12: ");
	if(CheckBit(gRegVal_SCU41C,5))
		printf("NRTS1\n");
	else
		printf("GPIOM5\n");

	printf(" C13: ");
	if(CheckBit(gRegVal_SCU41C,6))
		printf("TXD1\n");
	else
		printf("GPIOM6\n");

	printf(" D13: ");
	if(CheckBit(gRegVal_SCU41C,7))
		printf("RXD1\n");
	else
		printf("GPIOM7\n");
}

void pin_P25toM26() {
	printf(" P25: ");
	if(CheckBit(gRegVal_SCU41C,8))
		printf("NCTS2\n");
	else
		printf("GPION0\n");

	printf(" N23: ");
	if(CheckBit(gRegVal_SCU41C,9))
		printf("NDCD2\n");
	else
		printf("GPION1\n");

	printf(" N25: ");
	if(CheckBit(gRegVal_SCU41C,10))
		printf("NDSR2\n");
	else
		printf("GPION2\n");

	printf(" N24: ");
	if(CheckBit(gRegVal_SCU41C,11))
		printf("NRI2\n");
	else
		printf("GPION3\n");

	printf(" P26: ");
	if(CheckBit(gRegVal_SCU41C,12))
		printf("NDTR2\n");
	else
		printf("GPION4\n");

	printf(" M23: ");
	if(CheckBit(gRegVal_SCU41C,13))
		printf("NRTS2\n");
	else
		printf("GPION5\n");

	printf(" N26: ");
	if(CheckBit(gRegVal_SCU41C,14))
		printf("TXD2\n");
	else
		printf("GPION6\n");

	printf(" M26: ");
	if(CheckBit(gRegVal_SCU41C,15))
		printf("RXD2\n");
	else
		printf("GPION7\n");
}

void pin_AD26toAC23() {
	printf("AD26: ");
	if(CheckBit(gRegVal_SCU41C,16))
		printf("PWM0\n");
	else
		printf("GPIOO0\n");

	printf("AD22: ");
	if(CheckBit(gRegVal_SCU41C,17))
		printf("PWM1\n");
	else
		printf("GPIOO1\n");

	printf("AD23: ");
	if(CheckBit(gRegVal_SCU41C,18))
		printf("PWM2\n");
	else
		printf("GPIOO2\n");

	printf("AD24: ");
	if(CheckBit(gRegVal_SCU41C,19))
		printf("PWM3\n");
	else
		printf("GPIOO3\n");

	printf("AD25: ");
	if(CheckBit(gRegVal_SCU41C,20))
		printf("PWM4\n");
	else
		printf("GPIOO4\n");

	printf("AC22: ");
	if(CheckBit(gRegVal_SCU41C,21))
		printf("PWM5\n");
	else
		printf("GPIOO5\n");

	printf("AC24: ");
	if(CheckBit(gRegVal_SCU41C,22))
		printf("PWM6\n");
	else
		printf("GPIOO6\n");

	printf("AC23: ");
	if(CheckBit(gRegVal_SCU41C,23))
		printf("PWM7\n");
	else
		printf("GPIOO7\n");
}

void pin_AB22toY23() {
	printf("AB22: ");
	if(CheckBit(gRegVal_SCU41C,24))
		printf("PWM8\n");
	else if(CheckBit(gRegVal_SCU4BC,24) ||
			CheckBit(gRegVal_SCU510,28))
		printf("THRUIN0\n");
	else
		printf("GPIOP0\n");

	printf(" W24: ");
	if(CheckBit(gRegVal_SCU41C,25))
		printf("PWM9\n");
	else if(CheckBit(gRegVal_SCU4BC,25) ||
			CheckBit(gRegVal_SCU510,28))
		printf("THRUOUT0\n");
	else
		printf("GPIOP1\n");

	printf("AA23: ");
	if(CheckBit(gRegVal_SCU41C,26))
		printf("PWM10\n");
	else if(CheckBit(gRegVal_SCU4BC,26) ||
			CheckBit(gRegVal_SCU510,28))
		printf("THRUIN1\n");
	else
		printf("GPIOP2\n");

	printf("AA24: ");
	if(CheckBit(gRegVal_SCU41C,27))
		printf("PWM11\n");
	else if(CheckBit(gRegVal_SCU4BC,27) ||
			CheckBit(gRegVal_SCU510,28))
		printf("THRUOUT1\n");
	else
		printf("GPIOP3\n");

	printf(" W23: ");
	if(CheckBit(gRegVal_SCU41C,28))
		printf("PWM12\n");
	else if(CheckBit(gRegVal_SCU4BC,28) ||
			CheckBit(gRegVal_SCU510,28))
		printf("THRUIN2\n");
	else
		printf("GPIOP4\n");

	printf("AB23: ");
	if(CheckBit(gRegVal_SCU41C,29))
		printf("PWM13\n");
	else if(CheckBit(gRegVal_SCU4BC,29) ||
			CheckBit(gRegVal_SCU510,28))
		printf("THRUOUT2\n");
	else
		printf("GPIOP5\n");

	printf("AB24: ");
	if(CheckBit(gRegVal_SCU41C,30))
		printf("PWM14\n");
	else
		printf("GPIOP6\n");

	printf(" Y23: ");
	if(CheckBit(gRegVal_SCU41C,31))
		printf("PWM15\n");
	else
		printf("HEARTBEAT\n");
}

void pin_AA25toAA26() {
	printf(" AA25: ");
	if(CheckBit(gRegVal_SCU430,0))
		printf("TACH0\n");
	else
		printf("GPIOQ0\n");

	printf(" AB25: ");
	if(CheckBit(gRegVal_SCU430,1))
		printf("TACH1\n");
	else
		printf("GPIOQ1\n");

	printf("  Y24: ");
	if(CheckBit(gRegVal_SCU430,2))
		printf("TACH2\n");
	else
		printf("GPIOQ2\n");

	printf(" AB26: ");
	if(CheckBit(gRegVal_SCU430,3))
		printf("TACH3\n");
	else
		printf("GPIOQ3\n");

	printf("  Y26: ");
	if(CheckBit(gRegVal_SCU430,4))
		printf("TACH4\n");
	else
		printf("GPIOQ4\n");

	printf(" AC26: ");
	if(CheckBit(gRegVal_SCU430,5))
		printf("TACH5\n");
	else
		printf("GPIOQ5\n");

	printf(" Y25: ");
	if(CheckBit(gRegVal_SCU430,6))
		printf("TACH6\n");
	else
		printf("GPIOQ6\n");

	printf(" AA26: ");
	if(CheckBit(gRegVal_SCU430,7))
		printf("TACH7\n");
	else
		printf("GPIOQ7\n");
}

void pin_V25toU26() {
	printf(" V25: ");
	if(CheckBit(gRegVal_SCU430,8))
		printf("TACH8\n");
	else
		printf("GPIOR0\n");

	printf(" U24: ");
	if(CheckBit(gRegVal_SCU430,9))
		printf("TACH9\n");
	else
		printf("GPIOR1\n");

	printf(" V24: ");
	if(CheckBit(gRegVal_SCU430,10))
		printf("TACH10\n");
	else
		printf("GPIOR2\n");
		
	printf(" V26: ");
	if(CheckBit(gRegVal_SCU430,11))
		printf("TACH11\n");
	else
		printf("GPIOR3\n");

	printf(" U25: ");
	if(CheckBit(gRegVal_SCU430,12))
		printf("TACH12\n");
	else
		printf("GPIOR4\n");

	printf(" T23: ");
	if(CheckBit(gRegVal_SCU430,13))
		printf("TACH13\n");
	else
		printf("GPIOR5\n");
		
	printf(" W26: ");
	if(CheckBit(gRegVal_SCU430,14))
		printf("TACH14\n");
	else
		printf("GPIOR6\n");
		
	printf(" U26: ");
	if(CheckBit(gRegVal_SCU430,15))
		printf("TACH15\n");
	else
		printf("GPIOR7\n");
}

void pin_R23toT24() {
	printf(" R23: ");
	if(CheckBit(gRegVal_SCU430,16))
		printf("MDC1\n");
	else
		printf("GPIOS0\n");

	printf(" T25: ");
	if(CheckBit(gRegVal_SCU430,17))
		printf("MDIO1\n");
	else
		printf("GPIOS1\n");

	printf(" T26: ");
	if(CheckBit(gRegVal_SCU430,18))
		printf("PEWAKE#\n");
	else
		printf("GPIOS2\n");

	printf(" R24: ");
	if(CheckBit(gRegVal_SCU430,19))
		printf("OSCCLK\n");
	else
		printf("GPIOS3\n");

	printf(" R26: ");
	if(CheckBit(gRegVal_SCU430,20))
		printf("TXD10\n");
	else
		printf("GPIOS4\n");

	printf(" P24: ");
	if(CheckBit(gRegVal_SCU430,21))
		printf("RXD10\n");
	else
		printf("GPIOS5\n");

	printf(" P23: ");
	if(CheckBit(gRegVal_SCU430,22))
		printf("TXD11\n");
	else
		printf("GPIOS6\n");

	printf(" T24: ");
	if(CheckBit(gRegVal_SCU430,23))
		printf("RXD11\n");
	else
		printf("GPIOS7\n");
}

void pin_AD20toAE18() {
	printf("AD20: ");
	if(CheckBit(gRegVal_SCU430,24))
		printf("GPIT0\n");
	else
		printf("ADC0\n");

	printf("AC18: ");
	if(CheckBit(gRegVal_SCU430,25))
		printf("GPIT1\n");
	else
		printf("ADC1\n");

	printf("AE19: ");
	if(CheckBit(gRegVal_SCU430,26))
		printf("GPIT2\n");
	else
		printf("ADC2\n");

	printf("AD19: ");
	if(CheckBit(gRegVal_SCU430,27))
		printf("GPIT3\n");
	else
		printf("ADC3\n");

	printf("AC19: ");
	if(CheckBit(gRegVal_SCU430,28))
		printf("GPIT4\n");
	else
		printf("ADC4\n");

	printf("AB19: ");
	if(CheckBit(gRegVal_SCU430,29))
		printf("GPIT5\n");
	else
		printf("ADC5\n");

	printf("AB18: ");
	if(CheckBit(gRegVal_SCU430,30))
		printf("GPIT6\n");
	else
		printf("ADC6\n");

	printf("AE18: ");
	if(CheckBit(gRegVal_SCU430,31))
		printf("GPIT7\n");
	else
		printf("ADC7\n");
}

void pin_AB16toAC17() {
	printf("AB16: ");
	if(CheckBit(gRegVal_SCU434,0) &&
	   !CheckBit(gRegVal_SCU694,16) &&
	   CheckBit(gRegVal_SCU4D4,0))
		printf("SALT9\n");
	else if(CheckBit(gRegVal_SCU434,0) &&
	        !CheckBit(gRegVal_SCU4D4,0))
		printf("GPIU0\n");
	else
		printf("ADC8\n");

	printf("AA17: ");
	if(CheckBit(gRegVal_SCU434,1) &&
	   !CheckBit(gRegVal_SCU694,17) &&
	   CheckBit(gRegVal_SCU4D4,1))
		printf("SALT10\n");
	else if(CheckBit(gRegVal_SCU434,1) &&
	        !CheckBit(gRegVal_SCU4D4,1))
		printf("GPIU1\n");
	else
		printf("ADC9\n");

	printf("AB17: ");
	if(CheckBit(gRegVal_SCU434,2) &&
	   !CheckBit(gRegVal_SCU694,18) &&
	   CheckBit(gRegVal_SCU4D4,2))
		printf("SALT11\n");
	else if(CheckBit(gRegVal_SCU434,2) &&
	        !CheckBit(gRegVal_SCU4D4,2))
		printf("GPIU2\n");
	else
		printf("ADC10\n");

	printf("AE16: ");
	if(CheckBit(gRegVal_SCU434,3) &&
	   !CheckBit(gRegVal_SCU694,19) &&
	   CheckBit(gRegVal_SCU4D4,3))
		printf("SALT12\n");
	else if(CheckBit(gRegVal_SCU434,3) &&
	        !CheckBit(gRegVal_SCU4D4,3))
		printf("GPIU3\n");
	else
		printf("ADC11\n");

	printf("AC16: ");
	if(CheckBit(gRegVal_SCU434,4) &&
	   !CheckBit(gRegVal_SCU694,20) &&
	   CheckBit(gRegVal_SCU4D4,4))
		printf("SALT13\n");
	else if(CheckBit(gRegVal_SCU434,4) &&
	        !CheckBit(gRegVal_SCU4D4,4))
		printf("GPIU4\n");
	else
		printf("ADC12\n");

	printf("AA16: ");
	if(CheckBit(gRegVal_SCU434,5) &&
	   !CheckBit(gRegVal_SCU694,21) &&
	   CheckBit(gRegVal_SCU4D4,5))
		printf("SALT14\n");
	else if(CheckBit(gRegVal_SCU434,5) &&
	        !CheckBit(gRegVal_SCU4D4,5))
		printf("GPIU5\n");
	else
		printf("ADC13\n");

	printf("AD16: ");
	if(CheckBit(gRegVal_SCU434,6) &&
	   !CheckBit(gRegVal_SCU694,22) &&
	   CheckBit(gRegVal_SCU4D4,6))
		printf("SALT15\n");
	else if(CheckBit(gRegVal_SCU434,6) &&
	        !CheckBit(gRegVal_SCU4D4,6))
		printf("GPIU6\n");
	else
		printf("ADC14\n");

	printf("AC17: ");
	if(CheckBit(gRegVal_SCU434,7) &&
	   !CheckBit(gRegVal_SCU694,23) &&
	   CheckBit(gRegVal_SCU4D4,7))
		printf("SALT16\n");
	else if(CheckBit(gRegVal_SCU434,7) &&
	        !CheckBit(gRegVal_SCU4D4,7))
		printf("GPIU7\n");
	else
		printf("ADC15\n");
}

void pin_AB15toAF15() {
	printf("AB15: ");
	if(CheckBit(gRegVal_SCU434,8) ||
	   CheckBit(gRegVal_SCU510,5))
		printf("SIOS3#\n");
	else
		printf("GPIOV0\n");

	printf("AF14: ");
	if(CheckBit(gRegVal_SCU434,9) ||
	   CheckBit(gRegVal_SCU510,5))
		printf("SIOS5#\n");
	else
		printf("GPIOV1\n");

	printf("AD14: ");
	if(CheckBit(gRegVal_SCU434,10) ||
	   CheckBit(gRegVal_SCU510,5))
		printf("SIOPWREQ#\n");
	else
		printf("GPIOV2\n");

	printf("AC15: ");
	if(CheckBit(gRegVal_SCU434,11) ||
	   CheckBit(gRegVal_SCU510,5))
		printf("SIOONCTRL#\n");
	else
		printf("GPIOV3\n");

	printf("AE15: ");
	if(CheckBit(gRegVal_SCU434,12) ||
	   CheckBit(gRegVal_SCU510,5))
		printf("SIOPWRGD\n");
	else
		printf("GPIOV4\n");

	printf("AE14: ");
	if(CheckBit(gRegVal_SCU434,13))
		printf("LPCPD#\n");
	else
		printf("GPIOV5\n");

	printf("AD15: ");
	if(CheckBit(gRegVal_SCU434,14))
		printf("LPCPME\n");
	else
		printf("GPIOV6\n");

	printf("AF15: ");
	if(CheckBit(gRegVal_SCU434,15))
		printf("LPCSMI#\n");
	else
		printf("GPIOV7\n");
}

void pin_AB7toAD8() {
	printf("AB7: ");
	if(CheckBit(gRegVal_SCU434,16) &&
	   !CheckBit(gRegVal_SCU510,6))
		printf("ESPID0\n");
	else if(CheckBit(gRegVal_SCU434,16) &&
	        CheckBit(gRegVal_SCU510,6))
		printf("LAD0\n");
	else
		printf("GPIOW0\n");

	printf("AB8: ");
	if(CheckBit(gRegVal_SCU434,17) &&
	   !CheckBit(gRegVal_SCU510,6))
		printf("ESPID1\n");
	else if(CheckBit(gRegVal_SCU434,17) &&
	        CheckBit(gRegVal_SCU510,6))
		printf("LAD1\n");
	else
		printf("GPIOW1\n");

	printf("AC8: ");
	if(CheckBit(gRegVal_SCU434,18) &&
	   !CheckBit(gRegVal_SCU510,6))
		printf("ESPID2\n");
	else if(CheckBit(gRegVal_SCU434,18) &&
	        CheckBit(gRegVal_SCU510,6))
		printf("LAD2\n");
	else
		printf("GPIOW2\n");

	printf("AC7: ");
	if(CheckBit(gRegVal_SCU434,19) &&
	   !CheckBit(gRegVal_SCU510,6))
		printf("ESPID3\n");
	else if(CheckBit(gRegVal_SCU434,19) &&
	        CheckBit(gRegVal_SCU510,6))
		printf("LAD3\n");
	else
		printf("GPIOW3\n");

	printf("AE7: ");
	if(CheckBit(gRegVal_SCU434,20) &&
	   !CheckBit(gRegVal_SCU510,6))
		printf("ESPICK\n");
	else if(CheckBit(gRegVal_SCU434,20) &&
	        CheckBit(gRegVal_SCU510,6))
		printf("LCLK\n");
	else
		printf("GPIOW4\n");

	printf("AF7: ");
	if(CheckBit(gRegVal_SCU434,21) &&
	   !CheckBit(gRegVal_SCU510,6))
		printf("ESPICS#\n");
	else if(CheckBit(gRegVal_SCU434,21) &&
	        CheckBit(gRegVal_SCU510,6))
		printf("LFRAME#\n");
	else
		printf("GPIOW5\n");

	printf("AD7: ");
	if(CheckBit(gRegVal_SCU434,22) &&
	   !CheckBit(gRegVal_SCU510,6))
		printf("ESPIALT#\n");
	else if(CheckBit(gRegVal_SCU434,22) &&
	        CheckBit(gRegVal_SCU510,6))
		printf("LSIRQ#\n");
	else
		printf("GPIOW6\n");

	printf("AD8: ");
	if(CheckBit(gRegVal_SCU434,23) &&
	   !CheckBit(gRegVal_SCU510,6))
		printf("ESPIRST#\n");
	else if(CheckBit(gRegVal_SCU434,23) &&
	        CheckBit(gRegVal_SCU510,6))
		printf("LPCRST#\n");
	else
		printf("GPIOW7\n");
}

void pin_AE8toAB10() {
	printf("AE8: ");
	if(CheckBit(gRegVal_SCU434,24))
		printf("SPI2CS0#\n");
	else
		printf("GPIOX0\n");

	printf("AA9: ");
	if(CheckBit(gRegVal_SCU434,25))
		printf("SPI2CS1#\n");
	else
		printf("GPIOX1\n");

	printf("AC9: ");
	if(CheckBit(gRegVal_SCU434,26))
		printf("SPI2CS2#\n");
	else
		printf("GPIOX2\n");

	printf("AF8: ");
	if(CheckBit(gRegVal_SCU434,27))
		printf("SPI2CSK\n");
	else
		printf("GPIOX3\n");

	printf("AB9: ");
	if(CheckBit(gRegVal_SCU434,28))
		printf("SPI2MOSI\n");
	else
		printf("GPIOX4\n");

	printf("AD9: ");
	if(CheckBit(gRegVal_SCU434,29))
		printf("SPI2MISO\n");
	else
		printf("GPIOX5\n");

	printf("AF9: ");
	if(CheckBit(gRegVal_SCU434,30))
		printf("SPI2DQ2\n");
	else if(CheckBit(gRegVal_SCU4D4,30))
		printf("TXD12\n");
	else
		printf("GPIOX6\n");

	printf("AB10: ");
	if(CheckBit(gRegVal_SCU434,31))
		printf("SPI2DQ3\n");
	else if(CheckBit(gRegVal_SCU4D4,31))
		printf("RXD12\n");
	else
		printf("GPIOX7\n");
}

void pin_AF11toAB12() {
	printf("AF11: ");
	if(CheckBit(gRegVal_SCU438,0))
		printf("SALT5\n");
	else if(CheckBit(gRegVal_SCU4D8,0))
		printf("WDTRST1#\n");
	else
		printf("GPIOY0\n");

	printf("AD12: ");
	if(CheckBit(gRegVal_SCU438,1))
		printf("SALT6\n");
	else if(CheckBit(gRegVal_SCU4D8,1))
		printf("WDTRST2#\n");
	else
		printf("GPIOY1\n");

	printf("AE11: ");
	if(CheckBit(gRegVal_SCU438,2))
		printf("SALT7\n");
	else if(CheckBit(gRegVal_SCU4D8,2))
		printf("WDTRST3#\n");
	else
		printf("GPIOY2\n");

	printf("AA12: ");
	if(CheckBit(gRegVal_SCU438,3))
		printf("SALT8\n");
	else if(CheckBit(gRegVal_SCU4D8,3))
		printf("WDTRST4#\n");
	else
		printf("GPIOY3\n");

	printf("AE12: ");
	if(CheckBit(gRegVal_SCU438,4) ||
	   CheckBit(gRegVal_SCU510,22))
		printf("FWSPIDQ2\n");
	else
		printf("GPIOY4\n");

	printf("AF12: ");
	if(CheckBit(gRegVal_SCU438,5) ||
	   CheckBit(gRegVal_SCU510,22))
		printf("FWSPIDQ3\n");
	else
		printf("GPIOY5\n");

	printf("AC12: ");
	if(CheckBit(gRegVal_SCU438,6) ||
	   CheckBit(gRegVal_SCU510,22))
		printf("FWSPIABR\n");
	else
		printf("GPIOY6\n");

	printf("AB12: ");
	if(CheckBit(gRegVal_SCU438,7) ||
	   CheckBit(gRegVal_SCU510,22))
		printf("FWSPIWP#\n");
	else
		printf("GPIOY7\n");
}

void pin_AC10toAF10() {
	printf("AC10: ");
	if(CheckBit(gRegVal_SCU438,8) ||
	   (CheckBit(gRegVal_SCU510,16) &&
	    !CheckBit(gRegVal_SCU510,18)))
		printf("SPI1CS1#\n");
	else
		printf("GPIOZ0\n");

	printf("AD10: ");
	if(CheckBit(gRegVal_SCU438,9) ||
	   CheckBit(gRegVal_SCU510,17) ||
	   CheckBit(gRegVal_SCU510,27))
		printf("SPI1ABR\n");
	else
		printf("GPIOZ1\n");

	printf("AE10: ");
	if(CheckBit(gRegVal_SCU438,10) ||
	   CheckBit(gRegVal_SCU510,27))
		printf("SPI1WP#\n");
	else
		printf("GPIOZ2\n");

	printf("AB11: ");
	if(CheckBit(gRegVal_SCU438,11))
		printf("SPI1CK\n");
	else
		printf("GPIOZ3\n");

	printf("AC11: ");
	if(CheckBit(gRegVal_SCU438,12))
		printf("SPI1MOSI\n");
	else
		printf("GPIOZ4\n");

	printf("AA11: ");
	if(CheckBit(gRegVal_SCU438,13))
		printf("SPI1MISO\n");
	else
		printf("GPIOZ5\n");

	printf("AD11: ");
	if(CheckBit(gRegVal_SCU438,14) ||
	   CheckBit(gRegVal_SCU510,27))
		printf("SPI1DQ2\n");
	else if(CheckBit(gRegVal_SCU4D8,14) &&
	        !CheckBit(gRegVal_SCU4B8,2))
		printf("TXD13\n");
	else
		printf("GPIOZ6\n");

	printf("AF10: ");
	if(CheckBit(gRegVal_SCU438,15) ||
	   CheckBit(gRegVal_SCU510,27))
		printf("SPI1DQ3\n");
	else if(CheckBit(gRegVal_SCU4D8,15) &&
	        !CheckBit(gRegVal_SCU4B8,3))
		printf("RXD13\n");
	else
		printf("GPIOZ7\n");
}

void pin_C6toA2() {
	printf("  C6: ");
	if(CheckBit(gRegVal_SCU400,0) &&
	   CheckBit(gRegVal_SCU500,6))
		printf("RGMII1TXCK\n");
	else if(CheckBit(gRegVal_SCU400,0) &&
	        !CheckBit(gRegVal_SCU500,6))
		printf("RMIIRCLKO\n");
	else
		printf("GPIO18A0\n");

	printf("  D6: ");
	if(CheckBit(gRegVal_SCU400,1) &&
	   CheckBit(gRegVal_SCU500,6))
		printf("RGMII1TXCTL\n");
	else if(CheckBit(gRegVal_SCU400,1) &&
	        !CheckBit(gRegVal_SCU500,6))
		printf("RMMII1TXEN\n");
	else
		printf("GPIO18A1\n");

	printf("  D5: ");
	if(CheckBit(gRegVal_SCU400,2) &&
	   CheckBit(gRegVal_SCU500,6))
		printf("RGMII1TXD0\n");
	else if(CheckBit(gRegVal_SCU400,2) &&
	        !CheckBit(gRegVal_SCU500,6))
		printf("RMII1TXD0\n");
	else
		printf("GPIO18A2\n");

	printf("  A3: ");
	if(CheckBit(gRegVal_SCU400,3) &&
	   CheckBit(gRegVal_SCU500,6))
		printf("RGMII1TXD1\n");
	else if(CheckBit(gRegVal_SCU400,3) &&
	        !CheckBit(gRegVal_SCU500,6))
		printf("RMII1TXD1\n");
	else
		printf("GPIO18A3\n");

	printf("  C5: ");
	if(CheckBit(gRegVal_SCU400,4) &&
	   CheckBit(gRegVal_SCU500,6))
		printf("RGMII1TXD2\n");
	else
		printf("GPIO18A4\n");

	printf("  E6: ");
	if(CheckBit(gRegVal_SCU400,5) &&
	   CheckBit(gRegVal_SCU500,6))
		printf("RGMII1TXD3\n");
	else
		printf("GPIO18A5\n");

	printf("  B3: ");
	if(CheckBit(gRegVal_SCU400,6) &&
	   CheckBit(gRegVal_SCU500,6))
		printf("RGMII1RXCK\n");
	else if(CheckBit(gRegVal_SCU400,6) &&
	        !CheckBit(gRegVal_SCU500,6))
		printf("RMII1RCLKI\n");
	else
		printf("GPIO18A6\n");

	printf("  A2: ");
	if(CheckBit(gRegVal_SCU400,7) &&
	   CheckBit(gRegVal_SCU500,6))
		printf("RGMII1RXCTL\n");
	else
		printf("GPIO18A7\n");
}

void pin_B2toD3() {
	printf("  B2: ");
	if(CheckBit(gRegVal_SCU400,8) &&
	   CheckBit(gRegVal_SCU500,6))
		printf("RGMII1RXD0\n");
	else if(CheckBit(gRegVal_SCU400,8) &&
	        !CheckBit(gRegVal_SCU500,6))
		printf("RMII1RXD0\n");
	else
		printf("GPIO18B0\n");

	printf("  B1: ");
	if(CheckBit(gRegVal_SCU400,9) &&
	   CheckBit(gRegVal_SCU500,6))
		printf("RGMII1RXD1\n");
	else if(CheckBit(gRegVal_SCU400,9) &&
	        !CheckBit(gRegVal_SCU500,6))
		printf("RMII1RXD1\n");
	else
		printf("GPIO18B1\n");

	printf("  C4: ");
	if(CheckBit(gRegVal_SCU400,10) &&
	   CheckBit(gRegVal_SCU500,6))
		printf("RGMII1RXD2\n");
	else if(CheckBit(gRegVal_SCU400,10) &&
	        !CheckBit(gRegVal_SCU500,6))
		printf("RMII1CRSDV\n");
	else
		printf("GPIO18B2\n");

	printf("  E5: ");
	if(CheckBit(gRegVal_SCU400,11) &&
	   CheckBit(gRegVal_SCU500,6))
		printf("RGMII1RXD3\n");
	else if(CheckBit(gRegVal_SCU400,11) &&
	        !CheckBit(gRegVal_SCU500,6))
		printf("RMII1RXER\n");
	else
		printf("GPIO18B3\n");

	printf("  D4: ");
	if(CheckBit(gRegVal_SCU400,12) &&
	   CheckBit(gRegVal_SCU500,7))
		printf("RGMII2TXCK\n");
	else if(CheckBit(gRegVal_SCU400,12) &&
	        !CheckBit(gRegVal_SCU500,7))
		printf("RMII2RCLKO\n");
	else
		printf("GPIO18B4\n");

	printf("  C2: ");
	if(CheckBit(gRegVal_SCU400,13) &&
	   CheckBit(gRegVal_SCU500,7))
		printf("RGMII2TXCTL\n");
	else if(CheckBit(gRegVal_SCU400,13) &&
	        !CheckBit(gRegVal_SCU500,7))
		printf("RMII2TXEN\n");
	else
		printf("GPIO18B5\n");

	printf("  C1: ");
	if(CheckBit(gRegVal_SCU400,14) &&
	   CheckBit(gRegVal_SCU500,7))
		printf("RGMII2TXD0\n");
	else if(CheckBit(gRegVal_SCU400,14) &&
	        !CheckBit(gRegVal_SCU500,7))
		printf("RMII2TXD0\n");
	else
		printf("GPIO18B6\n");

	printf("  D3: ");
	if(CheckBit(gRegVal_SCU400,15) &&
	   CheckBit(gRegVal_SCU500,7))
		printf("RGMII2TXD1\n");
	else if(CheckBit(gRegVal_SCU400,15) &&
	        !CheckBit(gRegVal_SCU500,7))
		printf("RMII2TXD1\n");
	else
		printf("GPIO18B7\n");
}

void pin_E4toE1() {
	printf("  E4: ");
	if(CheckBit(gRegVal_SCU400,16) &&
	   CheckBit(gRegVal_SCU500,7))
		printf("RGMII2TXD2\n");
	else
		printf("GPIO18C0\n");

	printf("  F5: ");
	if(CheckBit(gRegVal_SCU400,17) &&
	   CheckBit(gRegVal_SCU500,7))
		printf("RGMII2TXD3\n");
	else
		printf("GPIO18C1\n");

	printf("  D2: ");
	if(CheckBit(gRegVal_SCU400,18) &&
	   CheckBit(gRegVal_SCU500,7))
		printf("RGMII2RXCK\n");
	else if(CheckBit(gRegVal_SCU400,18) &&
			!CheckBit(gRegVal_SCU500,7))
		printf("RMII2RCLKI\n");
	else
		printf("GPIO18C2\n");

	printf("  E3: ");
	if(CheckBit(gRegVal_SCU400,19) &&
	   CheckBit(gRegVal_SCU500,7))
		printf("RGMII2RXCTL\n");
	else
		printf("GPIO18C3\n");

	printf("  D1: ");
	if(CheckBit(gRegVal_SCU400,20) &&
	   CheckBit(gRegVal_SCU500,7))
		printf("RGMII2RXD0\n");
	else if(CheckBit(gRegVal_SCU400,20) &&
			!CheckBit(gRegVal_SCU500,7))
		printf("RMII2RXD0\n");
	else
		printf("GPIO18C4\n");

	printf("  F4: ");
	if(CheckBit(gRegVal_SCU400,21) &&
	   CheckBit(gRegVal_SCU500,7))
		printf("RGMII2RXD1\n");
	else if(CheckBit(gRegVal_SCU400,21) &&
			!CheckBit(gRegVal_SCU500,7))
		printf("RMII2RXD1\n");
	else
		printf("GPIO18C5\n");

	printf("  E2: ");
	if(CheckBit(gRegVal_SCU400,22) &&
	   CheckBit(gRegVal_SCU500,7))
		printf("RGMII2RXD2\n");
	else if(CheckBit(gRegVal_SCU400,22) &&
			!CheckBit(gRegVal_SCU500,7))
		printf("RMII2CRSDV\n");
	else
		printf("GPIO18C6\n");

	printf("  E1: ");
	if(CheckBit(gRegVal_SCU400,23) &&
	   CheckBit(gRegVal_SCU500,7))
		printf("RGMII2RXD3\n");
	else if(CheckBit(gRegVal_SCU400,23) &&
			!CheckBit(gRegVal_SCU500,7))
		printf("RMII2RXER\n");
	else
		printf("GPIO18C7\n");
}

void pin_AB4toAC5() {
	printf(" AB4: ");
	if(CheckBit(gRegVal_SCU400,24))
		printf("EMMCCLK\n");
	else
		printf("GPIO18D0\n");

	printf(" AA4: ");
	if(CheckBit(gRegVal_SCU400,25))
		printf("EMMCCMD\n");
	else
		printf("GPIO18D1\n");

	printf(" AC4: ");
	if(CheckBit(gRegVal_SCU400,26))
		printf("EMMCDAT0\n");
	else
		printf("GPIO18D2\n");

	printf(" AA5: ");
	if(CheckBit(gRegVal_SCU400,27))
		printf("EMMCDAT1\n");
	else
		printf("GPIO18D3\n");

	printf("  Y5: ");
	if(CheckBit(gRegVal_SCU400,28))
		printf("EMMCDAT2\n");
	else
		printf("GPIO18D4\n");

	printf(" AB5: ");
	if(CheckBit(gRegVal_SCU400,29))
		printf("EMMCDAT3\n");
	else
		printf("GPIO18D5\n");

	printf(" AB6: ");
	if(CheckBit(gRegVal_SCU400,30))
		printf("EMMCCD#\n");
	else
		printf("GPIO18D6\n");

	printf(" AC5: ");
	if(CheckBit(gRegVal_SCU400,31))
		printf("EMMCWP#\n");
	else
		printf("GPIO18D7\n");
}

void pin_Y1toY4() {
	printf("  Y1: ");
	if(CheckBit(gRegVal_SCU500,3))
		printf("FWSPI18CS#\n");
	else if(CheckBit(gRegVal_SCU500,5))
		printf("VBCS#\n");
	else if(CheckBit(gRegVal_SCU404,0))
		printf("EMMCDAT4\n");
	else
		printf("GPIO18E0\n");

	printf("  Y2: ");
	if(CheckBit(gRegVal_SCU500,3))
		printf("FWSPI18CK\n");
	else if(CheckBit(gRegVal_SCU500,5))
		printf("VBCK\n");
	else if(CheckBit(gRegVal_SCU404,1))
		printf("EMMCDAT5\n");
	else
		printf("GPIO18E1\n");

	printf("  Y3: ");
	if(CheckBit(gRegVal_SCU500,3))
		printf("FWSPI18MOSI\n");
	else if(CheckBit(gRegVal_SCU500,5))
		printf("VBMOSI\n");
	else if(CheckBit(gRegVal_SCU404,2))
		printf("EMMCDAT6\n");
	else
		printf("GPIO18E2\n");

	printf("  Y4: ");
	if(CheckBit(gRegVal_SCU500,3))
		printf("FWSPI18MISO\n");
	else if(CheckBit(gRegVal_SCU500,5))
		printf("VBMISO\n");
	else if(CheckBit(gRegVal_SCU404,3))
		printf("EMMCDAT7\n");
	else
		printf("GPIO18E3\n");
}

int GetAspeedRegVal(uint64_t target, uint64_t *read_result)
{
    int fd;
    void *map_base, *virt_addr;
    char *endp = NULL;
    int access_size = 4;
	unsigned offset;
    int f_align_check = 1; // flag to require alignment
    int f_loop = 1;        // flag to do looping
    unsigned int pagesize = (unsigned)sysconf(_SC_PAGESIZE);
    unsigned int map_size = pagesize;
    errno = 0;

    if (errno != 0 || (endp && 0 != *endp))
    {
        printerr("Invalid memory address: 0x%x\n", target);
        return -1;
    }

    if ((target + access_size - 1) < target)
    {
        printerr("ERROR: rolling over end of memory\n");
        return -1;
    }

    if ((sizeof(off_t) < sizeof(int64_t)) && (target > UINT32_MAX))
    {
        printerr("The address 0x%x > 32 bits. Try to rebuild in 64-bit mode.\n", target);
        // Consider mmap2() instead of this check
        return -1;
    }

    offset = (unsigned int)(target & (pagesize - 1));
    if (offset + access_size > pagesize)
    {
        // Access straddles page boundary:  add another page:
        map_size += pagesize;
    }

    if (f_dbg)
    {
        printerr("Address: 0x%x" PRIX64 " op.size=%d\n", target, access_size);
    }
    if (f_align_check && offset & (access_size - 1))
    {
        printerr("ERROR: address not aligned on %d!\n", access_size);
        return -1;
    }

    fd = open("/dev/mem", O_RDWR | O_SYNC);
    if (fd == -1)
    {
        printerr("Error opening /dev/mem (%d) : %s\n", errno, strerror(errno));
        return -1;
    }
    //printf("/dev/mem opened.\n");
    //fflush(stdout);

    map_base = mmap(0, map_size, PROT_READ | PROT_WRITE, MAP_SHARED,
            fd,
            target & ~((typeof(target))pagesize - 1));
    if (map_base == (void *)-1)
    {
        printerr("Error mapping (%d) : %s\n", errno, strerror(errno));
		close(fd);
        return -1;
    }
    //printf("Memory mapped at address %p.\n", map_base);
    //fflush(stdout);

    virt_addr = map_base + offset;

        int i;
        for (i = 0; i < f_loop; i++)
        {
            switch (access_size)
            {
                case 1:
                    *read_result = *((volatile uint8_t *)virt_addr);
                    break;
                case 2:
                    *read_result = *((volatile uint16_t *)virt_addr);
                    break;
                case 4:
                    *read_result = *((volatile uint32_t *)virt_addr);
                    break;
                case 8:
                    *read_result = *((volatile uint64_t *)virt_addr);
                    break;
            }
        }

        //printf("Value at address 0x%lld (%p): 0x%lu\n", (long long)target, virt_addr, read_result);
        //fflush(stdout);

    if (munmap(map_base, map_size) != 0)
    {
        printerr("ERROR munmap (%d) %s\n", errno, strerror(errno));
    }

    close(fd);

    return 0;
}


void listMulitFunPin()
{

	printf("*****MULTI-FUN-SCU-REG*****\n");
	if(0 == GetAspeedRegVal(ADDR_SCU400, &gRegVal_SCU400))
		printf("SCU400: 0x%08" PRIX64 "\n", gRegVal_SCU400);

	if(0 == GetAspeedRegVal(ADDR_SCU404, &gRegVal_SCU404))
		printf("SCU404: 0x%08" PRIX64 "\n", gRegVal_SCU404);

	if(0 == GetAspeedRegVal(ADDR_SCU410, &gRegVal_SCU410))
		printf("SCU410: 0x%08" PRIX64 "\n", gRegVal_SCU410);

	if(0 == GetAspeedRegVal(ADDR_SCU414, &gRegVal_SCU414))
		printf("SCU414: 0x%08" PRIX64 "\n", gRegVal_SCU414);

	if(0 == GetAspeedRegVal(ADDR_SCU418, &gRegVal_SCU418))
		printf("SCU418: 0x%08" PRIX64 "\n", gRegVal_SCU418);


	if(0 == GetAspeedRegVal(ADDR_SCU41C, &gRegVal_SCU41C))
		printf("SCU41C: 0x%08" PRIX64 "\n", gRegVal_SCU41C);

	if(0 == GetAspeedRegVal(ADDR_SCU430, &gRegVal_SCU430))
		printf("SCU430: 0x%08" PRIX64 "\n", gRegVal_SCU430);

	if(0 == GetAspeedRegVal(ADDR_SCU434, &gRegVal_SCU434))
		printf("SCU434: 0x%08" PRIX64 "\n", gRegVal_SCU434);

	if(0 == GetAspeedRegVal(ADDR_SCU438, &gRegVal_SCU438))
		printf("SCU438: 0x%08" PRIX64 "\n", gRegVal_SCU438);

	if(0 == GetAspeedRegVal(ADDR_SCU450, &gRegVal_SCU450))
		printf("SCU450: 0x%08" PRIX64 "\n", gRegVal_SCU450);
		

	if(0 == GetAspeedRegVal(ADDR_SCU470, &gRegVal_SCU470))
		printf("SCU470: 0x%08" PRIX64 "\n", gRegVal_SCU470);

	if(0 == GetAspeedRegVal(ADDR_SCU4B0, &gRegVal_SCU4B0))
		printf("SCU4B0: 0x%08" PRIX64 "\n", gRegVal_SCU4B0);

	if(0 == GetAspeedRegVal(ADDR_SCU4B4, &gRegVal_SCU4B4))
		printf("SCU4B4: 0x%08" PRIX64 "\n", gRegVal_SCU4B4);

	if(0 == GetAspeedRegVal(ADDR_SCU4B8, &gRegVal_SCU4B8))
		printf("SCU4B8: 0x%08" PRIX64 "\n", gRegVal_SCU4B8);

	if(0 == GetAspeedRegVal(ADDR_SCU4BC, &gRegVal_SCU4BC))
		printf("SCU4BC: 0x%08" PRIX64 "\n", gRegVal_SCU4BC);


	if(0 == GetAspeedRegVal(ADDR_SCU4D4, &gRegVal_SCU4D4))
		printf("SCU4D4: 0x%08" PRIX64 "\n", gRegVal_SCU4D4);

	if(0 == GetAspeedRegVal(ADDR_SCU4D8, &gRegVal_SCU4D8))
		printf("SCU4D8: 0x%08" PRIX64 "\n", gRegVal_SCU4D8);

	if(0 == GetAspeedRegVal(ADDR_SCU500, &gRegVal_SCU500))
		printf("SCU500: 0x%08" PRIX64 "\n", gRegVal_SCU500);

	if(0 == GetAspeedRegVal(ADDR_SCU510, &gRegVal_SCU510))
		printf("SCU510: 0x%08" PRIX64 "\n", gRegVal_SCU510);

	if(0 == GetAspeedRegVal(ADDR_SCU690, &gRegVal_SCU690))
		printf("SCU690: 0x%08" PRIX64 "\n", gRegVal_SCU690);


	if(0 == GetAspeedRegVal(ADDR_SCU694, &gRegVal_SCU694))
		printf("SCU694: 0x%08" PRIX64 "\n", gRegVal_SCU694);

	printf("***************************\n");


	pin_AF23toAF24();
	printf("\n");
	pin_M24toK25();
	printf("\n");
	pin_J26toJ24();
	printf("\n");
	pin_H24toG24();
	printf("\n");
	pin_F23toE25();
	printf("\n");
	pin_D26toB24();
	printf("\n");
	pin_D22toA23();
	printf("\n");
	pin_E21toB21();
	printf("\n");
	pin_A18toE18();
	printf("\n");
	pin_D17toA15();
	printf("\n");
	pin_B20toD19();
	printf("\n");
	pin_A11toE12();
	printf("\n");
	pin_D15toC14();
	printf("\n");
	pin_D14toD13();
	printf("\n");
	pin_P25toM26();
	printf("\n");
	pin_AD26toAC23();
	printf("\n");
	pin_AB22toY23();
	printf("\n");
	pin_AA25toAA26();
	printf("\n");
	pin_V25toU26();
	printf("\n");
	pin_R23toT24();
	printf("\n");
	pin_AD20toAE18();
	printf("\n");
	pin_AB16toAC17();
	printf("\n");
	pin_AB15toAF15();
	printf("\n");
	pin_AB7toAD8();
	printf("\n");
	pin_AE8toAB10();
	printf("\n");
	pin_AF11toAB12();
	printf("\n");
	pin_AC10toAF10();
	printf("\n");
	pin_C6toA2();
	printf("\n");
	pin_B2toD3();
	printf("\n");
	pin_E4toE1();
	printf("\n");
	pin_AB4toAC5();
	printf("\n");
	pin_Y1toY4();
}

void listGpioPin()
{
	unsigned int i=0;
	char ch=0;
	printf("*****GPIO-REG*****\n");
	if(0 == GetAspeedRegVal(ADDR_GPIO000_ABCD_DATA, &gRegVal_DATA_ABCD))
		printf("ABCD DATA: 0x%08" PRIX64 "\n", gRegVal_DATA_ABCD);
	if(0 == GetAspeedRegVal(ADDR_GPIO004_ABCD_DIR, &gRegVal_DIR_ABCD))
		printf("ABCD  DIR: 0x%08" PRIX64 "\n", gRegVal_DIR_ABCD);

	if(0 == GetAspeedRegVal(ADDR_GPIO020_EFGH_DATA, &gRegVal_DATA_EFGH))
		printf("EFGH DATA: 0x%08" PRIX64 "\n", gRegVal_DATA_EFGH);
	if(0 == GetAspeedRegVal(ADDR_GPIO024_EFGH_DIR, &gRegVal_DIR_EFGH))
		printf("EFGH  DIR: 0x%08" PRIX64 "\n", gRegVal_DIR_EFGH);

	if(0 == GetAspeedRegVal(ADDR_GPIO070_IJKL_DATA, &gRegVal_DATA_IJKL))
		printf("IJKL DATA: 0x%08" PRIX64 "\n", gRegVal_DATA_IJKL);
	if(0 == GetAspeedRegVal(ADDR_GPIO074_IJKL_DIR, &gRegVal_DIR_IJKL))
		printf("IJKL  DIR: 0x%08" PRIX64 "\n", gRegVal_DIR_IJKL);

	if(0 == GetAspeedRegVal(ADDR_GPIO078_MNOP_DATA, &gRegVal_DATA_MNOP))
		printf("MNOP DATA: 0x%08" PRIX64 "\n", gRegVal_DATA_MNOP);
	if(0 == GetAspeedRegVal(ADDR_GPIO07C_MNOP_DIR, &gRegVal_DIR_MNOP))
		printf("MNOP  DIR: 0x%08" PRIX64 "\n", gRegVal_DIR_MNOP);

	if(0 == GetAspeedRegVal(ADDR_GPIO080_QRST_DATA, &gRegVal_DATA_QRST))
		printf("QRST DATA: 0x%08" PRIX64 "\n", gRegVal_DATA_QRST);
	if(0 == GetAspeedRegVal(ADDR_GPIO084_QRST_DIR, &gRegVal_DIR_QRST))
		printf("QRST  DIR: 0x%08" PRIX64 "\n", gRegVal_DIR_QRST);

	if(0 == GetAspeedRegVal(ADDR_GPIO088_UVWX_DATA, &gRegVal_DATA_UVWX))
		printf("UVWX DATA: 0x%08" PRIX64 "\n", gRegVal_DATA_UVWX);
	if(0 == GetAspeedRegVal(ADDR_GPIO08C_UVWX_DIR, &gRegVal_DIR_UVWX))
		printf("UVWX  DIR: 0x%08" PRIX64 "\n", gRegVal_DIR_UVWX);

	if(0 == GetAspeedRegVal(ADDR_GPIO1E0_YZ_DATA, &gRegVal_DATA_YZ))
		printf("YZ   DATA: 0x%08" PRIX64 "\n", gRegVal_DATA_YZ);
	if(0 == GetAspeedRegVal(ADDR_GPIO1E4_YZ_DIR, &gRegVal_DIR_YZ))
		printf("YZ    DIR: 0x%08" PRIX64 "\n", gRegVal_DIR_YZ);

	if(0 == GetAspeedRegVal(ADDR_GPIO800_ABCD_DATA, &gRegVal_DATA_18ABCD))
		printf("18ABCD DATA: 0x%08" PRIX64 "\n", gRegVal_DATA_18ABCD);
	if(0 == GetAspeedRegVal(ADDR_GPIO804_ABCD_DIR, &gRegVal_DIR_18ABCD))
		printf("18ABCD DIR: 0x%08" PRIX64 "\n", gRegVal_DIR_18ABCD);

	if(0 == GetAspeedRegVal(ADDR_GPIO800_E_DATA, &gRegVal_DATA_18E))
		printf("18E   DATA: 0x%08" PRIX64 "\n", gRegVal_DATA_18E);
	if(0 == GetAspeedRegVal(ADDR_GPIO804_E_DIR, &gRegVal_DIR_18E))
		printf("18E    DIR: 0x%08" PRIX64 "\n", gRegVal_DIR_18E);
	printf("*****GPIO-REG*****\n");

	//ABCD
	for(i = 0; i < 32; i++){
		ch = 0x41 + i / 8;
		printf("GPIO%c%d:   %s   %s\n", ch
										 , i % 8
										 , CheckBit(gRegVal_DATA_ABCD, i) ? "H" : "L"
										 , CheckBit(gRegVal_DIR_ABCD, i) ? "OUTPUT" : " INPUT");
	}

	//EFGH
	for(i = 0; i < 32; i++){
		ch = 0x45 + i / 8;
		printf("GPIO%c%d:   %s   %s\n", ch
										 , i % 8
										 , CheckBit(gRegVal_DATA_EFGH, i) ? "H" : "L"
										 , CheckBit(gRegVal_DIR_EFGH, i) ? "OUTPUT" : " INPUT");
	}

	//IJKL
	for(i = 0; i < 32; i++){
		ch = 0x49 + i / 8;
		printf("GPIO%c%d:   %s   %s\n", ch
										 , i % 8
										 , CheckBit(gRegVal_DATA_IJKL, i) ? "H" : "L"
										 , CheckBit(gRegVal_DIR_IJKL, i) ? "OUTPUT" : " INPUT");
	}

	//MNOP
	for(i = 0; i < 32; i++){
		ch = 0x4d + i / 8;
		printf("GPIO%c%d:   %s   %s\n", ch
										 , i % 8
										 , CheckBit(gRegVal_DATA_MNOP, i) ? "H" : "L"
										 , CheckBit(gRegVal_DIR_MNOP, i) ? "OUTPUT" : " INPUT");
	}

	//QRST
	for(i = 0; i < 32; i++){
		ch = 0x51 + i / 8;
		printf("GPIO%c%d:   %s   %s\n", ch
										 , i % 8
										 , CheckBit(gRegVal_DATA_QRST, i) ? "H" : "L"
										 , CheckBit(gRegVal_DIR_QRST, i) ? "OUTPUT" : " INPUT");
	}

	//UVWX
	for(i = 0; i < 32; i++){
		ch = 0x55 + i / 8;
		printf("GPIO%c%d:   %s   %s\n", ch
										 , i % 8
										 , CheckBit(gRegVal_DATA_UVWX, i) ? "H" : "L"
										 , CheckBit(gRegVal_DIR_UVWX, i) ? "OUTPUT" : " INPUT");
	}

	//YZ
	for(i = 0; i < 16; i++){
		ch = 0x59 + i / 8;
		printf("GPIO%c%d:   %s   %s\n", ch
										 , i % 8
										 , CheckBit(gRegVal_DATA_YZ, i) ? "H" : "L"
										 , CheckBit(gRegVal_DIR_YZ, i) ? "OUTPUT" : " INPUT");
	}

	//ABCD_2
	for(i = 0; i < 32; i++){
		ch = 0x41 + i / 8;
		printf("GPIO18%c%d:  %s   %s\n", ch
										 , i % 8
										 , CheckBit(gRegVal_DATA_18ABCD, i) ? "H" : "L"
										 , CheckBit(gRegVal_DIR_18ABCD, i) ? "OUTPUT" : " INPUT");
	}

	//E_2
	for(i = 0; i < 8; i++){
		ch = 0x45 + i / 8;
		printf("GPIO18%c%d:  %s   %s\n", ch
										 , i % 8
										 , CheckBit(gRegVal_DATA_18E, i) ? "H" : "L"
										 , CheckBit(gRegVal_DIR_18E, i) ? "OUTPUT" : " INPUT");
	}
}


int main(int argc, char **argv)
{
	int ret, opt;

	while ((opt = getopt_long(argc, argv,  sopts, lopts, NULL)) != EOF) {
		switch (opt) {
		case 'h':
			print_usage(argv[0]);
			break;
		case 'v':
			fprintf(stderr, "%s - %s\n", argv[0], VERSION);
			break;
		case 'm':
			listMulitFunPin();
			break;
		case 'g':
			listGpioPin();
		default:
			break;
		}
	}

	return 0;
}