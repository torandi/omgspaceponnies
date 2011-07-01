#include "texture.h"

#include <SDL.h>
#include <SDL_image.h>
#include <GL/gl.h>
#include <cassert>

Texture::Texture(const char* path, int nhorizontal, int nvertical):
	Resource(path),
	_filename(path),
	_texture(-1),
	_width(0),
	_height(0),
	_nhorizontal(nhorizontal),
	_nvertical(nvertical){

	load_texture();
}
Texture::~Texture(){
	free_texture();
}

int Texture::width() const {
	return _width;
}

int Texture::height() const {
	return _height;
}

int Texture::count() const {
	return _nhorizontal * _nvertical;
}

void Texture::bind() const {
	glBindTexture(GL_TEXTURE_2D, _texture);
}

void Texture::unbind() const {
	glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::texcoord_t Texture::index_to_texcoord(int i) const {
	assert(i < count());

	texcoord_t t;

	// Convert 1D index to 2D position
	int x = i % _nhorizontal;
	int y = i / _nhorizontal;

	// Calculate size of each tile
	float w = 1.0f / _nhorizontal;
	float h = 1.0f / _nvertical;

	t.a[0] = x * w;	t.a[1] = y * h;
	t.b[0] = x * w; t.b[1] = y * h + h;
	t.c[0] = x * w+w; t.c[1] = y * h +h;
	t.d[0] = x * w+w; t.d[1] = y * h;

	return t;
}

void Texture::reload(){
	free_texture();
	load_texture();
}

void Texture::load_texture() {
	/* Load image using SDL Image */
	SDL_Surface* surface = IMG_Load(_filename);
	if ( !surface ){
	  fprintf(stderr, "Failed to load texture at %s\n", _filename);
	  exit(1);
	}

	/* To properly support all formats the surface must be copied to a new
	 * surface with a prespecified pixel format suitable for opengl.
	 *
	 * This snippet is a slightly modified version of code posted by
	 * Sam Lantinga to the SDL mailinglist at Sep 11 2002.
	 */
	SDL_Surface* rgba_surface = SDL_CreateRGBSurface(
			SDL_SWSURFACE,
			surface->w, surface->h,
			32,
#if SDL_BYTEORDER == SDL_LIL_ENDIAN /* OpenGL RGBA masks */
			0x000000FF,
			0x0000FF00,
			0x00FF0000,
			0xFF000000
#else
			0xFF000000,
			0x00FF0000,
			0x0000FF00,
			0x000000FF
#endif
	);

	if ( !rgba_surface ) {
	  fprintf(stderr, "Failed to create RGBA surface\n");
	  exit(1);
	}

	/* Save the alpha blending attributes */
	Uint32 saved_flags = surface->flags&(SDL_SRCALPHA|SDL_RLEACCELOK);
	Uint8 saved_alpha = surface->format->alpha;
	if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
		SDL_SetAlpha(surface, 0, 0);
	}

	SDL_BlitSurface(surface, 0, rgba_surface, 0);

	/* Restore the alpha blending attributes */
	if ( (saved_flags & SDL_SRCALPHA) == SDL_SRCALPHA ) {
		SDL_SetAlpha(surface, saved_flags, saved_alpha);
	}

	/* Generate texture and copy pixels to it */
	glGenTextures(1, &_texture);
	glBindTexture(GL_TEXTURE_2D, _texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba_surface->w, rgba_surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_surface->pixels );

	/* The size of the texture is considered as the size of a tile */
	_width = rgba_surface->w / _nhorizontal;
	_height = rgba_surface->h / _nvertical;

	SDL_FreeSurface(rgba_surface);
	SDL_FreeSurface(surface);
}

void Texture::free_texture(){
	glDeleteTextures(1, &_texture);
}
