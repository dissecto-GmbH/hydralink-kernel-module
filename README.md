# HydraLink kernel module

This allows you to use HydraLink without the python utility, configuring it using ethtool instead.

## Which branch should I use?

The out-of-tree module tracks upstream Linux API changes across several kernel generations. **You must pick the branch that matches the kernel you are building against** (the one from `uname -r`), not the newest branch.

### Check your kernel version

```bash
uname -r
# Example output: 6.18.33-1-lts
```

Use the **major.minor** part (e.g. `6.18` → use `release`; `6.12` → use `release-v6.11`; `6.8` → use `release-v6.8`).

### Pick the matching branch

| Your kernel (`uname -r`) | Branch to checkout | Typical environments |
|--------------------------|-------------------|----------------------|
| **6.15 and newer** (6.15, 6.16, 6.18, …) | `release` | Current Arch, Fedora, dissectos / Raspberry Pi OS on recent kernels |
| **6.11 – 6.14** | `release-v6.11` | Older rolling/stable distros still on 6.12 LTS |
| **6.8 – 6.10** | `release-v6.8` | Legacy LTS systems, older embedded images |

```bash
# After cloning, checkout exactly one branch:
git checkout release          # kernel 6.15+
# git checkout release-v6.11  # kernel 6.11 – 6.14
# git checkout release-v6.8   # kernel 6.8 – 6.10
```

### Quick examples

| `uname -r` | Command |
|------------|---------|
| `6.18.36-v8-16k-dissectos` | `git checkout release` |
| `6.12.27-1-lts` | `git checkout release-v6.11` |
| `6.8.0-58-generic` | `git checkout release-v6.8` |

## Download sources and prerequisites

```bash
# Install prerequisites (ubuntu, may be different on other distros)
sudo apt install linux-headers-$(uname -r) build-essential

# Clone the project
git clone https://github.com/dissecto-GmbH/hydralink-kernel-module.git
cd hydralink-kernel-module

# Checkout the branch for your kernel (see table above)
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
# Load the modules we just compiled
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
