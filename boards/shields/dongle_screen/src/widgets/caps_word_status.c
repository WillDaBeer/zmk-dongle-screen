#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>
#include "caps_word_status.h"
#include "caps_word_ind.h"
#include <fonts.h> // NerdFont declarations (NerdFonts_Regular_40)

#include <zmk/hid_indicators.h> // host LED state (caps-lock) on the central

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// Shift glyph (U+F0636) — the same icon the modifier widget uses for Shift,
// so caps-word reads as the shift-lock state it is, in the mod widget's visual
// language. Confirmed present in the bundled NerdFonts_Regular_40.
#define CAPS_WORD_GLYPH "󰘶"

// HID keyboard LED report: Num=0x01, Caps=0x02, Scroll=0x04 (USB HID spec).
// Same constant the zmk-dongle-display hid_indicators widget uses (LED_CLCK).
#define LED_CAPS_LOCK 0x02

static bool caps_lock_active(void)
{
    return (zmk_hid_indicators_get_current_profile() & LED_CAPS_LOCK) != 0;
}

// Show the shift glyph when EITHER caps-word or caps-lock is active. Caps-lock
// is distinguished by boxing the glyph (background + border on the container);
// caps-word shows the plain glyph. Both off -> blank.
static void update_caps_word_status(struct zmk_widget_caps_word_status *widget)
{
    bool word = caps_word_ind_is_active();
    bool lock = caps_lock_active();

    lv_label_set_text(widget->label, (word || lock) ? CAPS_WORD_GLYPH : "");

    // Box the container only for caps-lock.
    lv_opa_t bg = lock ? LV_OPA_COVER : LV_OPA_TRANSP;
    lv_coord_t border = lock ? 2 : 0;
    lv_obj_set_style_bg_opa(widget->obj, bg, 0);
    lv_obj_set_style_border_width(widget->obj, border, 0);
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
    // Default state: transparent, borderless, no padding — just the glyph.
    // update_caps_word_status() flips bg_opa + border_width on for caps-lock to
    // draw a box around the glyph; the border colour + radius are set here so
    // the box looks intentional when it appears.
    lv_obj_set_style_bg_opa(widget->obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_bg_color(widget->obj, lv_color_black(), 0);
    lv_obj_set_style_border_width(widget->obj, 0, 0);
    lv_obj_set_style_border_color(widget->obj, lv_color_white(), 0);
    lv_obj_set_style_radius(widget->obj, 6, 0);
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
