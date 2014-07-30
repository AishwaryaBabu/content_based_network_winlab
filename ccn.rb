#define each router, host, client as properties
#structure: defProperty('name_of_resource', 'resource-sliceName', 'description of resource')
defProperty('h1', 'h1-labZero', 'Host 1')
defProperty('h2', 'h2-labZero', 'Host 2')
defProperty('c1', 'c1-labZero', 'Client 1')
defProperty('c2', 'c2-labZero', 'Client 2')
defProperty('r1', 'r1-labZero', 'Router 1')
defProperty('r2', 'r2-labZero', 'Router 2')
defProperty('r3', 'r3-labZero', 'Router 3')

defApplication('runRouter') do |app|
  
#  app.binary_path = "/usr/local/bin/runRouter.sh"
  app.binary_path = "/users/content_based_network/runRouter.sh"
  app.description = "Runs router" 
#  app.pkg_tarball = "https://dl.dropboxusercontent.com/u/98992183/geni/content_based_network.tar" 

=begin
  app.defProperty("my_first_param", "Some info about", "", :type => :string, :dynamic => false, :order => 1)
  app.defProperty("my_second_param", "Some info about", "", :type => :string, :dynamic => false, :order => 2)
    
  app.defMeasurement('myMeasurementPoint') do |mp|
    mp.defMetric('metric1',:int)
    mp.defMetric('metric2',:string)
  end
=end  
end

defApplication('runHost') do |app|
  
#  app.binary_path = "/usr/local/bin/runHost.sh"
  app.binary_path = "/users/content_based_network/runHost.sh"
  app.description = "Runs host" 
#  app.pkg_tarball = "https://dl.dropboxusercontent.com/u/98992183/geni/content_based_network.tar" 

  app.defProperty("content_id_add", "This is a  string of content ids the node hosts", :type => :string)
=begin
  app.defProperty("my_first_param", "Some info about", "", :type => :string, :dynamic => false, :order => 1)
  app.defProperty("my_second_param", "Some info about", "", :type => :string, :dynamic => false, :order => 2)
    
  app.defMeasurement('myMeasurementPoint') do |mp|
    mp.defMetric('metric1',:int)
    mp.defMetric('metric2',:string)
  end
=end  
end

defApplication('runClient') do |app|
  
# app.binary_path = "/usr/local/bin/runClient.sh"
  app.binary_path = "/users/content_based_network/runHost.sh"
  app.description = "Runs client" 
# app.pkg_tarball = "https://dl.dropboxusercontent.com/u/98992183/geni/content_based_network.tar" 
  app.defProperty("content_id_get", "This is a  string of content ids the node hosts", :type => :string)

=begin
  app.defProperty("my_first_param", "Some info about", "", :type => :string, :dynamic => false, :order => 1)
  app.defProperty("my_second_param", "Some info about", "", :type => :string, :dynamic => false, :order => 2)
    
  app.defMeasurement('myMeasurementPoint') do |mp|
    mp.defMetric('metric1',:int)
    mp.defMetric('metric2',:string)
  end
=end  
end


#resource_foo = array of resources   
defGroup('routers', property.r1, property.r2, property.r3) do |node|
  node.addApplication("runRouter") do |app|
#    app.setProperty('my_first_param', 'r1')
#    app.setProperty('my_second_param', 'r2')
#    app.measure('myMeasurementPoint', :sample =>1)
  end
end

#Each script needs varying number of arguments  
#run individual hosts and clients
defGroup('host1', property.h1) do |node|
  node.addApplication("runHost") do |app|
    app.setProperty('content_id_add', '1 2 3')
#   app.setProperty('my_second_param', 'r2')
#    app.measure('myMeasurementPoint', :sample =>1)
  end
end

defGroup('host2', property.h2) do |node|
  node.addApplication("runHost") do |app|
    app.setProperty('content_id_add', '1 4 5')
#    app.measure('myMeasurementPoint', :sample =>1)
  end
end


defGroup('client1', property.c1) do |node|
  node.addApplication("runClient") do |app|
    app.setProperty('content_id_get', '2')
#    app.setProperty('my_second_param', 'r2')
#    app.measure('myMeasurementPoint', :sample =>1)
  end
end

defGroup('client2', property.c2) do |node|
  node.addApplication("runClient") do |app|
    app.setProperty('content_id_get', '1')
#    app.measure('myMeasurementPoint', :sample =>1)
  end
end

onEvent(:ALL_UP_AND_INSTALL) do |node|
  
  info("Starting routers")
  group("routers").startApplications

  info("Starting hosts")
  group("host1").startApplications
  group("host2").startApplications

  after 35 do
  info("Starting client1")
  group("client1").startApplications

  after 45 do
  info("Starting client2")
  group("client2").startApplications  

  after 105 do
    info(" ----- Stopping all applications")
    allGroups.stopApplications
  end

  after 110 do
    info(" ------ Now exit the experiment")
    Experiment.done
  end

end
