#ifndef COREMAP_H
#define COREMAP_H

#include "opt-paging.h"
#include <types.h>

#if OPT_PAGING

void free_frames_init(void);
int free_frames_alloc(void);

#endif

#endif