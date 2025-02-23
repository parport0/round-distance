#include "LCD_test.h"

void core1_entry() {
    while (1) {
        uint32_t result = 0;
        uint8_t ch[4];
        for (int i = 0; i < 4; i++) {
                ch[i] = uart_getc(uart0);
        }
        if (ch[0] != 0xff) {
            // Failure -- shift the window
            uart_getc(uart0);
            continue;
        } else {
            if ((ch[0] + ch[1] + ch[2]) & 0x00FF != ch[3]) {
                // Failure -- shift the window
                uart_getc(uart0);
                continue;
            } else {
                result = (ch[1] << 8) + ch[2];
                multicore_fifo_push_blocking(result);
            }
        }
    }
}

int LCD_1in28_LVGL_Test(void)
{
    if (DEV_Module_Init() != 0)
    {
        return -1;
    } 
    
    uart_init(uart0, 9600);
    gpio_set_function(17, UART_FUNCSEL_NUM(UART_ID, 17));
    multicore_launch_core1(core1_entry);

    LCD_1IN28_Init(HORIZONTAL);
    LCD_1IN28_Clear(WHITE);
    DEV_SET_PWM(100);
    CST816S_init(CST816S_Point_Mode);

    LVGL_Init();
    Widgets_Init();
    
    while(1)
    {
      lv_task_handler();
      DEV_Delay_ms(5); 
    }

    DEV_Module_Exit();
    return 0;
}
