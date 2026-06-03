#pragma once

#include <stdbool.h>

/* True while the &caps_ind caps-word behavior is active. Polled by the
 * caps_word_status dongle-screen widget. */
bool caps_word_ind_is_active(void);
