#include <stdio.h>
#include <ruby.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "image.h"
#include "graphics.h"

VALUE gam_cImage;

void Init_image(VALUE module)
{
	gam_cImage = rb_define_class_under(module, "Image", rb_cObject);
	rb_define_alloc_func(gam_cImage, image_allocate);
	
	rb_define_method(gam_cImage, "initialize", image_initialize, 1);
	rb_define_method(gam_cImage, "draw", image_draw, 0);
	rb_define_method(gam_cImage, "width", image_get_width, 0);
	rb_define_method(gam_cImage, "height", image_get_height, 0);
}

VALUE image_allocate(VALUE klass)
{
	VALUE mtxClass;
	image_t *img = malloc(sizeof(image_t));
	
	img->image 	= NULL;
	
	mtxClass	= rb_const_get(rb_cObject, rb_intern("Mutex"));
	img->mutex	= rb_class_new_instance(0, NULL, mtxClass);
	
	return Data_Wrap_Struct(klass, image_mark, image_free, img);
}

void image_free(image_t *img)
{
	SDL_FreeSurface(img->image);
	free(img);
}

void image_mark(image_t *img)
{}

VALUE image_initialize(VALUE object, VALUE filename)
{
	image_t *img;
	SDL_Surface *image_surf;
	VALUE string;
	char *c_string;

	if(!rb_respond_to(object, rb_intern("to_s")))
		rb_raise(rb_eArgError, "filename should respond to to_s");
	
	string = rb_funcall(filename, rb_intern("to_s"), 0);
	c_string = (char*)StringValuePtr(string);
	
	image_surf = IMG_Load(c_string);
	
	if(!image_surf)
		rb_raise(rb_eRuntimeError, "error loading the image file, check Gamer.error");
	
	Data_Get_Struct(object, image_t, img);
	img->image = image_surf;
	
	return object;
}

VALUE image_get_width(VALUE object)
{
	image_t *img;
	
	Data_Get_Struct(object, image_t, img);
	
	return INT2NUM(img->image->w);
}

VALUE image_get_height(VALUE object)
{
	image_t *img;
	
	Data_Get_Struct(object, image_t, img);
	
	return INT2NUM(img->image->h);
}

VALUE image_draw(VALUE object)
{
	image_t *i;
	VALUE graph, graphClass;
	graphClass 	= rb_const_get(rb_const_get(rb_cModule, rb_intern("Gamer")), rb_intern("Graphics"));
	graph		= rb_class_new_instance(1, &object, graphClass);
	
	Data_Get_Struct(object, image_t, i);
	
	graphics_lock(graph);
	rb_yield(graph);
	graphics_unlock(graph);
	
	return Qnil;
}
