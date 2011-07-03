#ifndef VECTOR_H
#define VECTOR_H

#include <math.h>

struct vector_t {
  float x;
  float y;

  vector_t()
  : x(0)
  , y(0){

  }

  vector_t(float x, float y)
  : x(x)
  , y(y){

  }

  vector_t(const vector_t &v) 
  : x(v.x)
  , y(v.y) {
  }

  vector_t operator-(const vector_t& rhs) const {
    return vector_t(x - rhs.x, y - rhs.y);
  }

  vector_t &operator=(const vector_t &other) {
	x = other.x;
	y = other.y;
	return *(this);
  }

  vector_t operator*(const float m) {
	return vector_t(x*m, y*m);
  }

  float norm() {
		return sqrt(x*x+y*y);
  }
};

#endif
