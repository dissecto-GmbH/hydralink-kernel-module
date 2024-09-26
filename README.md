# HydraLink kernel module

This allows you to use HydraLink without the python utility, configuring it using ethtool instead.

## Compilation
```bash
# Install prerequisites (ubuntu-specific, might be different on other distros)
sudo apt install linux-headers-$(uname -r)
# Clone the project
git clone https://github.com/dissecto-GmbH/hydralink-kernel-module.git
cd hydralink-kernel-module
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
sudo rmmod lan78xx_mod
sudo rmmod bcm89881
# Load the modules we just compile
sudo insmod phy_driver/bcm89881.ko
sudo insmod lan78xx_mod.ko
```


## Configuration

```bash
# Example 1: 1Gbps master
sudo ethtool -s eth0 master-slave forced-master speed 1000
# Example 2: 100 Mbps slave
sudo ethtool -s eth0 master-slave forced-slave speed 100
```
