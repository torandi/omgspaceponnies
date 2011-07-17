#ifndef VECTOR_H
#define VECTOR_H

#include <cmath>

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

  vector_t &operator+=(const vector_t &other) {
	x += other.x;
	y += other.y;
	return *(this);
  }

  vector_t operator+(const vector_t &other) const {
	return vector_t(x+other.x,y+other.y);
  }

  vector_t &operator-=(const vector_t &other) {
	x -= other.x;
	y -= other.y;
	return *(this);
  }

  vector_t operator*(const float m) const {
	return vector_t(x*m, y*m);
  }

  vector_t operator*(const vector_t &other) const {
	return vector_t(x*other.x, y*other.y);
  }

  vector_t operator/(const float f) const {
	return vector_t(x/f, y/f);
  }

  vector_t operator*=(const float m) {
	x *= m;
	y *= m;
	return *(this);
  }

  vector_t operator*=(const vector_t &other) {
	x *= other.x;
	y *= other.y;
	return *(this);
  }
  float norm() const {
		return std::sqrt(x*x+y*y);
  }

  vector_t normalized() const {
		return vector_t(x/norm(), y/norm());
  }

  vector_t abs() const{
		return vector_t(std::abs(x),std::abs(y));
  }

  vector_t rotate(double angle) const {
		return vector_t(x*std::cos(angle) - y*std::sin(angle), x*std::sin(angle) + y*std::cos(angle));
  }
};

#endif
