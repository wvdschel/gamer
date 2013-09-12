#!/usr/bin/env ruby
require 'mkmf'

abort("SDL library required")  unless have_library("SDL", "SDL_Init")
abort("SDL library headers required") unless have_header("SDL/SDL.h")
abort("SDL_gfx library required") unless have_library("SDL_gfx", "aalineColor")
abort("SDL_gfx library headers required") unless have_header("SDL/SDL_gfxPrimitives.h")
abort("SDL_image library required") unless have_library("SDL_image", "IMG_Load")
abort("SDL_image library headers required") unless have_header("SDL/SDL_image.h")

dir_config('gamer')
create_makefile('gamer')
