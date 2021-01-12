# Heartbeat
Heart rate monitoring layer for Yocto.  
Adapted by Lorenzo Chiola.


## Adding the meta-heartbeat layer to your build
- Clone this repository in your Yocto folder:  
	cd (...)/poky  
	git clone https://github.com/LorenzoChiola/meta-heartbeat.git meta-heartbeat  
- Use bitbake-layers to add this layer to your build's bblayers.conf file.  
	bitbake-layers add-layer meta-heartbeat  
- Add the following lines to your build's local.conf file, to allow the inclusion in your build:  
	IMAGE_INSTALL_append = " heartbeat"  
	IMAGE_INSTALL_append = " heartmonkmod"  
	KERNEL_MODULE_AUTOLOAD += "heartmonkmod"  
- Currently tested only on qemu, with the following in local.conf:  
	MACHINE ?= "qemuarm"  


## Architecture
The application is composed of a kernel module, heartmonkmod, that collects data from hardware sensors, and a userspace application, heartbeat, which deduces the heart rate in bpm from the data colleccted by heartmonkmod.

### heartmonkmod
This module currently presents one device (/dev/heartmon0) as a character device generating predefined data at the rate of 50 per second. Every call to read() will get the next datapoint (after a wait, if no data is ready) as an integer in ASCII format, followed by a newline.  
  
The layer is configured to load this module at boot if you add the KERNEL_MODULE_AUTOLOAD line to your build's local.conf file (see above).  
  
The device file is created automatically by the module.  

### heartbeat
This userspace application reads 2048 datapoints from the character device presented by heartmonkmod.  
The heart rate (displayed in bpm) is extracted from the data using an FFT transform.  
Note: a heart rate reading is output every 2048 * (1 / 50 Hz) = 41 seconds.  
  
The default device file is "/dev/heartmon0"; this can be changed entering the path of another file as the only command line parameter of heartbeat.  
  
Exit with Control-C .
