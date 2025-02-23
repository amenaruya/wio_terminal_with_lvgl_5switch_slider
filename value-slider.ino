#include <lvgl.h>
#include <TFT_eSPI.h>

/* tick period */
constexpr uint16_t      TICK_PERIOD = 5;

/* TFT LCD */
static TFT_eSPI         TFT;
/* display buffer */
static lv_disp_buf_t    DISP_BUF;
/* color buffer */
static lv_color_t       COLOR_BUF[LV_HOR_RES_MAX * 10];

/* input device */
static lv_indev_t*      INPUT_DEVICE;
/* input deviceの影響を被るグループ */
static lv_group_t*      INPUT_GROUP;
/* slider */
static lv_obj_t*        SLIDER;
/* input deviceの値を示すラベル */
static lv_obj_t*        LABEL_SLIDERS_VALUE;

/* 画面初期化 */
void display_flush(
    lv_disp_drv_t*      disp,
    const lv_area_t*    area,
    lv_color_t*         color_p
);

/* Reading input device (嘘) */
bool read_encoder(
    lv_indev_drv_t*     indev,
    lv_indev_data_t*    data
);

/* system tick */
static void tick_handler();

/* 設定 */
void backend_setup();

/* slider event callback */
static void slider_event_cb(
    lv_obj_t*           SLIDER,
    lv_event_t          event
);

/* 画面要素 */
void set_contents();

/* 5way switch */
void wio_joy_handler();

void set_contents() {
    /* sliderの設定 */
    SLIDER = lv_slider_create(lv_scr_act(), NULL);
    /* 幅 */
    lv_obj_set_width(SLIDER, LV_DPI * 2);
    /* 画面中央に配置する */
    lv_obj_align(SLIDER, NULL, LV_ALIGN_CENTER, 0, 0);
    /* callbackを設定する */
    lv_obj_set_event_cb(SLIDER, slider_event_cb);
    /* 値の取る範囲 */
    lv_slider_set_range(SLIDER, 0, 100);
    /* アニメーションの時間 */
    lv_slider_set_anim_time(SLIDER, 75);

    /* slider値を示すlabelの設定 */
    LABEL_SLIDERS_VALUE = lv_label_create(lv_scr_act(), NULL);
    /* 初期表示 */
    lv_label_set_text(LABEL_SLIDERS_VALUE, "0");
    /* 自動整理 */
    lv_obj_set_auto_realign(LABEL_SLIDERS_VALUE, true);
    /* sliderを基準とし、下部中央に配置する */
    lv_obj_align(LABEL_SLIDERS_VALUE, SLIDER, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    /* 説明labelの設定 */
    /* 1行目 */
    lv_obj_t* info1 = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_text(
        info1,
        "This is the joy switch + SLIDER demo."
    );
    lv_obj_align(info1, NULL, LV_ALIGN_IN_TOP_MID, 0, 10);
    /* 2行目 */
    lv_obj_t* info2 = lv_label_create(lv_scr_act(), NULL);
    /* スクロール */
    lv_label_set_long_mode(info2, LV_LABEL_LONG_SROLL_CIRC);
    /* 幅 */
    lv_obj_set_width(info2, LV_HOR_RES_MAX);
    /* スクロール設定後に文章を定める */
    lv_label_set_text(
        info2,
        "Move the switch right/up, SLIDER moves to right."
    );
    lv_obj_align(info2, info1, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    /* 3行目 */
    lv_obj_t* info3 = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_long_mode(info3, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_width(info3, LV_HOR_RES_MAX);
    lv_label_set_text(
        info3,
        "Move the switch left/down, SLIDER moves to left."
    );
    lv_obj_align(info3, info2, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
    /* 4行目 */
    lv_obj_t* info4 = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_long_mode(info4, LV_LABEL_LONG_SROLL_CIRC);
    lv_obj_set_width(info4, LV_HOR_RES_MAX);
    lv_label_set_text(
        info4,
        "Press the switch, SLIDER moves to the left end."
    );
    lv_obj_align(info4, info3, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);

    /* groupの設定 */
    INPUT_GROUP = lv_group_create();
    /* input deviceを設定する */
    lv_indev_set_group(INPUT_DEVICE, INPUT_GROUP);
    /* sliderを追加する */
    lv_group_add_obj(INPUT_GROUP, SLIDER);
}

void wio_joy_handler() {
    /*
    ↑: LV_KEY_UP
    ↓: LV_KEY_DOWN
    ←: LV_KEY_LEFT
    →: LV_KEY_RIGHT

    UP or RIGHT:  increment
    DOWN or LEFT: decrement

    PRESS: 0
    */
    if (digitalRead(WIO_5S_UP) == LOW) {
        /* groupに入力LV_KEY_UPを送る */
        lv_group_send_data(INPUT_GROUP, LV_KEY_UP);
    }
    else if (digitalRead(WIO_5S_DOWN) == LOW) {
        /* groupに入力LV_KEY_DOWNを送る */
        lv_group_send_data(INPUT_GROUP, LV_KEY_DOWN);
    }
    else if (digitalRead(WIO_5S_LEFT) == LOW) {
        /* groupに入力LV_KEY_LEFTを送る */
        lv_group_send_data(INPUT_GROUP, LV_KEY_LEFT);
    }
    else if (digitalRead(WIO_5S_RIGHT) == LOW) {
        /* groupに入力LV_KEY_RIGHTを送る */
        lv_group_send_data(INPUT_GROUP, LV_KEY_RIGHT);
    }
    else if (digitalRead(WIO_5S_PRESS) == LOW) {
        /* sliderの値を0にする */
        lv_slider_set_value(SLIDER, 0, LV_ANIM_ON);
    }
    else {
        /* 何もない時は何もしない */
        return;
    }
    /* イベントは勝手に起こらないため、ここで起こす */
    lv_event_send(SLIDER, LV_EVENT_VALUE_CHANGED, NULL);
}

void setup() {
    /* 設定 */
    backend_setup();

    /* 画面構成 */
    set_contents();
}

void loop() {
    tick_handler();
    lv_task_handler();
    wio_joy_handler();
    delay(TICK_PERIOD);
}
