#include "main_module.h"

int main ( int argc, char **argv )
{
	 int pci_handle;                                                                      // handle for pci attach/detach
	 int RS = 0;                                                                          // errors check variable
      uint16_t DeviceID = BOARD_ID;
      uint16_t VendorID = VENDOR_ID;
      pci_dev_info inf_F;                                                                 // stores data
      
      void *dev_handle;                                                                   // handle for pci_device
      char *dev_memory;                                                                   // pointer to mapped device memory
      
      _pci_config_regs PCR;                                                               // pci configuration structure
      
      int i;
      int main_bar_idx = 6;                                                               // index of memory Base Address
	  // ------------------------------------------------------------------------------------------------------------------------------
	  pci_handle = pci_attach( 0 );
	  if( pci_handle == (-1) )
	  {    printf( "PCI attach error ( check root priveleges )\n" );
	       return( EXIT_FAILURE );   }
      
       //memset( &inf_F, 0, sizeof( pci_dev_info ));
	  inf_F.DeviceId = DeviceID;
	  inf_F.VendorId = VendorID;
	
      dev_handle = find_and_attach ( &inf_F );                                            // find device with given DeviceID
      
      if( dev_handle == NULL )
      {    perror( "\n...Attach error. Device has not been found \n\n" );
           return(EXIT_FAILURE);   }
 
      pci_read_config( dev_handle, 0, sizeof( _pci_config_regs ), 1, &PCR );              // show configs of found device
      pci_config_show( &PCR );
      
      for( i=0; i<5; i++ )
      {    if( inf_F.BaseAddressSize[i] >= 0x800000 )
                main_bar_idx = i;    }
                
      if( main_bar_idx == 6 )
      {   perror( "\n...BAR memory finding error. There is no register bigger than 0x800000\n\n" );    }
      
      dev_memory = (BYTE *) mmap_device_memory( 0,  inf_F.BaseAddressSize [main_bar_idx], PROT_WRITE|PROT_NOCACHE, 0, inf_F.PciBaseAddress[main_bar_idx]);
      
      printf( "Board  type is: %d\n\n", (int*)(((*dev_memory) >> 1) & 0x1f) );
      printf( "Mumber of channels: %d\n\n", (int*)(((*dev_memory) >> 6) & 0x1f) );
      
      pci_detach( pci_handle );
      return( EXIT_SUCCESS );
}

void* find_and_attach(  pci_dev_info* inf_F )
{
	uint32_t flags;                                                                       // flags for PCI resourses handle
	uint16_t idx;                                                                         // the index for device
	void *handle;                                                                        // handle for pci_attach_device()
	// ----------------------------------------------------------------------------------------------------------------------
	flags = (PCI_SEARCH_BUSDEV | PCI_SHARE);                        // PCI_SHARE is for multiple access
	
	handle = pci_attach_device( NULL, flags, idx, inf_F );        //attach pci_driver to the device
	
	return handle;
}

void pci_config_show (_pci_config_regs *PC )
{
    int i;
    printf( "------------------_pci_config_regs-------------------\n" );
    printf( "VendorID : DevID : Class       ->  ");
    printf( "0x%04x : 0x%04x : 0x%02x%02x%02x \n", PC->Vendor_ID, PC->Device_ID,
        PC->Class_Code[2],PC->Class_Code[1],PC->Class_Code[0] );
        
    printf( "SubVend: SubSys : Revision_ID  ->  " );
    printf( "0x%04x : 0x%04x : 0x%02x \n",
        PC->Sub_Vendor_ID, PC->Sub_System_ID, PC->Revision_ID );
        
    printf( "Command : Status               ->  " );
    printf( "0x%04x : 0x%04x \n", PC->Command, PC->Status );
    
    printf( "Interrupt ( Line : Pin )       ->  " );
    printf( "0x%02x   : 0x%02x \n", PC->Interrupt_Line, PC->Interrupt_Pin );
    
    printf( "Base_Address_Regs[6]           -> \n" );
    for( i=0; i<6; i++ )
       printf( "0x%08x \n", PC->Base_Address_Regs[i] );
    printf( "\n" );
    
    printf( "ROM_Base_Address               -> " );
    printf( "0x%08x  \n", PC->ROM_Base_Address );
    printf( "--------------------end of module--------------------\n\n\n" );
}
