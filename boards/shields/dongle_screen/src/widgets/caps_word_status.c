#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>
#include "caps_word_status.h"
#include "caps_word_ind.h"

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// The bundled NerdFonts_Regular_40 has no caps glyph (and no ASCII letters),
// so render plain text in lv_font_montserrat_40 (full ASCII, same font the
// layer widget uses). Shown when caps-word is active, blank when inactive.
#define CAPS_WORD_TEXT "CAPS"

static void update_caps_word_status(struct zmk_widget_caps_word_status *widget)
{
    bool active = caps_word_ind_is_active();
    lv_label_set_text(widget->label, active ? CAPS_WORD_TEXT : "");
}

static void caps_word_status_timer_cb(struct k_timer *timer)
{
    struct zmk_widget_caps_word_status *widget = k_timer_user_data_get(timer);
    update_caps_word_status(widget);
}

static struct k_timer caps_word_status_timer;

int zmk_widget_caps_word_status_init(struct zmk_widget_caps_word_status *widget, lv_obj_t *parent)
{
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 60, 40);

    widget->label = lv_label_create(widget->obj);
    lv_obj_align(widget->label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(widget->label, "");
    lv_obj_set_style_text_font(widget->label, &lv_font_montserrat_40, 0);
    lv_obj_set_style_text_color(widget->label, lv_color_white(), 0);

    k_timer_init(&caps_word_status_timer, caps_word_status_timer_cb, NULL);
    k_timer_user_data_set(&caps_word_status_timer, widget);
    k_timer_start(&caps_word_status_timer, K_MSEC(100), K_MSEC(100));

    return 0;
}

lv_obj_t *zmk_widget_caps_word_status_obj(struct zmk_widget_caps_word_status *widget)
{
    return widget->obj;
}
