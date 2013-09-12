require 'gamer.so'
require 'timer'

module Gamer
	def error_print(e)
		$stderr.print "#{e.class}: #{e.to_s}\n"
		e.backtrace.each do |trace|
			puts "\t#{trace}\n"
		end
	end

	class Surface
		def wait_for_event
			while(@get_events)
				while(@get_events)
					event = poll_for_event
					if(event)
						yield event
					else
						break
					end
				end
				#Thread.pass # -> Uses all available CPU time
				# This has an acceptable response time, and prevents
				# the thread from using up all CPU time when idle.
				sleep(0.001)
			end
		end
		
		def start_main_loop
			unless(@main_loop_started)
				@main_thread = Thread.new do
					@main_loop_started 	= true
					@get_events 		= true
					wait_for_event do |event|
						case event.type
							when KEYDOWN
								if(listener.respond_to? 'key_down')
									begin
										listener.key_down event.keycode, event.unicode
									rescue Exception => e
										error_print e
									end
								end
							when KEYUP
								if(listener.respond_to? 'key_up')
									begin
										listener.key_up event.keycode, event.unicode
									rescue Exception => e
										error_print e
									end
								end
							when MOUSEDOWN
								if(listener.respond_to? 'mouse_down')
									begin
										button = 0
										[1..10] do |i|
											if event.mouse_button? i
												button = i
												break
											end
										end
										listener.mouse_down button, event.mouse_x, event.mouse_y
									rescue Exception => e
										error_print e
									end
								end
							when MOUSEUP
								if(listener.respond_to? 'mouse_up')
									begin
										button = 0
										[1..10] do |i|
											if event.mouse_button? i
												button = i
												break
											end
										end
										listener.mouse_up button, event.mouse_x, event.mouse_y
									rescue Exception => e
										error_print e
									end
								end
							when MOUSEMOVE
								if(listener.respond_to? 'mouse_moved')
									begin
										button = 0
										[1..10] do |i|
											if event.mouse_button? i
												button = i
												break
											end
										end
										listener.mouse_down button, event.mouse_x, event.mouse_y
									rescue Exception => e
										error_print e
									end
								end
							when QUIT
								if(listener.respond_to? 'quit')
									# Stop getting events if listener.quit returns true
									begin
										@get_events = false if listener.quit
									rescue Exception => e
										error_print e
										@get_events = false
									end
								else # If no quit method is available, we quit anyway
									@get_events = false
								end
								close unless @get_events
							when EXPOSE
								if(listener.respond_to? 'surface_exposed')
									begin
										listener.surface_exposed
									rescue Exception => e
										error_print e
									end
								end
							when RESIZE
								if(listener.respond_to? 'surface_resized')
									begin
										listener.surface_resized width, height
									rescue Exception => e
										error_print e
									end
								end
							when ACTIVE
								if(listener.respond_to? 'surface_activated')
									begin
										listener.surface_resized width, height
									rescue Exception => e
										#error_print e
									end
								end
						end
					end
					@main_loop_started = false
				end
				return @main_thread
			end
		end
		
		attr_reader :main_thread
	end
end
