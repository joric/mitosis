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

#define KEYMAP( \
  k00, k01, k02, k03, k04,      k05, k06, k07, k08, k09, \
  k10, k11, k12, k13, k14,      k15, k16, k17, k18, k19, \
  k20, k21, k22, k23, k24,      k25, k26, k27, k28, k29, \
       k31, k32, k33, k34,      k35, k36, k37, k38,      \
       k41, k42, k43, k44,      k45, k46, k47, k48       \
) \
{ \
	{ k00,   k01, k02, k03, k04,      k05, k06, k07, k08, k09   }, \
	{ k10,   k11, k12, k13, k14,      k15, k16, k17, k18, k19   }, \
	{ k20,   k21, k22, k23, k24,      k25, k26, k27, k28, k29   }, \
	{ KC_NO, k31, k32, k33, k34,      k35, k36, k37, k38, KC_NO }  \
	{ KC_NO, k41, k42, k43, k44,      k45, k46, k47, k48, KC_NO }, \
}

#define set_led_off
#define set_led_red
#define set_led_blue

#endif //KEYMAP_H
