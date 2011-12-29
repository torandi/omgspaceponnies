#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <vector>

#define REF_FPS 30
#define REF_DT (1.0/REF_FPS)

//#define PI 3.14159265

#define SPEED 300.0f

extern FILE* verbose;

extern bool IS_SERVER;

double curtime();
#endif /* COMMON_H */
