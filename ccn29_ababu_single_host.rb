defProperty('r1', "r1-ccn29", "Router 1")
defProperty('r2', "r2-ccn29", "Router 2")
defProperty('r3', "r3-ccn29", "Router 3")
defProperty('r4', "r4-ccn29", "Router 4")
defProperty('r5', "r5-ccn29", "Router 5")
defProperty('r6', "r6-ccn29", "Router 6")
defProperty('r7', "r7-ccn29", "Router 7")
defProperty('r8', "r8-ccn29", "Router 8")
defProperty('r9', "r9-ccn29", "Router 9")
defProperty('r10', "r10-ccn29", "Router 10")
defProperty('r11', "r11-ccn29", "Router 11")
defProperty('r12', "r12-ccn29", "Router 12")
defProperty('r13', "r13-ccn29", "Router 13")
defProperty('r14', "r14-ccn29", "Router 14")

defProperty('h1', "h1-ccn29", "Host 1")
defProperty('h2', "h2-ccn29", "Host 2")

defProperty('c1', "c1-ccn29", "Client 1")
defProperty('c2', "c2-ccn29", "Client 2")
defProperty('c3', "c3-ccn29", "Client 3")
defProperty('c4', "c4-ccn29", "Client 4")
defProperty('c5', "c5-ccn29", "Client 5")
defProperty('c6', "c6-ccn29", "Client 6")
defProperty('c7', "c7-ccn29", "Client 7")
defProperty('c8', "c8-ccn29", "Client 8")
defProperty('c9', "c9-ccn29", "Client 9")
defProperty('server', "server-ccn29", "Server to collect rtt")

#Needs to be edited according to the server public ip and username
defProperty('serverhostname', "ababu@143.215.216.193", "IP address of server for oml application")

defApplication('runRouter') do |app|
  app.description = "Runs router"
  app.binary_path = '/bin/bash /users/ccn_ababu/runRouter.sh'
end

defApplication('runHost') do |app|
  app.description = "Runs host"
  app.binary_path = '/bin/bash /users/ccn_ababu/runHost.sh'
  app.defProperty('content_id_add', "This is a string of content ids the node will host", nil, {:type => :string})
end

defApplication('runClient') do |app|
  app.description = "Runs client"
  app.binary_path = '/bin/bash /users/ccn_ababu/runClient.sh'
  app.defProperty('content_id_get', "This is a string of content ids the node asks for", nil, {:type => :string})
end

defApplication('sendToServer') do |app|
  app.description = "Sends rtt value to server"
  app.binary_path = '/bin/bash /users/ccn_ababu/sendToServer.sh'
  app.defProperty('server_hostname', "The public IP address (hostname) of the server", nil, {:type => :string})
end


#Flow statistics for Server to run
#ababu to be replaced with the username 
#defProperty('pathfile', "/users/ababu/rtt.txt", "Path to file")

=begin
defApplication('rtt') do |app|
  app.binary_path = '/users/ccn_ababu/collect.rb'
  app.defProperty('target', 'Address to rtt file', '-f', {:type => :string})
  app.defProperty("interval","Interval",'-i', {:type => :string})
  app.defMeasurement('rtt') do |m|
    m.defMetric(':time_taken', :double)
  end
end
=end

=begin
defGroup('Server', property.server) do |node|
  node.addApplication("rtt") do |app|
    app.setProperty('target', property.pathfile)
    app.measure('rtt', :samples => 1)
  end
end
=end

#Group1
defGroup('group1r', property.r1, property.r2, property.r3) do |node|
  node.addApplication("runRouter") do |app|
  end
end

defGroup('group1h', property.h1, property.h2) do |node|
  node.addApplication("runHost") do |app|
    app.setProperty('content_id_add', '1 2 3 4 5')
  end
end

defGroup('group1c', property.c1) do |node|
  node.addApplication("runClient") do |app|
    app.setProperty('content_id_get', '2')
  end
end

#Group2
defGroup('group2r', property.r4, property.r5, property.r6) do |node|
  node.addApplication("runRouter") do |app|
  end
end

defGroup('group2c', property.c2) do |node|
  node.addApplication("runClient") do |app|
    app.setProperty('content_id_get', '2')
  end
end

#Group3  
defGroup('group3c', property.c3) do |node|
  node.addApplication("runClient") do |app|
    app.setProperty('content_id_get', '2')
  end
end

#Group 4
defGroup('group4r', property.r7, property.r8) do |node|
  node.addApplication("runRouter") do |app|
  end
end

defGroup('group4c', property.c4) do |node|
  node.addApplication("runClient") do |app|
    app.setProperty('content_id_get', '2')
  end
end
  
#Group5
defGroup('group5c', property.c5) do |node|
  node.addApplication("runClient") do |app|
    app.setProperty('content_id_get', '2')
  end
end

#Group6
defGroup('group6r', property.r9, property.r10) do |node|
  node.addApplication("runRouter") do |app|
  end
end

defGroup('group6c', property.c6) do |node|
  node.addApplication("runClient") do |app|
    app.setProperty('content_id_get', '2')
  end
end
  
#Group7
defGroup('group7r', property.r11, property.r12) do |node|
  node.addApplication("runRouter") do |app|
  end
end

defGroup('group7c', property.c7) do |node|
  node.addApplication("runClient") do |app|
    app.setProperty('content_id_get', '2')
  end
end  
  
#Group8
defGroup('group8r', property.r13) do |node|
  node.addApplication("runRouter") do |app|
  end
end

defGroup('group8c', property.c8) do |node|
  node.addApplication("runClient") do |app|
    app.setProperty('content_id_get', '2')
  end
end
  
#Group9
defGroup('group9r', property.r14) do |node|
  node.addApplication("runRouter") do |app|
  end
end

defGroup('group9c', property.c9) do |node|
  node.addApplication("runClient") do |app|
    app.setProperty('content_id_get', '2')
  end
end
  
=begin
  node.addApplication("sendToServer") do |node|
    app.setProperty('server_hostname', property.serverhostname)
  end
=end

=begin
#Define Graph:
defGraph 'RTT' do |g|
  g.ms('rtt').select(:oml_seq, :time_taken)
  g.caption "Round trip time"
  g.type 'line_chart3'
  g.mapping :x_axis => :oml_seq, :y_axis => :time_taken
  g.xaxis :legend => 'OML sequence number'
  g.yaxis :legend => 'RTT [ms]', :ticks => {:format => 's'}
end
=end

onEvent(:ALL_UP_AND_INSTALLED) do |event|
#Group1
  after 2 do
    info(" ----- Group 1")
    group('group1r').startApplications
    group('group1h').startApplications
  end

  after 15 do
    group('group1c').startApplications
  end
  
#Group2
  after 20 do
    info(" ----- Group 2")
    group('group2r').startApplications
  end
  
  after 35 do
    group('group2c').startApplications
  end
  
#Group 3
  after 40 do
    info(" ----- Group 3")
    group('group3c').startApplications
  end
  
#Group 4
  after 45 do
    info(" ----- Group 4")
    group('group4r').startApplications
  end
  
  after 60 do
    group('group4c').startApplications
  end

#Group 5
  after 65 do
    info(" ----- Group 5")
    group('group5c').startApplications
  end  
  
#Group 6
  after 70 do
    info(" ----- Group 6")
    group('group6r').startApplications
  end
  
  after 90 do
    group('group6c').startApplications
  end  
  
#Group 7
  after 95 do
    info(" ----- Group 7")
    group('group7r').startApplications
  end
  
  after 115 do
    group('group7c').startApplications
  end
  
#Group 8
  after 120 do
    info(" ----- Group 8")
    group('group8r').startApplications
  end
  
  after 135 do
    group('group8c').startApplications
  end

#Group 9
  after 140 do
    info(" ----- Group 9")
    group('group9r').startApplications
  end
  
  after 160 do
    group('group9c').startApplications
  end  
  
  after 170 do
    info(" ----- Stopping all applications")
    allGroups.stopApplications
    info(" ------ Now exit the experiment")
    Experiment.done
  end
end

