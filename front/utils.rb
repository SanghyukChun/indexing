require 'readline'
require 'time'
require 'ipaddr'

class String
	def int?
		return self.match(/^\d+$/)
	end
end

class IPAddr
	def first
		return self.to_range.first.to_i
	end
	def last
		return self.to_range.last.to_i
	end
end
