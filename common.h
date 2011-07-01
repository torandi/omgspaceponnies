#ifndef COMMON_H
#define COMMON_H

#include <cstdio>
#include <stdint.h>

/**
 * Current animation step.
 * 0.0 -> 1.0
 */
extern float step;

/**
 * Port to broadcast on
 */
extern int port;


extern FILE* verbose;

#endif /* COMMON_H */
