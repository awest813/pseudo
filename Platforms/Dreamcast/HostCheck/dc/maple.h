/* Host-check stub of KallistiOS <dc/maple.h>. */
#ifndef PSEUDO_HOSTCHECK_DC_MAPLE_H
#define PSEUDO_HOSTCHECK_DC_MAPLE_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAPLE_FUNC_CONTROLLER 0x01000000

typedef struct maple_device maple_device_t;

maple_device_t *maple_enum_type(int n, unsigned int func);
void *maple_dev_status(maple_device_t *dev);

#ifdef __cplusplus
}
#endif

#endif
