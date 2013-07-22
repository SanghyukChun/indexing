require 'readline'

LIST = [
  'host', 'shost', 'dhost', 'src host', 'dst host',
  'ether src', 'ether dst', 'ether host', 'gateway',
  'net' 'snet', 'dnet', 'src net', 'dst net',
  'port', 'sport', 'dport', 'src port', 'dst port',
  'less', 'greater',
  'ip proto',
  'and', 'or', 'not',
  'help', 'quit', 'exit'
].sort

def err
  puts "unexpected command"
  return -1
end

def init_cmd
  return {:operation => nil, :not => false, :cmd => [], :arg =>nil}
end

def parse_cmd cmd
  args = cmd.split(/\s+/)
  cmds  = []
  cmd = []

  args.each do |arg|
    arg = arg.downcase
    cmd << arg

    if %w(and or).include? arg
      cmds << {:cmd=>cmd[0..-1], :opt=>cmd[-1]}
      cmd = []
    end
  end
  cmds << {:cmd=>cmd}
  send cmds
end

def send cmds
  puts cmds
end

def readline_with_history line_num
  cmd = Readline.readline("flosis(query):%03d> " % line_num, true)
  return nil if cmd.nil?
  if cmd =~ /^\s*$/ or Readline::HISTORY.to_a[-2] == cmd
    Readline::HISTORY.pop
  end
  cmd
end

def print_help
  puts "flosis query client documents"
  puts "commands"
  puts "help : print help"
  puts "exit / quit : exit program"
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
      when /^\s*help\s*/
        print_help
      else
        parse_cmd cmd
      end
    end
  end
rescue Interrupt => e
  puts "interrput occured. exit flosis query"
end
