
#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include "pico/stdlib.h"
#include "inc/ssd1306.h"
#include <string.h>
#include "hardware/i2c.h"

#define I2C_SDA 14
#define I2C_SCL 15

void RenderizaTextoDisplay(char *text1, char *text2, char *text3);
void ConfiguraDisplay();

#endif // DISPLAY_H
