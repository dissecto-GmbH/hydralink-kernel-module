#include <linux/ethtool.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/mdio.h>

#define PHY_ID_BCM89881 (0xae025030)
#define PHY_ID_BCM89881_MASK (0xfffffff0)

static int bcm89881_read_status(struct phy_device *phydev)
{
	int ret = 0;

	phydev->duplex = DUPLEX_FULL;
	phydev->pause = 0;
	phydev->asym_pause = 0;

	ret = genphy_c45_read_pma(phydev);
	int pma_pmd_status = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_STAT1);
	int ieee_pma_pmd_baset1_control = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PMA_PMD_BT1_CTRL);

	phydev->speed = (ieee_pma_pmd_baset1_control & MDIO_PMA_PMD_BT1_CTRL_STRAP_B1000) == 0 ? SPEED_100 : SPEED_1000;
	phydev->link = (pma_pmd_status & BMSR_LSTATUS) ? 1 : 0;
	phydev->master_slave_state = (ieee_pma_pmd_baset1_control & MDIO_PMA_PMD_BT1_CTRL_CFG_MST) ? MASTER_SLAVE_STATE_MASTER : MASTER_SLAVE_STATE_SLAVE;

	linkmode_set_bit(ETHTOOL_LINK_MODE_MII_BIT, phydev->supported);
	phydev->port = PORT_MII;

	return 0;
}

static int bcm89881_config_init(struct phy_device *phydev)
{
	phydev->autoneg = AUTONEG_DISABLE;

	// RGMII delay and voltage settings
	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, 0xa010, 0x0001);
	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, 0xa015, 0x0000);

	// set leds
	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, 0xa027, 0x0f15);
	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, 0x931e, 0x0063);
	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, 0x931d, 0x0010);
	phydev_info(phydev, "Initialization done\n");

	// populate configuration for ethtool
	return genphy_c45_pma_baset1_read_master_slave(phydev);
}

static int bcm89881_config_aneg(struct phy_device *phydev)
{
	if (phydev->autoneg == AUTONEG_DISABLE) {
		switch (phydev->speed) {
		case SPEED_100:
			phy_modify_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_CTRL1, BMCR_SPEED100 | BMCR_SPEED1000, BMCR_SPEED100);
			break;
		case SPEED_1000:
			phy_modify_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_CTRL1, BMCR_SPEED1000 | BMCR_SPEED100, BMCR_SPEED1000);
			break;
		default:
			return -EINVAL;
		}
	}

	genphy_c45_pma_baset1_setup_master_slave(phydev);
	// repopulate configuration for ethtool
	return genphy_c45_pma_baset1_read_master_slave(phydev);
}

static struct phy_driver bcm89881_driver[] = { {
	PHY_ID_MATCH_MODEL(PHY_ID_BCM89881),
	.name = "Broadcom BCM89881",
	.read_status = bcm89881_read_status,
	.config_aneg = bcm89881_config_aneg,
	.get_features = genphy_c45_pma_read_abilities,
	.config_init = bcm89881_config_init,
} };

module_phy_driver(bcm89881_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Broadcom BCM89881 PHY driver");
