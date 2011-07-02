#ifndef VECTOR_H
#define VECTOR_H

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

  vector_t operator-(const vector_t& rhs) const {
    return vector_t(x - rhs.x, y - rhs.y);
  }
};

#endif
