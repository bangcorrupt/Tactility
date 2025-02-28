#pragma once

#include "driver/spi_common.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "hal/YellowDisplayConstants.h"

// SPI 2 - display
#define TWODOTFOUR_SPI2_PIN_SCLK GPIO_NUM_14
#define TWODOTFOUR_SPI2_PIN_MOSI GPIO_NUM_13
#define TWODOTFOUR_SPI2_TRANSACTION_LIMIT TWODOTFOUR_LCD_DRAW_BUFFER_SIZE

// SPI 3 - sdcard
#define TWODOTFOUR_SPI3_PIN_SCLK GPIO_NUM_18
#define TWODOTFOUR_SPI3_PIN_MOSI GPIO_NUM_23
#define TWODOTFOUR_SPI3_PIN_MISO GPIO_NUM_19
#define TWODOTFOUR_SPI3_TRANSACTION_LIMIT 8192 // TODO: Determine proper limit
