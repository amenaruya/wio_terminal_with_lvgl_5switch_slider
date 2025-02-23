/*
画面初期化
画面いっぱい色で塗り潰している
*/
void display_flush(
    lv_disp_drv_t*      disp,
    const lv_area_t*    area,
    lv_color_t*         color_p
) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    TFT.startWrite();
    TFT.setAddrWindow(area->x1, area->y1, w, h);
    TFT.pushColors(&color_p->full, w * h, true);
    TFT.endWrite();

    /* callbackではこれを召喚しなければならない */
    lv_disp_flush_ready(disp);
}

/* Reading input device (嘘) */
bool read_encoder(
    lv_indev_drv_t*     indev,
    lv_indev_data_t*    data
) {
    /* no input */
    return false;
}

/* system tick */
void tick_handler() {
    lv_tick_inc(TICK_PERIOD);
}

/* slider event callback */
static void slider_event_cb(lv_obj_t* SLIDER, lv_event_t event) {
    if (event == LV_EVENT_VALUE_CHANGED) {
        static char buf[4]; /* max 3 bytes for number plus 1 null terminating byte */
        snprintf(buf, 4, "%u", lv_slider_get_value(SLIDER));
        lv_label_set_text(LABEL_SLIDERS_VALUE, buf);
    }
}

/* 設定 */
void backend_setup() {
    /* lvglの初期化 */
    lv_init();

    /* TFTの初期設定 */
    TFT.begin();
    TFT.setRotation(3);

    /* 画面表示用bufferそのものの初期化 */
    lv_disp_buf_init(&DISP_BUF, COLOR_BUF, NULL, LV_HOR_RES_MAX * 10);

    /* 画面制御設定 */
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res    = LV_HOR_RES_MAX;
    disp_drv.ver_res    = LV_VER_RES_MAX;
    disp_drv.flush_cb   = display_flush;
    disp_drv.buffer     = &DISP_BUF;
    lv_disp_t* pDisplay = lv_disp_drv_register(&disp_drv);

    /* 入力の設定 */
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type      = LV_INDEV_TYPE_ENCODER;
    indev_drv.read_cb   = read_encoder;
    INPUT_DEVICE = lv_indev_drv_register(&indev_drv);

    /* enable 5way switch */
    pinMode(WIO_5S_UP, INPUT);
    pinMode(WIO_5S_DOWN, INPUT);
    pinMode(WIO_5S_LEFT, INPUT);
    pinMode(WIO_5S_RIGHT, INPUT);
    pinMode(WIO_5S_PRESS, INPUT);
}
