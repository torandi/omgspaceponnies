#ifndef RENDER_OBJECT_H
#define RENDER_OBJECT_H

#include "render.h"
#include "vector.h"

struct RenderObject {
	animation_t anim;
	vector_t size;

	RenderObject();
	RenderObject(const char * sprite, unsigned int frames, unsigned int fps, const vector_t &s);

	void render(double dt);
};

#endif
