#!/usr/bin/env ruby
require 'rubygems'
require 'oml4r'

#defining measurement point
class RTT < OML4R::MPBase
    name :rtt_stats
    param :time_taken, :type => :double
    param :units, :type => :string
end

def start()
end

#parsing the output file
def processOutput(row)
    column = row.split(" ")
    RTT.inject("#{column[[0]}", "#{column[1]}")
end

def readRTT()
    row = @pingio.readline
    processOutput(row)
end 

begin 


end
