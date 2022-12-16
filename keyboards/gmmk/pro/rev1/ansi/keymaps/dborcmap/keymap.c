/* Copyright 2021 Glorious, LLC <salman@pcgamingrace.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H

static uint32_t light_timeout = 300000; // 5 minutes
static uint32_t key_timer; // timer for the key
static void refresh_rgb(void); // refreshes the activity timer and RGB, invoke whenever activity happens
static void check_rgb_timeout(void); // checks if enough time has passed for RGB to timeout
static bool is_rgb_timeout = false; // store if RGB has timed out or not in a boolean
static bool light_timer = true; // store if light timer is enabled or not in a boolean


enum custom_keycodes {
   RGB_SWITCH = SAFE_RANGE, 
   LOCKPC,
   RIGHT_CLK,
   RGB_one_min, 
   RGB_five_min, 
   RGB_ten_min, 
   RGB_fifteen_min
};

enum layer_names {
    _BASE, _FUNC, _RGB, _XL
};

typedef union {
  uint32_t raw;
  struct {
    bool     light_timer_config :1;
    uint8_t  rgb_timeout_config :5;
  };
} user_config_t;

user_config_t user_config;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

//      ESC      F1       F2       F3       F4       F5       F6       F7       F8       F9       F10      F11      F12	     Del       Volume.up/dn(Tap-Pause, Hold-prev/next)
//      ~        1        2        3        4        5        6        7        8        9        0         -       (=)	     BackSpc           Home
//      Tab      Q        W        E        R        T        Y        U        I        O        P        [        ]        \                 End
//      Caps     A        S        D        F        G        H        J        K        L        ;        "                 Enter             PgUp
//      Sh_L              Z        X        C        V        B        N        M        ,        .        ?                 Sh_R     Up       PgDn
//      Ct_L     Win_L    Alt_L                               SPACE                               Alt_R    FN       Ct_R     Left     Down     Right


    [_BASE] = LAYOUT(
      //┌────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┐
         KC_ESC  , KC_F1   , KC_F2   , KC_F3   , KC_F4   , KC_F5   , KC_F6   , KC_F7   , KC_F8   , KC_F9   , KC_F10  , KC_F11  , KC_F12  ,KC_DELETE,LT(_FUNC, KC_MPLY),
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         KC_GRV  , KC_1    , KC_2    , KC_3    , KC_4    , KC_5    , KC_6    , KC_7    , KC_8    , KC_9    , KC_0    , KC_MINS , KC_EQL  , KC_BSPC ,           KC_HOME ,
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         KC_TAB  , KC_Q    , KC_W    , KC_E    , KC_R    , KC_T    , KC_Y    , KC_U    , KC_I    , KC_O    , KC_P    , KC_LBRC , KC_RBRC , KC_BSLS ,           KC_END  ,
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         KC_CAPS , KC_A    , KC_S    , KC_D    , KC_F    , KC_G    , KC_H    , KC_J    , KC_K    , KC_L    , KC_SCLN , KC_QUOT , KC_ENT  ,                     KC_PGUP ,
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         KC_LSFT , KC_Z    , KC_X    , KC_C    , KC_V    , KC_B    , KC_N    , KC_M    , KC_COMM , KC_DOT  , KC_SLSH , KC_RSFT ,                     KC_UP   , KC_PGDN ,
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         KC_LCTL , KC_LGUI , KC_LALT,                           KC_SPC,                            KC_RALT ,TT(_FUNC), KC_RCTL ,           KC_LEFT , KC_DOWN , KC_RGHT
      //└────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┘
    ),
    [_FUNC] = LAYOUT(
      //┌────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┐
       RGB_SWITCH, TO(_XL) , KC_WSCH , KC_CALC , KC_MSEL , KC_MPRV , KC_MNXT , KC_MPLY , KC_MSTP , KC_VOLD , KC_VOLU , KC_PSCR , KC_SLCK , KC_INS ,            TO(_RGB),
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         EE_CLR  , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______,    RGB_fifteen_min ,
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         _______ , _______ , _______ , _______ , NK_TOGG , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ ,TO(_BASE),       RGB_ten_min ,
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , LOCKPC  , _______ , _______ , _______ ,                RGB_five_min ,
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         _______ , _______ , _______ , _______ , _______ , _______ , KC_NUM  , _______ , _______ , _______ , _______ , _______ ,                     _______ , RGB_one_min ,
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         _______ , _______ , _______ ,                          _______,                           _______ , XXXXXXX , _______ ,           _______ , _______ , _______ 
      //└────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┘
    ),
    [_RGB] = LAYOUT(
      //┌────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┐
         RGB_TOG , RGB_MOD , RGB_RMOD, RGB_HUI , RGB_HUD , RGB_SAI , RGB_SAD , RGB_SPI , RGB_SPD , _______ , _______ , _______ , _______ ,RGB_SWITCH,          TO(_BASE),
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ ,TO(_BASE),           _______ ,
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ ,           _______ ,
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ ,                     _______ ,
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ , _______ ,                     _______ , _______ ,
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         _______ , _______ , _______ ,                          _______,                           _______ , TO(_BASE), _______ ,           _______ , _______ , _______ 
      //└────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┘
    ),
    [_XL] = LAYOUT(
      //┌────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┐
         KC_ESC  ,RIGHT_CLK, KC_F2  , KC_F3   , KC_F4   , KC_F5   , KC_F6   , KC_F7   , KC_F8   , KC_F9   , KC_F10  , KC_F11  , KC_F12  ,KC_DELETE,LT(_BASE, KC_MPLY),
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         KC_GRV  , KC_1    , KC_2    , KC_3    , KC_4    , KC_5    , KC_6    , KC_7    , KC_8    , KC_9    , KC_0    , KC_MINS , KC_EQL  , KC_BSPC ,           KC_HOME ,
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         KC_TAB  , KC_Q    , KC_W    , KC_E    , KC_R    , KC_T    , KC_Y    , KC_U    , KC_I    , KC_O    , KC_P    , KC_LBRC , KC_RBRC , KC_BSLS ,           KC_END  ,
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         KC_CAPS , KC_A    , KC_S    , KC_D    , KC_F    , KC_G    , KC_H    , KC_J    , KC_K    , KC_L    , KC_SCLN , KC_QUOT , KC_ENT  ,                     KC_PGUP ,
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         KC_LSFT , KC_Z    , KC_X    , KC_C    , KC_V    , KC_B    , KC_N    , KC_M    , KC_COMM , KC_DOT  , KC_SLSH , KC_RSFT ,                     KC_UP   , KC_PGDN ,
      //├────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤
         KC_LCTL , KC_LGUI , KC_LALT,                           KC_SPC,                            KC_RALT ,TO(_FUNC), KC_RCTL ,           KC_LEFT , KC_DOWN , KC_RGHT
      //└────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┘
    )
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

layer_state_t layer_state_set_user(layer_state_t state) {
   if(prev == _RGB) {
      get_rgb();
   }
   current = biton32(state);
	switch (current) {
		case _BASE:
      case _XL:
         // ! To prevent reset before layers are initalized, load rgb from eeprom will not work otherwise
		   if(prev == _FUNC || prev == _RGB || prev == _XL) {
            reset_rgb();
         }
		   break;
		case _FUNC:
         if(light_timer) {
            rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_GREEN);
         } else {
            rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_RED);
         }
         break;
      case _RGB:
         reset_rgb();
         break;
      }
   prev = current;
   return state;
}


void refresh_rgb() {
  key_timer = timer_read(); // store time of last refresh
  if (is_rgb_timeout && light_timer) { // only do something if rgb has timed out
    is_rgb_timeout = false;
    rgblight_enable();

    if(biton32(layer_state) == _BASE) {
      reset_rgb();
    }
  }
}

void check_rgb_timeout() {
  if (!is_rgb_timeout && light_timer && timer_elapsed(key_timer) >= light_timeout) {
   rgblight_disable_noeeprom();
   is_rgb_timeout = true;
  }
}


/* Runs at the end of each scan loop, check if RGB timeout has occured */
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
  light_timer = user_config.light_timer_config;
  if(user_config.rgb_timeout_config == 0) {
      user_config.rgb_timeout_config = milli_to_min(light_timeout);
      eeconfig_update_user(user_config.raw);
  }
  light_timeout = min_to_milli(user_config.rgb_timeout_config);
   get_rgb();
}


/* Encoder 
*/
bool encoder_update_user(uint8_t index, bool clockwise) {
   if(clockwise || !clockwise) refresh_rgb();
   switch(biton32(layer_state)) {
      case _BASE:
         clockwise ? tap_code(KC_VOLU) : tap_code(KC_VOLD);
      break;
      case _FUNC:
         clockwise ? tap_code(KC_MNXT) : tap_code(KC_MPRV);
         break;
      case _RGB:
         clockwise ? rgblight_increase_val() : rgblight_decrease_val();
         break;
      default:
         break;
   }
   return false;
}

/* Macros 
 * RGB_SWITCH will turn on and off the 5min light time out 
*/
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
   if (record->event.pressed) refresh_rgb();
   switch (keycode) {
      case RGB_SWITCH:
         if (record->event.pressed) {
            if(light_timer) {
               light_timer = false;
               rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_RED);
               user_config.light_timer_config = false;
               eeconfig_update_user(user_config.raw); // * Saves light time out config to eeprom
            } else {
               light_timer = true;
               rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_GREEN);             
               user_config.light_timer_config = true;
               eeconfig_update_user(user_config.raw); // * Saves light time out config to eeprom
            }
         }
         return true;
      break;
      case RIGHT_CLK:
         if (record->event.pressed) {
            register_code(KC_LEFT_SHIFT);
            register_code(KC_F10);
         } else {
            unregister_code(KC_LEFT_SHIFT);
            unregister_code(KC_F10);
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
      case RGB_one_min:
         if (record->event.pressed) {
            light_timeout = 60000;
            user_config.rgb_timeout_config = milli_to_min(60000);
            eeconfig_update_user(user_config.raw); // * Saves light time out config to eeprom
         } 
         return true;
      break;
      case RGB_five_min:
         if (record->event.pressed) {
            light_timeout = 300000;
            user_config.rgb_timeout_config = milli_to_min(300000);
            eeconfig_update_user(user_config.raw); // * Saves light time out config to eeprom
         } 
         return true;
      break;
      case RGB_ten_min:
         if (record->event.pressed) {
            light_timeout = 600000;
            user_config.rgb_timeout_config = milli_to_min(600000);
            eeconfig_update_user(user_config.raw); // * Saves light time out config to eeprom
         } 
         return true;
      break;
      case RGB_fifteen_min:
         if (record->event.pressed) {
            light_timeout = 900000;
            user_config.rgb_timeout_config = milli_to_min(900000);
            eeconfig_update_user(user_config.raw); // * Saves light time out config to eeprom
         } 
         return true;
      break;

   }
   return true;
};

// Will trigger Caps lock RGB, only on base layer
void led_set_user(uint8_t usb_led) {
   if ((current == _BASE) && (usb_led & (1<<USB_LED_CAPS_LOCK))) {
      rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_BREATHING_RED); 
   } else if(current == _BASE) { // * Cap lock indicator will only show on base layer
      reset_rgb();
   }
}

// RGB led number layout, function of the key

//  67, led 01   0, ESC    6, F1      12, F2      18, F3   23, F4   28, F5      34, F6   39, F7   44, F8      50, F9   56, F10   61, F11    66, F12    69, Prt       Rotary(Mute)   68, led 12
//  70, led 02   1, ~      7, 1       13, 2       19, 3    24, 4    29, 5       35, 6    40, 7    45, 8       51, 9    57, 0     62, -_     78, (=+)   85, BackSpc   72, Home       71, led 13
//  73, led 03   2, Tab    8, Q       14, W       20. E    25, R    30, T       36, Y    41, U    46, I       52, O    58, P     63, [{     89, ]}     93, \|        75, PgUp       74, led 14
//  76, led 04   3, Caps   9, A       15, S       21, D    26, F    31, G       37, H    42, J    47, K       53, L    59, ;:    64, '"                96, Enter     86, PgDn       77, led 15
//  80, led 05   4, Sh_L   10, Z      16, X       22, C    27, V    32, B       38, N    43, M    48, ,<      54, .<   60, /?               90, Sh_R   94, Up        82, End        81, led 16
//  83, led 06   5, Ct_L   11,Win_L   17, Alt_L                     33, SPACE                     49, Alt_R   55, FN             65, Ct_R   95, Left   97, Down      79, Right      84, led 17
//  87, led 07                                                                                                                                                                      88, led 18
//  91, led 08 