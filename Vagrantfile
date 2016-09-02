# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|
  # Every Vagrant development environment requires a box. You can search for
  # boxes at https://atlas.hashicorp.com/search.
  config.vm.box = "ubuntu/trusty64"

  # Create a private network, which allows host-only access to the machine
  # using a specific IP.
  config.vm.network "private_network", ip: "192.168.137.77"

  config.push.define "local-exec" do |push|
    push.inline = <<-SCRIPT
      vagrant ssh -c 'cd /vagrant && make clean all'
    SCRIPT
  end
end
