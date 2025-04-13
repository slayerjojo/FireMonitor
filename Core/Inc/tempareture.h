#ifndef __TEMPARETURE_H__
#define __TEMPARETURE_H__

#include <stdint.h>

void tempareture_init(void);
void tempareture_update(void);

void tempareture_minmax_set(int16_t max, int16_t min);
int16_t tempareture_get(void);

#endif
