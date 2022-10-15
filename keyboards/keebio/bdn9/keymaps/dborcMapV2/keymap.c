#include QMK_KEYBOARD_H

#define LIGHT_TIMEOUT 10    // in minutes
static uint16_t idle_timer = 0;
static uint8_t halfmin_counter = 0;
static bool led_on = true;
static bool light_timer = true;

enum custom_keycodes {
    AUDIO_OUT, MIC_IN, MIC_MUTE, CONNECT, MIXER, TRIM, DC_MIC, DC_STM, RGB_SWITCH
};

enum layer_names {
    _BASE, _MEDIA, _MOD, _RGB
};

typedef union {
  uint32_t raw;
  struct {
    bool     light_timer_config :1;
  };
} user_config_t;

user_config_t user_config;

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_BASE] = LAYOUT(
     //┌───────────┬───────────┬───────────┐
        MIC_MUTE   , LT(_MEDIA, KC_MPLY), TO(_MOD),
     //├───────────┼───────────┼───────────┤
        MIXER      , KC_ESCAPE , CONNECT,
     //├───────────┼───────────┼──────────┤
        MIC_IN     , TRIM      , AUDIO_OUT
     //└───────────┴───────────┴───────────┘
    ),
    [_MEDIA] = LAYOUT(
     //┌───────────┬───────────┬───────────┐
        _______    , _______   , _______,
     //├───────────┼───────────┼───────────┤
        _______    , _______   , KC_MUTE,
     //├───────────┼───────────┼───────────┤
        _______    , _______   , _______
     //└───────────┴───────────┴───────────┘
    ),
    [_MOD] = LAYOUT(
     //┌───────────┬───────────┬───────────┐
        RGB_SWITCH , TO(_RGB)  , TO(_BASE),
     //├───────────┼───────────┼───────────┤
        DC_MIC     , XXXXXXX   , RGB_TOG,
     //├───────────┼───────────┼───────────┤
        DC_STM     , XXXXXXX   , XXXXXXX
     //└───────────┴───────────┴───────────┘
    ),
    [_RGB] = LAYOUT(
     //┌───────────┬───────────┬───────────┐
        RGB_SPD    , TO(_BASE) , RGB_SPI,
     //├───────────┼───────────┼───────────┤
        RGB_HUI    , RGB_MOD   , RGB_SAI,
     //├───────────┼───────────┼───────────┤
        RGB_HUD    , RGB_RMOD  , RGB_SAD
     //└───────────┴───────────┴───────────┘
    ),
};

void keyboard_post_init_user(void) {
  // Read the user config from EEPROM
  user_config.raw = eeconfig_read_user();
  // Update light_timer to eeprom
  light_timer = user_config.light_timer_config;
}

//lighting timeout
void matrix_scan_user(void) {
    if (idle_timer == 0) idle_timer = timer_read();
    if ( led_on && timer_elapsed(idle_timer) > 30000) {
        halfmin_counter++;
        idle_timer = timer_read();
    }
    if (led_on && halfmin_counter >= LIGHT_TIMEOUT * 2) {
        //rgb lighting
        if(light_timer == true) {
            rgblight_disable();
        }
        led_on = false;
        halfmin_counter = 0;
    }
}

void lighting_idle_wake(void) {
    if (led_on == false || rgblight_is_enabled()) {
        //rgb lighting
        rgblight_enable();
        led_on = true;
    }
    idle_timer = timer_read();
    halfmin_counter = 0;
}

/* Encoder 
*/
bool encoder_update_user(uint8_t index, bool clockwise) {
    if(clockwise || !clockwise) lighting_idle_wake();
	if(layer_state_is(_MEDIA)) { //layer MEDIA
        if(index == 2) {
            clockwise ? tap_code(KC_MPRV) : tap_code(KC_MNXT);
        }
	} else if (layer_state_is(_BASE)) { // layer BASE
        if(index == 2) {
            clockwise ? tap_code(KC_VOLD) : tap_code(KC_VOLU);
        }
	} else if (layer_state_is(_MOD)) { // layer MOD
        if(index == 2) {
            clockwise ? rgblight_decrease_val() : rgblight_increase_val();
        }
	} else if (layer_state_is(_RGB)) { // layer RGB
        if(index == 2) {
            clockwise ? rgblight_decrease_val() : rgblight_increase_val();
        }
	}
    return true;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) lighting_idle_wake();
    switch (keycode) {
        case RGB_SWITCH:
            if (record->event.pressed) {
                if(light_timer) {
                    light_timer = false;
                    rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_RED);
                    user_config.light_timer_config = false;
                    eeconfig_update_user(user_config.raw);
                } else {
                    light_timer = true;
                    rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_GREEN);
                    user_config.light_timer_config = true;
                    eeconfig_update_user(user_config.raw);
                }
            } else {
            }
            return false;
            break;
    	case AUDIO_OUT:
    	    if (record->event.pressed) {
                register_code(KC_LCTRL);
                register_code(KC_LSHIFT);
                register_code(KC_LALT);
                tap_code(KC_S);
            } else {
                unregister_code(KC_LCTRL);
                unregister_code(KC_LSHIFT);
                unregister_code(KC_LALT);
            }
            return false;
            break;
	    case MIC_IN:
    	    if (record->event.pressed) {
                register_code(KC_LCTRL);
                register_code(KC_LSHIFT);
                register_code(KC_LALT);
                tap_code(KC_V);
            } else {
                unregister_code(KC_LCTRL);
                unregister_code(KC_LSHIFT);
                unregister_code(KC_LALT);
            }
            return false;
            break;
        case MIC_MUTE:
    	    if (record->event.pressed) {
                register_code(KC_LCTRL);
                register_code(KC_LALT);
                tap_code(KC_M);
            } else {
                unregister_code(KC_LCTRL);
                unregister_code(KC_LALT);
            }
            return false;
            break;
        case MIXER:
    	    if (record->event.pressed) {
                register_code(KC_LCTRL);
                register_code(KC_LALT);
                tap_code(KC_N);
            } else {
                unregister_code(KC_LCTRL);
                unregister_code(KC_LALT);
            }
            return false;
            break;
        case CONNECT:
    	    if (record->event.pressed) {
                register_code(KC_LGUI);
                tap_code(KC_K);
            } else {
                unregister_code(KC_LGUI);
            }
            return false;
            break;
        case TRIM:
            if (record->event.pressed) {
                register_code(KC_LGUI);
                register_code(KC_LSHIFT);
                tap_code(KC_S);
            } else {
                unregister_code(KC_LGUI);
                unregister_code(KC_LSHIFT);
            }
            return false;
            break;
        case DC_MIC:
            if (record->event.pressed) {
                register_code(KC_LCTRL);
                register_code(KC_LSHIFT);
                register_code(KC_LALT);
                tap_code(KC_U);
            } else {
                unregister_code(KC_LCTRL);
                unregister_code(KC_LSHIFT);
                unregister_code(KC_LALT);
            }
            return false;
            break;
	    case DC_STM:
            if (record->event.pressed) {
                register_code(KC_LCTRL);
                register_code(KC_LSHIFT);
                register_code(KC_LALT);
                tap_code(KC_J);
            } else {
                unregister_code(KC_LCTRL);
                unregister_code(KC_LSHIFT);
                unregister_code(KC_LALT);
            }
            return false;
            break;
    }
    return true;
};

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
	get_rgb();
    rgblight_enable();
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

uint32_t layer_state_set_user(uint32_t state) {
    if(prev == _RGB) {
        get_rgb();
    }
    current = biton32(state);
	switch (current) {
		case _BASE:
		    if(prev == _MOD || prev == _MEDIA || prev == _RGB) {
                reset_rgb();
            }		  
		    break;
		case _MOD:
            if(light_timer) {
                rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_GREEN);
            } else {
                rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_RED);
            }
            break;
        case _MEDIA:
            rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MEDIA_BREATHING_WHITE);
            break;
        case _RGB:
            reset_rgb();
            break;
    }
    prev = current;
    return state;
}

// caps lock
void led_set_user(uint8_t usb_led) {
   if ((current == _BASE) && (usb_led & (1<<USB_LED_CAPS_LOCK))) {
       rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_RED);
   } else if(current == _BASE) {
       reset_rgb();
   }
}