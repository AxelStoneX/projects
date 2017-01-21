  /*============================================================================*
 * FILE:                 E X A M P L E _ R T 2 A . C
 *============================================================================*
 *
 * COPYRIGHT (C) 2001, 2002, 2003, 2004 BY
 *          CONDOR ENGINEERING, INC., SANTA BARBARA, CALIFORNIA
 *          ALL RIGHTS RESERVED.
 *
 *          THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED AND
 *          COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND WITH
 *          THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR ANY
 *          OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
 *          AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
 *          SOFTWARE IS HEREBY TRANSFERRED.
 *
 *          THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
 *          NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY CONDOR
 *          ENGINEERING.
 *
 *===========================================================================*
 *
 * FUNCTION:    EXAMPLE PROGRAM
 *              This is a basic example program that sets up a simple
 *				RT Simulation.  Simulates RT1 with two subaddresses, 
 *				SA1 RECEIVE and SA2 TRANSMIT.
 *
 *              This program uses INTERRUPT EVENTS to detect when
 *				RT1 SA1 has received new data.
 *
 *				This is a variant of Example_rt2.c - in this case we use the
 *				RT1 SA1 Receive data to determine the RT1 SA2 Transmit data.
 *				I am using only one data buffer for each SA, but this can be
 *				changed with the NUM_BUFFERS constant.
 *
 *				Each time this program receives a message for RT1 SA1 Receive,
 *				it prints an asterisk and uses the receive data to derive
 *				transmit data for RT1 SA2 Transmit.  In this case, we reverse
 *				the order of the 32 data words.
 *
 *				Use a bus analyzer (BusTools/1553) as the bus controller
 *				sending messages to RT1 SA1 Receive and RT1 SA2 Transmit.
 *				Monitor the data sent by RT1 SA2 Transmit.  The data sent
 *				by SA2 should be the same as the data received by SA1 but
 *				in reverse word order.
 *
 *				NOTE:  There must be enough time between the RECEIVE message
 *				and the TRANSMIT message to allow the operating system to
 *				react to the interrupt (from the receive message), read the
 *				receive data, process the data, and write the data to the
 *				transmit buffer.  Therefore, if your transmit message follows
 *				the receive message very quickly you are likely to see partial
 *				data buffers - some old data and some new data.  The time needed
 *				to handle the interrupt is very SYSTEM DEPENDENT - a real-time
 *				operating system like VxWorks will do better than Windows. 
 *				On my Windows 98 system using a PCI-1553 board, I needed to 
 *				allow an intermessage gap of about 300 microseconds in order
 *				to have reliable transmit data.  At 250us and below I saw some 
 *				partial data buffers.
 *
 *===========================================================================*/

/* $Revision:  X.xx Release $
   Date        Revision
  ----------   ---------------------------------------------------------------
	11/07/01	Initial version.  RSW
	03/15/02	Removed conio.h and getc calls.  RSW.
    09/20/04    Added option to initialize with BusTools_API_LoadChannel.  RSW
    11/16/05    Cleaned up some of the debug printfs.  RSW
*/

#include "busapi.h"
#include <stdio.h>


//----------------------- Initializion Method Selection ---------------------------
// There are two methods of initializing the board:
//      1.  BusTools_API_InitExtended() - This is the "old-style" method.  It is
//          a bit more complicated but it applies to all platforms and products.
//
//      2.  BusTools_API_OpenChannel() - This is the "New, improved" method.  This
//          is less complicated (fewer parameters to get right), but does not work
//          with all possible platforms and products.  See the documentation in
//          the Software Reference Manual for this function to determine if this
//          method can be used with your platform and product.  This method is 
//          available with API version 5.10 and later.
//
// If you want to use method 1, define _USE_INIT_EXTENDED_.
// If you want to use method 2, comment out the definition for _USE_INIT_EXTENDED_.


// Constants to be used as parameters to BusTools_FindDevice and 
// BusTools_API_OpenChannel.

//---------------------------------------------------------------------------------

// Number of buffers to use for each subaddress.
#define NUM_BUFFERS		5

// Global variables
	BT_U16BIT			base_data = 0x0000; // Upper byte will increment on each buffer
										    // transmitted by RT1 SA2 Transmit
    int                 cardnum;

// Prototype for event handler function
BT_INT _stdcall my_event_handler(BT_UINT cardnum, struct api_int_fifo *pFIFO);
BT_INT _stdcall my_hwi_event_handler(BT_UINT cardnum, struct api_int_fifo *pFIFO);

BT_INT debugIsr;
BT_INT debugIsr1;
BT_INT debugIsr2;
BT_INT debugIsr3;
BT_INT debugIsr4;
BT_INT debugIsr5;
BT_INT debugIsr6;
BT_INT		debugIsr7;
BT_INT		debugIsr8;
BT_INT		debugSUT1;
BT_INT		debugSUT2;
BT_INT		debugSUT3;
BT_INT		debugSUT4;
BT_INT		debugSUT5;
BT_INT		debugSUT6;
BT_INT		debugSUT7;
BT_INT		debugSUT8;
BT_INT		debugSUT9;
BT_INT		debugSUT10;
BT_INT		debugSUT11;
BT_INT debugSignal1;
BT_INT debugSignal2;
BT_INT debugSignal3;
BT_INT debugSignal4;		
BT_INT		irq1_counter;
BT_INT		irq2_counter;
BT_INT		sig_cond_pmutex_lock_ret;
BT_INT		sig_cond_ret;
BT_INT		sig_cond_pmutex_unlock_ret;
BT_INT		sig_cond_wait_pmutex_lock_ret;
BT_INT		sig_cond_wait_ret;
BT_INT		sig_cond_wait_pmutex_unlock_ret;
BT_INT		intNotice[12];
BT_INT event_handler_counter;
BT_INT Interrupt_Setup_ThreadID;
BT_INT RegisterFunctionThreadID;
BT_INT LocalInterruptFunctionThreadID;

API_INT_FIFO		sIntFIFO1;	  // For interrupt handling
API_INT_FIFO		sInt_hwi_FIFO1;	  // For interrupt handling


int MainThreadID;

// Main program
void main() {
	BT_INT				status;
	API_RT_ABUF			Abuf_RT1;	  // RT address buffer structure.
	API_RT_CBUF			Cbuf_RT1SA1R; // RT control buffer structures.
	API_RT_CBUF			Cbuf_RT1SA2T;
	API_RT_MBUF_WRITE	msg_buffer_write;
	int					i, buff_num;

	static int pass_count;

	int happyindex;

//-------------------------- Initialize API and board -----------------------------
    int  mode;
		
		//Initialize debug variables:
		
		debugIsr = 0;
		debugIsr2 = 0;
		debugIsr4 = 0;
		debugIsr5 = 0;
		debugIsr6 = 0;
		debugIsr7 = 0;
		debugIsr8 = 0;
		debugSUT1 = 0;
		debugSUT2 = 0;
		debugSUT3 = 0;
		debugSUT4 = 0;
		debugSUT5 = 0;
		debugSUT6 = 0;
		debugSUT7 = 0;
		debugSUT8 = 0;
		debugSUT9 = 0;
		debugSUT10 = 0;
		debugSUT11 = 0;
		debugSignal1 = 0;
		debugSignal2 = 0;
		debugSignal3 = 0;
		debugSignal4 = 0;			
		irq1_counter = 0;
		irq2_counter = 0;
		sig_cond_pmutex_lock_ret = 0;
		sig_cond_ret = 0;
		sig_cond_pmutex_unlock_ret = 0;
		sig_cond_wait_pmutex_lock_ret = 0;
		sig_cond_wait_ret = 0;
		sig_cond_wait_pmutex_unlock_ret = 0;
		for(happyindex=0;happyindex<12;happyindex++) intNotice[happyindex]=0;
		
		
		debugIsr = 0; //count total HW Ints...
		event_handler_counter = 0; //count total HW handler Ints...
		pass_count = 0;

   printf("Initializing API with BusTools_API_OpenChannel . . . "); 

   mode = API_B_MODE | API_HW_ONLY_INT;  // 1553B protocol, use SW interrupts.
   status = BusTools_API_OpenChannel( &cardnum, mode, 0, CHANNEL_1);
   printf("BusTools_API_OpenChannel = %d\n",status);  

   // Select External Bus.
   status = BusTools_SetInternalBus(cardnum, EXTERNAL_BUS);
   printf("BusTools_SetInternalBus = %d\n",status);

   // Now lets set up an RT.
   printf("Initializing RT functionality . . . ");
   status = BusTools_RT_Init(cardnum, 0);
   printf("BusTools_RT_Init = %d\n",status);

   // Setup RT address buffer for our RT (RT1)
   Abuf_RT1.enable_a = 1;			// Respond on bus A
   Abuf_RT1.enable_b = 1;			// Respond on bus B
   Abuf_RT1.inhibit_term_flag = 1;	// Inhibit terminal flag in status word
   Abuf_RT1.status = 0x0800;		// Set status word
   Abuf_RT1.bit_word = 0x0000;		// Set BIT word (for mode code 19)
   status = BusTools_RT_AbufWrite(cardnum, 1, &Abuf_RT1);
   printf("BusTools_RT_Init = %d\n",status);			

   // Setup a control buffer - RT1, SA1, Receive, NUM_BUFFERS buffers.
   Cbuf_RT1SA1R.legal_wordcount = 0xFFFFFFFF;  // any word count is legal.
   status = BusTools_RT_CbufWrite(cardnum, 1, 1, 0, NUM_BUFFERS, &Cbuf_RT1SA1R);
   printf("BusTools_RT_CbufWrite = %d\n",status);

   // Setup a control buffer - RT1, SA2, Transmit, NUM_BUFFERS buffers.
   Cbuf_RT1SA2T.legal_wordcount = 0xFFFFFFFF;  // any word count is legal.
   status = BusTools_RT_CbufWrite(cardnum, 1, 2, 1, NUM_BUFFERS, &Cbuf_RT1SA2T);
   printf("BusTools_RT_CbufWrite = %d\n",status);   			

   // Clear our receive buffers and enable interrupt.
   for (buff_num=0; buff_num<NUM_BUFFERS; buff_num++) 
   {
      msg_buffer_write.enable = BT1553_INT_END_OF_MESS; // Int on msg complete
      msg_buffer_write.error_inj_id = 0;	// No error injection

      for (i=0; i<32; i++)
         msg_buffer_write.mess_data[i] = 0;

      status = BusTools_RT_MessageWrite(cardnum, 1, 1, 0, buff_num, &msg_buffer_write);
      printf("BusTools_RT_MessageWrite = %d\n",status);

	  msg_buffer_write.enable = 0; // Int on msg complete
      for (i=0; i<32; i++)
         msg_buffer_write.mess_data[i] = 0xFFFF;

      status = BusTools_RT_MessageWrite(cardnum, 1, 2, 1, buff_num, &msg_buffer_write);
      printf("BusTools_RT_MessageWrite = %d\n",status);
   }

   // Setup for our interrupt event handling functions
   //memset(&sIntFIFO1, 0, sizeof(sIntFIFO1));
   memset(&sInt_hwi_FIFO1, 0, sizeof(sInt_hwi_FIFO1));	
   sInt_hwi_FIFO1.function       = my_event_handler;
   sInt_hwi_FIFO1.iPriority      = THREAD_PRIORITY_ABOVE_NORMAL;
   sInt_hwi_FIFO1.dwMilliseconds = INFINITE;
   sInt_hwi_FIFO1.iNotification  = 0;
   sInt_hwi_FIFO1.FilterType     = EVENT_RT_MESSAGE;
   sInt_hwi_FIFO1.FilterMask[1][0][1] = 0xFFFFFFFF;  // Enable all messages, RT1 RCV SA1						

    // Call the register function to register and start the thread.
   status = BusTools_RegisterFunction(cardnum, &sInt_hwi_FIFO1, REGISTER_FUNCTION);			
   printf("BusTools_RegisterFunction REGISTER_FUNCTION = %d\n",status);			

   // Now lets turn on our RT 
   status = BusTools_RT_StartStop(cardnum, RT_START);
   printf("BusTools_RT_StartStop RT_START = %d\n",status);   

			// OUR EVENT HANDLER IS RUNNING!
			// User input to stop and exit.
   printf("\nRT simulation running.  Hit Q to stop and exit.\n");
   getchar();
			
   status = BusTools_RT_StartStop(cardnum, RT_STOP);
   printf("BusTools_RT_StartStop RT_STOP = %d\n",status);			

   status = BusTools_RegisterFunction(cardnum, &sInt_hwi_FIFO1, UNREGISTER_FUNCTION);			
   printf("BusTools_RegisterFunction UNREGISTER_FUNCTION = %d\n",status);			
		
		// We're done.  Close API and board
   printf("\nClosing API . . . ");
   status = BusTools_API_Close(cardnum);
   printf("BusTools_API_Close = %d\n",status);

		 
	printf("FINISHED.\n");
} // End of main


/****************************************************************************
*
*  Function:  my_event_handler
*
*  Description:  Show how to register an interrupt thread and process the
*                resulting thread FIFO entries and interrupt calls.
*                This function processes the interrupt calls.
*
****************************************************************************/
BT_INT _stdcall my_event_handler(BT_UINT cardnum, struct api_int_fifo *sIntFIFO)
{
   BT_INT						tail;	           // FIFO Tail index
   BT_INT						status;
   API_RT_MBUF_READ				msg_buffer_read;
   static API_RT_MBUF_WRITE		msg_buffer_write;
   BT_UINT rt,sa,tr,wc,buf,i;


   printf("+");
   // Loop through all entries in the FIFO
   // Fetch entries from the FIFO: Get the tail pointer and extract the entry
   // it points to.   When (head == tail) then FIFO is empty.
   tail = sIntFIFO->tail_index;
   while ( tail != sIntFIFO->head_index )
   {
      
          //  Process a RT interrupt:
          //sIntFIFO->fifo[tail].buffer_off       // Byte address of buffer
      rt = sIntFIFO->fifo[tail].rtaddress;        // RT address
      tr = sIntFIFO->fifo[tail].transrec;         // Transmit/Receive
      sa = sIntFIFO->fifo[tail].subaddress;       // Subaddress number
      wc = sIntFIFO->fifo[tail].wordcount;        // Word count
      buf = sIntFIFO->fifo[tail].bufferID ;        // RT message number

	  if(wc==0)
		  wc=32;

				// Read the data received.
      status = BusTools_RT_MessageRead(cardnum, rt, sa, tr, buf, &msg_buffer_read);
				

				// Reverse the order of the 32 words and save to be written
				// into our transmit buffer.  Note that you could do just about
				// any desired manipulation of the data here.

      for (i=0; i<wc; i++) 
					msg_buffer_write.mess_data[i] = msg_buffer_read.mess_data[31 - i];

       // Now we have a new set of transmit data derived from the receive data.
      // Write the data to the transmit buffer.
      msg_buffer_write.enable = 0; // No interrupt
      msg_buffer_write.error_inj_id = 0;	// No error injection

      // Write data to the inactive buffer.
      status = BusTools_RT_MessageWrite(cardnum, 1, 2, 1, 
										sIntFIFO->fifo[tail].bufferID, &msg_buffer_write);		  

      // Now update and store the tail pointer.
      tail++;                         // Next entry
      tail &= sIntFIFO->mask_index;   // Wrap the index
      sIntFIFO->tail_index = tail;    // Save the index
   }
   return( API_SUCCESS );
}


