require 'readline'
require 'time'
require 'ipaddr'
require 'socket'
include Socket::Constants

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

class Hash
	def to_arg
		return self.to_s
	end
end

def usage
	puts "Usage: #{File.basename($0)} -h [host] -p [port]"
	exit(-1)
end

def help_message
return string = <<-HELP
flosis query client documents
commands
help : print help
exit / quit : exit program
HELP
end

def connect_server
	port, host = nil
	port = ARGV[ARGV.index("-p")+1] if ARGV.include? "-p"
	host = ARGV[ARGV.index("-h")+1] if ARGV.include? "-h"

	usage if port.nil? or host.nil?

	socket = Socket.new( AF_INET, SOCK_STREAM, IPPROTO_TCP )
	sockaddr = Socket.pack_sockaddr_in( port, host )
	socket.connect(sockaddr)

	return socket
end