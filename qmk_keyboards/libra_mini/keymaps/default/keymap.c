/* SPDX-License-Identifier: GPL-2.0-or-later */

#include QMK_KEYBOARD_H

#define MO2_SPC LT(2, KC_SPC)
#define SFT_SPC RSFT_T(KC_SPC)

// tap dance
// [single tap, single hold, multi tap, tap hold, tapping term]
const tap_dance_entry_t PROGMEM tap_dance_predefined_entries[NUM_TAP_DANCE_PREDEFINED_ENTRIES] = {
  // TD(0) - Apple Fn key + IME switch
  {KC_LNG2, APPLE_FF, KC_LNG1, APPLE_FF, TAPPING_TERM},
  // TD(1) - Protect layer 4 from misstouch, MENU + MO(3)
  {KC_APP, KC_APP, KC_APP, MO(4), TAPPING_TERM},
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  // mac base layer
  [0] = LAYOUT(
    KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,        KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSLS, KC_BSPC,
    KC_LCTL, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,        KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_ENT,
    KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,        KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
    TD(0),   KC_LGUI,                   KC_LALT, MO2_SPC,     SFT_SPC, MO(3),                              KC_RGUI, TD(1)
  ),

  // standard dase layer
  [1] = LAYOUT(
    KC_TAB,  KC_Q,    KC_W,    KC_E,    KC_R,    KC_T,        KC_Y,    KC_U,    KC_I,    KC_O,    KC_P,    KC_BSLS, KC_BSPC,
    KC_LCTL, KC_A,    KC_S,    KC_D,    KC_F,    KC_G,        KC_H,    KC_J,    KC_K,    KC_L,    KC_SCLN, KC_ENT,
    KC_LSFT, KC_Z,    KC_X,    KC_C,    KC_V,    KC_B,        KC_B,    KC_N,    KC_M,    KC_COMM, KC_DOT,  KC_SLSH, KC_RSFT,
    TD(0),   KC_LGUI,                   KC_LALT, MO2_SPC,     SFT_SPC, MO(3),                              KC_RGUI, TD(1)
   ),

  // 40% supplement layer
  [2] = LAYOUT(
    KC_GRV,  KC_1,    KC_2,    KC_3,    KC_4,    KC_5,        KC_6,    KC_7,    KC_8,    KC_9,    KC_0,    KC_MINS, KC_EQL,
    KC_TILD, KC_EXLM, KC_AT,   KC_HASH, KC_DLR,  KC_PERC,     KC_CIRC, KC_AMPR, KC_ASTR, KC_LPRN, KC_RPRN, KC_QUOT,
    _______, _______, _______, _______, _______, _______,     _______, _______, _______, KC_LBRC, KC_RBRC, _______, _______,
    _______, _______,                   _______, _______,     _______, _______,                            _______, _______
  ),

  // HHKB-like fn layer
  [3] = LAYOUT(
    KC_CAPS, KC_F1,   KC_F2,   KC_F3,   KC_F4,   _______,    _______, KC_PSCR, KC_SCRL, KC_PAUS, KC_UP,   KC_INS,  KC_DEL,
    _______, KC_F5,   KC_F6,   KC_F7,   KC_F8,   KC_PAST,    KC_PSLS, KC_HOME, KC_PGUP, KC_LEFT, KC_RGHT, KC_PENT,
    _______, KC_F9,   KC_F10,  KC_F11,  KC_F12,  _______,    KC_PPLS, KC_PMNS, KC_END,  KC_PGDN, KC_DOWN, _______, _______,
    _______, _______,                   _______, _______,    _______, _______,                            _______, _______
  ),

  // settings, amethyst
  // row 2: normal settings, +shift key for unusual reversed setting
  // +Shift key for RGB decrement functions
  [4] = LAYOUT(
    QK_BOOT,  KC_F16,  KC_F17,  KC_F18,  _______, _______,    _______, _______, KC_F13,  KC_F14,  KC_F15,  TERM_LCK,KC_ESC,
    _______, _______,  _______, _______, _______, _______,    _______, _______, _______, _______, _______, _______,
    _______, RHID_OFF, MAC_ON,  USJ_OFF, NK_ON,   CL_NORM,    AG_NORM, BS_NORM, _______, _______, _______, _______, _______,
    _______, AG_TOGG,                    _______, _______,    _______, _______,                            _______, _______
  )
};
// clang-format on
