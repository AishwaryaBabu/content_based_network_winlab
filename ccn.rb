defApplication('runRouter') do |app|
  
#change binary path
  app.binary_path = "/usr/local/bin/runRouter.sh"
  app.description = "Runs router" 
  app.pkg_tarball = "https://dl.dropboxusercontent.com/u/98992183/geni/content_based_network.tar" 

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
  
#change binary path
  app.binary_path = "/usr/local/bin/runHost.sh"
  app.description = "Runs host" 
  app.pkg_tarball = "https://dl.dropboxusercontent.com/u/98992183/geni/content_based_network.tar" 

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
  
#change binary path
  app.binary_path = "/usr/local/bin/runClient.sh"
  app.description = "Runs client" 
  app.pkg_tarball = "https://dl.dropboxusercontent.com/u/98992183/geni/content_based_network.tar" 

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
defGroup('routers', 'resource_foo') do |node|
  node.addApplication("runRouter") do |app|
    app.setProperty('my_first_param', 'r1')
    app.setProperty('my_second_param', 'r2')
    app.measure('myMeasurementPoint', :sample =>1)
  end
end
  
#run individual hosts and clients
defGroup('host', 'resource_foo') do |node|
  node.addApplication("myapp") do |app|
    app.setProperty('my_first_param', 'r1')
    app.setProperty('my_second_param', 'r2')
    app.measure('myMeasurementPoint', :sample =>1)
  end
end

defGroup('routers', 'resource_foo') do |node|
  node.addApplication("myapp") do |app|
    app.setProperty('my_first_param', 'r1')
    app.setProperty('my_second_param', 'r2')
    app.measure('myMeasurementPoint', :sample =>1)
  end
end

onEvent(:ALL_UP_AND_INSTALL) do |node|
  
  info("Starting routers")
  group("routers").startApplications

  info("Starting hosts")
  group("h1").startApplications

  after 60 do
    info(" ------ Now stop my app")
    group('video_group1').stopApplications
  end

  after 70 do
    info(" ------ Now exit the experiment")
    Experiment.done
  end

end
