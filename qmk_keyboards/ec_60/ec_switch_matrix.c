/* Copyright 2023 Cipulot
 * Modified 2023 masafumi
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

#include "ec_switch_matrix.h"

#include <analog.h>
#include <atomic_util.h>
#include <math.h>
#include <wait.h>

#include "ec_config.h"

// Pin and port array
const uint32_t row_pins[] = MATRIX_ROW_PINS;
const uint32_t amux_sel_pins[] = AMUX_SEL_PINS;
const uint32_t amux_en_pins[] = AMUX_EN_PINS;
const uint8_t amux_n_col_sizes[] = AMUX_COL_CHANNELS_SIZES;
const uint8_t amux_n_col_channels[][AMUX_MAX_COLS_COUNT] = {AMUX_COL_CHANNELS};
#define AMUX_SEL_PINS_COUNT (sizeof(amux_sel_pins) / sizeof(amux_sel_pins[0]))
#define EXPECTED_AMUX_SEL_PINS_COUNT ceil(log2(AMUX_MAX_COLS_COUNT)
// Checks for the correctness of the configuration
_Static_assert(
  (sizeof(amux_en_pins) / sizeof(amux_en_pins[0])) == AMUX_COUNT,
  "AMUX_EN_PINS doesn't have the minimum number of bits required to enable all the multiplexers available");
// Check that number of select pins is enough to select all the channels
_Static_assert(AMUX_SEL_PINS_COUNT == EXPECTED_AMUX_SEL_PINS_COUNT), "AMUX_SEL_PINS doesn't have the minimum number of bits required address all the channels");
// Check that number of elements in AMUX_COL_CHANNELS_SIZES is enough to specify the number of channels for all the
// multiplexers available
_Static_assert((sizeof(amux_n_col_sizes) / sizeof(amux_n_col_sizes[0])) == AMUX_COUNT,
               "AMUX_COL_CHANNELS_SIZES doesn't have the minimum number of elements required to specify the number of "
               "channels for all the multiplexers available");

#ifdef EC_DEBUG
uint16_t sw_value[MATRIX_ROWS][MATRIX_COLS];
#endif
static adc_mux adcMux;

static void ec_bootoming_reading(void);
static void init_row(void);
static void init_amux(void);
static inline void select_amux_channel(uint8_t channel, uint8_t col);
static inline void disable_unused_amux(uint8_t channel);
static inline void discharge_capacitor(void);
static inline void charge_capacitor(uint8_t row);
static uint16_t ec_readkey_raw(uint8_t channel, uint8_t row, uint8_t col);
static inline bool ec_update_key(matrix_row_t* current_row, uint8_t row, uint8_t col, uint16_t sw_value);

#define MATRIX_READ_LOOP(...)                                               \
  uint8_t col = 0;                                                          \
  for (uint8_t amux = 0; amux < AMUX_COUNT; amux++) {                       \
    disable_unused_amux(amux);                                              \
    for (int amux_col = 0; amux_col < amux_n_col_sizes[amux]; amux_col++) { \
      for (int row = 0; row < MATRIX_ROWS; row++) {                         \
        uint16_t value = ec_readkey_raw(amux, row, amux_col);               \
        __VA_ARGS__                                                         \
      }                                                                     \
      col++;                                                                \
    }                                                                       \
  }

// QMK hook functions
// -----------------------------------------------------------------------------------

// Initialize the peripherals pins
void matrix_init_custom(void) {
  // Initialize ADC
  palSetLineMode(ANALOG_PORT, PAL_MODE_INPUT_ANALOG);
  adcMux = pinToMux(ANALOG_PORT);

  // Dummy call to make sure that adcStart() has been called in the appropriate state
  adc_read(adcMux);

  // Initialize discharge pin as discharge mode
  writePinLow(DISCHARGE_PIN);
  setPinOutputOpenDrain(DISCHARGE_PIN);

  // Initialize drive lines
  init_row();

  // Initialize AMUXs
  init_amux();

  ec_calibrate_noise_floor();
}

bool matrix_scan_custom(matrix_row_t current_matrix[]) {
  bool updated = false;

  // Bottoming calibration mode: update bottoming out values and avoid keycode state change
  // IF statement is higher in the function to avoid the overhead of the execution of normal operation mode
  if (ec_config.bottoming_calibration) {
    ec_bootoming_reading();
    // Return false to avoid keycode state change
    return false;
  }

  // Normal operation mode: update key state
  MATRIX_READ_LOOP(
#ifdef EC_DEBUG
    sw_value[row][col] = value;
#endif
    updated |= ec_update_key(&current_matrix[row], row, col, value);)
  return updated;
}

// static routines
// -----------------------------------------------------------------------------------

// Get the noise floor
void ec_calibrate_noise_floor(void) {
  // Initialize the noise floor to 0
  for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
      ec_config.noise_floor[row][col] = 0;
      ec_config.extremum[row][col] = 0;
    }
  }

  // Get the noise floor
  // max: ec_config.noise_floor[row][col]
  // min: ec_config.extremum[row][col]
  for (uint8_t i = 0; i < NOISE_FLOOR_SAMPLING_COUNT; i++) {
    wait_ms(5);
    MATRIX_READ_LOOP(
      // max value
      if (value > ec_config.noise_floor[row][col]) { ec_config.noise_floor[row][col] = value; }
      // min value
      if (ec_config.extremum[row][col] == 0 || value < ec_config.extremum[row][col]) {
        ec_config.extremum[row][col] = value;
      })
  }

  // Average the noise floor
  for (uint8_t row = 0; row < MATRIX_ROWS; row++) {
    for (uint8_t col = 0; col < MATRIX_COLS; col++) {
      ec_config.noise[row][col] = ec_config.noise_floor[row][col] - ec_config.extremum[row][col];
      ec_config.noise_floor[row][col] = (ec_config.noise_floor[row][col] + ec_config.extremum[row][col]) / 2;
      ec_config.extremum[row][col] = ec_config.noise_floor[row][col];
    }
  }
}

static void ec_bootoming_reading(void) {
  MATRIX_READ_LOOP(
#ifdef EC_DEBUG
    sw_value[row][col] = value;
#endif
    if (value > (ec_config.noise_floor[row][col] + 32)) {
      if (ec_config.bottoming_calibration_starter[row][col]) {
        eeprom_ec_config.bottoming_reading[row][col] = value;
        ec_config.bottoming_calibration_starter[row][col] = false;
      } else if (value > eeprom_ec_config.bottoming_reading[row][col]) {
        eeprom_ec_config.bottoming_reading[row][col] = value;
      }
    })
}

// Initialize the row pins
static void init_row(void) {
  // Set all row pins as output and low
  for (uint8_t idx = 0; idx < MATRIX_ROWS; idx++) {
    setPinOutput(row_pins[idx]);
    writePinLow(row_pins[idx]);
  }
}

// Initialize the multiplexers
static void init_amux(void) {
  for (uint8_t idx = 0; idx < AMUX_COUNT; idx++) {
    setPinOutput(amux_en_pins[idx]);
    writePinLow(amux_en_pins[idx]);
  }
  for (uint8_t idx = 0; idx < AMUX_SEL_PINS_COUNT; idx++) {
    setPinOutput(amux_sel_pins[idx]);
  }
}

// Select the multiplexer channel of the specified multiplexer
static inline void select_amux_channel(uint8_t channel, uint8_t col) {
  // Get the channel for the specified multiplexer
  uint8_t ch = amux_n_col_channels[channel][col];
  // momentarily disable specified multiplexer
  writePinHigh(amux_en_pins[channel]);
  // Select the multiplexer channel
  writePin(amux_sel_pins[0], ch & 1);
  writePin(amux_sel_pins[1], ch & 2);
  writePin(amux_sel_pins[2], ch & 4);
  // re enable specified multiplexer
  writePinLow(amux_en_pins[channel]);
}

// Disable all the unused multiplexers
static inline void disable_unused_amux(uint8_t channel) {
  // disable all the other multiplexers apart from the current selected one
  for (uint8_t idx = 0; idx < AMUX_COUNT; idx++) {
    if (idx != channel) {
      writePinHigh(amux_en_pins[idx]);
    }
  }
}
// Discharge the peak hold capacitor
static inline void discharge_capacitor(void) { writePinLow(DISCHARGE_PIN); }

// Charge the peak hold capacitor
static inline void charge_capacitor(uint8_t row) {
  writePinHigh(DISCHARGE_PIN);
  writePinHigh(row_pins[row]);
}

// Read the capacitive sensor value
static uint16_t ec_readkey_raw(uint8_t channel, uint8_t row, uint8_t col) {
  uint16_t sw_value = 0;

  // Select the multiplexer
  select_amux_channel(channel, col);

  // Set the row pin to low state to avoid ghosting
  writePinLow(row_pins[row]);

  ATOMIC_BLOCK_FORCEON {
    // Set the row pin to high state and have capacitor charge
    charge_capacitor(row);
    // Read the ADC value
    sw_value = adc_read(adcMux);
  }
  // Discharge peak hold capacitor
  discharge_capacitor();
  // Waiting for the ghost capacitor to discharge fully
  wait_us(DISCHARGE_TIME);

  return sw_value;
}

// Update press/release state of key
static inline bool ec_update_key(matrix_row_t* current_row, uint8_t row, uint8_t col, uint16_t sw_value) {
  uint16_t extremum = sw_value < ec_config.deadzone[row][col] ? ec_config.deadzone[row][col] : sw_value;
  if ((*current_row >> col) & 1) {
    // key pressed
    switch (ec_config.release[row][col].reference.mode) {
      case EC_RELEASE_MODE_STATIC:
        if (sw_value < ec_config.release[row][col].reference.value) {
          ec_config.extremum[row][col] = extremum;
          *current_row &= ~(1 << col);
          return true;
        }
        break;
      case EC_RELEASE_MODE_DYNAMIC:
        if (sw_value < ec_config.deadzone[row][col] ||
            (sw_value < ec_config.extremum[row][col] &&
             ec_config.extremum[row][col] - sw_value > ec_config.release[row][col].reference.value)) {
          ec_config.extremum[row][col] = extremum;
          *current_row &= ~(1 << col);
          return true;
        }
        break;
    }
    // Is key still moving down?
    if (extremum > ec_config.extremum[row][col]) {
      ec_config.extremum[row][col] = extremum;
    }

  } else {
    // key released

    // 14-15 bit: actuation_mode
    switch (ec_config.actuation[row][col].reference.mode) {
      case EC_ACTUATION_MODE_STATIC:
        if (sw_value > ec_config.actuation[row][col].reference.value) {
          *current_row |= (1 << col);
          ec_config.extremum[row][col] = extremum;
          return true;
        }
        break;
      case EC_ACTUATION_MODE_DYNAMIC:
        if (sw_value > ec_config.extremum[row][col] &&
            sw_value - ec_config.extremum[row][col] > ec_config.actuation[row][col].reference.value) {
          // Has key moved down enough to be pressed?
          ec_config.extremum[row][col] = extremum;
          *current_row |= (1 << col);
          return true;
        }
        break;
    }
    // Is key still moving up
    if (extremum < ec_config.extremum[row][col]) {
      ec_config.extremum[row][col] = extremum;
    }
  }
  return false;
}
