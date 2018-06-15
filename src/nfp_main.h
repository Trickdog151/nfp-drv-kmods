/*
 * Copyright (C) 2015-2017 Netronome Systems, Inc.
 *
 * This software is dual licensed under the GNU General License Version 2,
 * June 1991 as shown in the file COPYING in the top-level directory of this
 * source tree or the BSD 2-Clause License provided below.  You have the
 * option to license this software under the complete terms of either license.
 *
 * The BSD 2-Clause License:
 *
 *     Redistribution and use in source and binary forms, with or
 *     without modification, are permitted provided that the following
 *     conditions are met:
 *
 *      1. Redistributions of source code must retain the above
 *         copyright notice, this list of conditions and the following
 *         disclaimer.
 *
 *      2. Redistributions in binary form must reproduce the above
 *         copyright notice, this list of conditions and the following
 *         disclaimer in the documentation and/or other materials
 *         provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*
 * nfp_main.h
 * Author: Jason McMullan <jason.mcmullan@netronome.com>
 */

#ifndef NFP_MAIN_H
#define NFP_MAIN_H

#include "nfpcore/kcompat.h"

#include <linux/ethtool.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/msi.h>
#include <linux/mutex.h>
#include <linux/pci.h>
#include <linux/workqueue.h>
#if COMPAT__HAS_DEVLINK
#include <net/devlink.h>
#endif

struct dentry;
struct device;
struct pci_dev;
struct platform_device;

struct nfp_cpp;
struct nfp_cpp_area;
struct nfp_eth_table;
struct nfp_hwinfo;
struct nfp_mip;
struct nfp_net;
struct nfp_nsp_identify;
struct nfp_port;
struct nfp_rtsym;
struct nfp_rtsym_table;
struct nfp_shared_buf;

/**
 * struct nfp_dumpspec - NFP FW dump specification structure
 * @size:	Size of the data
 * @data:	Sequence of TLVs, each being an instruction to dump some data
 *		from FW
 */
struct nfp_dumpspec {
	u32 size;
	u8 data[0];
};

/**
 * struct nfp_pf - NFP PF-specific device structure
 * @pdev:		Backpointer to PCI device
 * @cpp:		Pointer to the CPP handle
 * @app:		Pointer to the APP handle
 * @nfp_dev_cpp:	Pointer to the NFP Device handle
 * @nfp_net_vnic:	Handle for ARM VNIC device
 * @data_vnic_bar:	Pointer to the CPP area for the data vNICs' BARs
 * @ctrl_vnic_bar:	Pointer to the CPP area for the ctrl vNIC's BAR
 * @qc_area:		Pointer to the CPP area for the queues
 * @mac_stats_bar:	Pointer to the CPP area for the MAC stats
 * @mac_stats_mem:	Pointer to mapped MAC stats area
 * @vf_cfg_bar:		Pointer to the CPP area for the VF configuration BAR
 * @vf_cfg_mem:		Pointer to mapped VF configuration area
 * @vfcfg_tbl2_area:	Pointer to the CPP area for the VF config table
 * @vfcfg_tbl2:		Pointer to mapped VF config table
 * @mbox:		RTSym of per-PCI PF mailbox (under devlink lock)
 * @irq_entries:	Array of MSI-X entries for all vNICs
 * @msix:		Single MSI-X entry for non-netdev mode event monitor
 * @limit_vfs:		Number of VFs supported by firmware (~0 for PCI limit)
 * @num_vfs:		Number of SR-IOV VFs enabled
 * @fw_loaded:		Is the firmware loaded?
 * @ctrl_vnic:		Pointer to the control vNIC if available
 * @debug_ctrl_netdev:	Pointer to "debug pipe" netdev of the control vNIC
 * @mip:		MIP handle
 * @rtbl:		RTsym table
 * @hwinfo:		HWInfo table
 * @dumpspec:		Debug dump specification
 * @dump_flag:		Store dump flag between set_dump and get_dump_flag
 * @dump_len:		Store dump length between set_dump and get_dump_flag
 * @eth_tbl:		NSP ETH table
 * @nspi:		NSP identification info
 * @hwmon_dev:		pointer to hwmon device
 * @ddir:		Per-device debugfs directory
 * @max_data_vnics:	Number of data vNICs app firmware supports
 * @num_vnics:		Number of vNICs spawned
 * @vnics:		Linked list of vNIC structures (struct nfp_net)
 * @ports:		Linked list of port structures (struct nfp_port)
 * @wq:			Workqueue for running works which need to grab @lock
 * @port_refresh_work:	Work entry for taking netdevs out
 * @shared_bufs:	Array of shared buffer structures if FW has any SBs
 * @num_shared_bufs:	Number of elements in @shared_bufs
 * @lock:		Protects all fields which may change after probe
 */
struct nfp_pf {
	struct pci_dev *pdev;

	struct nfp_cpp *cpp;

	struct nfp_app *app;

	struct platform_device *nfp_dev_cpp;
	struct platform_device *nfp_net_vnic;

	struct nfp_cpp_area *data_vnic_bar;
	struct nfp_cpp_area *ctrl_vnic_bar;
	struct nfp_cpp_area *qc_area;
	struct nfp_cpp_area *mac_stats_bar;
	u8 __iomem *mac_stats_mem;
	struct nfp_cpp_area *vf_cfg_bar;
	u8 __iomem *vf_cfg_mem;
	struct nfp_cpp_area *vfcfg_tbl2_area;
	u8 __iomem *vfcfg_tbl2;

	const struct nfp_rtsym *mbox;

	struct msix_entry *irq_entries;

	struct msix_entry msix;

	unsigned int limit_vfs;
	unsigned int num_vfs;

	bool fw_loaded;

	struct nfp_net *ctrl_vnic;
	struct net_device __rcu *debug_ctrl_netdev;

	const struct nfp_mip *mip;
	struct nfp_rtsym_table *rtbl;
	struct nfp_hwinfo *hwinfo;
	struct nfp_dumpspec *dumpspec;
	u32 dump_flag;
	u32 dump_len;
	struct nfp_eth_table *eth_tbl;
	struct nfp_nsp_identify *nspi;

	struct device *hwmon_dev;

	struct dentry *ddir;

	unsigned int max_data_vnics;
	unsigned int num_vnics;

	struct list_head vnics;
	struct list_head ports;

	struct workqueue_struct *wq;
	struct work_struct port_refresh_work;

	struct nfp_shared_buf *shared_bufs;
	unsigned int num_shared_bufs;

	struct mutex lock;
};

extern int nfp_dev_cpp;
extern bool nfp_net_vnic;

extern struct pci_driver nfp_netvf_pci_driver;

extern const struct devlink_ops nfp_devlink_ops;

#ifdef CONFIG_NFP_NET_PF
int nfp_net_pci_probe(struct nfp_pf *pf);
void nfp_net_pci_remove(struct nfp_pf *pf);
#else
static inline int nfp_net_pci_probe(struct nfp_pf *pf)
{
	return -ENODEV;
}

static inline void nfp_net_pci_remove(struct nfp_pf *pf)
{
}
#endif

int nfp_hwmon_register(struct nfp_pf *pf);
void nfp_hwmon_unregister(struct nfp_pf *pf);

void
nfp_net_get_mac_addr(struct nfp_pf *pf, struct net_device *netdev,
		     struct nfp_port *port);

bool nfp_ctrl_tx(struct nfp_net *nn, struct sk_buff *skb);

int nfp_ctrl_debug_start(struct nfp_pf *pf);
void nfp_ctrl_debug_stop(struct nfp_pf *pf);
void nfp_ctrl_debug_rx(struct nfp_pf *pf, struct sk_buff *skb);
void nfp_ctrl_debug_deliver_tx(struct nfp_pf *pf, struct sk_buff *skb);

#define NFP_DEV_CPP_TYPE	"nfp-dev-cpp"

#ifdef CONFIG_NFP_USER_SPACE_CPP
int nfp_dev_cpp_init(void);
void nfp_dev_cpp_exit(void);
#else
static inline int nfp_dev_cpp_init(void)
{
	return -ENODEV;
}

static inline void nfp_dev_cpp_exit(void)
{
}
#endif

int nfp_pf_rtsym_read_optional(struct nfp_pf *pf, const char *format,
			       unsigned int default_val);
u8 __iomem *
nfp_pf_map_rtsym(struct nfp_pf *pf, const char *name, const char *sym_fmt,
		 unsigned int min_size, struct nfp_cpp_area **area);
int nfp_mbox_cmd(struct nfp_pf *pf, u32 cmd, void *in_data, u64 in_length,
		 void *out_data, u64 out_length);

enum nfp_dump_diag {
	NFP_DUMP_NSP_DIAG = 0,
};

struct nfp_dumpspec *
nfp_net_dump_load_dumpspec(struct nfp_cpp *cpp, struct nfp_rtsym_table *rtbl);
s64 nfp_net_dump_calculate_size(struct nfp_pf *pf, struct nfp_dumpspec *spec,
				u32 flag);
int nfp_net_dump_populate_buffer(struct nfp_pf *pf, struct nfp_dumpspec *spec,
				 struct ethtool_dump *dump_param, void *dest);

#if COMPAT__HAS_DEVLINK_SB
int nfp_shared_buf_register(struct nfp_pf *pf);
void nfp_shared_buf_unregister(struct nfp_pf *pf);
int nfp_shared_buf_pool_get(struct nfp_pf *pf, unsigned int sb, u16 pool_index,
			    struct devlink_sb_pool_info *pool_info);
int nfp_shared_buf_pool_set(struct nfp_pf *pf, unsigned int sb,
			    u16 pool_index, u32 size,
			    enum devlink_sb_threshold_type threshold_type);
#else
static inline int nfp_shared_buf_register(struct nfp_pf *pf)
{
	return 0;
}

static inline void nfp_shared_buf_unregister(struct nfp_pf *pf)
{
}
#endif
#endif /* NFP_MAIN_H */
