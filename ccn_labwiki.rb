defProperty('r1', "r1-labZero", "Router 1")
defProperty('r2', "r2-labZero", "Router 2")
defProperty('r3', "r2-labZero", "Router 3")
defProperty('h1', "h1-labZero", "Host 1")
defProperty('h2', "h2-labZero", "Host 2")
defProperty('c1', "c1-labZero", "Client 1")
defProperty('c2', "c2-labZero", "Client 2")

defApplication('runRouter') do |app|
  app.description = "Runs router"
  app.binary_path = '/bin/bash /users/content_based_network/runRouter.sh'
end

defApplication('runHost') do |app|
  app.description = "Runs host"
  app.binary_path = '/bin/bash /users/content_based_network/runHost.sh'
  app.defProperty('content_id_add', "This is a string of content ids the node will host", :type=> :string)
end

defApplication('runClient') do |app|
  app.description = "Runs client"
  app.binary_path = '/bin/bash /users/content_based_network/runClient.sh'
  app.defProperty('content_id_get', "This is a string of content ids the node asks for", :type=> :string)
end

defGroup('routers', property.r1, property.r2, property.r3) do |node|
  node.addApplication("runRouter") do |app|
  end
end

defGroup('host1', property.h1) do |node|
  node.addApplication("runHost") do |app|
    app.setProperty('content_id_add', '1 2 3')
  end
end

defGroup('host2', property.h2) do |node|
  node.addApplication("runHost") do |app|
    app.setProperty('content_id_add', '1 4 5')
  end
end

defGroup('client1', property.c1) do |node|
  node.addApplication("runClient") do |app|
    app.setProperty('content_id_get', '2')
  end
end

defGroup('client2', property.c2) do |node|
  node.addApplication("runClient") do |app|
    app.setProperty('content_id_get', '1')
  end
end

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
  
  after 40 do
    info("Starting client1")
    group('client1').startApplications
  end

  after 60 do
    info("Starting client2")
    group('client2').startApplications
  end
  
  after 100 do
    info(" ----- Stopping all applications")
    allGroups.stopApplications
  end

  after 110 do
    info(" ------ Now exit the experiment")
    Experiment.done
  end
end
