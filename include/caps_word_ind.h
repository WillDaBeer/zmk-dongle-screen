#pragma once

#include <stdbool.h>

/* True while the &caps_ind caps-word behavior is active. Polled by the
 * modifiers (mod_status) dongle-screen widget to colour its caps glyph. */
bool caps_word_ind_is_active(void);
