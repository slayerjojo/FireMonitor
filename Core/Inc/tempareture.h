#ifndef __TEMPARETURE_H__
#define __TEMPARETURE_H__

#include <stdint.h>

void tempareture_init(void);
void tempareture_update(void);

int16_t core_tempareture_get(void);

#endif
