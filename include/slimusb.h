// =======================================================================
//
// v0.1 (c) 2011 Jouni 'Mr.Spiv' Korhonen
// v0.2 (c) 2016 fixed a compiler warning 
//
// This is not GPL.
//
//
//
//
// =======================================================================
#ifndef _usbsupport_h
#define _usbsupport_h
//=============================================================================

#include <exception>
#include "libusb.h"
#include "slimexc.h"

//

#define PCE_ROM_ALIGNMENT	0x40000   // 2Mbits
#define NEO_VENDOR			0xffab		// Neoflash
#define NEO_PRODUCT_IV		0xdd03		// Neoflash v4
#define NEO_TTL				8000
#undef	SLV4_DEBUG
#define BULK_WAIT_TIMES		10


//=============================================================================



//=============================================================================


class buffercallback {
public:
	virtual ~buffercallback() {}
	
	// input: buffer with data from the SL4/cart,
	//        length in octets, 0 for a "nop", -1 for EOF
	// return: true if continue, false if abort
	
	virtual bool recv( uint8_t*, int ) throw()=0;

	// input: buffer to save data into to SL4/cart
	//        length of buffer, 0 for a "nop", -1 if no more data wanted
	// return: number of octets loaded, 0 is no data, -1 if error/abort
	//         (two lower bits MUST always be 0, max 131072)

	virtual int send( uint8_t*, int ) throw()=0;
};

class null_cb : public buffercallback {
public:
	null_cb() {}
	~null_cb() {}
	bool recv( uint8_t* a, int b ) throw() {
		return true;
	}
	int send( uint8_t* a, int b) throw() {
		return 0;
	}
};

static null_cb NULL_cb;

//=============================================================================

#if 0		// this stuff is still in progress
struct cart_info {
	uint8_t type;
	uint8_t banks;
	uint8_t chips;
	uint8_t pad;
	int cart_size_bytes;
	int ram_size_bytes;
	int ram_base;

	cart_info() {
		type=0; banks=0; chips=0; cart_size_bytes=0;
		ram_base=0; ram_size_bytes=0;
	}
	~cart_info() {}
};
#endif


//=============================================================================

struct device_info {
	libusb_device_descriptor desc;
	libusb_config_descriptor* configs;
	
	device_info();
	~device_info();
};

//=============================================================================

class slimLoaderV4 {
private:
	libusb_context* m_ctx;
	libusb_device_handle* m_hndl;
	int m_index;
	int m_iface;
	int m_bulkin;
	int m_bulkout;
	int m_cfg;
	int m_usberror;
	int m_ttl;
	//
	static slimLoaderV4* m_instance;
	static int m_refCnt;

	slimLoaderV4();
	int init( int=0 ) throw();
	void done() throw();

	int m_get_id_string( bool, uint8_t* ) throw(USBError);
	int m_get_id_string2( uint8_t* ) throw(USBError);
	int m_switch_bank( int, bool ) throw(USBError);
	int m_read_bulk_init( int, int ) throw(USBError);
	int m_write_bulk_init( int, int ) throw(USBError);
	int m_wait_bulk( int ) throw();
protected:
	~slimLoaderV4();
public:
	static slimLoaderV4* getInstance() throw (USBError, std::bad_alloc);
	static void delInstance() throw();

	// USB primitives do not throw exceptions

	int claim_for_use( int, int ) throw();
	int get_usb_error() throw () { return m_usberror; }
	int recv_bulk_bytes_to_do( int ) throw();
	int send_ctrl_msg_bulk_len( int, int ) throw();
	int send_data_bulk_len( int, int, int ) throw();
	int re_init() throw();
	device_info* get_device_info() throw();

	// Combined SL4+USB things may throw exceptions.
	// These are supposed to be the main utility functions
	// when accessing the flash cart.

	//int cart_detect( cart_info& ) throw();
	int cart_query( buffercallback& = NULL_cb ) throw();
	int cart_dump( int, int, buffercallback&, int=131072 ) throw();
	int cart_flash( int, int, buffercallback&, int=131072 ) throw();
	int cart_nop( int ) throw();
	int cart_format() throw();
	const char* str_error( int ) throw();
	
	//int ram_dump( int, int, buffercallback&, int=65536 ) throw();
	//int ram_flash( int, int, buffercallback&, int=65536 ) throw();
	
	// Error codes returned by all libusb wrapper functions.
	enum errs {
		ERR_SLV4_OK=		0,
		ERR_SLV4_NOTFOUND=	-1,
		ERR_SLV4_INIT=		-2,
		ERR_SLV4_IFACE=		-3,
		ERR_SLV4_NOMEM=		-4,
		ERR_SLV4_PARAM=		-5,
		ERR_SLV4_ALT=		-6,
		ERR_SLV4_USBERR=	-7,	//
		ERR_SLV4_ALIGNMENT=	-8,
		ERR_SLV4_FILEIO=	-9,	
		
		ERR_SLV4_BULK_IN=	-100,
		ERR_SLV4_BULK_OUT=	-101,
		ERR_SLV4_CTRL_IN=	-102,
		ERR_SLV4_CTRL_OUT=	-103,
		ERR_SLV4_CTRL_HDR=	-104,	// wrong type returned on
									// 00ff0000 or 04fb0000
		ERR_SLV4_BULK_WAIT=	-105,	// 
		ERR_SLV4_VERIFY=    -106
		
	};
	enum direction {
		SLV4_IN,
		SLV4_OUT,
		SLV4_OUT_BANK
	};
	enum bulktype {
		SLV4_DATA,
		SLV4_CTRL
	};
	
};

//-----------------------------------------------------------------------------
/* Known command sequences.. this definitely uncomplete and probably contains
   loads of misinformation.
 
   There are number of "control messages" listed here as a 4 octet sequences
   such as 0xc0 0xa2 0x01 0x00 or 0x40 0xaa 0x01 0x01. These mean:
 
   1. octet request_type 0xc0 = LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR
			             0x40 = LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR
   2. octet = request
   3. octet = value
   4. octet = index
 

1) Read remaining bulk bytes to arrive or bytes sent to the SL4. Also used to
   report back when SL4 has completed its bulk transfer.
   or finished its task. There are three known to me uses of this command.
   All 4 octet long words are in little endian.
 
   Send control message for reading 8 octets:
    0xc0 0xa2 0x01 0x00 length 0x08 (i.e. you need 8 octet input buffer)
 
   1.1) After sending other control message using this command you can read how
        many octets there are remaining for you to send using bulk transfer to
        the SL4. In that case the 0xc0 0xa2 0x01 0x00 command retunrs:
 
		-> 0x04fb0000 0xnnnnnnnn, where nn's are octets to be sent to the SL4
 
   1.2) After bulk in/out data transfer (to the cart, not to control the SL4)
        you can read how many octets are still pending. In that case the
        0xc0 0xa2 0x01 0x00 command retunrs:
 
        -> 0x00ff0000 0xnnnnnnnn, where nn's >= 0 when transfer is complete
        -> 0x04fb0000 0xnnnnnnnn, where nn's is (pending octets)>>2
 
        There is only one case I know when 0x00ff0000 0xnnnnnnnn is returned
		and nn's is something else than 0. That is when I send the 36 octets
        length bank command to the SL4 after 0x40 0xaa 0x01 0x01 command.
        Beats me.. 
 
2) Send a control message to set the length of the following extended 
   command sequence that is the sent over bulk data. This command is used for
   multiple purposes and has three known to me uses:
   - Sending a control message to read 'len' octets from SL4.
   - Sending a control message to write 'len' octet to the SL4.
   - Sending control messages to set cart bank.
 
   Send a control message for reading from SL4:
    0x40 0xaa 0x01 0x02 and 4 octets of 'len'
   Send a control message for writing to SL4:
    0x40 0xaa 0x01 0x01 and 4 octets of 'len'
   Send a control message setting the bank (also write):
    0x40 0xaa 0x01 0x00 and 4 octets of 'len'

   The 4 'len' octets are in little endian. After sending this command you
   are supposed to 'ack' it using the command in (1). After that the bulk
   transfer of 'len' octets follow (index 0x02 for reading, indices 0x00 and
   0x02 for writing). After the bulk transfer 'ack' it using the command in
   (1) again.
 

3) Send a control message to read or write the flash cart over bulk data.
   With this command it is possible to read or write the flash cart at any
   1Mbits divisible cart position in units of 4 octets. From the SL4 to stay
   in a "sane" state, reading and writing should be done at 1Mbits chunks.
   Write and read lengths can be smaller but in that case if the USB bulk 
   transfer gets prematurely aborted, the SL4 ends up to a confused state and
   reseting the SL4 is needed (more or less..).
 
   Send a control message to set read 'len' octets and 'pos':
    0x40 0xa0 0x01 0x00 and 12 octets of command data
     0x00 0x00 0x00 0x00 '4 octet pos'>>1 '4 octet len'>>2

    then read from bulk 'len' octets in maximum 131072 octet blocks (1Mbits).
 
   Send a control message to set write 'len' octets and 'pos':
    0x40 0xa0 0x01 0x00 and 12 octets of command data
     0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 '4 octet pos'>>1 '4 octet len'>>2
 
    then write to bulk 'len' octets in maximum 131072 octet blocks (1Mbits).
    All 4 octet long words are in little endian.
 
*/
//-----------------------------------------------------------------------------
#endif // _usbsupport_h
//-----------------------------------------------------------------------------
