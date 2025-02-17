

#include "display.h"

//funcao que renderiza o texto no display oled
void RenderizaTextoDisplay(char *text1, char *text2, char *text3) {
    struct render_area frame_area = {0, ssd1306_width - 1, 0, ssd1306_n_pages - 1};
    calculate_render_area_buffer_length(&frame_area);

    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    ssd1306_draw_string(ssd, 0, 20, text1);
    ssd1306_draw_string(ssd, 0, 35, text2);
    ssd1306_draw_string(ssd, 6, 40, text3);

    render_on_display(ssd, &frame_area);
}

//funcao que seta as configurações iniciais do display oled
void ConfiguraDisplay() {
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init();
}
