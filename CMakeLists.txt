zephyr_include_directories(include)
zephyr_library()

if(CONFIG_SHIELD_DONGLE_SCREEN)

        add_subdirectory(${ZEPHYR_CURRENT_MODULE_DIR}/drivers/display)

endif()

# Caps-word-with-state behavior. Central-only: it subscribes to
# zmk_keycode_state_changed and calls zmk_hid_* — subsystems that only exist on
# the central. Peripherals don't process keymap bindings, so they don't need the
# driver; the &caps_ind devicetree node still parses fine without it.
# (Pattern: dhruvinsh/zmk-tri-state.)
if((NOT CONFIG_ZMK_SPLIT) OR CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
        target_sources_ifdef(CONFIG_ZMK_BEHAVIOR_CAPS_WORD_IND app PRIVATE
                ${ZEPHYR_CURRENT_MODULE_DIR}/behaviors/behavior_caps_word_ind.c)
endif()