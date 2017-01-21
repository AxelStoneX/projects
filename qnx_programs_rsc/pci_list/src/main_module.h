#ifndef __MAIN_MODULE_H__
#define __MAIN_MODULE_H__

#include <stdio.h>
#include <sys/neutrino.h>
#include <inttypes.h>
#include <stdlib.h>
#include <hw/pci.h>                // pci_dev_info 
#include <hw/pci_devices.h>
#include <errno.h>

#define FAIL_R      -1             // Typical return error code

//type definition
typedef uint32_t LWORD;
typedef struct _pci_config_regs _pci_config_regs;
typedef struct pci_dev_info pci_dev_info;

// PCI specific constants
#define PCI_DEV_MAX    31         // 5 bits: 0..31
#define PCI_FUNC_MAX    7         // 3 bits: 0..7


void pci_config_show( _pci_config_regs *PC );
void scan_attach ( LWORD bus_number );
void Out_Addr( LWORD bus, LWORD dev, LWORD func);

#endif // __MAIN_MODULE_H__

