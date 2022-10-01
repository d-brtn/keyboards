/* Copyright 2022 masafumi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include QMK_KEYBOARD_H

#include "custom_config.h"

#include "eeprom.h"
#include "radial_controller.h"

typedef union {
  uint32_t raw;
  struct {
    bool mac : 1;  // mac mode.
    bool usj : 1;  // ANSI layou on JIS.
  };
} kb_config_t;

typedef union {
  uint32_t raw;
  struct {
    rc_dial_mode_t dial_mode : 1;  // mode 0:ENCODER, 1:KEYSWITCH
    uint16_t deg_per_click : 9;    // encoder mode: degree per key press or encoder click.
    uint16_t deg_per_sec : 9;      // keyswitch mode: degree per second
  };
} rc_config_t;

static kb_config_t kb_config;
static rc_config_t rc_config;

void custom_config_reset() {
  kb_config.raw = 0;
  kb_config.mac = true;
  eeconfig_update_kb(kb_config.raw);
  rc_config.raw = 0;
  rc_config.dial_mode = RADIAL_CONTROLLER_DIAL_MODE_DEFAULT;
  rc_config.deg_per_click = RADIAL_CONTROLLER_DEGREE_PER_CLICK_DEFAULT;
  rc_config.deg_per_sec = RADIAL_CONTROLLER_DEGREE_PER_SEC_DEFAULT;
  eeprom_update_dword((uint32_t *)RADIAL_CONTROLLER_EEPROM_ADDR, rc_config.raw);
}

void custom_config_init() {
  kb_config.raw = eeconfig_read_kb();
  rc_config.raw = eeprom_read_dword((uint32_t *)RADIAL_CONTROLLER_EEPROM_ADDR);
}

bool custom_config_is_mac() { return kb_config.mac; }

void custom_config_toggle_mac() { custom_config_set_mac(!kb_config.mac); }

void custom_config_set_mac(bool mac) {
  if (mac != kb_config.mac) {
    kb_config.mac = mac;
    eeconfig_update_kb(kb_config.raw);
    // reboot for changing USB device descriptor
    soft_reset_keyboard();
  }
}

bool custom_config_is_usj() { return kb_config.usj; }

void custom_config_toggle_usj() { custom_config_set_usj(!kb_config.usj); }

void custom_config_set_usj(bool usj) {
  if (usj != kb_config.usj) {
    kb_config.usj = usj;
    eeconfig_update_kb(kb_config.raw);
  }
}

rc_dial_mode_t custom_config_get_rc_dial_mode() { return rc_config.dial_mode; }

void custom_config_set_rc_dial_mode(rc_dial_mode_t dial_mode) {
  if (rc_config.dial_mode != dial_mode) {
    rc_config.dial_mode = dial_mode;
    eeprom_update_dword((uint32_t *)RADIAL_CONTROLLER_EEPROM_ADDR, rc_config.raw);
  }
}

uint16_t custom_config_get_rc_deg_per_click(void) {
  if (rc_config.deg_per_click > 0 && rc_config.deg_per_click <= 360) {
    return rc_config.deg_per_click;
  }
  return RADIAL_CONTROLLER_DEGREE_PER_CLICK_DEFAULT;
}

void custom_config_set_rc_deg_per_click(uint16_t deg_per_click) {
  if (rc_config.deg_per_click != deg_per_click) {
    rc_config.deg_per_click =
        deg_per_click > 0 && deg_per_click <= 360 ? deg_per_click : RADIAL_CONTROLLER_DEGREE_PER_CLICK_DEFAULT;
    eeprom_update_dword((uint32_t *)RADIAL_CONTROLLER_EEPROM_ADDR, rc_config.raw);
  }
}

uint16_t custom_config_get_rc_deg_per_sec(void) {
  if (rc_config.deg_per_sec > 0 && rc_config.deg_per_sec <= 360) {
    return rc_config.deg_per_sec;
  }
  return RADIAL_CONTROLLER_DEGREE_PER_CLICK_DEFAULT;
}

void custom_config_set_rc_deg_per_sec(uint16_t deg_per_sec) {
  if (rc_config.deg_per_sec != deg_per_sec) {
    rc_config.deg_per_sec =
        deg_per_sec > 0 && deg_per_sec <= 360 ? deg_per_sec : RADIAL_CONTROLLER_DEGREE_PER_SEC_DEFAULT;
    eeprom_update_dword((uint32_t *)RADIAL_CONTROLLER_EEPROM_ADDR, rc_config.raw);
  }
}

#ifndef VIAL_ENABLE
void dynamic_tap_dance_reset(const tap_dance_entry_t *entry, uint8_t len) {
  tap_dance_entry_t empty = {0};
  tap_dance_entry_t data;
  for (int i = 0; i < TAP_DANCE_ENTRIES; i++) {
    pgm_memcpy(&data, &entry[i], 10);
    eeprom_update_block((uint8_t *)(i < len ? &data : &empty), (uint8_t *)(DYNAMIC_TAP_DANCE_EEPROM_ADDR + 10 * i), 10);
  }
}

uint16_t dynamic_tap_dance_keycode(uint16_t index, tap_dance_state_t state) {
  if (index >= 0 && index < TAP_DANCE_ENTRIES) {
    uint16_t *adrs = (uint16_t *)(DYNAMIC_TAP_DANCE_EEPROM_ADDR + 10 * index);
    switch (state) {
      case TD_SINGLE_TAP:
        return eeprom_read_word(adrs);
      case TD_SINGLE_HOLD:
        return eeprom_read_word(adrs + 1);
      case TD_MULTI_TAP:
        return eeprom_read_word(adrs + 2);
      case TD_TAP_HOLD:
        return eeprom_read_word(adrs + 3);
      default:
        return KC_NO;
    }
  }
  return KC_NO;
}

uint16_t dynamic_tap_dance_tapping_term(uint16_t index) {
  if (index >= 0 && index < TAP_DANCE_ENTRIES) {
    return eeprom_read_word((uint16_t *)(DYNAMIC_TAP_DANCE_EEPROM_ADDR + 8 + 10 * index));
  }
  return TAPPING_TERM;
}
#endif

void pgm_memcpy(void *dest, const void *src, size_t len) {
  for (size_t i = 0; i < len; i++) {
    *(uint8_t *)dest++ = pgm_read_byte((uint8_t *)src++);
  }
}