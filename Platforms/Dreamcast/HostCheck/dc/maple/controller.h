/* Host-check stub of KallistiOS <dc/maple/controller.h>.
 *
 * Note: cont_state_t.buttons is ACTIVE-HIGH (bit set = pressed), the
 * maple driver normalizes the active-low wire format before returning it.
 */
#ifndef PSEUDO_HOSTCHECK_DC_CONTROLLER_H
#define PSEUDO_HOSTCHECK_DC_CONTROLLER_H

#ifdef __cplusplus
extern "C" {
#endif

#define CONT_C          (1 << 0)
#define CONT_B          (1 << 1)
#define CONT_A          (1 << 2)
#define CONT_START      (1 << 3)
#define CONT_DPAD_UP    (1 << 4)
#define CONT_DPAD_DOWN  (1 << 5)
#define CONT_DPAD_LEFT  (1 << 6)
#define CONT_DPAD_RIGHT (1 << 7)
#define CONT_Z          (1 << 8)
#define CONT_Y          (1 << 9)
#define CONT_X          (1 << 10)
#define CONT_D          (1 << 11)

typedef struct {
    unsigned int buttons; /* active-high CONT_* bitmask */
    int ltrig;            /* 0-255 */
    int rtrig;            /* 0-255 */
    int joyx, joyy;
    int joy2x, joy2y;
} cont_state_t;

#ifdef __cplusplus
}
#endif

#endif
