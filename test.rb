#!/usr/bin/env ruby
require 'gamer'
include Gamer

# Load an image
img = Image.new('cube.png')

s = Surface.new('GameR test', 640, 480)
t = Timer.new('0ms')
s.show

x, y = 10, 10
xinc, yinc = 3, 2

t.run do |timer|
	s.draw do |gfx|
		# Clear the entire window
		gfx.clear
		gfx.draw_line(0,0,s.width,s.height)
		# Bright red, no green, no blue, 100% opaque
		gfx.color = 0xFF0000FF
		gfx.fill_rect(100, 100, 320, 240)
		# Bright blue, a bit transparent
		gfx.color = 0x0000FF99
		gfx.fill_circle(320,240,100)
		# Bright green
		gfx.color = 0x00FF00FF
		gfx.fill_polygon([10,20,30],[10,100,50])
		gfx.draw_image(img, x, y)
		
		# Change the direction when needed
		yinc = -yinc if y >= s.height-img.height || y <= 0
		xinc = -xinc if x >= s.width-img.width || x <= 0
		
		# Move the cube
		y += yinc; x += xinc
	end
end

# Wait for the main thread to end
s.main_thread.join
