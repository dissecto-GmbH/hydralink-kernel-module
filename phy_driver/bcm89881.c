#include <linux/ethtool.h>
#include <linux/module.h>
#include <linux/phy.h>

#define PHY_ID_BCM89881 (0xae025032)

static int bcm89881_read_status(struct phy_device *phydev)
{
	int ret = 0;

	phydev->link = 1;

	phydev->speed = 1000;
	phydev->duplex = DUPLEX_UNKNOWN;
	phydev->pause = 0;
	phydev->asym_pause = 0;

	ret = genphy_c45_read_pma(phydev);

	linkmode_set_bit(ETHTOOL_LINK_MODE_MII_BIT, phydev->supported);
	phydev->port = PORT_MII;

	phydev->master_slave_state = MASTER_SLAVE_STATE_UNKNOWN;
	phydev->master_slave_set = MASTER_SLAVE_CFG_UNKNOWN;

	return ret;
}

static int modify(struct phy_device *phydev, int devad, int addr, int set,
		  int clr)
{
	int reg = phy_read_mmd(phydev, devad, addr);
	int old = reg;

	reg &= (~clr) & 0xffff;
	reg |= set;

	if (old != reg)
		phy_write_mmd(phydev, devad, addr, reg);

	return reg;
}

static int bcm89881_config_init(struct phy_device *phydev)
{
	phydev->autoneg = AUTONEG_DISABLE;

	return 0;
}

static int bcm89881_match_phy_device(struct phy_device *phydev)
{
	return phydev->c45_ids.device_ids[1] == PHY_ID_BCM89881;
}

static int bcm89881_config_aneg(struct phy_device *phydev)
{
	modify(phydev, 1, 0x0000, 0x8000, 0x0000);

	if (phydev->autoneg == AUTONEG_DISABLE) {
		modify(phydev, 1, 0x0000, 0x8000, 0x0000);

		switch (phydev->speed) {
		case SPEED_100:
			modify(phydev, 1, 0x0000, 0x0040, 0x2000);
			modify(phydev, 1, 0x0834, 0x0000, 0x000f);
			break;
		case SPEED_1000:
			modify(phydev, 1, 0x0000, 0x2000, 0x0040);
			modify(phydev, 1, 0x0834, 0x0001, 0x000e);
			break;
		default:
			return -EINVAL;
		}

		switch (phydev->master_slave_set) {
		case MASTER_SLAVE_CFG_MASTER_PREFERRED:
		case MASTER_SLAVE_CFG_MASTER_FORCE:
			modify(phydev, 1, 0x0834, 0x4000, 0x0000);
			break;
		case MASTER_SLAVE_CFG_SLAVE_FORCE:
		case MASTER_SLAVE_CFG_SLAVE_PREFERRED:
			modify(phydev, 1, 0x0834, 0x0000, 0x4000);
			break;
		case MASTER_SLAVE_CFG_UNKNOWN:
			break;
		default:
			phydev_warn(phydev, "Unsupported Master/Slave mode\n");
			return -EOPNOTSUPP;
		}

		modify(phydev, 1, 0x0000, 0x0000, 0x8000);

		phy_write_mmd(phydev, 1, 0xa010, 0x0001);

		// set leds
		phy_write_mmd(phydev, 1, 0x931e, 0x0010);
		phy_write_mmd(phydev, 1, 0x931d, 0x0063);
	}
	return 0;
}

static struct phy_driver bcm89881_driver[] = { {
	.phy_id = PHY_ID_BCM89881,
	.phy_id_mask = 0xfffffff0,
	.name = "Broadcom BCM89881",
	.read_status = bcm89881_read_status,
	.match_phy_device = bcm89881_match_phy_device,
	.config_aneg = bcm89881_config_aneg,
	.get_features = genphy_c45_pma_read_abilities,
	.config_init = bcm89881_config_init,
} };

module_phy_driver(bcm89881_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Broadcom BCM89881 PHY driver");
