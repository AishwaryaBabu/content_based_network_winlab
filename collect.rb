#!/usr/bin/env ruby
require 'rubygems'
require 'oml4r'
require 'file-tail'

#defining measurement point
class RTT < OML4R::MPBase
    name :rtt
    param :time_taken, :type => :double
end

class RttWrapper
    def initialize(args)
     @addr = nil
            leftover = OML4R::init(args, :appName => 'rtt') { |argParser|
            argParser.banner = "Reports time taken to receive a requested content \n"
            argParser.on("-f", "--file_path ADDRESS", "Path where output is saved") {|address| @addr = address}
            argParser.on("-i","--interval INNUM","Interval to tail"){ |if_num| @if_num ="#{in_num.to_i()}" }
    }
            #This is basic error handling in case the user forgets to enter a file name, your app won't crash
    unless @addr !=nil
        raise "You did not specify path of file (-p option)"
    end
end
    #This is the main code which actually reads the file you are writing into
class MyFile < File
        include File::Tail
end
def start()
        log = MyFile.new("#{@addr}")
        log.interval = @in_num
        log.backward(1)
        puts "#{@in_num}"
        log.tail { |line| print line
            processOutput(line)
        }
end
    
    #parsing the output file
def processOutput(row)
    column = row.split(" ")
    RTT.inject(column[0])
    
    #RTT.inject(row) #If this does not work, uncomment the above two lines
end

end
#This is the method which actually runs your app
begin
    app = RttWrapper.new(ARGV)
    app.start() #calls the start API which begins reading the file with your measurements
    rescue SystemExit #This is required to recognize the command line equivalent of Ctrl+C
    rescue SignalException
    puts "RTTWrapper stopped."
    rescue Exception => ex
    puts "Error - When executing the wrapper application!"
    puts "Error - Type: #{ex.class}"
    puts "Error - Message: #{ex}\n\n"
    # Uncomment the next line to get more info on errors
    # puts "Trace - #{ex.backtrace.join("\n\t")}"
end
OML4R::close()
# Local Variables:
# mode:ruby
# End:
# vim: ft=ruby:sw=2

