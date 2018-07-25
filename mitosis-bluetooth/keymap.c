// this is the style you want to emulate.
// This is the canonical layout file for the Quantum project. If you want to add another keyboard,

#include "mitosis.h"

// Each layer gets a name for readability, which is then used in the keymap matrix below.
// The underscores don't mean anything - you can have a layer called STUFF or any other name.
// Layer names don't all need to be of the same length, obviously, and you can also skip them
// entirely and just use numbers.
enum mitosis_layers
{
  _BL,
  _LOWER,
  _RAISE
};

enum mitosis_keycodes
{
  FNKEY = SAFE_RANGE,
  RAISE,
  LOWER
};


#define LONGPRESS_DELAY 150
#define LAYER_TOGGLE_DELAY 300

// Fillers to make layering more clear
#define _______ KC_TRNS
#define XXXXXXX KC_NO
#define DUAL_L  LT(_RAISE,KC_SPC)
#define DUAL_R  LT(_LOWER,KC_ENT)


const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

[_BL] = { /* test mitosis-mjt-like qwerty layout */
  {KC_Q,       KC_W,    KC_E,    KC_R,    KC_T,           KC_Y,    KC_U,    KC_I,    KC_O,        KC_P    },
  {KC_A,       KC_S,    KC_D,    KC_F,    KC_G,           KC_H,    KC_J,    KC_K,    KC_L,        KC_SCLN },
  {KC_Z,       KC_X,    KC_C,    KC_V,    KC_B,           KC_N,    KC_M,    KC_COMM, KC_DOT,      KC_SLSH },
  {XXXXXXX,    KC_LCTL, KC_ESC,  KC_TAB,  KC_QUOT,        KC_LEFT, KC_DOWN, KC_UP,   KC_RIGHT,    XXXXXXX },
  {XXXXXXX,    KC_LGUI, KC_LALT, KC_SPC,  KC_LSFT,        DUAL_L,  KC_BSPC, DUAL_R,  KC_RGUI,     XXXXXXX }
},



[_RAISE] = { /* Shifted Layer, layered so that tri_layer can be used, or selectively
                                 able to modify individual key's shifted behaviour */
  {KC_1,    KC_2,    KC_3,   KC_4,     KC_5,          KC_6,    KC_7,    KC_8,    KC_9,    KC_0    },
  {KC_F1,   KC_F2,   KC_F3,  KC_F4,    KC_F5,         KC_F6,   KC_4,    KC_5,    KC_6,    _______ },
  {KC_F7,   KC_F8,   KC_F9,  KC_F10,   KC_F11,        KC_F12,  KC_1,    KC_2,    KC_3,    _______ },
  {XXXXXXX, _______, _______, _______, _______,       _______, _______, _______, _______, XXXXXXX },
  {XXXXXXX, _______, _______, _______, _______,       _______, _______, _______, _______, XXXXXXX }
},



[_LOWER] = { /* Function Layer, primary alternative layer featuring numpad on right hand,
                                   cursor keys on left hand, and all symbols*/
  {S(KC_1), S(KC_2), S(KC_3),   S(KC_4),  S(KC_5),       S(KC_6),   S(KC_7),   S(KC_8),  S(KC_9),   S(KC_0)    },
  {_______, _______, S(KC_QUOT),S(KC_GRV),_______,       S(KC_BSLS),S(KC_MINS),S(KC_EQL),S(KC_LBRC),S(KC_RBRC) },
  {_______, _______, KC_QUOT,   KC_GRV,   _______,       KC_BSLS,   KC_MINS,   KC_EQL,   KC_LBRC,   KC_RBRC    },
  {_______, _______, _______,   _______,  _______,       _______,   _______,   KC_HOME,  KC_PGUP,   XXXXXXX    },
  {XXXXXXX, _______, _______,   _______,  _______,       _______,   _______,   KC_END,   KC_PGDN,   XXXXXXX    }
}

};





void matrix_scan_user(void) {
    uint8_t layer = biton32(layer_state);

    switch (layer) {
      case _BL:
        set_led_off;
        break;
        case _RAISE:
            set_led_blue;
            break;
        case _LOWER:
            set_led_red;
            break;
        default:
            break;
    }
};

