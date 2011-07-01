#ifndef TEXTURE_H
#define TEXTURE_H

#include "resource.h"

class Texture: public Resource {
	public:
		Texture(const char* path, int nhorizontal = 1, int nvertical = 1);
		~Texture();

		int width() const;
		int height() const;
		int count() const;

		void bind() const;
		void unbind() const;

		struct texcoord_t {
			float a[2];
			float b[2];
			float c[2];
			float d[2];
		};

		texcoord_t index_to_texcoord(int i) const;

		virtual void reload();

	private:
		void load_texture();
		void free_texture();

		const char* _filename;
		unsigned int _texture;
		int _width;
		int _height;
		int _nhorizontal;
		int _nvertical;
};

#endif /* TEXTURE_H */
