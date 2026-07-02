/* Host-check stub of KallistiOS <kos.h>.
 *
 * Provides just enough of the KOS API for PSeudo's Dreamcast sources to
 * compile and link on a PC (see the host branch of ../Makefile).
 */
#ifndef PSEUDO_HOSTCHECK_KOS_H
#define PSEUDO_HOSTCHECK_KOS_H

#include <stdio.h>
#include <stdlib.h>

#include <dc/maple.h>
#include <dc/maple/controller.h>

#ifdef __cplusplus
extern "C" {
#endif

#define INIT_DEFAULT 0

#define KOS_INIT_FLAGS(flags) \
    unsigned int __kos_init_flags __attribute__((unused)) = (flags)

/* Threads */
typedef struct kthread kthread_t;

kthread_t *thd_create(int detach, void *(*routine)(void *), void *param);
int thd_join(kthread_t *thd, void **value_ptr);
void thd_pass(void);

#ifdef __cplusplus
}
#endif

#endif
