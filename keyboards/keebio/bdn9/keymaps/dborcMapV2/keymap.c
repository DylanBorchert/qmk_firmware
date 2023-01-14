#include QMK_KEYBOARD_H

static uint32_t light_timeout = 300000; // 5 minutes
static uint32_t key_timer; // timer for the key
static void refresh_rgb(void); // refreshes the activity timer and RGB, invoke whenever activity happens
static void check_rgb_timeout(void); // checks if enough time has passed for RGB to timeout
static bool is_rgb_timeout = false; // store if RGB has timed out or not in a boolean
static bool light_timer = true; // store if light timer is enabled or not in a boolean

enum custom_keycodes {
    RGB_SWITCH = SAFE_RANGE,
    AUDIO_OUT, 
    MIC_IN, 
    MIC_MUTE, 
    CONNECT, 
    MIXER, 
    TRIM, 
    DC_MIC, 
    DC_STM, 
    RGB_PLUS,
    RGB_MINUS,
    LOCKPC
};

enum layer_names {
    _BASE, _MEDIA, _MOD, _RGB
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
        RGB_MINUS  , XXXXXXX   , RGB_PLUS,
     //├───────────┼───────────┼───────────┤
        _______    , LOCKPC    , _______,
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

void refresh_rgb() {
    printf("RGB keytimer %d\n", timer_elapsed(key_timer));
    printf("RGB Timeout: %ld\n", light_timeout);
    key_timer = timer_read(); // store time of last refresh
    if (is_rgb_timeout && light_timer) { // only do something if rgb has timed out
        rgblight_enable();
        is_rgb_timeout = false;

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

/* Encoder 
*/
bool encoder_update_user(uint8_t index, bool clockwise) {
    if(clockwise || !clockwise) refresh_rgb();
    if(index == 2) {
        switch(biton32(layer_state)) {
            case _BASE:
                clockwise ? tap_code(KC_VOLD) : tap_code(KC_VOLU);
                return false;
                break;
            case _MOD:
                clockwise ? rgblight_decrease_val() : rgblight_increase_val();
                return false;
                break;
            case _MEDIA:
                clockwise ? tap_code(KC_MPRV) : tap_code(KC_MNXT);
                return false;
                break;
            case _RGB:
                clockwise ? rgblight_decrease_val() : rgblight_increase_val();
                return false;
                break;
            default:
                return true;
        }

    }
    return true;
    
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
        printf("RGB Timout Increased from: %d to %d \n", timeMinute, milli_to_min(light_timeout));
        user_config.rgb_timeout_config = milli_to_min(light_timeout);
        eeconfig_update_user(user_config.raw);
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
        printf("RGB Timout Decreased from: %d to %d \n", timeMinute, milli_to_min(light_timeout));
        user_config.rgb_timeout_config = milli_to_min(light_timeout);
        eeconfig_update_user(user_config.raw);
    }

}

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
                    print("RGB Timeout: OFF\n");
                } else {
                    light_timer = true;
                    rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_GREEN);             
                    user_config.light_timer_config = true;
                    eeconfig_update_user(user_config.raw); // * Saves light time out config to eeprom
                    print("RGB Timeout: ON\n");
                }
            }
            return true;
            break;
        case RGB_PLUS:
    	    if (record->event.pressed) {
                increase_rgb_timeout();
            }
            return true;
            break;
        case RGB_MINUS:
    	    if (record->event.pressed) {
                decrease_rgb_timeout();
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
    	case AUDIO_OUT:
    	    if (record->event.pressed) {
                register_code(KC_LCTL);
                register_code(KC_LSFT);
                register_code(KC_LALT);
                tap_code(KC_S);
            } else {
                unregister_code(KC_LCTL);
                unregister_code(KC_LSFT);
                unregister_code(KC_LALT);
            }
            return true;
            break;
	    case MIC_IN:
    	    if (record->event.pressed) {
                register_code(KC_LCTL);
                register_code(KC_LSFT);
                register_code(KC_LALT);
                tap_code(KC_V);
            } else {
                unregister_code(KC_LCTL);
                unregister_code(KC_LSFT);
                unregister_code(KC_LALT);
            }
            return true;
            break;
        case MIC_MUTE:
    	    if (record->event.pressed) {
                register_code(KC_LCTL);
                register_code(KC_LALT);
                tap_code(KC_M);
            } else {
                unregister_code(KC_LCTL);
                unregister_code(KC_LALT);
            }
            return true;
            break;
        case MIXER:
    	    if (record->event.pressed) {
                register_code(KC_LCTL);
                register_code(KC_LALT);
                tap_code(KC_N);
            } else {
                unregister_code(KC_LCTL);
                unregister_code(KC_LALT);
            }
            return true;
            break;
        case CONNECT:
    	    if (record->event.pressed) {
                register_code(KC_LGUI);
                tap_code(KC_K);
            } else {
                unregister_code(KC_LGUI);
            }
            return true;
            break;
        case TRIM:
            if (record->event.pressed) {
                register_code(KC_LGUI);
                register_code(KC_LSFT);
                tap_code(KC_S);
            } else {
                unregister_code(KC_LGUI);
                unregister_code(KC_LSFT);
            }
            return true;
            break;
        case DC_MIC:
            if (record->event.pressed) {
                register_code(KC_LCTL);
                register_code(KC_LSFT);
                register_code(KC_LALT);
                tap_code(KC_U);
            } else {
                unregister_code(KC_LCTL);
                unregister_code(KC_LSFT);
                unregister_code(KC_LALT);
            }
            return true;
            break;
	    case DC_STM:
            if (record->event.pressed) {
                register_code(KC_LCTL);
                register_code(KC_LSFT);
                register_code(KC_LALT);
                tap_code(KC_J);
            } else {
                unregister_code(KC_LCTL);
                unregister_code(KC_LSFT);
                unregister_code(KC_LALT);
            }
            return true;
            break;
    }
    return true;
};

// Will trigger Caps lock RGB, only on base layer
void led_set_user(uint8_t usb_led) {
   if ((current == _BASE) && (usb_led & (1<<USB_LED_CAPS_LOCK))) {
      rgb_matrix_mode_noeeprom(RGB_MATRIX_CUSTOM_MOD_SPIRAL_RED); 
      refresh_rgb();
   } else if(current == _BASE) { // * Cap lock indicator will only show on base layer
      reset_rgb();
   }
}