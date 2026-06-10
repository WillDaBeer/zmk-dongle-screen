#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zmk/hid.h>
#include <lvgl.h>
#include "mod_status.h"
#include <fonts.h> // <-- Wichtig für LV_FONT_DECLARE

#include "caps_word_ind.h"      // caps-word state (caps_word_ind_is_active)
#include <zmk/hid_indicators.h> // host caps-lock LED state on the central

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// Caps glyph (U+F0632, apple-keyboard-caps), appended to the modifier row in a
// recolor span: green = caps-word, white = caps-lock (caps-lock wins if both),
// hidden when off. Using one recolored label (not a second positioned label)
// keeps the whole thing in one centered object — so it centers when caps is the
// only thing shown, flows after the mods when they are held, and avoids the
// per-tick lv_obj_align_to() that crashed the previous approach.
#define CAPS_GLYPH "󰘲"
#define CAPS_WORD_COLOR "00ff00" // green
#define CAPS_LOCK_COLOR "ffffff" // white
#define LED_CAPS_LOCK 0x02       // HID keyboard LED report bit (Caps=0x02)

static bool caps_lock_active(void)
{
    return (zmk_hid_indicators_get_current_profile() & LED_CAPS_LOCK) != 0;
}

static void update_mod_status(struct zmk_widget_mod_status *widget)
{
    uint8_t mods = zmk_hid_get_keyboard_report()->body.modifiers;
    char text[64] = "";
    int idx = 0;

    // Order: cmd, opt, ctrl, shift.
    char *syms[4];
    int n = 0;

    if (mods & (MOD_LGUI | MOD_RGUI))
    // GUI/cmd icon per CONFIG_DONGLE_SCREEN_SYSTEM_ICON (0=mac,1=linux,2=win)
#if CONFIG_DONGLE_SCREEN_SYSTEM_ICON == 1
        syms[n++] = "󰌽"; // U+F033D
#elif CONFIG_DONGLE_SCREEN_SYSTEM_ICON == 2
        syms[n++] = ""; // U+E62A
#else
        syms[n++] = "󰘳"; // U+F0633 (cmd)
#endif
    if (mods & (MOD_LALT | MOD_RALT))
        syms[n++] = "󰘵"; // U+F0635 (opt/alt)
    if (mods & (MOD_LCTL | MOD_RCTL))
        syms[n++] = "󰘴"; // U+F0634 (ctrl)
    if (mods & (MOD_LSFT | MOD_RSFT))
        syms[n++] = "󰘶"; // U+F0636 (shift)

    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
            idx += snprintf(&text[idx], sizeof(text) - idx, " ");
        idx += snprintf(&text[idx], sizeof(text) - idx, "%s", syms[i]);
    }

#if CONFIG_DONGLE_SCREEN_CAPSWORD_ACTIVE
    // Append the caps glyph (caps-lock wins) as a recolor span, after a space if
    // any modifiers are already shown.
    bool lock = caps_lock_active();
    bool word = caps_word_ind_is_active();
    if (lock || word)
    {
        if (idx > 0)
            idx += snprintf(&text[idx], sizeof(text) - idx, " ");
        idx += snprintf(&text[idx], sizeof(text) - idx, "#%s %s#",
                        lock ? CAPS_LOCK_COLOR : CAPS_WORD_COLOR, CAPS_GLYPH);
    }
#endif

    lv_label_set_text(widget->label, idx ? text : "");
}

static void mod_status_timer_cb(struct k_timer *timer)
{
    struct zmk_widget_mod_status *widget = k_timer_user_data_get(timer);
    update_mod_status(widget);
}

static struct k_timer mod_status_timer;

int zmk_widget_mod_status_init(struct zmk_widget_mod_status *widget, lv_obj_t *parent)
{
    widget->obj = lv_obj_create(parent);
    lv_obj_set_size(widget->obj, 180, 40);

    widget->label = lv_label_create(widget->obj);
    lv_obj_align(widget->label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_recolor(widget->label, true); // enable #RRGGBB ...# spans
    lv_label_set_text(widget->label, "");
    lv_obj_set_style_text_font(widget->label, &NerdFonts_Regular_40, 0); // <-- NerdFont setzen

    k_timer_init(&mod_status_timer, mod_status_timer_cb, NULL);
    k_timer_user_data_set(&mod_status_timer, widget);
    k_timer_start(&mod_status_timer, K_MSEC(100), K_MSEC(100));

    return 0;
}

lv_obj_t *zmk_widget_mod_status_obj(struct zmk_widget_mod_status *widget)
{
    return widget->obj;
}
