// Declare custom effects using the RGB_MATRIX_EFFECT macro
// (note the lack of semicolon after the macro!)
RGB_MATRIX_EFFECT(MEDIA)
RGB_MATRIX_EFFECT(BREATHING_RED)
RGB_MATRIX_EFFECT(MOD_SPIRAL_GREEN)
RGB_MATRIX_EFFECT(MOD_SPIRAL_RED)


#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

    // Leds to be effected by indicator
    // 0 - 69 function row and caps key
    // 67 - 92 side lights
    int rgblight_indicator[31] = {0, 3, 6, 12, 18, 23, 28, 34, 39, 44, 50, 56, 61, 66, 69, 67, 68, 70, 71, 73, 74, 76, 77, 80, 81, 83, 84, 87, 88, 91, 92};
    int rgb_sideLighting[16] = {67, 68, 70, 71, 73, 74, 76, 77, 80, 81, 83, 84, 87, 88, 91, 92};

    HSV optimise_HSV(void) {
        HSV hsv = rgb_matrix_config.hsv;
        if (hsv.v < 150) {
            hsv.v = 150;
        }
        hsv.s = 255;
        return hsv;
    }

    static bool MEDIA(effect_params_t* params) {
        RGB_MATRIX_USE_LIMITS(led_min, led_max);
        rgb_matrix_config.hsv = optimise_HSV();
        for (uint8_t i = 0; i < 31; i++) {
            rgb_matrix_set_color(rgblight_indicator[i], RGB_WHITE);
        }
        return led_max < DRIVER_LED_TOTAL;
    }

    bool BREATHING_RED(effect_params_t* params) {
        RGB_MATRIX_USE_LIMITS(led_min, led_max);
        rgb_matrix_config.hsv = optimise_HSV();
        HSV hsv       = rgb_matrix_config.hsv;
        uint16_t time = scale16by8(g_rgb_timer, 200 / 8);
        hsv.v         = scale8(abs8(sin8(time) - 128) * 2, hsv.v);
        hsv.s         = 255;
        hsv.h         = 0;
        RGB rgb       = rgb_matrix_hsv_to_rgb(hsv);
        for (uint8_t i = 0; i < 31; i++) {
            RGB_MATRIX_TEST_LED_FLAGS();
            rgb_matrix_set_color(rgblight_indicator[i], rgb.r, rgb.g, rgb.b);
        }
        return led_max < DRIVER_LED_TOTAL;
    }

    typedef HSV (*dx_dy_dist_f)(HSV hsv, int16_t dx, int16_t dy, uint8_t dist, uint8_t time);

    bool effect_runner_mod(effect_params_t* params, dx_dy_dist_f effect_func) {
        RGB_MATRIX_USE_LIMITS(led_min, led_max);
        uint8_t time = scale16by8(g_rgb_timer, rgb_matrix_config.speed / 2);
        for (uint8_t i = 0; i < 31; i++) {
            RGB_MATRIX_TEST_LED_FLAGS();
            int16_t dx   = g_led_config.point[rgblight_indicator[i]].x - k_rgb_matrix_center.x;
            int16_t dy   = g_led_config.point[rgblight_indicator[i]].y - k_rgb_matrix_center.y;
            uint8_t dist = sqrt16(dx * dx + dy * dy);
            RGB     rgb  = rgb_matrix_hsv_to_rgb(effect_func(rgb_matrix_config.hsv, dx, dy, dist, time));
            rgb_matrix_set_color(rgblight_indicator[i], rgb.r, rgb.g, rgb.b);
        }
        return led_max < DRIVER_LED_TOTAL;
    }

    static HSV MOD_SPIRAL_GREEN_math(HSV hsv, int16_t dx, int16_t dy, uint8_t dist, uint8_t time) {
        rgb_matrix_config.hsv = optimise_HSV();
        hsv.h = 85;
        rgb_matrix_config.speed = 200;
        hsv.v = scale8(hsv.v + dist - time - atan2_8(dy, dx), hsv.v);
        return hsv;
    }

    bool MOD_SPIRAL_GREEN(effect_params_t* params) { return effect_runner_mod(params, &MOD_SPIRAL_GREEN_math); }

    static HSV MOD_SPIRAL_RED_math(HSV hsv, int16_t dx, int16_t dy, uint8_t dist, uint8_t time) {
        rgb_matrix_config.hsv = optimise_HSV();
        hsv.h = 0;
        rgb_matrix_config.speed = 200;
        hsv.v = scale8(hsv.v + dist - time - atan2_8(dy, dx), hsv.v);
        return hsv;
    }

    bool MOD_SPIRAL_RED(effect_params_t* params) { return effect_runner_mod(params, &MOD_SPIRAL_RED_math); }

#endif