// Declare custom effects using the RGB_MATRIX_EFFECT macro
// (note the lack of semicolon after the macro!)
RGB_MATRIX_EFFECT(BREATHING_RED)
RGB_MATRIX_EFFECT(MOD_SPIRAL_GREEN_ONE)
RGB_MATRIX_EFFECT(MOD_SPIRAL_GREEN_FIVE)
RGB_MATRIX_EFFECT(MOD_SPIRAL_GREEN_TEN)
RGB_MATRIX_EFFECT(MOD_SPIRAL_GREEN_FIFTEEN)
RGB_MATRIX_EFFECT(MOD_SPIRAL_RED_ONE)
RGB_MATRIX_EFFECT(MOD_SPIRAL_RED_FIVE)
RGB_MATRIX_EFFECT(MOD_SPIRAL_RED_TEN)
RGB_MATRIX_EFFECT(MOD_SPIRAL_RED_FIFTEEN)


#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

    // Leds to be effected by indicator
    // 0 - 13 function row and caps key

    int rgblight_indicator[14] = {0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13};
    int rgblight_five[1] = {58};
    int rgblight_ten[2] = {58, 44};
    int rgblight_fifteen[3] = {58, 29, 44};

    HSV optimise_HSV(void) {
        HSV hsv = rgb_matrix_config.hsv;
        if (hsv.v < 150) {
            hsv.v = 150;
        }
        hsv.s = 255;
        return hsv;
    }

    bool BREATHING_RED(effect_params_t* params) {
        //combine array  rgblight_indicator and rgblight_fifteen
        int fullSize = (sizeof(rgblight_indicator) / sizeof(rgblight_indicator[0])) + (sizeof(rgblight_fifteen) / sizeof(rgblight_fifteen[0]));
        int rgblight_full[fullSize];
        memcpy(rgblight_full, rgblight_indicator, sizeof(rgblight_indicator));
        memcpy(rgblight_full + (sizeof(rgblight_indicator) / sizeof(rgblight_indicator[0])), rgblight_fifteen, sizeof(rgblight_fifteen));
        
        RGB_MATRIX_USE_LIMITS(led_min, led_max);
        rgb_matrix_config.hsv = optimise_HSV();
        HSV hsv       = rgb_matrix_config.hsv;
        uint16_t time = scale16by8(g_rgb_timer, 200 / 8);
        hsv.v         = scale8(abs8(sin8(time) - 128) * 2, hsv.v);
        hsv.s         = 255;
        hsv.h         = 0;
        RGB rgb       = rgb_matrix_hsv_to_rgb(hsv);
        int length = sizeof(rgblight_full) / sizeof(rgblight_full[0]);
        for (uint8_t i = 0; i < length; i++) {
            RGB_MATRIX_TEST_LED_FLAGS();
            rgb_matrix_set_color(rgblight_full[i], rgb.r, rgb.g, rgb.b);
        }
        return led_max < RGB_MATRIX_LED_COUNT;
    }

    typedef HSV (*dx_dy_dist_f)(HSV hsv, int16_t dx, int16_t dy, uint8_t dist, uint8_t time);

    bool effect_runner_mod(effect_params_t* params, dx_dy_dist_f effect_func, int timeout) {
        int rgblight_full_size = sizeof(rgblight_indicator) / sizeof(rgblight_indicator[0]);
        int fullSize = rgblight_full_size;
        int addedSize = 0;
        switch (timeout) {
            case 5:
                addedSize = sizeof(rgblight_five) / sizeof(rgblight_five[0]);
                break;
            case 10:
                addedSize = sizeof(rgblight_ten) / sizeof(rgblight_ten[0]);
                break;
            case 15:
                addedSize = sizeof(rgblight_fifteen) / sizeof(rgblight_fifteen[0]);
                break;
        }
        fullSize += addedSize;
        int rgblight_full[fullSize];
        memcpy(rgblight_full, rgblight_indicator, sizeof(rgblight_indicator));
        switch (timeout) {
            case 5:
                memcpy(rgblight_full + rgblight_full_size, rgblight_five, sizeof(rgblight_five));
                break;
            case 10:
                memcpy(rgblight_full + rgblight_full_size, rgblight_ten, sizeof(rgblight_ten));
                break;
            case 15:
                memcpy(rgblight_full + rgblight_full_size, rgblight_fifteen, sizeof(rgblight_fifteen));
                break;
        }

        RGB_MATRIX_USE_LIMITS(led_min, led_max);
        uint8_t time = scale16by8(g_rgb_timer, rgb_matrix_config.speed / 2);
        int length = sizeof(rgblight_full) / sizeof(rgblight_full[0]);
        for (uint8_t i = 0; i < length; i++) {
            RGB_MATRIX_TEST_LED_FLAGS();
            int16_t dx   = g_led_config.point[rgblight_full[i]].x - k_rgb_matrix_center.x;
            int16_t dy   = g_led_config.point[rgblight_full[i]].y - k_rgb_matrix_center.y;
            uint8_t dist = sqrt16(dx * dx + dy * dy);
            RGB     rgb  = rgb_matrix_hsv_to_rgb(effect_func(rgb_matrix_config.hsv, dx, dy, dist, time));
            rgb_matrix_set_color(rgblight_full[i], rgb.r, rgb.g, rgb.b);
        }
        return led_max < RGB_MATRIX_LED_COUNT;
    }

    static HSV MOD_SPIRAL_GREEN_math(HSV hsv, int16_t dx, int16_t dy, uint8_t dist, uint8_t time) {
        rgb_matrix_config.hsv = optimise_HSV();
        hsv.h = 85;
        rgb_matrix_config.speed = 200;
        hsv.v = scale8(hsv.v + dist - time - atan2_8(dy, dx), hsv.v);
        return hsv;
    }

    bool MOD_SPIRAL_GREEN_ONE(effect_params_t* params) { return effect_runner_mod(params, &MOD_SPIRAL_GREEN_math, 1); }
    bool MOD_SPIRAL_GREEN_FIVE(effect_params_t* params) { return effect_runner_mod(params, &MOD_SPIRAL_GREEN_math, 5); }
    bool MOD_SPIRAL_GREEN_TEN(effect_params_t* params) { return effect_runner_mod(params, &MOD_SPIRAL_GREEN_math, 10); }
    bool MOD_SPIRAL_GREEN_FIFTEEN(effect_params_t* params) { return effect_runner_mod(params, &MOD_SPIRAL_GREEN_math, 15); }

    static HSV MOD_SPIRAL_RED_math(HSV hsv, int16_t dx, int16_t dy, uint8_t dist, uint8_t time) {
        rgb_matrix_config.hsv = optimise_HSV();
        hsv.h = 0;
        rgb_matrix_config.speed = 200;
        hsv.v = scale8(hsv.v + dist - time - atan2_8(dy, dx), hsv.v);
        return hsv;
    }

    bool MOD_SPIRAL_RED_ONE(effect_params_t* params) { return effect_runner_mod(params, &MOD_SPIRAL_RED_math, 1); }
    bool MOD_SPIRAL_RED_FIVE(effect_params_t* params) { return effect_runner_mod(params, &MOD_SPIRAL_RED_math, 5); }
    bool MOD_SPIRAL_RED_TEN(effect_params_t* params) { return effect_runner_mod(params, &MOD_SPIRAL_RED_math, 10); }
    bool MOD_SPIRAL_RED_FIFTEEN(effect_params_t* params) { return effect_runner_mod(params, &MOD_SPIRAL_RED_math, 15); }


#endif