/* Copyright 2021 @ Keychron (https://www.keychron.com)
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

static uint32_t light_timeout = 300000; // 5 minutes
static uint32_t key_timer; // timer for the key
void refresh_rgb(void); // refreshes the activity timer and RGB, invoke whenever activity happens
void check_rgb_timeout(void); // checks if enough time has passed for RGB to timeout
void rgb_timeout_lights(void); // turns rgb lights based on timout
static bool is_rgb_timeout = false; // store if RGB has timed out or not in a boolean
static bool light_timer = true; // store if light timer is enabled or not in a boolean


enum custom_keycodes {
   RGB_SWITCH = SAFE_RANGE, 
   LOCKPC,
   RGB_PLUS,
   RGB_MINUS,
   RIGHT_CLK
};

enum layers{
    MAC_BASE,
    MAC_FN,
    WIN_BASE,
    WIN_FN,
    _RGB
};

typedef union {
  uint32_t raw;
  struct {
    bool     light_timer_config :1;
    uint8_t  rgb_timeout_config :5;
  };
} user_config_t;

user_config_t user_config;

#define KC_TASK LGUI(KC_TAB)
#define KC_FLXP LGUI(KC_E)

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
    [MAC_BASE] = LAYOUT_ansi_82(
     //┌────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┐
        KC_ESC,   KC_BRID,  KC_BRIU,  KC_NO,    KC_NO,    RGB_VAD,  RGB_VAI,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  KC_DEL,           LT(MAC_FN, KC_MPLY),
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,            KC_HOME,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,            KC_END,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,             KC_INSERT,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        KC_LSFT,            KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,            KC_RSFT,  KC_UP,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        KC_LCTL,  KC_LOPT,  KC_LCMD,                                KC_SPC,                                 KC_RCMD,MO(MAC_FN), KC_RCTL,  KC_LEFT,  KC_DOWN,  KC_RGHT),
     //└────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┘

    [MAC_FN] = LAYOUT_ansi_82(
     //┌────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┐
      RGB_SWITCH,  KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   QK_CLEAR_EEPROM,    XXXXXXX,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        QK_MAKE,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______, RGB_MINUS,RGB_PLUS,  _______,            KC_PGDN,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  TO(_RGB),           KC_PGUP,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  LOCKPC,  _______,  _______,             _______,            _______,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        _______,            _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,  _______,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,  _______,  _______,  _______),
     //└────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┘

    [WIN_BASE] = LAYOUT_ansi_82(
     //┌────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┐
        KC_ESC,   KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,    KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,   KC_F11,   KC_F12,   KC_DEL,           LT(WIN_FN, KC_MPLY),
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        KC_GRV,   KC_1,     KC_2,     KC_3,     KC_4,     KC_5,     KC_6,     KC_7,     KC_8,     KC_9,     KC_0,     KC_MINS,  KC_EQL,   KC_BSPC,            KC_HOME,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        KC_TAB,   KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,     KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,     KC_LBRC,  KC_RBRC,  KC_BSLS,            KC_END,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        KC_CAPS,  KC_A,     KC_S,     KC_D,     KC_F,     KC_G,     KC_H,     KC_J,     KC_K,     KC_L,     KC_SCLN,  KC_QUOT,            KC_ENT,             KC_INSERT,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        KC_LSFT,            KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,     KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,            KC_RSFT,  KC_UP,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        KC_LCTL,  KC_LCMD,  KC_LALT,                                KC_SPC,                                 KC_RALT,MO(WIN_FN), KC_RCTL,  KC_LEFT,  KC_DOWN,  KC_RGHT),
     //└────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┘

    [WIN_FN] = LAYOUT_ansi_82(
     //┌────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┐
      RGB_SWITCH, KC_BRID,  KC_BRIU,  KC_TASK,  KC_FLXP,  RGB_VAD,  RGB_VAI,  KC_MPRV,  KC_MPLY,  KC_MNXT,  KC_MUTE,  KC_VOLD,  KC_VOLU,  QK_CLEAR_EEPROM,    XXXXXXX,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        QK_MAKE,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______, RGB_MINUS,RGB_PLUS,  _______,            KC_PGDN,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______, TO(_RGB),            KC_PGUP,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        _______,  _______, _______,   _______,  _______,  _______,  _______,  _______,  _______,  LOCKPC,  _______,  _______,            _______,             _______,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        _______,            _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,  _______,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,  _______,  _______,  _______),
     //└────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┘

     [_RGB] = LAYOUT_ansi_82(
     //┌────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┐
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            XXXXXXX,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        RGB_TOG,  RGB_MOD,  RGB_VAI,  RGB_HUI,  RGB_SAI,  RGB_SPI,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  TO(0),              _______,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        _______,  RGB_RMOD, RGB_VAD,  RGB_HUD,  RGB_SAD,  RGB_SPD,  _______,  _______,  _______,  _______,  _______,  _______,            _______,            _______,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        _______,            _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,  _______,            _______,  _______,
     //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
        _______,  _______,  _______,                                _______,                                _______,  _______,  _______,  _______,  _______,  _______),
     //└────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┘
};


/* RGB Change on Layer 
*/
uint8_t current;
uint8_t prev;
uint16_t mode;
uint8_t hue;
uint8_t sat;
uint8_t val;
uint8_t speed;

void get_rgb(void) {
	hue = rgblight_get_hue();
	sat = rgblight_get_sat();
    val = rgblight_get_val();
    speed = rgblight_get_speed();
    mode = rgblight_get_mode();
}

void reset_rgb(void) {
	rgblight_sethsv(hue, sat, val);
   rgblight_set_speed(speed);
   rgblight_mode(mode);
}

void matrix_init_user() {
	get_rgb(); // * init rgb from eeprom to current session 
   rgblight_enable(); // * Will enable rgb on startup
}

void set_rgb(uint8_t h, uint8_t s, uint8_t v, uint8_t e, uint16_t m) {
    rgblight_set_speed_noeeprom(e);
    rgblight_mode_noeeprom(m);
    if(val < 150) {
        rgblight_sethsv_noeeprom(h, s, 150);
    } else {
        rgblight_sethsv_noeeprom(h, s, v);
    }
}

void refresh_rgb() {
    //printf("RGB keytimer %d / %ld\n", timer_elapsed(key_timer), light_timeout);
    //printf("is_rgb_timeout=%d : light_timer=%d\n------------------\n", is_rgb_timeout, light_timer);
    key_timer = timer_read(); // store time of last refresh
    if (is_rgb_timeout && light_timer) { // only do something if rgb has timed out
        rgblight_enable();
        reset_rgb();
        //printf("RGB Enabled\n");
        is_rgb_timeout = false;

        if(biton32(layer_state) == WIN_BASE || biton32(layer_state) == MAC_BASE) {
            reset_rgb();
            //printf("RGB Reset\n");
        }
    }
}

void check_rgb_timeout() {
  if (!is_rgb_timeout && light_timer && timer_elapsed(key_timer) >= light_timeout) {
   rgblight_disable_noeeprom();
   is_rgb_timeout = true;
  }
}

void housekeeping_task_user(void) {
   check_rgb_timeout();
}

uint32_t min_to_milli(uint16_t minutes) {
   return minutes * 60000;
}

uint8_t milli_to_min(uint32_t milli) {
   return milli / 60000;
}

/* Load eeprom
 * Loads light time out state from eeprom 
 */
void keyboard_post_init_user(void) {
    rgblight_enable();
    // Read the user config from EEPROM
    user_config.raw = eeconfig_read_user();
    // Update light_timer to eeprom
    // If light_timer eeprom is not set
    if(user_config.rgb_timeout_config == 0) {
        user_config.rgb_timeout_config = milli_to_min(light_timeout);
        user_config.light_timer_config = true;
        eeconfig_update_user(user_config.raw);
    }
    light_timer = user_config.light_timer_config;
    light_timeout = min_to_milli(user_config.rgb_timeout_config);
    get_rgb(); 
}

layer_state_t layer_state_set_user(layer_state_t state) {
    if(prev == _RGB) {
        get_rgb();
    }
    current = biton32(state);
    switch (current) {
        case WIN_BASE:
        case MAC_BASE:
            // ! To prevent reset before layers are initalized, load rgb from eeprom will not work otherwise
            if(prev == WIN_BASE || prev == MAC_BASE || prev == WIN_FN || prev == MAC_FN || prev == _RGB) {
                reset_rgb();
            }
            break;
        case WIN_FN:
        case MAC_FN:
            rgb_timeout_lights();
            break;
        case _RGB:
            reset_rgb();
            break;
    }
   prev = current;
   return state;
}

//switch through rgb timouts
void increase_rgb_timeout(void) {
    uint8_t timeMinute = milli_to_min(light_timeout);
    switch (timeMinute)
    {
    case 1:
        light_timeout = min_to_milli(5);
        break;
    case 5:
        light_timeout = min_to_milli(10);
        break;
    case 10:
        light_timeout = min_to_milli(15);
        break;
    case 15:
        light_timeout = min_to_milli(15);
        break;
    }
    if(timeMinute != milli_to_min(light_timeout)) {
        //printf("RGB Timout Increased from: %d to %d \n------------------\n", timeMinute, milli_to_min(light_timeout));
        user_config.rgb_timeout_config = milli_to_min(light_timeout);
        eeconfig_update_user(user_config.raw);
    } else {
        //printf("RGB Timout not changed: %d \n------------------\n", timeMinute);
    }
}

void decrease_rgb_timeout(void) {
    uint8_t timeMinute = milli_to_min(light_timeout);
    switch (timeMinute)
    {
    case 15:
        light_timeout = min_to_milli(10);
        break;
    case 10:
        light_timeout = min_to_milli(5);
        break;
    case 5:
        light_timeout = min_to_milli(1);
        break;
    case 1:
        light_timeout = min_to_milli(1);
        break;
    }
    if(timeMinute != milli_to_min(light_timeout)) {
        //printf("RGB Timout Decreased from: %d to %d \n------------------\n", timeMinute, milli_to_min(light_timeout));
        user_config.rgb_timeout_config = milli_to_min(light_timeout);
        eeconfig_update_user(user_config.raw);
    } else {
        //printf("RGB Timout not changed: %d \n------------------\n", timeMinute);
    }
}

void rgb_timeout_lights(void) {
    reset_rgb();
    uint8_t timeMinute = milli_to_min(light_timeout);
    switch (timeMinute) {
        case 1:
            if(light_timer) {
                rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_GREEN_ONE);
            } else {
                rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_RED_ONE);
            }
            break;
        case 5:
            if(light_timer) {
                rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_GREEN_FIVE);
            } else {
                rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_RED_FIVE);
            }
            break;
        case 10:
            if(light_timer) {
                rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_GREEN_TEN);
            } else {
                rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_RED_TEN);
            }
            break;
        case 15:
            if(light_timer) {
                rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_GREEN_FIFTEEN);
            } else {
                rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_RED_FIFTEEN);
            }
            break;
    }
}

bool encoder_update_user(uint8_t index, bool clockwise) {
    if(clockwise || !clockwise) refresh_rgb();
    switch(biton32(layer_state)) {
        case WIN_BASE:
        case MAC_BASE:
            clockwise ? tap_code(KC_VOLU) : tap_code(KC_VOLD);
            return false;
            break;
        case WIN_FN:
        case MAC_FN:
            clockwise ? tap_code(KC_MNXT) : tap_code(KC_MPRV);
            return false;
            break;
        default:
            return true;
            break;
    }
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) refresh_rgb();
    switch (keycode) {
        case RGB_SWITCH:
            if (record->event.pressed) {
                if(light_timer) {
                    light_timer = false;
                    user_config.light_timer_config = false;
                    eeconfig_update_user(user_config.raw); // * Saves light time out config to eeprom
                } else {
                    light_timer = true;
                    user_config.light_timer_config = true;
                    eeconfig_update_user(user_config.raw); // * Saves light time out config to eeprom
                }
                rgb_timeout_lights();          
            }
            return true;
            break;
        case LOCKPC:
            if (record->event.pressed) {
                register_code(KC_LGUI);
                register_code(KC_L);
            } else {
                unregister_code(KC_LGUI);
                unregister_code(KC_L);
                rgblight_disable_noeeprom();
                is_rgb_timeout = true;
            }
            return true;
            break;
        case RGB_PLUS:
    	    if (record->event.pressed) {
                increase_rgb_timeout();
                rgb_timeout_lights();
            }
            return true;
            break;
        case RGB_MINUS:
    	    if (record->event.pressed) {
                decrease_rgb_timeout();
                rgb_timeout_lights();
            }
            return true;
            break;
        default:
            return true;
            break;
    }

}

// Will trigger Caps lock RGB, only on base layer
void led_set_user(uint8_t usb_led) {
   if ((current == WIN_BASE || current == MAC_BASE) && (usb_led & (1<<USB_LED_CAPS_LOCK))) {
      rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_BREATHING_RED); 
   } else if(current == WIN_BASE || current == MAC_BASE) { // * Cap lock indicator will only show on base layer
      reset_rgb();
   }
}