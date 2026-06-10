#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>
#include "caps_word_status.h"
#include "caps_word_ind.h"
#include <fonts.h> // NerdFont declarations (NerdFonts_Regular_40)

#include <zmk/hid_indicators.h> // host LED state (caps-lock) on the central

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// One glyph for both states (U+F0636, the shift icon the mod widget uses).
// Using a single glyph keeps the height constant, so neither state can overrun
// into the WPM widget above (the dedicated caps-lock glyph U+F0632 was 29px vs
// this glyph's 20px and collided). Caps-lock is distinguished by a thin box
// drawn on the LABEL (which wraps the glyph tightly) — not the 48px container —
// so the box stays within the glyph's footprint and shifts nothing.
#define CAPS_GLYPH "󰘶"

// HID keyboard LED report: Num=0x01, Caps=0x02, Scroll=0x04 (USB HID spec).
// Same constant the zmk-dongle-display hid_indicators widget uses (LED_CLCK).
#define LED_CAPS_LOCK 0x02

static bool caps_lock_active(void)
{
    return (zmk_hid_indicators_get_current_profile() & LED_CAPS_LOCK) != 0;
}

// caps-lock -> glyph + thin box; caps-word -> glyph, no box; else blank.
// Caps-lock takes priority if both are somehow active.
static void update_caps_word_status(struct zmk_widget_caps_word_status *widget)
{
    bool lock = caps_lock_active();
    bool show = lock || caps_word_ind_is_active();

    lv_label_set_text(widget->label, show ? CAPS_GLYPH : "");
    // Box only for caps-lock: flip the label's 1px border on. With 0 pad (set in
    // init) it draws flush to the glyph, adding no height -> can't reach WPM.
    lv_obj_set_style_border_width(widget->label, lock ? 1 : 0, 0);
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
    // Box style for caps-lock, pre-set on the label. ZERO padding so the 1px
    // border draws flush to the glyph's own bounding box and adds no vertical
    // space beyond the glyph (which is the same height as the caps-word glyph).
    // This is what keeps caps-lock from extending up into the WPM widget. Width
    // 0 here; update_caps_word_status() flips it to 1 for caps-lock only.
    lv_obj_set_style_border_color(widget->label, lv_color_white(), 0);
    lv_obj_set_style_radius(widget->label, 2, 0);
    lv_obj_set_style_pad_all(widget->label, 0, 0);
    lv_obj_set_style_border_width(widget->label, 0, 0);

    k_timer_init(&caps_word_status_timer, caps_word_status_timer_cb, NULL);
    k_timer_user_data_set(&caps_word_status_timer, widget);
    k_timer_start(&caps_word_status_timer, K_MSEC(100), K_MSEC(100));

    return 0;
}

lv_obj_t *zmk_widget_caps_word_status_obj(struct zmk_widget_caps_word_status *widget)
{
    return widget->obj;
}
