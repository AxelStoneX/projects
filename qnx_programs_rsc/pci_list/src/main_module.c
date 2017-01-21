#include "main_module.h"

int main (int argc, char **argv)
{
   
    int pci_handle;                         //handle for pci server connection
     
    LWORD Last_bus;                         // the number of last PCI bus in the system
    LWORD Version;                          // the version number of the PCI interface
    LWORD Hardware;                         // specific hardware support haracteristics. Can be NULL.
	  int RS = 0;                             // error control variable
	
	  int bus;                                // bus cycle counter
	
	
	  pci_handle = pci_attach( 0 );           // connect to the PCI server
	  if( pci_handle == -1 )
	  {  printf("PCI attach error ( check root priveleges )\n" );
	     return( EXIT_FAILURE );  }
	  
	  RS = pci_present( &Last_bus, &Version, &Hardware );
	  if( RS == FAIL_R )
	  {  printf( "pci_present error in module main_module.c\n" );
	     return(EXIT_FAILURE);  }
	
	  for ( bus=0; bus<=Last_bus; bus++ )
	     { scan_attach( bus ); }
	  
	  pci_detach( pci_handle ); 
    return( EXIT_SUCCESS );
	
}

void scan_attach ( LWORD bus_number )
{
	  uint8_t dev;                            // devices cycle counter
	  uint8_t func;                           // functions cycle counter
	  uint32_t flags;                         // flags that tell the PCI server how you whant it to handle resuorces
	  uint16_t idx;                           // The index of the device: 0 for the first device and so on
	  pci_dev_info inf_F;                    // Find structure
	  pci_dev_info inf_L;                    // Load structures
	  
	  _pci_config_regs PCR;                   // Pci config regs structure. Buffer for read and print
	  
	  void *handle;                           // Handle for pci_attach_device() function
	  
	  flags = PCI_SEARCH_BUSDEV | PCI_SHARE;  // PCI_SHARE is for multiple access to device
	  
	  for( dev=0; dev<=PCI_DEV_MAX; dev++)
	  {
	      for( func=0; func<=PCI_FUNC_MAX; func++ )
	      {
	         //memset( &inf_F, 0, sizeof( pci_dev_info ) );          // Clear all of the structure with 0 values
	         inf_F.BusNumber = (uint8_t) ( bus_number & 0xFF );      // Bus number field is cut-off
	         inf_F.DevFunc = (dev << 3) | func;                      // Device and function field are packed in one
	         
	         handle = pci_attach_device( NULL, flags, idx, &inf_F ); //attach pci_driver to the device
	         if( handle != NULL )
	         {
	            Out_Addr( inf_F.BusNumber, dev, func );
	            
	            pci_read_config( handle, 0, sizeof(_pci_config_regs), 1, &PCR );
	            pci_config_show( &PCR );
	            pci_detach_device ( handle );
	         }
	         //else
	         //{  perror( "...Attach error\n" );  }
	      }//func cycle end
	  }//dev cycle end
	  
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

void Out_Addr( LWORD bus, LWORD dev, LWORD func)
{
    printf( ">>>>> Bus : dev : func -> " );
    printf( " %d : %02d :%2d  >>>>>  \n", bus, dev, func );
}
