# HydraLink kernel module

This allows you to use HydraLink without the python utility, configuring it using ethtool instead.

## Compilation
```bash
sudo apt install linux-headers-($uname -r)
git clone https://github.com/dissecto-GmbH/usb2ae-kernel-module.git
cd usb2ae-kernel-module
make
cd phy_driver
make
cd ..
```

## Usage

```bash
sudo rmmod lan78xx
sudo rmmod lan78xx_mod
sudo rmmod bcm89881
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
