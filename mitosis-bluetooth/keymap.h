#ifndef KEYMAP_H
#define KEYMAP_H

#include <stdint.h>
#include "keycode.h"

#define MATRIX_ROWS 5
#define MATRIX_COLS 10

#define SAFE_RANGE 0x8000
#define PROGMEM

extern int biton32(int x);
extern int layer_state;
extern const uint16_t keymaps[][MATRIX_ROWS][MATRIX_COLS];

#define	QK_LSFT 0x0200
#define	QK_FUNCTION 0x2000
#define	QK_LAYER_TAP 0x4000

#define LT(layer, kc) (kc | QK_LAYER_TAP | ((layer & 0xF) << 8))

#define LSFT(kc) (kc | QK_LSFT)
#define FUNC(kc) (kc | QK_FUNCTION)

#define S(kc) LSFT(kc)
#define F(kc) FUNC(kc)

#define set_led_off
#define set_led_red
#define set_led_blue

#endif //KEYMAP_H
