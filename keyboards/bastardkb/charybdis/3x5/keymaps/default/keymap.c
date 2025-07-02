/**
 * Copyright 2021 Charly Delay <charly@codesink.dev> (@0xcharly)
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
#include "caps_word.h"

#define SYM_NUM LT(SYMNUM, KC_SPC)
#define FN_DEL  LT(FN, KC_DEL)
#define FN_ENT  LT(FN, KC_ENT)

#define LBRC S(KC_QUOT)
#define RBRC S(KC_NUHS)

#define CTL_DEL CTL_T(KC_DEL)
#define LSF_TAB LSFT_T(KC_TAB)
#define RSF_TAB RSFT_T(KC_TAB)

#define HRM_R GUI_T(KC_R)
#define HRM_N LALT_T(KC_N)
#define HRM_D CTL_T(KC_D)

#define HRM_J LALT_T(KC_J)
#define HRM_L CTL_T(KC_L)

 enum layers{
     ALPHA,
     QWERTY,
     SYMNUM,
     FN,
     };

// =================================================================
//                             TAP DANCE
// =================================================================

 enum tap_dance{
     QST,      // Tap: ? | Hold: ¿
     EXC,      // Tap: ! | Hold: ¡
     COM,      // Tap: " | Hold: '
     MAQ,      // Tap: . | Hold: >
     MEQ,      // Tap: , | Hold: <
     GRV,      // Tap: | | Hold: °
     APP,      // Tap: ~ | Hold: ¬
     SLS,      // Tap: / | Hold: "\" 
     LPR,      // Tap: ( | Hold: {
     RPR,      // Tap: ) | Hold: }
     CAL,      // Tap: * | Hold: CALC
     ARR,      // Tap: q | Hold: @
     DPT,      // Tap: . | Hold: :
     
     UND,      // Tap: z | Hold: CTRL+z
     RED,      // Tap: y | Hold: CTRL+y
     CUT,      // Tap: x | Hold: CTRL+x
     COP,      // Tap: c | Hold: CTRL+c
     PAS,      // Tap: v | Hold: CTRL+v
     SEA,      // Tap: f | Hold: CTRL+f
     TAB,      // Tap: t | Hold: CTRL+t

     TDA,      // Tap: a | Hold: á
     TDE,      // Tap: e | Hold: é
     TDI,      // Tap: i | Hold: í
     TDO,      // Tap: o | Hold: ó
     TDU,      // Tap: u | Hold: ú
 };

 typedef struct {
     uint16_t tap;
     uint16_t hold1;      
     uint16_t hold2;      
     uint16_t held;
     bool is_dual_hold;   
 } tap_dance_tap_hold_t;

// Variable global para rastrear si caps_word está activo durante tap dance
static bool caps_word_was_active = false;
 
 void tap_dance_tap_hold_finished(tap_dance_state_t *state, void *user_data) {
     tap_dance_tap_hold_t *tap_hold = (tap_dance_tap_hold_t *)user_data;
     
     // Guardar el estado de caps_word al inicio
     caps_word_was_active = is_caps_word_on();
     
     if (state->pressed) {
         if (state->count == 1
 #ifndef PERMISSIVE_HOLD
             && !state->interrupted
 #endif
         ) {
             // Hold action
             if (tap_hold->is_dual_hold && tap_hold->hold2 != 0) {
                 // Para acentos: aplicar caps_word solo si es una letra
                 if (caps_word_was_active) {
                     // Determinar si necesitamos capitalizar
                     bool should_capitalize = false;
                     uint16_t base_key = tap_hold->hold2;
                     
                     switch (base_key) {
                         case KC_A:
                         case KC_E:
                         case KC_I:
                         case KC_O:
                         case KC_U:
                             should_capitalize = true;
                             break;
                     }
                     
                     if (should_capitalize) {
                         // Enviar tilde
                         tap_code16(tap_hold->hold1);
                         // Enviar letra mayúscula
                         tap_code16(S(base_key));
                     } else {
                         tap_code16(tap_hold->hold1);
                         tap_code16(tap_hold->hold2);
                     }
                 } else {
                     tap_code16(tap_hold->hold1);
                     tap_code16(tap_hold->hold2);
                 }
                 tap_hold->held = 0;
             } else {
                 // Para otros holds (como CTRL+Z)
                 register_code16(tap_hold->hold1);
                 tap_hold->held = tap_hold->hold1;
             }
         } else {
             // Tap action
             if (caps_word_was_active) {
                 // Verificar si es una letra que debe capitalizarse
                 bool should_capitalize = false;
                 uint16_t base_key = tap_hold->tap;
                 
                 switch (base_key) {
                     case KC_A:
                     case KC_E:
                     case KC_I:
                     case KC_O:
                     case KC_U:
                     case KC_Z:
                     case KC_Y:
                     case KC_X:
                     case KC_C:
                     case KC_V:
                     case KC_F:
                     case KC_T:
                     case KC_Q:
                         should_capitalize = true;
                         break;
                 }
                 
                 if (should_capitalize) {
                     register_code16(S(base_key));
                 } else {
                     register_code16(tap_hold->tap);
                 }
             } else {
                 register_code16(tap_hold->tap);
             }
             tap_hold->held = tap_hold->tap;
         }
     }
 }
 
 void tap_dance_tap_hold_reset(tap_dance_state_t *state, void *user_data) {
     tap_dance_tap_hold_t *tap_hold = (tap_dance_tap_hold_t *)user_data;
 
     if (tap_hold->held) {
         unregister_code16(tap_hold->held);
         tap_hold->held = 0;
     }
 }
 
 #define ACTION_TAP_DANCE_TAP_HOLD(tap, hold) \
     { \
         .fn = {NULL, tap_dance_tap_hold_finished, tap_dance_tap_hold_reset}, \
         .user_data = (void *)&((tap_dance_tap_hold_t){tap, hold, 0, 0, false}), \
     }
 
 #define ACTION_TAP_DANCE_TAP_DUAL_HOLD(tap, hold1, hold2) \
     { \
         .fn = {NULL, tap_dance_tap_hold_finished, tap_dance_tap_hold_reset}, \
         .user_data = (void *)&((tap_dance_tap_hold_t){tap, hold1, hold2, 0, true}), \
     }
 
 tap_dance_action_t tap_dance_actions[] = {
     [QST] = ACTION_TAP_DANCE_TAP_HOLD(S(KC_MINUS), KC_EQUAL),
     [EXC] = ACTION_TAP_DANCE_TAP_HOLD(S(KC_1), S(KC_EQUAL)),
     [COM] = ACTION_TAP_DANCE_TAP_HOLD(S(KC_2), KC_MINUS),
     [MAQ] = ACTION_TAP_DANCE_TAP_HOLD(KC_DOT, S(KC_NUBS)),
     [MEQ] = ACTION_TAP_DANCE_TAP_HOLD(KC_COMM, KC_NUBS),
     [GRV] = ACTION_TAP_DANCE_TAP_HOLD(KC_GRV, S(KC_GRV)),
     [APP] = ACTION_TAP_DANCE_TAP_HOLD(RALT(KC_RBRC), RALT(KC_GRV)),
     [SLS] = ACTION_TAP_DANCE_TAP_HOLD(S(KC_7), RALT(KC_MINS)),
     [LPR] = ACTION_TAP_DANCE_TAP_HOLD(S(KC_8), KC_QUOT),
     [RPR] = ACTION_TAP_DANCE_TAP_HOLD(S(KC_9), KC_NUHS),
     [CAL] = ACTION_TAP_DANCE_TAP_HOLD(KC_PSLS, KC_CALC),
     [ARR] = ACTION_TAP_DANCE_TAP_HOLD(KC_Q, RALT(KC_Q)),
     [DPT] = ACTION_TAP_DANCE_TAP_HOLD(KC_PDOT, S(KC_DOT)),

     [UND] = ACTION_TAP_DANCE_TAP_HOLD(KC_Z, C(KC_Z)),
     [RED] = ACTION_TAP_DANCE_TAP_HOLD(KC_Y, C(KC_Y)),
     [CUT] = ACTION_TAP_DANCE_TAP_HOLD(KC_X, C(KC_X)),
     [COP] = ACTION_TAP_DANCE_TAP_HOLD(KC_C, C(KC_C)),
     [PAS] = ACTION_TAP_DANCE_TAP_HOLD(KC_V, C(KC_V)),
     [SEA] = ACTION_TAP_DANCE_TAP_HOLD(KC_F, C(KC_F)),
     [TAB] = ACTION_TAP_DANCE_TAP_HOLD(KC_T, C(KC_T)),

     [TDA] = ACTION_TAP_DANCE_TAP_DUAL_HOLD(KC_A, KC_LBRC, KC_A),  // á = ´ + a
     [TDE] = ACTION_TAP_DANCE_TAP_DUAL_HOLD(KC_E, KC_LBRC, KC_E),  // é = ´ + e
     [TDI] = ACTION_TAP_DANCE_TAP_DUAL_HOLD(KC_I, KC_LBRC, KC_I),  // í = ´ + i
     [TDO] = ACTION_TAP_DANCE_TAP_DUAL_HOLD(KC_O, KC_LBRC, KC_O),  // ó = ´ + o
     [TDU] = ACTION_TAP_DANCE_TAP_DUAL_HOLD(KC_U, KC_LBRC, KC_U),  // ú = ´ + u
 };
 
 bool process_record_user(uint16_t keycode, keyrecord_t *record) {
     tap_dance_action_t *action;
     
     switch (keycode) {
         case TD(QST):         case TD(EXC):         case TD(COM):         case TD(MAQ):         case TD(MEQ):         
         case TD(GRV):         case TD(APP):         case TD(SLS):         case TD(LPR):         case TD(RPR):         
         case TD(CAL):         case TD(UND):         case TD(RED):         case TD(CUT):         case TD(COP):         
         case TD(PAS):         case TD(SEA):         case TD(TAB):         case TD(TDA):         case TD(TDE):
         case TD(TDI):         case TD(TDO):         case TD(TDU):         case TD(ARR):         case TD(DPT):
             action = &tap_dance_actions[QK_TAP_DANCE_GET_INDEX(keycode)];
             if (!record->event.pressed && action->state.count && !action->state.finished) {
                 tap_dance_tap_hold_t *tap_hold = (tap_dance_tap_hold_t *)action->user_data;
                 
                 // Aplicar caps_word solo si es una letra
                 if (is_caps_word_on()) {
                     bool should_capitalize = false;
                     uint16_t base_key = tap_hold->tap;
                     
                     switch (base_key) {
                         case KC_A:
                         case KC_E:
                         case KC_I:
                         case KC_O:
                         case KC_U:
                         case KC_Z:
                         case KC_Y:
                         case KC_X:
                         case KC_C:
                         case KC_V:
                         case KC_F:
                         case KC_T:
                         case KC_Q:
                             should_capitalize = true;
                             break;
                     }
                     
                     if (should_capitalize) {
                         tap_code16(S(base_key));
                     } else {
                         tap_code16(tap_hold->tap);
                     }
                 } else {
                     tap_code16(tap_hold->tap);
                 }
             }
             break;
     }

     return true;
}
// ==================================================================
//                         FIN DE TAP DANCE
// ==================================================================



// ==================================================================
//                             CAPS WORD
// ==================================================================

 bool caps_word_press_user(uint16_t keycode) {
    switch (keycode) {

        // Regular keycodes that continue Caps Word, with shift applied.
        case KC_A ... KC_Z:
        case KC_SCLN:           // ñ
            add_weak_mods(MOD_BIT(KC_LSFT));  // Apply shift to next key.
            return true;

        // Tap dance keys - let them handle their own capitalization
        case TD(TDA):           // á/A
        case TD(TDE):           // é/E
        case TD(TDI):           // í/I
        case TD(TDO):           // ó/O
        case TD(TDU):           // ú/U
        case TD(UND):           // z/CTRL+z
        case TD(RED):           // y/CTRL+y
        case TD(CUT):           // x/CTRL+x
        case TD(COP):           // c/CTRL+c
        case TD(PAS):           // v/CTRL+v
        case TD(SEA):           // f/CTRL+f
        case TD(TAB):           // t/CTRL+t
        case TD(ARR):           // q/@
        case TD(DPT):           // ./:
            return true;        // Continue caps word, but don't apply shift

        // Keycodes that continue Caps Word, without shifting.
        case KC_1 ... KC_0:
        case KC_BSPC:
        case KC_DEL:
        case KC_SPC:
        case KC_LCTL:           // Left Control
        case KC_RCTL:           // Right Control
        case KC_LSFT:           // Left Shift
        case KC_RSFT:           // Right Shift
        case KC_LALT:           // Left Alt
        case KC_RALT:           // Right Alt
        case KC_LGUI:           // Left GUI
        case KC_RGUI:           // Right GUI
        case KC_LEFT:           // Flecha Izquierda
        case KC_RIGHT:          // Flecha Derecha
        case KC_UP:             // Flecha Arriba
        case KC_DOWN:           // Flecha Abajo
        case KC_HOME:           // Home
        case KC_END:            // End
        case KC_PGUP:           // Page Up
        case KC_PGDN:           // Page Down
        case KC_LBRC:           // Tilde "´"
            return true;

        default:
            return false;  // Deactivate Caps Word.
    }
}

// ==================================================================
//                          FIN DE CAPS WORD
// ==================================================================



// ==================================================================
//                               COMBOS
// ==================================================================

 enum combos{
     R_BSPC,
     R_BSPC_2,
     R_BSPC_3,
     L_BSPC,
     R_ESC,
     L_ESC,
     L_GUI,
     L_ALT,
     R_ENTER,
     R_ENTER_2,
    
     LR_CAPS,
     TG_SYMNUM,
     TG_ALPHA,

     RBOOT,
     LBOOT,
     };

     
 const uint16_t PROGMEM r_bspc_combo[]   = {TD(ARR), TD(TDU), COMBO_END};
 const uint16_t PROGMEM r_bspc_2_combo[] = {KC_9, KC_PMNS, COMBO_END};
 const uint16_t PROGMEM r_bspc_3_combo[] = {KC_END,  KC_RCTL, COMBO_END};
 const uint16_t PROGMEM l_bspc_combo[]   = {KC_P, KC_G, COMBO_END};
 const uint16_t PROGMEM r_esc_combo[]    = {KC_J, KC_L, COMBO_END};
 const uint16_t PROGMEM l_esc_combo[]    = {TD(RED), KC_W, COMBO_END};
 const uint16_t PROGMEM l_gui_combo[]    = {KC_H, TD(TDA), COMBO_END};
 const uint16_t PROGMEM l_alt_combo[]    = {TD(UND), TD(CUT), COMBO_END};
 const uint16_t PROGMEM r_enter_combo[]  = {KC_SLSH, TD(MAQ), COMBO_END};
 const uint16_t PROGMEM r_enter_2_combo[]= {KC_3, TD(DPT), COMBO_END};

 const uint16_t PROGMEM lr_caps_combo[]  = {RSF_TAB, KC_LSFT, COMBO_END};
 const uint16_t PROGMEM tg_symnum_combo[]= {TD(ARR), TD(TDO), COMBO_END};
 const uint16_t PROGMEM tg_alpha_combo[] = {KC_PMNS, TD(CAL), COMBO_END};
 const uint16_t PROGMEM RBOOT_combo[]    = {KC_VOLD, KC_MPRV, COMBO_END};
 const uint16_t PROGMEM LBOOT_combo[]    = {KC_F11, KC_F12, COMBO_END};

 combo_t key_combos[] = {
     [R_BSPC]        = COMBO(r_bspc_combo, KC_BSPC),
     [R_BSPC_2]      = COMBO(r_bspc_2_combo, KC_BSPC),
     [R_BSPC_3]      = COMBO(r_bspc_3_combo, KC_BSPC),
     [L_BSPC]        = COMBO(l_bspc_combo, KC_BSPC),
     [R_ESC]         = COMBO(r_esc_combo, KC_ESC),
     [L_ESC]         = COMBO(l_esc_combo, KC_ESC),
     [L_GUI]         = COMBO(l_gui_combo, KC_LGUI),
     [L_ALT]         = COMBO(l_alt_combo, KC_LALT),
     [R_ENTER]       = COMBO(r_enter_combo, KC_ENT),
     [R_ENTER_2]     = COMBO(r_enter_2_combo, KC_ENT),

     [LR_CAPS]       = COMBO(lr_caps_combo, KC_CAPS),
     [TG_SYMNUM]     = COMBO(tg_symnum_combo, TG(SYMNUM)),
     [TG_ALPHA]      = COMBO(tg_alpha_combo, TG(SYMNUM)),
     [RBOOT]         = COMBO(RBOOT_combo, QK_BOOT),
     [LBOOT]         = COMBO(LBOOT_combo, QK_BOOT),

     };

// ==================================================================
//                             FIN DE COMBOS
// ==================================================================



// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
 [ALPHA] = LAYOUT(
  // ╭─────────────────────────────────────────────╮ ╭───────────────────────────────────────────────╮
      TD(RED),  KC_W,   TD(SEA),  KC_P,    KC_G,         KC_J,   KC_L,  TD(TDI),  TD(TDU),  TD(ARR),
  // ├─────────────────────────────────────────────┤ ├───────────────────────────────────────────────┤
       KC_H,   TD(TDA),  HRM_R,  TD(TAB), TD(PAS),       KC_S,   HRM_N, TD(TDE),  HRM_D,    TD(TDO),
  // ├─────────────────────────────────────────────┤ ├─────────────────────────── ───────────────────┤
      TD(UND), TD(CUT),  KC_K,   TD(COP),   KC_B,      KC_SCLN,  KC_M,  TD(MEQ),  TD(MAQ),  KC_SLSH,
  // ╰─────────────────────────────────────────────┤ ├───────────────────────────────────────────────╯
                       CTL_DEL,  RSF_TAB,  FN_ENT,     SYM_NUM,  KC_LSFT,  FN_DEL
  //                 ╰─────────────────────────────╯ ╰─────────────────────────────╯
  ),

 [QWERTY] = LAYOUT(
  // ╭─────────────────────────────────────────────╮ ╭────────────────────────────────────────────────╮
        KC_Q,    KC_W,  TD(TDE),   KC_R,   KC_T,        KC_Y,  TD(TDU),  TD(TDI),  TD(TDO),    KC_P,
  // ├─────────────────────────────────────────────┤ ├────────────────────────────────────────────────┤
       TD(TDA),  KC_S,    KC_D,   KC_F,    KC_G,        KC_H,   HRM_J,     KC_K,     HRM_L,   KC_SCLN,
  // ├─────────────────────────────────────────────┤ ├────────────────────────────────────────────────┤
        KC_Z,    KC_X,    KC_C,   KC_V,    KC_B,        KC_N,    KC_M,   TD(MEQ),   TD(MAQ),  KC_SLSH,
  // ╰─────────────────────────────────────────────┤ ├────────────────────────────────────────────────╯
                         _______, _______, _______,    _______, _______, _______
  //                   ╰───────────────────────────╯ ╰───────────────────────────╯
  ),

 [SYMNUM] = LAYOUT(
  // ╭────────────────────────────────────────────────╮ ╭──────────────────────────────────────────────╮
        TD(EXC), TD(COM), S(KC_3), S(KC_4), S(KC_5),      KC_PPLS,  KC_7,    KC_8,    KC_9,   KC_PMNS,   
  // ├────────────────────────────────────────────────┤ ├──────────────────────────────────────────────┤
        S(KC_6), TD(SLS), TD(LPR), TD(RPR), S(KC_0),      KC_PAST,  KC_4,    KC_5,    KC_6,   TD(CAL),   
  // ├────────────────────────────────────────────────┤ ├──────────────────────────────────────────────┤
        TD(GRV), TD(APP),  LBRC,    RBRC,   TD(QST),        KC_0,   KC_1,    KC_2,    KC_3,   TD(DPT),    
  // ╰────────────────────────────────────────────────┤ ├──────────────────────────────────────────────╯
                           _______, _______, _______,     _______, _______, KC_0
  //                     ╰────────────────────────────╯ ╰────────────────────────╯
  ),

 [FN] = LAYOUT(
  // ╭────────────────────────────────────────────────╮ ╭─────────────────────────────────────────────╮
       KC_F1,    KC_F2,    KC_F3,    KC_F4,   KC_F5,      KC_MUTE, KC_HOME, KC_UP,   KC_END,  KC_RCTL,
  // ├────────────────────────────────────────────────┤ ├─────────────────────────────────────────────┤
       KC_F6,    KC_F7,    KC_F8,    KC_F9,   KC_F10,     KC_VOLU, KC_LEFT, KC_DOWN, KC_RGHT, KC_RSFT,
  // ├────────────────────────────────────────────────┤ ├─────────────────────────────────────────────┤
       XXXXXXX,  XXXXXXX,  XXXXXXX,  KC_F11,  KC_F12,     KC_VOLD, KC_MPRV, KC_MPLY, KC_MNXT, KC_ENT,
  // ╰────────────────────────────────────────────────┤ ├─────────────────────────────────────────────╯
                            _______, _______, _______,    _______, _______, _______ 
  //                      ╰───────────────────────────╯ ╰───────────────────────────╯
  ),

};
// clang-format on