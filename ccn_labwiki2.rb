defProperty('r1', "r1-ccn2", "Router 1")
defProperty('r2', "r2-ccn2", "Router 2")
defProperty('r3', "r3-ccn2", "Router 3")
defProperty('r4', "r4-ccn2", "Router 4")
defProperty('h1', "h1-ccn2", "Host 1")
defProperty('h2', "h2-ccn2", "Host 2")
defProperty('c1', "c1-ccn2", "Client 1")
defProperty('c2', "c2-ccn2", "Client 2")

#Flow statistics
defProperty('pathfile', "/users/ccn_ababu/Client/rtt.txt", "Path to file")

defApplication('rtt_stats') do |app|
  app.binary_path = '/usr/bin/ruby /users/ccn_ababu/collect.rb'
  app.defProperty('target', 'Address to rtt file', '-f', {:type => :string})
  app.defProperty("interval","Interval",'-i', {:type => :string})
  app.defMeasurement('rtt') do |m|
    m.defMetric(':time_taken', :double)
  end
end

#applications
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

#groups
defGroup('routers', property.r1, property.r2, property.r3, property.r4) do |node|
  node.addApplication("runRouter") do |app|
  end
end

defGroup('host1', property.h1) do |node|
  node.addApplication("runHost") do |app|
    app.setProperty('content_id_add', '1 2 3 6')
  end
end

defGroup('host2', property.h2) do |node|
  node.addApplication("runHost") do |app|
    app.setProperty('content_id_add', '1 4 5 2')
  end
end

defGroup('client1', property.c1) do |node|
  node.addApplication("runClient") do |app|
    app.setProperty('content_id_get', '2')
  end
  node.addApplication("rtt") do |app|
    app.setProperty('target', property.pathfile)
    app.measure('rtt', :samples => 1)
  end
end

defGroup('client2', property.c2) do |node|
  node.addApplication("runClient") do |app|
    app.setProperty('content_id_get', '3')
  end
  node.addApplication("rtt") do |app|
    app.setProperty('target', property.pathfile)
    app.measure('rtt', :samples => 1)
  end
end

#Define Graph:
defGraph 'RTT' do |g|
  g.ms('rtt').select(:oml_seq, :time_taken)
  g.caption "Round trip time"
  g.type 'line_chart3'  
  g.mapping :x_axis => :oml_seq, :y_axis => :time_taken
  g.xaxis :legend => 'OML sequence number'
  g.yaxis :legend => 'RTT [ms]', :ticks => {:format => 's'}
end

#Experiments start
onEvent(:ALL_UP_AND_INSTALLED) do |event|
  after 5 do
    info("Starting routers")
    group('routers').startApplications
  end

  after 15 do
    info("Starting hosts")
    group('host1').startApplications
    group('host2').startApplications
  end
  
    
  after 30 do
    info("Starting client2")
    group('client2').startApplications
  end
  
  after 40 do
    info("Starting client1")
    group('client1').startApplications
  end
  
  after 50 do
    info(" ----- Stopping all applications")
    allGroups.stopApplications
    info(" ------ Now exit the experiment")
    Experiment.done
  end
end

