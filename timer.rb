require 'gamer'
require 'time'

module Gamer
	class Timer
		attr_reader		:timed_thread
		attr_reader		:interval
	
		def initialize(time_interval)
			@mutex        = Mutex.new
			@running      = false
			self.interval = time_interval
		end
		
		def stop
			@running = false
		end
		
		def run(sync = true)
			if @running
				raise 'This thread is already running!'
			else
				@running = true
				@timed_thread = Thread.new do
					while(@running)
						if sync
							b = Time.new
							yield(self)
							e = Time.new
							time_diff = (e -b).to_f
							sleep(@interval - time_diff) unless(time_diff > @interval)
						else
							Thread.new do
								yield(self)
							end
							sleep(@interval)
						end
					end
				end
			end
		end
		
		def interval=(time_interval)
			@mutex.lock
			if(time_interval.is_a? Numeric)
				if(time_interval >= 0)
					@interval = time_interval
				else
					@mutex.unlock
					raise ArgumentError.new('the interval should be positive')
				end
			elsif(time_interval.respond_to?('to_s'))
				@interval = Timer.convert_interval(time_interval)
			else
				@mutex.unlock
				raise ArgumentError.new('the interval should either be a string or a number')
			end
			@mutex.unlock
		end
		
		def self.convert_interval(time_interval)
			time_string = time_interval.to_s
			retval = 0
			
			# Miliseconds
			time_string.scan(/([0-9]+)ms([ \t\n]|$)/) do
				retval += $1.to_f / 1000
			end
			# Seconds
			time_string.scan(/([0-9]+)s(ec(onds?)?)?([ \t\n]|$)/) do
				retval += $1.to_i
			end
			# Minutes
			time_string.scan(/([0-9]+)m(in(utes?)?)?([ \t\n]|$)/) do
				retval += $1.to_i * 60
			end
			
			return retval
		end
	end
end
