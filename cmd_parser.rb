require './utils.rb'
MAX_LINE = 1000

abort help_message unless ARGV.index("--help").nil?
socket = connect_server

puts <<WELCOME
===========================================
========    flosis query client    ========
===========================================

WELCOME

cmds = {}
begin
%w(stime etime src_ip dst_ip src_port dst_port bpf).each do |opt|
	line = Readline.readline("#{opt}: ", true)

	if line.strip.empty?
		cmd = -1
	else
		case opt.to_sym
		when :stime, :etime
			cmd = Time.parse(line).strftime("%s") #change format
		when :src_ip, :dst_ip
			ip = IPAddr.new(line)
			abort "only ipv4 is available" unless ip.ipv4?
			cmd = [ip.first, ip.last]
		when :src_port, :dst_port
			ports = line.split(/[\/,\s\-\.~]/).reject{|x| x.empty?}
			abort "invalid option" if ports.empty?
			ports.concat(ports) if ports.size == 1
			abort "too many ports" if ports.size > 2
			abort "only integer is available" unless ports.count{|x|x.match(/\d+/)} == ports.size
			abort "invalid port" unless (0..65535).include? line.to_i
			cmd = ports
		else
			cmd = line
		end
	end
	cmds[opt.to_sym] = cmd
end

socket.write(cmds.to_arg)

while (response = socket.recv MAX_LINE)
	abort "error" if response == "error" #TODO edit
	print response
end
rescue Interrupt
	puts "Interrupt"
rescue Exception => e
	puts e
end
