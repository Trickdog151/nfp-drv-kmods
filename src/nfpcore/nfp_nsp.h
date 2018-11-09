/* SPDX-License-Identifier: (GPL-2.0-only OR BSD-2-Clause) */
/* Copyright (C) 2015-2018 Netronome Systems, Inc. */

#ifndef NSP_NSP_H
#define NSP_NSP_H 1

#include <linux/types.h>
#include <linux/if_ether.h>

struct firmware;
struct nfp_cpp;
struct nfp_nsp;

struct nfp_nsp *nfp_nsp_open(struct nfp_cpp *cpp);
void nfp_nsp_close(struct nfp_nsp *state);
u16 nfp_nsp_get_abi_ver_major(struct nfp_nsp *state);
u16 nfp_nsp_get_abi_ver_minor(struct nfp_nsp *state);
int nfp_nsp_wait(struct nfp_nsp *state);
int nfp_nsp_device_soft_reset(struct nfp_nsp *state);
int nfp_nsp_load_fw(struct nfp_nsp *state, const struct firmware *fw);
int nfp_nsp_write_flash(struct nfp_nsp *state, const struct firmware *fw);
int nfp_nsp_mac_reinit(struct nfp_nsp *state);
int nfp_nsp_load_stored_fw(struct nfp_nsp *state);
int nfp_nsp_hwinfo_lookup(struct nfp_nsp *state, void *buf, unsigned int size);

static inline bool nfp_nsp_has_mac_reinit(struct nfp_nsp *state)
{
	return nfp_nsp_get_abi_ver_minor(state) > 20;
}

static inline bool nfp_nsp_has_stored_fw_load(struct nfp_nsp *state)
{
	return nfp_nsp_get_abi_ver_minor(state) > 23;
}

static inline bool nfp_nsp_has_hwinfo_lookup(struct nfp_nsp *state)
{
	return nfp_nsp_get_abi_ver_minor(state) > 24;
}

enum nfp_eth_interface {
	NFP_INTERFACE_NONE	= 0,
	NFP_INTERFACE_SFP	= 1,
	NFP_INTERFACE_SFP_2G	= 2,
	NFP_INTERFACE_SFP_5G	= 5,
	NFP_INTERFACE_SFPP	= 10,
	NFP_INTERFACE_SFP28	= 28,
	NFP_INTERFACE_QSFP	= 40,
	NFP_INTERFACE_RJ45	= 45,
	NFP_INTERFACE_CXP	= 100,
	NFP_INTERFACE_QSFP28	= 112,
};

enum nfp_eth_media {
	NFP_MEDIA_DAC_PASSIVE = 0,
	NFP_MEDIA_DAC_ACTIVE,
	NFP_MEDIA_FIBRE,
	NFP_MEDIA_TP,
};

enum nfp_eth_aneg {
	NFP_ANEG_AUTO = 0,
	NFP_ANEG_SEARCH,
	NFP_ANEG_25G_CONSORTIUM,
	NFP_ANEG_25G_IEEE,
	NFP_ANEG_DISABLED,
};

enum nfp_eth_fec {
	NFP_FEC_AUTO_BIT = 0,
	NFP_FEC_BASER_BIT,
	NFP_FEC_REED_SOLOMON_BIT,
	NFP_FEC_DISABLED_BIT,
};

#define NFP_FEC_AUTO		BIT(NFP_FEC_AUTO_BIT)
#define NFP_FEC_BASER		BIT(NFP_FEC_BASER_BIT)
#define NFP_FEC_REED_SOLOMON	BIT(NFP_FEC_REED_SOLOMON_BIT)
#define NFP_FEC_DISABLED	BIT(NFP_FEC_DISABLED_BIT)

/* Calculate port expander port index from cluster perspective.
 * This is the way the port expander firmware identifies ports.
 */
#define NFP_PORTEX_PORTS_PER_CLUSTER	8
#define NFP_PORTEX_PORT_INDEX(cluster, port) \
	((((cluster)) * NFP_PORTEX_PORTS_PER_CLUSTER) + (port))

/**
 * struct nfp_eth_table - ETH table information
 * @count:	number of table entries
 * @max_index:	max of @index fields of all @ports
 * @ports:	table of ports
 *
 * @ports.eth_index:	port index according to legacy ethX numbering
 * @ports.index:	chip-wide first channel index
 * @ports.nbi:		NBI index
 * @ports.base:		first MAC port base (within NBI)
 * @ports.lanes:	number of channels
 * @ports.speed:	interface speed (in Mbps)
 * @ports.channel_base:	first egress channel index
 * @ports.interface:	interface (module) plugged in
 * @ports.media:	media type of the @interface
 * @ports.fec:		forward error correction mode
 * @ports.aneg:		auto negotiation mode
 * @ports.mac_addr:	interface MAC address
 * @ports.label_port:	port id
 * @ports.label_subport:  id of interface within port (for split ports)
 * @ports.enabled:	is enabled?
 * @ports.tx_enabled:	is TX enabled?
 * @ports.rx_enabled:	is RX enabled?
 * @ports.link:	does port have link
 * @ports.override_changed: is media reconfig pending?
 *
 * @ports.port_type:	one of %PORT_* defines for ethtool
 * @ports.port_lanes:	total number of lanes on the port (sum of lanes of all
 *			subports)
 * @ports.is_split:	is interface part of a split port
 * @ports.fec_modes_supported:	bitmap of FEC modes supported
 *
 * @ports.cluster:	port expander cluster number
 * @ports.cluster_port:	port expander port number within cluster
 */
struct nfp_eth_table {
	unsigned int count;
	unsigned int max_index;
	struct nfp_eth_table_port {
		unsigned int eth_index;
		unsigned int index;
		unsigned int nbi;
		unsigned int base;
		unsigned int lanes;
		unsigned int speed;
		unsigned int channel_base;

		unsigned int interface;
		enum nfp_eth_media media;

		enum nfp_eth_fec fec;
		enum nfp_eth_aneg aneg;

		u8 mac_addr[ETH_ALEN];

		u8 label_port;
		u8 label_subport;

		bool enabled;
		bool tx_enabled;
		bool rx_enabled;
		bool link;

		bool override_changed;

		/* Computed fields */
		u8 port_type;

		unsigned int port_lanes;

		bool is_split;

		unsigned int fec_modes_supported;

		/* Port expander calculated fields */
		unsigned int cluster;
		unsigned int cluster_port;
	} ports[0];
};

struct nfp_eth_table *nfp_eth_read_ports(struct nfp_cpp *cpp);
struct nfp_eth_table *
__nfp_eth_read_ports(struct nfp_cpp *cpp, struct nfp_nsp *nsp);

int nfp_eth_set_mod_enable(struct nfp_cpp *cpp, unsigned int idx, bool enable);
int nfp_eth_set_configured(struct nfp_cpp *cpp, unsigned int idx,
			   bool configed);
int
nfp_eth_set_fec(struct nfp_cpp *cpp, unsigned int idx, enum nfp_eth_fec mode);

static inline bool nfp_eth_can_support_fec(struct nfp_eth_table_port *eth_port)
{
	return !!eth_port->fec_modes_supported;
}

static inline unsigned int
nfp_eth_supported_fec_modes(struct nfp_eth_table_port *eth_port)
{
	return eth_port->fec_modes_supported;
}

struct nfp_nsp *nfp_eth_config_start(struct nfp_cpp *cpp, unsigned int idx);
int nfp_eth_config_commit_end(struct nfp_nsp *nsp);
void nfp_eth_config_cleanup_end(struct nfp_nsp *nsp);

int __nfp_eth_set_aneg(struct nfp_nsp *nsp, enum nfp_eth_aneg mode);
int __nfp_eth_set_speed(struct nfp_nsp *nsp, unsigned int speed);
int __nfp_eth_set_split(struct nfp_nsp *nsp, unsigned int lanes);

/**
 * struct nfp_nsp_identify - NSP static information
 * @version:      opaque version string
 * @flags:        version flags
 * @br_primary:   branch id of primary bootloader
 * @br_secondary: branch id of secondary bootloader
 * @br_nsp:       branch id of NSP
 * @primary:      version of primarary bootloader
 * @secondary:    version id of secondary bootloader
 * @nsp:          version id of NSP
 * @sensor_mask:  mask of present sensors available on NIC
 */
struct nfp_nsp_identify {
	char version[40];
	u8 flags;
	u8 br_primary;
	u8 br_secondary;
	u8 br_nsp;
	u16 primary;
	u16 secondary;
	u16 nsp;
	u64 sensor_mask;
};

struct nfp_nsp_identify *__nfp_nsp_identify(struct nfp_nsp *nsp);

enum nfp_nsp_sensor_id {
	NFP_SENSOR_CHIP_TEMPERATURE,
	NFP_SENSOR_ASSEMBLY_POWER,
	NFP_SENSOR_ASSEMBLY_12V_POWER,
	NFP_SENSOR_ASSEMBLY_3V3_POWER,
};

int nfp_hwmon_read_sensor(struct nfp_cpp *cpp, enum nfp_nsp_sensor_id id,
			  long *val);

#endif
