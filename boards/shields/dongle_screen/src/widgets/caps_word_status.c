#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>
#include "caps_word_status.h"
#include "caps_word_ind.h"
#include <fonts.h> // NerdFont declarations (NerdFonts_Regular_40)

#include <zmk/hid_indicators.h> // host LED state (caps-lock) on the central

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// Two distinct NerdFont glyphs, both confirmed present in the bundled
// NerdFonts_Regular_40 subset:
//   caps-word -> shift glyph    U+F0636 (the icon the mod widget uses for Shift)
//   caps-lock -> caps-lock glyph U+F0632 (apple-keyboard-caps)
#define CAPS_WORD_GLYPH "󰘶"
#define CAPS_LOCK_GLYPH "󰘲"

// HID keyboard LED report: Num=0x01, Caps=0x02, Scroll=0x04 (USB HID spec).
// Same constant the zmk-dongle-display hid_indicators widget uses (LED_CLCK).
#define LED_CAPS_LOCK 0x02

static bool caps_lock_active(void)
{
    return (zmk_hid_indicators_get_current_profile() & LED_CAPS_LOCK) != 0;
}

// caps-lock -> caps-lock glyph; else caps-word -> shift glyph; else blank.
// Caps-lock takes priority if both are somehow active. No box.
static void update_caps_word_status(struct zmk_widget_caps_word_status *widget)
{
    const char *glyph = caps_lock_active() ? CAPS_LOCK_GLYPH
                      : caps_word_ind_is_active() ? CAPS_WORD_GLYPH
                      : "";
    lv_label_set_text(widget->label, glyph);
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
    lv_obj_set_size(widget->obj, 48, 48);
    // Transparent, borderless, no padding — just the glyph, no box. (caps-word
    // and caps-lock are now distinguished by different glyphs, not a box.)
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(widget->obj, 0, 0);
    lv_obj_set_style_pad_all(widget->obj, 0, 0);

    widget->label = lv_label_create(widget->obj);
    lv_obj_align(widget->label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(widget->label, "");
    lv_obj_set_style_text_font(widget->label, &NerdFonts_Regular_40, 0);
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
