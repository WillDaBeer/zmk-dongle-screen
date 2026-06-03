#pragma once

#include <lvgl.h>
#include <zmk/display.h>

struct zmk_widget_caps_word_status
{
    sys_snode_t node;
    lv_obj_t *obj;
    lv_obj_t *label;
};

int zmk_widget_caps_word_status_init(struct zmk_widget_caps_word_status *widget, lv_obj_t *parent);
lv_obj_t *zmk_widget_caps_word_status_obj(struct zmk_widget_caps_word_status *widget);
