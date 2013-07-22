require 'readline'

LIST = [
  'quit', 'exit'
].sort

def parse_cmd cmd
	cmd_split = cmd.split(" and ")
	cmd_list  = []
	i = 0
	while not cmd_split[i].nil?
		cmd_opts = cmd_split[i].split(" ")
		if cmd_opts.length != 2
			puts "flosis: command not found: #{cmd_opts[0]}"
			return
		end
		cmd_list << cmd_opts
		i = i+1
	end
	puts cmd_list
end

def readline_with_history line_num
  cmd = Readline.readline("flosis(query):%03d> " % line_num, true)
  return nil if cmd.nil?
  if cmd =~ /^\s*$/ or Readline::HISTORY.to_a[-2] == cmd
    Readline::HISTORY.pop
  end
  cmd
end

puts "==========================================="
puts "========    flosis query client    ========"
puts "==========================================="
puts ""

i = 0

comp = proc { |s| LIST.grep( /^#{Regexp.escape(s)}/ ) }
Readline.completion_append_character = " "
Readline.completion_proc = comp

begin
  while cmd = readline_with_history(i)
    if cmd[0] != "#"
      i = i+1
      case cmd
        when /^\s*quit\s*/
        break;
      when /^\s*exit\s*/
        break;
      else
        parse_cmd cmd
      end
    end
  end
rescue Interrupt => e
  puts "interrput occured. exit flosis query"
end
