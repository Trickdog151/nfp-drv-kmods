/*
 * Copyright (C) 2018 Netronome Systems, Inc.
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

#ifndef __NFP_UAL_H__
#define __NFP_UAL_H__ 1

#include <linux/netdevice.h>

#include "nfp_app.h"
#include "nfp_net_repr.h"

struct nfp_cpp;
struct nfp_meta_parsed;

/* Representor port ID
 * ----------------------------------------------------------------
 *    3                   2                   1
 *  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |P|DevId|Ty |Index      |UAL defined                            |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */
#define NFP_MBL_PORTID_MBL_MASK		GENMASK(31, 20)
#define NFP_MBL_PORTID_MBL_PRESENCE	BIT(31)
#define NFP_MBL_PORTID_MBL_DEV_MASK	GENMASK(30, 28)
#define NFP_MBL_PORTID_MBL_TYPE_MASK	GENMASK(27, 26)
#define NFP_MBL_PORTID_MBL_INDEX_MASK	GENMASK(25, 20)
#define NFP_MBL_PORTID_UAL_MASK		GENMASK(19, 0)

#define NFP_UAL_PORTID_UNSPEC		NFP_MBL_PORTID_UAL_MASK

static inline u32 nfp_mbl_portid(u8 dev_index, u8 type, u8 index)
{
	return NFP_MBL_PORTID_MBL_PRESENCE |
		FIELD_PREP(NFP_MBL_PORTID_MBL_DEV_MASK, dev_index) |
		FIELD_PREP(NFP_MBL_PORTID_MBL_TYPE_MASK, type) |
		FIELD_PREP(NFP_MBL_PORTID_MBL_INDEX_MASK, index) |
		NFP_UAL_PORTID_UNSPEC;
}

enum nfp_mbl_dev_type {
	NFP_MBL_DEV_TYPE_MASTER_PF,
	NFP_MBL_DEV_TYPE_NICMOD,

	__NFP_MBL_DEV_TYPE_MAX,
};

#define NFP_MBL_DEV_TYPE_MAX		(__NFP_MBL_DEV_TYPE_MAX - 1)

#define NFP_MBL_DEV_ID_MAX		4
#define NFP_MBL_DEV_INDEX(type, id) \
	((type) * NFP_MBL_DEV_ID_MAX + (id))
#define NFP_MBL_DEV_INDEX_MAX \
	NFP_MBL_DEV_INDEX(NFP_MBL_DEV_TYPE_MAX, NFP_MBL_DEV_ID_MAX)
#define NFP_MBL_DEV_INDEX_PRIMARY \
	NFP_MBL_DEV_INDEX(NFP_MBL_DEV_TYPE_MASTER_PF, 0)

/**
 * enum nfp_mbl_status_type - type of MBL device probe status
 * @NFP_MBL_STATUS_PROBE:	devices are still in progress of being probed
 * @NFP_MBL_STATUS_TIMEOUT:	some devices have not been successfully probed
 *				before the timeout was reached
 * @NFP_MBL_STATUS_UNBOUND:	some devices have been unbound from the driver
 *				after being probed successfully initially
 * @NFP_MBL_STATUS_SUCCESS:	all devices are probed and ready
 */
enum nfp_mbl_status_type {
	NFP_MBL_STATUS_PROBE,
	NFP_MBL_STATUS_TIMEOUT,
	NFP_MBL_STATUS_UNBOUND,
	NFP_MBL_STATUS_SUCCESS,
};

/**
 * struct nfp_mbl_dev_ctx - device app context
 * This structure is used as the per device app priv structure, i.e. app->priv
 *
 * @app:		Back pointer to app
 * @nn:			Pointer to data vNIC
 * @type:		Type of device %NFP_MBL_DEV_TYPE_*
 * @pcie_unit:		PCIe unit number, e.g. 0-3 for main NFP
 */
struct nfp_mbl_dev_ctx {
	struct nfp_app *app;
	struct nfp_net *nn;
	enum nfp_mbl_dev_type type;
	u8 pcie_unit;
};

/**
 * struct nfp_ual_ops - UAL operations
 * @name:	get UAL name
 *
 * callbacks:
 * @init:	perform UAL init
 * @clean:	clean UAL state
 * @repr_open:	representor netdev open callback
 * @repr_stop:	representor netdev stop callback
 */
struct nfp_ual_ops {
	const char *name;

	int (*init)(void *cookie, enum nfp_mbl_status_type status);
	void (*clean)(void *cookie);

	int (*repr_open)(void *cookie, struct nfp_repr *repr);
	int (*repr_stop)(void *cookie, struct nfp_repr *repr);
};

int nfp_ual_register(const struct nfp_ual_ops *ops, void *cookie);
void *nfp_ual_unregister(void);

#endif