require 'readline'
require 'socket'
include Socket::Constants

CMD_LIST = %w(host shost dhost src dst ether gateway net snet dnet mask port sport dport less greater ip proto)

LIST = (CMD_LIST + [
  'and', 'or', 'not',
  'help', 'quit', 'exit'
]).sort

def usage
  puts "Usage: #{File.basename($0)} -h [host] -p [port]"
  exit(-1)
end

def parse_cmd cmd
  args = cmd.split(/\s+/)
  cmds  = []
  cmd = []

  args.each do |arg|
    cmd << arg

    if %w(and or).include? arg
      cmds << {:cmd=>cmd[0..-2], :opt=>cmd[-1]}
      cmd = []
    end
  end
  cmds << {:cmd=>cmd}
  return cmds
end

'''
simple DFA with :s (start), :n (not), :a (and / or / bracket start), :c (cmd), :arg (args), :e (bracket end)
:s -> :a, :n, :c
:a -> :n, :c
:n -> :c
:c -> :c, :arg
:arg -> :c, :arg
:e -> exit
'''
def cmd_validation cmds
  state = :s
  cmds.each do |cmd|
    return false, cmd if state == :e
    if cmd[0] == "(" or %w(and or).include? cmd
      return false, cmd if state != :s
      state = :a
      if cmd[0] == "("
        while cmd[0] == "("
          cmd = cmd[1..-1]
        end
      else
        next
      end
    end

    if cmd == "not"
      return false, cmd if state != :s or state != :a
      state = :n
      next
    end

    if cmd[-1] == ")"
      state = :e
      next
    end

    if CMD_LIST.include? cmd
      state = :c
    else
      return false, cmd if state != :c and state != :arg
      state = :arg
    end
  end
  return true, nil
end

def send cmds, socket
  br_num = 0
  cmds.each do |cmd|
    br_num += cmd[:cmd][0].count("(")
    br_num -= cmd[:cmd][-1].count(")")
  end
  puts "unmatched bracket" if br_num != 0

  cmds.each do |cmd|
    is_valid, error_at = cmd_validation cmd[:cmd]
    puts "#{error_at}: not valid command" unless is_valid
    socket.write(cmd)
  end
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

port, host = nil
port = ARGV[ARGV.index("-p")+1] if ARGV.include? "-p"
host = ARGV[ARGV.index("-h")+1] if ARGV.include? "-h"

usage if port.nil? or host.nil?

socket = Socket.new( AF_INET, SOCK_STREAM, IPPROTO_TCP )
sockaddr = Socket.pack_sockaddr_in( port, host )
socket.connect(sockaddr)

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
        cmds = parse_cmd(cmd)
        send cmds, socket
      end
    end
  end
rescue Interrupt => e
  puts "interrput occured. exit flosis query client"
end
socket.close
