#ifndef GAMER_IMAGE_H
#define GAMER_IMAGE_H

#include <ruby.h>
#include <SDL/SDL.h>

typedef struct image_s
{
	SDL_Surface	*image;
	VALUE		mutex;
} image_t;

void Init_image(VALUE module);

VALUE image_allocate(VALUE klass);
void image_free(image_t *img);
void image_mark(image_t *img);

VALUE image_initialize(VALUE object, VALUE filename);

VALUE image_get_width(VALUE object);
VALUE image_get_height(VALUE object);

VALUE image_draw(VALUE object);

#endif
