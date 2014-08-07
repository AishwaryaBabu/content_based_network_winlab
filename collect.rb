#!/usr/bin/env ruby
require 'rubygems'
require 'oml4r'
APPNAME = "rtt_stats"

#defining measurement point
class RTT < OML4R::MPBase
    name :rtt
    param :time_taken, :type => :double
    param :units, :type => :string
end

class rttWrapper
    def initialize(args)
    OML4R::init(args, :appname => "#{APPNAME}_wrapper", :domain => 'foo', :collect => 'file:-') { |argParser|
        argParser.banner = "Reports time taken  to receive a requested content \n"
        argParser.on("-f", "--file_path ADDRESS", "Path where output is saved") {|address| @addr = address}
    }
    end
end

#parsing the output file
def processOutput(row)
    column = row.split(" ")
    RTT.inject("#{column[[0]}", "#{column[1]}")
end

def start()
    row = @pingio.readline
    processOutput(row)
end 

begin 
    app = rttWrapper(ARGV) #ARGV - argument passed to the program : file name
    app.start()
end
OML4R::close()
