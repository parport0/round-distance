#include "LVGL_example.h" 
#include "src/core/lv_obj.h"
#include "src/misc/lv_area.h"

static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf0[DISP_HOR_RES * DISP_VER_RES/2];
static lv_color_t buf1[DISP_HOR_RES * DISP_VER_RES/2];
static lv_disp_drv_t disp_drv;

static lv_indev_drv_t indev_ts;
static lv_group_t *group;

static lv_obj_t *meter;
static lv_meter_indicator_t *needle;

static uint16_t ts_x;
static uint16_t ts_y;
static lv_indev_state_t ts_act;

// Timer 
static struct repeating_timer lvgl_timer;
static struct repeating_timer sensor_read_timer;

static void disp_flush_cb(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p);
static void touch_callback(uint gpio, uint32_t events);
static void ts_read_cb(lv_indev_drv_t * drv, lv_indev_data_t*data);
static void encoder_read_cb(lv_indev_drv_t * drv, lv_indev_data_t*data);
static void dma_handler(void);
static bool repeating_lvgl_timer_callback(struct repeating_timer *t); 
static bool repeating_sensor_read_timer_callback(struct repeating_timer *t);

void LVGL_Init(void)
{
    add_repeating_timer_ms(50, repeating_sensor_read_timer_callback, NULL, &sensor_read_timer);
    add_repeating_timer_ms(5,  repeating_lvgl_timer_callback,        NULL, &lvgl_timer);
    
    lv_init();

    lv_disp_draw_buf_init(&disp_buf, buf0, buf1, DISP_HOR_RES * DISP_VER_RES / 2); 
    lv_disp_drv_init(&disp_drv);    
    disp_drv.flush_cb = disp_flush_cb;
    disp_drv.draw_buf = &disp_buf;        
    disp_drv.hor_res = DISP_HOR_RES;
    disp_drv.ver_res = DISP_VER_RES;
    lv_disp_t *disp= lv_disp_drv_register(&disp_drv);   

#if INPUTDEV_TS
    lv_indev_drv_init(&indev_ts); 
    indev_ts.type = LV_INDEV_TYPE_POINTER;    
    indev_ts.read_cb = ts_read_cb;            
    lv_indev_t * ts_indev = lv_indev_drv_register(&indev_ts);
    //Enable touch IRQ
    DEV_IRQ_SET(Touch_INT_PIN, GPIO_IRQ_EDGE_RISE, &touch_callback);
#endif
    dma_channel_set_irq0_enabled(dma_tx, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_handler);
    irq_set_enabled(DMA_IRQ_0, true);

}

void Widgets_Init(void)
{
    meter = lv_meter_create(lv_scr_act());
    lv_obj_center(meter);
    lv_obj_set_size(meter, 240, 240);

    /*Add a scale first*/
    lv_meter_scale_t * scale = lv_meter_add_scale(meter);
    lv_meter_set_scale_ticks(meter, scale, 37, 2, 20, lv_palette_main(LV_PALETTE_GREY));
    lv_meter_set_scale_major_ticks(meter, scale, 5, 5, 20, lv_color_black(), 20);
    lv_meter_set_scale_range(meter, scale, 0, 360, 315, 90);

    lv_meter_indicator_t * indic;

    /*Add a green arc to the start*/
    indic = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_meter_set_indicator_start_value(meter, indic, 0);
    lv_meter_set_indicator_end_value(meter, indic, 20);

    /*Make the tick lines green at the start of the scale*/
    indic = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_GREEN), lv_palette_main(LV_PALETTE_GREEN), false, 0);
    lv_meter_set_indicator_start_value(meter, indic, 0);
    lv_meter_set_indicator_end_value(meter, indic, 40);

    /*Add a red arc to the end*/
    indic = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_RED), 0);
    lv_meter_set_indicator_start_value(meter, indic, 250);
    lv_meter_set_indicator_end_value(meter, indic, 360);

    /*Make the tick lines red at the end of the scale*/
    indic = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_RED), lv_palette_main(LV_PALETTE_RED), false, 0);
    lv_meter_set_indicator_start_value(meter, indic, 250);
    lv_meter_set_indicator_end_value(meter, indic, 360);

    /*Add a needle line indicator*/
    needle = lv_meter_add_needle_line(meter, scale, 4, lv_palette_main(LV_PALETTE_GREY), -40);
}

static void disp_flush_cb(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p)
{

    LCD_1IN28_SetWindows(area->x1, area->y1, area->x2 , area->y2);
    dma_channel_configure(dma_tx,
                          &c,
                          &spi_get_hw(LCD_SPI_PORT)->dr, 
                          color_p, // read address
                          ((area->x2 + 1 - area-> x1)*(area->y2 + 1 - area -> y1))*2,
                          true);
}

static void touch_callback(uint gpio, uint32_t events)
{
    if (gpio == Touch_INT_PIN)
    {
        CST816S_Get_Point();
        ts_x = Touch_CTS816.x_point;
        ts_y = Touch_CTS816.y_point;
        ts_act = LV_INDEV_STATE_PRESSED;
    }
}

static void ts_read_cb(lv_indev_drv_t * drv, lv_indev_data_t*data)
{
    data->point.x = ts_x;
    data->point.y = ts_y; 
    data->state = ts_act;
    ts_act = LV_INDEV_STATE_RELEASED;
}

static void dma_handler(void)
{
    if (dma_channel_get_irq0_status(dma_tx)) {
        dma_channel_acknowledge_irq0(dma_tx);
        lv_disp_flush_ready(&disp_drv);         /* Indicate you are ready with the flushing*/
    }
}

static bool repeating_lvgl_timer_callback(struct repeating_timer *t) 
{
    lv_tick_inc(5);
    return true;
}

static bool repeating_sensor_read_timer_callback(struct repeating_timer *t) 
{
    uint32_t result;
    if (multicore_fifo_pop_timeout_us(5, &result)) {
        lv_meter_set_indicator_value(meter, needle, result);
    }
    return true;
}
