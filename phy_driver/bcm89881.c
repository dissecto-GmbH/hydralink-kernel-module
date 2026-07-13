// SPDX-License-Identifier: GPL-2.0+
/*
 * Broadcom BCM89881 BASE-T1 PHY driver
 *
 * Used on the dissecto HydraLink (Microchip LAN7801 USB-to-Ethernet bridge
 * driving a Broadcom BCM89881 automotive Ethernet PHY over RGMII). The PHY
 * registers live in Clause-45 MMD space, reached through the LAN78xx MDIO
 * controller's Clause-22 MMD indirection.
 */

#include <linux/ethtool.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/mdio.h>

#define PHY_ID_BCM89881		0xae025030
#define PHY_ID_BCM89881_MASK	0xfffffff0

static int bcm89881_read_status(struct phy_device *phydev)
{
	int bt1_ctrl;
	int stat1;
	int ret;

	phydev->duplex = DUPLEX_FULL;
	phydev->pause = 0;
	phydev->asym_pause = 0;

	ret = genphy_c45_read_pma(phydev);
	if (ret < 0)
		return ret;

	stat1 = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_STAT1);
	if (stat1 < 0)
		return stat1;

	bt1_ctrl = phy_read_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_PMA_PMD_BT1_CTRL);
	if (bt1_ctrl < 0)
		return bt1_ctrl;

	phydev->speed = (bt1_ctrl & MDIO_PMA_PMD_BT1_CTRL_STRAP_B1000) ?
			SPEED_1000 : SPEED_100;
	phydev->link = (stat1 & BMSR_LSTATUS) ? 1 : 0;
	phydev->master_slave_state =
		(bt1_ctrl & MDIO_PMA_PMD_BT1_CTRL_CFG_MST) ?
		MASTER_SLAVE_STATE_MASTER : MASTER_SLAVE_STATE_SLAVE;

	linkmode_set_bit(ETHTOOL_LINK_MODE_MII_BIT, phydev->supported);
	phydev->port = PORT_MII;

	return 0;
}

static int bcm89881_config_init(struct phy_device *phydev)
{
	phydev->autoneg = AUTONEG_DISABLE;

	/* RGMII delay and voltage settings */
	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, 0xa010, 0x0001);
	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, 0xa015, 0x0000);

	/* LED configuration */
	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, 0xa027, 0x0f15);
	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, 0x931e, 0x0063);
	phy_write_mmd(phydev, MDIO_MMD_PMAPMD, 0x931d, 0x0010);

	phydev_info(phydev, "Initialization done\n");

	/* populate master/slave configuration for ethtool */
	return genphy_c45_pma_baset1_read_master_slave(phydev);
}

static int bcm89881_config_aneg(struct phy_device *phydev)
{
	if (phydev->autoneg == AUTONEG_DISABLE) {
		switch (phydev->speed) {
		case SPEED_UNKNOWN:
			break;
		case SPEED_100:
			phy_modify_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_CTRL1,
				       BMCR_SPEED100 | BMCR_SPEED1000,
				       BMCR_SPEED100);
			break;
		case SPEED_1000:
			phy_modify_mmd(phydev, MDIO_MMD_PMAPMD, MDIO_CTRL1,
				       BMCR_SPEED1000 | BMCR_SPEED100,
				       BMCR_SPEED1000);
			break;
		default:
			phydev_warn(phydev,
				    "Attempting to set unsupported speed %d\n",
				    phydev->speed);
			return -EINVAL;
		}
	}

	genphy_c45_pma_baset1_setup_master_slave(phydev);

	/* repopulate master/slave configuration for ethtool */
	return genphy_c45_pma_baset1_read_master_slave(phydev);
}

static struct phy_driver bcm89881_driver[] = { {
	PHY_ID_MATCH_MODEL(PHY_ID_BCM89881),
	.name		= "Broadcom BCM89881",
	.config_init	= bcm89881_config_init,
	.config_aneg	= bcm89881_config_aneg,
	.read_status	= bcm89881_read_status,
	.get_features	= genphy_c45_pma_read_abilities,
} };

module_phy_driver(bcm89881_driver);

static const struct mdio_device_id __maybe_unused bcm89881_tbl[] = {
	{ PHY_ID_MATCH_MODEL(PHY_ID_BCM89881) },
	{ }
};

MODULE_DEVICE_TABLE(mdio, bcm89881_tbl);

MODULE_DESCRIPTION("Broadcom BCM89881 PHY driver");
MODULE_LICENSE("GPL v2");
