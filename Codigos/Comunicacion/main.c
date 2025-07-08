
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "mpu.h"
#include "lcd.h"

#define ENA 16
#define PWM_WRAP 250
#define TOLERANCIA 30.0
#define FWD_GPIO    15
#define REV_GPIO    13


static inline void vfd_stop(void) {
    gpio_put(FWD_GPIO, false);
    gpio_put(REV_GPIO, false);
    sleep_ms(1);
}


static inline void vfd_set_direction(bool direction) {
    gpio_put(REV_GPIO, direction);
    sleep_ms(1);
}

static inline void vfd_forward(void) {
    gpio_put(REV_GPIO, false);
    gpio_put(FWD_GPIO, true);
    sleep_ms(1);
}

static inline void vfd_reverse(void) {
    gpio_put(FWD_GPIO, false);
    gpio_put(REV_GPIO, true);
    sleep_ms(1);
}

static inline void pwm_set_duty_percent(uint gpio, uint percent) {
    pwm_set_gpio_level(gpio, (uint16_t) (percent * PWM_WRAP / 100));
}

void configurar_pwm(uint gpio) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    uint slice = pwm_gpio_to_slice_num(gpio);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_wrap(&config, PWM_WRAP);
    pwm_config_set_clkdiv(&config, 10);
    pwm_init(slice, &config, true);
    pwm_set_duty_percent(gpio, 30);
  
}

int main() {
    stdio_init_all();
    gpio_init(FWD_GPIO);
    gpio_init(REV_GPIO);
    gpio_set_dir(FWD_GPIO, true);
    gpio_set_dir(REV_GPIO, true);
    configurar_pwm(ENA);
    mpu6050_init();

    lcd_init(i2c0, 0x27);
    lcd_clear();

    char buffer[64];
    int index = 0;
    int pitch_deseado = 0;
    pitch_deseado = (int) leer_pitch() ;

    adc_init();
    adc_gpio_init(27);
    adc_select_input(1);

    lcd_clear();
    lcd_string("Actual: ");
    lcd_set_cursor(1, 0);
    lcd_string("Deseado: ");

    while (true) {
        int adc_val = adc_read();
        double pitch_deseado = ((adc_val *360.0) / 4095.0) - 180;
        double pitch_actual = leer_pitch();
        sprintf(buffer, "%.2f", pitch_actual);
        lcd_set_cursor(0, 8);
        lcd_string(buffer);
        sprintf(buffer, "%5.1f", pitch_deseado);
        lcd_set_cursor(1, 9);
        lcd_string(buffer);

        if (pitch_actual < pitch_deseado - TOLERANCIA) {
            vfd_reverse();
            sleep_ms(1);        
        } else if (pitch_actual > pitch_deseado + TOLERANCIA) {
            vfd_forward();
             sleep_ms(1);
        } 
        else  {
            vfd_stop();
             sleep_ms(1);
        }

        sleep_ms(200);
    }

    return 0;
}

