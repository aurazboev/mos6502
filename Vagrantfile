Vagrant.configure("2") do |config|

  config.vm.box = "mrlesmithjr/fedora29-desktop"
  config.vm.synced_folder ".", "/home/vagrant/hawknest"
  config.vm.provision :shell, path: "https://gist.githubusercontent.com/khale/70ec9084752f998be71bedc978654723/raw/f235a43bac4196633c55ec1aed4458dbe28d2272/gistfile1.txt"

  config.vm.provider :libvirt do |libvirt|
    libvirt.features = ['acpi',  'gic version=\'3\'']
    libvirt.machine_type = "virt-6.0"

    libvirt.nvram = "/var/lib/libvirt/qemu/nvram/vagrant.fd"
    libvirt.loader = "/usr/share/edk2/aarch64/QEMU_EFI-silent-pflash.raw"

    libvirt.usb_controller :model => "nec-xhci"
    libvirt.input :type => "tablet", :bus => "usb"
    libvirt.graphics_type="none"
  end

  config.vm.provider "vmware_desktop" do |v|
    v.gui = true
  end

  config.vm.provider "virtualbox" do |v|
    v.gui = true
  end

end
