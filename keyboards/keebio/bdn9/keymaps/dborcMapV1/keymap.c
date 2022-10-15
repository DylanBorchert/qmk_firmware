#include QMK_KEYBOARD_H

#define LIGHT_TIMEOUT 5    // in minutes
static uint16_t idle_timer = 0;
static uint8_t halfmin_counter = 0;
static uint8_t old_backlight_level = -1;
static bool led_on = true;
static bool light_timer = false;


enum custom_keycodes {
    AUDIO_OUT, MIC_IN, MIC_MUTE, CONNECT, MIXER, TRIM, DC_MIC, DC_DEF, DC_STM, RGB_SWITCH
};

enum layer_names {
    _BASE, _MEDIA, _MOD, _RGB
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [_BASE] = LAYOUT(
     //┌───────────┬───────────┬───────────┐
        MIC_MUTE   , TO(_MOD)  , KC_MPLY,
     //├───────────┼───────────┼───────────┤
        MIXER      , KC_ESCAPE , CONNECT,
     //├───────────┼───────────┼───_───────┤
        MIC_IN     , TRIM      , AUDIO_OUT
     //└───────────┴───────────┴───────────┘
    ),
    [_MEDIA] = LAYOUT(
     //┌───────────┬───────────┬───────────┐
        _______    , _______   , _______,
     //├───────────┼───────────┼───────────┤
        _______    , _______   , KC_MUTE,
     //├───────────┼───────────┼───_───────┤
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
        RGB_SPD    ,TO(_BASE), RGB_SPI,
     //├───────────┼───────────┼───────────┤
        RGB_HUI    , RGB_MOD   , RGB_SAI,
     //├───────────┼───────────┼───────────┤
        RGB_HUD    , RGB_RMOD  , RGB_SAD
     //└───────────┴───────────┴───────────┘
    ),
};

void lighting_idle_wake(void) {
    if (led_on == false || rgblight_is_enabled()) {
        //rgb lighting
        rgblight_enable_noeeprom();
        led_on = true;
    }
    idle_timer = timer_read();
    halfmin_counter = 0;
}

void encoder_update_user(uint8_t index, bool clockwise) {
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
}
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    if (record->event.pressed) lighting_idle_wake();
    switch (keycode) {
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
            break;
        case CONNECT:
    	    if (record->event.pressed) {
                register_code(KC_LGUI);
                tap_code(KC_K);
            } else {
                unregister_code(KC_LGUI);
            }
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
            break;
        case DC_DEF:
            if (record->event.pressed) {
                register_code(KC_LCTRL);
                register_code(KC_LSHIFT);
                register_code(KC_LALT);
                tap_code(KC_I);
            } else {
                unregister_code(KC_LCTRL);
                unregister_code(KC_LSHIFT);
                unregister_code(KC_LALT);
            }
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
            break;
        case RGB_SWITCH:
            if (record->event.pressed) {
                if(light_timer) {
                    light_timer = false;
                    rgblight_sethsv(0, 255, 120);
                } else {
                    light_timer = true;
                    rgblight_sethsv(85, 255, 120);
                }
            } else {

            }
            break;
    }
    return true;
};

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
            rgblight_disable_noeeprom();
        }
        led_on = false;
        halfmin_counter = 0;
    }
}

uint8_t prev = _BASE;
uint16_t desired = 1;
uint8_t hue = 0;
uint8_t sat = 0;
uint8_t val = 120;
uint8_t sed = 255;
void get_hsv(void) {
	hue = rgblight_get_hue();
	sat = rgblight_get_sat();
	val = rgblight_get_val();
    sed = rgblight_get_speed();
}
void reset_hsv(void) {
	rgblight_sethsv(hue, sat, val);
    rgblight_set_speed(sed);
    rgblight_mode(desired);
}
void matrix_init_user() {
	desired = rgblight_get_mode();
	rgblight_enable();
	get_hsv();
}
uint32_t layer_state_set_user(uint32_t state) {
    uint8_t layer = biton32(state);
    if (prev!=_RGB) {
	  switch (layer) {
		case _BASE:
		  reset_hsv();		  
		  break;
		case _MOD:
            rgblight_set_speed_noeeprom(200);
            rgblight_mode_noeeprom(11);
            if(val < 150) {
                if(light_timer) {
                    rgblight_sethsv_noeeprom(85, 255, 150);
                } else {
                    rgblight_sethsv_noeeprom(0, 255, 150);
                }
            } else {
                if(light_timer) {
                    rgblight_sethsv_noeeprom(85, 255, val);
                } else {
                    rgblight_sethsv_noeeprom(0, 255, val);
                }
            }
            break;
        case _MEDIA: 
            rgblight_set_speed_noeeprom(200);
            rgblight_mode_noeeprom(5);
            if(val < 150) {
                rgblight_sethsv_noeeprom(255, 0, 150);
            } else {
                rgblight_sethsv_noeeprom(255, 0, val);
            }
            break;
      }
    } else {
        desired = rgblight_get_mode();
        get_hsv();
    }
    if(layer == _RGB) {
        reset_hsv();
    }
    prev = layer;
    return state;
}
