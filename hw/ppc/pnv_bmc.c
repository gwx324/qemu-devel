/*
 * QEMU PowerNV, BMC related functions
 *
 * Copyright (c) 2016-2017, IBM Corporation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include "qemu/osdep.h"
#include "hw/hw.h"
#include "sysemu/sysemu.h"
#include "target/ppc/cpu.h"
#include "qapi/error.h"
#include "qemu/log.h"
#include "hw/ipmi/ipmi.h"
#include "hw/ppc/fdt.h"

#include "hw/ppc/pnv.h"

#include <libfdt.h>

/* TODO: include definition in ipmi.h */
#define IPMI_SDR_FULL_TYPE 1

void pnv_bmc_populate_sensors(IPMIBmc *bmc, void *fdt)
{
    int offset;
    int i;
    const struct ipmi_sdr_compact *sdr;
    uint16_t nextrec;

    offset = fdt_add_subnode(fdt, 0, "/bmc");
    _FDT(offset);

    _FDT((fdt_setprop_string(fdt, offset, "name", "bmc")));
    _FDT((fdt_setprop_cell(fdt, offset, "#address-cells", 0x1)));
    _FDT((fdt_setprop_cell(fdt, offset, "#size-cells", 0x0)));

    offset = fdt_add_subnode(fdt, offset, "sensors");
    _FDT(offset);

    _FDT((fdt_setprop_cell(fdt, offset, "#address-cells", 0x1)));
    _FDT((fdt_setprop_cell(fdt, offset, "#size-cells", 0x0)));

    for (i = 0; !ipmi_bmc_sdr_find(bmc, i, &sdr, &nextrec); i++) {
        int off;
        char *name;

        if (sdr->header.rec_type != IPMI_SDR_COMPACT_TYPE &&
            sdr->header.rec_type != IPMI_SDR_FULL_TYPE) {
            continue;
        }

        name = g_strdup_printf("sensor@%x", sdr->sensor_owner_number);
        off = fdt_add_subnode(fdt, offset, name);
        _FDT(off);
        g_free(name);

        _FDT((fdt_setprop_cell(fdt, off, "reg", sdr->sensor_owner_number)));
        _FDT((fdt_setprop_string(fdt, off, "name", "sensor")));
        _FDT((fdt_setprop_string(fdt, off, "compatible", "ibm,ipmi-sensor")));
        _FDT((fdt_setprop_cell(fdt, off, "ipmi-sensor-reading-type",
                               sdr->reading_type)));
        _FDT((fdt_setprop_cell(fdt, off, "ipmi-entity-id",
                               sdr->entity_id)));
        _FDT((fdt_setprop_cell(fdt, off, "ipmi-entity-instance",
                               sdr->entity_instance)));
        _FDT((fdt_setprop_cell(fdt, off, "ipmi-sensor-type",
                               sdr->sensor_type)));
    }
}
