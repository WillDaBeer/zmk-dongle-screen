#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>
#include "caps_word_status.h"
#include "caps_word_ind.h"
#include <fonts.h> // NerdFont declarations (NerdFonts_Regular_40)

#include <zmk/hid_indicators.h> // host LED state (caps-lock) on the central

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// Two distinct NerdFont glyphs (both in the bundled NerdFonts_Regular_40 subset):
//   caps-word -> shift glyph     U+F0636  (box_h=20, ofs_y=4)
//   caps-lock -> caps-lock glyph U+F0632  (box_h=29, ofs_y=0 — 9px taller, and
//                its top sits 4px higher than the shift glyph)
#define CAPS_WORD_GLYPH "󰘶"
#define CAPS_LOCK_GLYPH "󰘲"

// The caps-lock glyph is taller and its top sits higher, so left centered it
// overruns UP into the WPM widget. Instead, shift it DOWN when caps-lock is on
// so its arrow TIP aligns with the shift glyph's tip and the extra height hangs
// downward into open space. Both glyphs are centered in the 48px box by default;
// nudging the caps-lock label down by ~ (height diff)/2 + (ofs_y diff) lines the
// tops up. 6px aligns the tips on-device (7 was 1px too low).
#define CAPS_LOCK_Y_NUDGE 6

// HID keyboard LED report: Num=0x01, Caps=0x02, Scroll=0x04 (USB HID spec).
// Same constant the zmk-dongle-display hid_indicators widget uses (LED_CLCK).
#define LED_CAPS_LOCK 0x02

static bool caps_lock_active(void)
{
    return (zmk_hid_indicators_get_current_profile() & LED_CAPS_LOCK) != 0;
}

// caps-lock -> caps-lock glyph nudged down (tips align); caps-word -> shift
// glyph centered; else blank. Caps-lock wins if both are somehow active.
static void update_caps_word_status(struct zmk_widget_caps_word_status *widget)
{
    bool lock = caps_lock_active();
    bool show = lock || caps_word_ind_is_active();

    lv_label_set_text(widget->label, lock ? CAPS_LOCK_GLYPH
                                   : show ? CAPS_WORD_GLYPH
                                   : "");
    // Push the taller caps-lock glyph down so its tip aligns with the shift
    // glyph's tip and its extra height hangs below (away from WPM).
    lv_obj_align(widget->label, LV_ALIGN_CENTER, 0, lock ? CAPS_LOCK_Y_NUDGE : 0);
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
    // Container stays transparent/borderless — the box (for caps-lock) is drawn
    // on the label instead, so it hugs the glyph and never grows this 48px box.
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(widget->obj, 0, 0);
    lv_obj_set_style_pad_all(widget->obj, 0, 0);

    widget->label = lv_label_create(widget->obj);
    lv_obj_align(widget->label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(widget->label, "");
    lv_obj_set_style_text_font(widget->label, &NerdFonts_Regular_40, 0);
    lv_obj_set_style_text_color(widget->label, lv_color_white(), 0);
    // No box — caps-lock is distinguished by its own (taller) glyph, nudged
    // down so it grows away from WPM (see update_caps_word_status()).

    k_timer_init(&caps_word_status_timer, caps_word_status_timer_cb, NULL);
    k_timer_user_data_set(&caps_word_status_timer, widget);
    k_timer_start(&caps_word_status_timer, K_MSEC(100), K_MSEC(100));

    return 0;
}

lv_obj_t *zmk_widget_caps_word_status_obj(struct zmk_widget_caps_word_status *widget)
{
    return widget->obj;
}
