# HydraLink kernel module

This allows you to use HydraLink without the python utility, configuring it using ethtool instead.

## Download sources and prerequisites
```bash
# Install prerequisites (ubuntu, may be different on other distros)
sudo apt install linux-headers-$(uname -r) build-essential

# Clone the project
git clone https://github.com/dissecto-GmbH/hydralink-kernel-module.git
cd hydralink-kernel-module

# Checkout the appropriate branch for your kernel
git checkout release-v6.8
```

The `release` branch is compatible with updated versions of the Linux kernel (6.11).

If you are using an older kernel, checkout the appropriate branch:

```bash
git checkout release-v6.11
git checkout release-v6.8
```

## Build
```bash
# Compile the kernel modules
make
cd phy_driver
make
cd ..
```

## Usage

```bash
# Remove existing modules, in case they were already loaded
sudo rmmod lan78xx
sudo rmmod hydralink
sudo rmmod bcm89881
# Load the modules we just compile
sudo insmod phy_driver/bcm89881.ko
sudo insmod hydralink.ko
```

On kernels where lan78xx can not be unloaded (for example, on Raspberry Pi OS), you will have to unbind the USB device from the lan78xx driver and bind it to hydralink.
First, use `ls` to find the USB device ID (`1-2:1.0` in the following example), then use the `unbind` `bind`.

Example:
```plain
$ ls /sys/bus/usb/drivers/lan78xx/
1-2:1.0  bind  module  new_id  remove_id  uevent  unbind
$ echo 1-2:1.0 | sudo tee /sys/bus/usb/drivers/lan78xx/unbind
1-2:1.0
$ echo 1-2:1.0 | sudo tee /sys/bus/usb/drivers/hydralink/bind
1-2:1.0
```

## Configuration

Configuration is done with `ethtool`:
```bash
# Example 1: 1Gbps master
sudo ethtool -s eth0 master-slave forced-master speed 1000
# Example 2: 100 Mbps slave
sudo ethtool -s eth0 master-slave forced-slave speed 100
```
