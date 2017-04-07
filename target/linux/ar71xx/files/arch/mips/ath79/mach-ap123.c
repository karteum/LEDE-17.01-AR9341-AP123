/*
 *  Headele HDR1003GWO+ (AP123 / AR9341)
 *
 *  Copyright (C) 2015 Adrien Demarez <adrien.demarez@bolloretelecom.eu>
 *  Copyright (C) 2015 Jeremy Lainé <jeremy.laine@spacinov.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */

#include <linux/gpio.h>
#include <linux/platform_device.h>

#include <asm/mach-ath79/ath79.h>
#include <asm/mach-ath79/ar71xx_regs.h>

#include "common.h"
#include "dev-eth.h"
#include "dev-gpio-buttons.h"
#include "dev-leds-gpio.h"
#include "dev-m25p80.h"
#include "dev-usb.h"
#include "dev-wmac.h"
#include "machtypes.h"

#define HDR1003GWO_KEYS_POLL_INTERVAL 20 /* msecs */
#define HDR1003GWO_NVRAM_ADDR 0x1f7f0000 /* #define HDR1003GWO_NVRAM_SIZE 0x10000 */
#define HDR1003GWO_GPIO_USB_POWER 11
#define HDR1003GWO_MACADDRESS_OFFSET (HDR1003GWO_NVRAM_ADDR + 6)

static const char *hdr1003gwo_part_probes[] = { "cmdlinepart", NULL, };

static struct flash_platform_data hdr1003gwo_flash_data = {
	.part_probes = hdr1003gwo_part_probes,
	.name = "ath-nor0",
};

static struct gpio_led hdr1003gwo_leds_gpio[] __initdata = {
	{ .gpio = 12, .active_low = 1, .name = "ap123:red:wan", },
	{ .gpio = 13, .active_low = 1, .name = "ap123:blue:wan", },
	{ .gpio = 14, .active_low = 1, .name = "ap123:green:wan", },
	{ .gpio = 18, .active_low = 1, .name = "ap123:green:lan", }
};

static struct gpio_keys_button hdr1003gwo_gpio_keys[] __initdata = {
	{
		.desc = "Reset button",
		.type = EV_SW,
		.code = KEY_RESTART,
		.debounce_interval = (3 * HDR1003GWO_KEYS_POLL_INTERVAL),
		.gpio = 16,
		.active_low = 1,
	}
};

static void __init hdr1003gwo_init(void)
{
	/* Disable JTAG, enabling GPIOs 0-3. Configure OBS4 line, for GPIO 4 */
	ath79_gpio_function_setup(AR934X_GPIO_FUNC_JTAG_DISABLE, AR934X_GPIO_FUNC_CLK_OBS4_EN);

	/* Bugfix for Headele GPIO-to-RESET pin */
	ath79_gpio_output_select(HDR1003GWO_GPIO_USB_POWER, AR934X_GPIO_OUT_GPIO);
	gpio_request_one(HDR1003GWO_GPIO_USB_POWER, GPIOF_OUT_INIT_LOW | GPIOF_EXPORT_DIR_FIXED, "USB power");

	ath79_register_leds_gpio(-1, ARRAY_SIZE(hdr1003gwo_leds_gpio), hdr1003gwo_leds_gpio);
	ath79_register_gpio_keys_polled(1, HDR1003GWO_KEYS_POLL_INTERVAL, ARRAY_SIZE(hdr1003gwo_gpio_keys), hdr1003gwo_gpio_keys);

	/* Register NOR flash */
	ath79_register_m25p80(&hdr1003gwo_flash_data);

	/* GMAC0 is connected to the PHY0 of the internal switch. */
	ath79_setup_ar934x_eth_cfg(AR934X_ETH_CFG_SW_PHY_SWAP);
	ath79_register_mdio(1, 0x0);
	ath79_init_mac(ath79_eth0_data.mac_addr, (u8 *) KSEG1ADDR(HDR1003GWO_MACADDRESS_OFFSET), 0);
	ath79_eth0_data.phy_if_mode = PHY_INTERFACE_MODE_MII;
	ath79_eth0_data.phy_mask = BIT(0);
	ath79_eth0_data.mii_bus_dev = &ath79_mdio1_device.dev;
	ath79_register_eth(0);

	ath79_register_usb();
}

MIPS_MACHINE(ATH79_MACH_AP123, "AP123", "Headele HDR1003GWO_AP123", hdr1003gwo_init);
