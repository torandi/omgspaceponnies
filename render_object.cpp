#include "render_object.h"
#include "render.h"
#include "texture.h"

RenderObject::RenderObject() {
	size = vector_t(0,0);
}

RenderObject::RenderObject(const char * sprite, unsigned int frames, unsigned int fps, const vector_t &s) : size(s) {
	anim  = load_anim(sprite, frames, fps);
}

void RenderObject::render(double dt) {
	if(size.x > 0 || size.y > 0) {
		glColor3f(texture_colors[0], texture_colors[1], texture_colors[2]);
		Texture::texcoord_t tc = prepare_animation(&anim, dt);
		glBegin(GL_QUADS);
			glTexCoord2f(tc.a[0],1-tc.a[1]); glVertex2f(0,size.y);
			glTexCoord2f(tc.b[0],1-tc.b[1]); glVertex2f(0,0);
			glTexCoord2f(tc.c[0],1-tc.c[1]); glVertex2f(size.x,0);
			glTexCoord2f(tc.d[0],1-tc.d[1]); glVertex2f(size.x,size.y);
		glEnd();
	}
}
