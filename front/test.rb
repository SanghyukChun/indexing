require './utils.rb'

puts <<WELCOME
===========================================
========    flosis query client    ========
===========================================

WELCOME

cmds = {}
%w(stime etime src_ip dst_ip src_port dst_port bpf).each do |opt|
	line = Readline.readline("#{opt}: ", true)
	case opt.to_sym
	when :stime, :etime
		cmd = Time.parse(line).strftime("%s") #change format
	when :src_ip, :dst_ip
		ip = IPAddr.new(line)
		abort "only ipv4 is available" unless ip.ipv4?
		cmd = [ip.first, ip.last]
	when :src_port, :dst_port
		abort "only integer is available" unless line.int?
		abort "invalid port" unless (0..65535).include? line.to_i
		cmd = line.to_i
	when :bpf
		abort "invalid query" unless line.valid_query?
		cmd = line
	else
		abort "wrong options"
	end
	cmds[opt.to_sym] = cmd
end

puts cmds.to_s