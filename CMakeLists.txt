zephyr_include_directories(include)
zephyr_library()

if(CONFIG_SHIELD_DONGLE_SCREEN)

        add_subdirectory(${ZEPHYR_CURRENT_MODULE_DIR}/drivers/display)

endif()

# Caps-word-with-state behavior. Compiled into the ZMK app for EVERY role that
# parses the (shared) keymap — central, peripheral, settings_reset — so a build
# that sees &caps_ind always has the driver. Peripherals don't use it; cost is
# negligible. Pattern from dhruvinsh/zmk-tri-state (minus the role gate).
target_sources_ifdef(CONFIG_ZMK_BEHAVIOR_CAPS_WORD_IND app PRIVATE
        ${ZEPHYR_CURRENT_MODULE_DIR}/behaviors/behavior_caps_word_ind.c)