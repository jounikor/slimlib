// =======================================================================
//
// v0.1 (c) 2011 Jouni 'Mr.Spiv' Korhonen
//
// This is not GPL.
//
//
//
//
// =======================================================================


#include <ios>
#include <iostream>
#include <new>
#include <memory>
#include "slimusb.h"

#include <cstring>
#include <cstdio>



//=================================================================

#define GET4S(b) ((((int8_t*)b)[0] << 24) |	\
                  (((int8_t*)b)[1] << 16) |	\
                  (((int8_t*)b)[2] << 8) |	\
				  (((int8_t*)b)[3]))


#define LGET4S(b) ((((int8_t*)b)[3] << 24) |	\
                   (((int8_t*)b)[2] << 16) |	\
                   (((int8_t*)b)[1] << 8) |	\
                   (((int8_t*)b)[0]))

#define LGET4U(b) ((((uint8_t*)b)[3] << 24) |	\
				(((uint8_t*)b)[2] << 16) |	\
				(((uint8_t*)b)[1] << 8) |	\
				(((uint8_t*)b)[0]))


#define GET4U(b) ((((uint8_t*)b)[0] << 24) |	\
                 (((uint8_t*)b)[1] << 16) |	\
                 (((uint8_t*)b)[2] << 8) |	\
                 (((uint8_t*)b)[3]))

#define LPUT4U(b,n) {((uint8_t*)b)[3] = (n >> 24);	\
				((uint8_t*)b)[2] = (n >> 16);	\
				((uint8_t*)b)[1] = (n >> 8);	\
				((uint8_t*)b)[0] = n; }	


//=================================================================

static uint8_t sl4_id_query_block[] = {
	0x00, 0xff, 0xd2, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x01, 0xd2, 0x00, 0x00, 0x02, 0x15, 0x00,
	0x00, 0xfe, 0x15, 0x00, 0x00, 0xe2, 0x15, 0x00,	0x00, 0xff, 0xd2, 0x00, 0x00, 0x00, 0x15, 0x00,
	0x00, 0x01, 0xd2, 0x00, 0x00, 0x02, 0x15, 0x00,	0x00, 0xfe, 0x15, 0x00, 0x00, 0x37, 0x20, 0x03,
	0x00, 0xff, 0xd2, 0x00, 0x00, 0x00, 0x15, 0x00,	0x00, 0x01, 0xd2, 0x00, 0x00, 0x02, 0x15, 0x00,
	0x00, 0xfe, 0x15, 0x00, 0x00, 0xda, 0x0e, 0x44,	0x00, 0xff, 0xd2, 0x00, 0x00, 0x00, 0x15, 0x00,
	0x00, 0x01, 0xd2, 0x00, 0x00, 0x02, 0x15, 0x00,	0x00, 0xfe, 0x15, 0x00, 0x00, 0xc4, 0x00, 0x00,
	0x05, 0x00, 0x05, 0x56, 0x00, 0xaa, 0x05, 0x00,	0x02, 0xab, 0x00, 0x55, 0x05, 0x00, 0x05, 0x56,
	0x00, 0x90, 0x05, 0x00, 0x05, 0x55, 0x00, 0xaa,	0x05, 0x00, 0x02, 0xaa, 0x00, 0x55, 0x05, 0x00,
	0x05, 0x55, 0x00, 0x90, 0x0c, 0x00, 0x00, 0x00,	0x08, 0x00, 0x00, 0x01, 0x08, 0x00, 0x00, 0x02,
	0x08, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0x00,	0x00, 0xff, 0x01, 0x00, 0x00, 0x01, 0x00, 0xff,
	0x01, 0x00, 0x00, 0x02, 0x00, 0xff, 0x01, 0x00,	0x00, 0x03, 0x00, 0xff, 0x01, 0x00, 0x00, 0x00,
	0x00, 0xf0, 0x01, 0x00, 0x00, 0x01, 0x00, 0xf0,	0x01, 0x00, 0x00, 0x02, 0x00, 0xf0, 0x01, 0x00,
	0x00, 0x03, 0x00, 0xf0		// len 0xc4
};

static uint8_t sl4_bank_select34[] = {	// cart query bank select
	0x05, 0x00, 0x00, 0x00, 0x90, 0x90, 0x05, 0x80,	0x00, 0x00, 0x90, 0x90, 0x0c, 0x00, 0x00, 0x00,
	0x08, 0x00, 0x00, 0x01, 0x08, 0x80, 0x00, 0x00,	0x08, 0x80, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00,
	0x00, 0xff, 0x01, 0x00, 0x00, 0x01, 0x00, 0xff,	0x01, 0x80, 0x00, 0x00, 0x00, 0xff, 0x01, 0x80,
	0x00, 0x01, 0x00, 0xff		// len 0x34
};

#if 1
static uint8_t sl4_unknown_id_block[] = {
	0x00, 0xff, 0xd2, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x01, 0xd2, 0x00, 0x00, 0x02, 0x15, 0x00,
	0x00, 0xfe, 0x15, 0x00, 0x00, 0xe2, 0x15, 0x00, 0x00, 0xff, 0xd2, 0x00, 0x00, 0x00, 0x15, 0x00,
	0x00, 0x01, 0xd2, 0x00, 0x00, 0x02, 0x15, 0x00, 0x00, 0xfe, 0x15, 0x00, 0x00, 0x37, 0x32, 0x02,
	0x00, 0xff, 0xd2, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x01, 0xd2, 0x00, 0x00, 0x02, 0x15, 0x00,
	0x00, 0xfe, 0x15, 0x00, 0x00, 0xda, 0x6e, 0x44, 0x00, 0xff, 0xd2, 0x00, 0x00, 0x00, 0x15, 0x00,
	0x00, 0x01, 0xd2, 0x00, 0x00, 0x02, 0x15, 0x00, 0x00, 0xfe, 0x15, 0x00, 0x00, 0xc4, 0x00, 0x00,
	0x05, 0x00, 0x05, 0x57, 0x00, 0xaa, 0x01, 0x00, 0x02, 0xac, 0x00, 0x55, 0x01, 0x00, 0x05, 0x57,
	0x00, 0x90, 0x01, 0x00, 0x05, 0x58, 0x00, 0xaa, 0x01, 0x00, 0x02, 0xad, 0x00, 0x55, 0x01, 0x00,
	0x05, 0x58, 0x00, 0x90, 0x01, 0x00, 0x05, 0x56, 0x00, 0xaa, 0x01, 0x00, 0x02, 0xab, 0x00, 0x55,
	0x01, 0x00, 0x05, 0x56, 0x00, 0x90, 0x01, 0x00, 0x05, 0x55, 0x00, 0xaa, 0x01, 0x00, 0x02, 0xaa,
	0x00, 0x55, 0x01, 0x00, 0x05, 0x55, 0x00, 0x90, 0x0c, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x01,
	0x08, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00, 0x03, 0x08, 0x00, 0x00, 0x04, 0x08, 0x00, 0x00, 0x05,
	0x08, 0x00, 0x00, 0x06, 0x08, 0x00, 0x00, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0x01, 0x00,
	0x00, 0x01, 0x00, 0xff, 0x01, 0x00, 0x00, 0x02, 0x00, 0xff, 0x01, 0x00, 0x00, 0x03, 0x00, 0xff,
	0x01, 0x00, 0x00, 0x04, 0x00, 0xff, 0x01, 0x00, 0x00, 0x05, 0x00, 0xff, 0x01, 0x00, 0x00, 0x06,
	0x00, 0xff, 0x01, 0x00, 0x00, 0x07, 0x00, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x01, 0x00,
	0x00, 0x01, 0x00, 0xf0, 0x01, 0x00, 0x00, 0x02, 0x00, 0xf0, 0x01, 0x00, 0x00, 0x03, 0x00, 0xf0,
	0x01, 0x00, 0x00, 0x04, 0x00, 0xf0, 0x01, 0x00, 0x00, 0x05, 0x00, 0xf0, 0x01, 0x00, 0x00, 0x06,
	0x00, 0xf0, 0x01, 0x00, 0x00, 0x07, 0x00, 0xf0, 0x00, 0xff, 0xd2, 0x00, 0x00, 0x00, 0x15, 0x00,
	0x00, 0x01, 0xd2, 0x00, 0x00, 0x02, 0x15, 0x00, 0x00, 0xfe, 0x15, 0x00, 0x00, 0xda, 0x8e, 0x44,
	0x00, 0xff, 0xd2, 0x00, 0x00, 0x00, 0x15, 0x00, 0x00, 0x01, 0xd2, 0x00, 0x00, 0x02, 0x15, 0x00,
	0x00, 0xfe, 0x15, 0x00, 0x00, 0xc4, 0x08, 0x00, 0x01, 0x00, 0x05, 0x56, 0x00, 0xaa, 0x01, 0x00,
	0x02, 0xab, 0x00, 0x55, 0x01, 0x00, 0x05, 0x56, 0x00, 0x90, 0x01, 0x00, 0x05, 0x55, 0x00, 0xaa,
	0x01, 0x00, 0x02, 0xaa, 0x00, 0x55, 0x01, 0x00, 0x05, 0x55, 0x00, 0x90, 0x08, 0x00, 0x00, 0x00,
	0x08, 0x00, 0x00, 0x01, 0x08, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00, 0x03, 0x01, 0x00, 0x00, 0x00,
	0x00, 0xff, 0x01, 0x00, 0x00, 0x01, 0x00, 0xff, 0x01, 0x00, 0x00, 0x02, 0x00, 0xff, 0x01, 0x00,
	0x00, 0x03, 0x00, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x01, 0x00, 0x00, 0x01, 0x00, 0xf0,
	0x01, 0x00, 0x00, 0x02, 0x00, 0xf0, 0x01, 0x00, 0x00, 0x03, 0x00, 0xf0		// len 0x1bc
};
#endif

static uint8_t sl4_unknown_sequence1[] = {
	0x00, 0xff, 0xd2, 0x00, 0x00, 0x00, 0x15, 0x00,
	0x00, 0x01, 0xd2, 0x00, 0x00, 0x02, 0x15, 0x00,
	0x00, 0xfe, 0x15, 0x00, 0x00, 0xe2, 0x15, 0x00,
	0x00, 0xff, 0xd2, 0x00, 0x00, 0x00, 0x15, 0x00,
	0x00, 0x01, 0xd2, 0x00, 0x00, 0x02, 0x15, 0x00,
	0x00, 0xfe, 0x15, 0x00, 0x00, 0x37, 0x20, 0x03,
	0x00, 0xff, 0xd2, 0x00, 0x00, 0x00, 0x15, 0x00,
	0x00, 0x01, 0xd2, 0x00, 0x00, 0x02, 0x15, 0x00,
	0x00, 0xfe, 0x15, 0x00, 0x00, 0xda, 0x0e, 0x44	// len 0x48 
};

static uint8_t sl4_bank_select24[] = {	// for writing
	0x21, 0xc0, 0x00, 0x10, 0x00, 0xff, 0x00, 0xff,	0xaa, 0x00, 0x55, 0x00, 0xff, 0x00, 0xe8, 0x00,
	0xd0, 0x00, 0x90, 0x00, 0x00, 0x00, 0x20, 0x00,	0xd0, 0x00, 0x60, 0x00, 0xd0, 0x00, 0x55, 0x05,
	0x00, 0xaa, 0x02, 0x00	// len 0x24
};



//=================================================================


device_info::device_info()
{
#ifdef SLV4_DEBUG
	std::cerr << "device_info::device_info()" << std::endl;
#endif
	configs = NULL;
}

device_info::~device_info()
{
#ifdef SLV4_DEBUG
	std::cerr << "device_info::~device_info()" << std::endl;
#endif
	if (configs) {	
		libusb_free_config_descriptor(configs);
	}
}



/*
//=================================================================

 
 Static stuff..
 
 */


slimLoaderV4* slimLoaderV4::m_instance = NULL;
int slimLoaderV4::m_refCnt = 0;

slimLoaderV4* slimLoaderV4::getInstance() throw (USBError, std::bad_alloc)
{
	int n;

	if (slimLoaderV4::m_instance == NULL) {
		m_instance = new slimLoaderV4();

		if ((n = m_instance->init()) < 0) {
			if (n == slimLoaderV4::ERR_SLV4_NOTFOUND) {
				throw  USBDeviceNotFound(__LINE__,__FILE__);
			} else {
				throw  USBDeviceInitError(__LINE__,__FILE__);
			}
		}
	} else {
		throw  USBDeviceInUseError(__LINE__,__FILE__);
	}
	
	return m_instance;
}

void slimLoaderV4::delInstance() throw()
{
	delete m_instance;
	m_instance = NULL;
}





//=================================================================

slimLoaderV4::slimLoaderV4() {
	m_refCnt = 1;
}


int slimLoaderV4::init( int level ) throw()
{
	// This sad function does not support multiple instances of SlimLoader4
	// connected to the USB bus.. to be fiexd.
	
	m_hndl = NULL;
	m_ctx = NULL;
	m_bulkin = -1;
	m_bulkout = -1;
	m_ttl = NEO_TTL;

	// open device here..
	if (libusb_init(&m_ctx) < 0) {
		return slimLoaderV4::ERR_SLV4_INIT;
	}

	// Quick open for SlimLoader 4..
	// No getting device list etc nice stuff.
	
	m_hndl = libusb_open_device_with_vid_pid(m_ctx, NEO_VENDOR, NEO_PRODUCT_IV);

	if (m_hndl == NULL) {
		libusb_exit(m_ctx);
		return slimLoaderV4::ERR_SLV4_NOTFOUND;
	}
	
	// Found and initialized..

	//libusb_set_debug(m_ctx, level);
	
	return slimLoaderV4::ERR_SLV4_OK;
}

int slimLoaderV4::claim_for_use( int iface, int alt ) throw ()
{
	const libusb_endpoint_descriptor* ep;
	int n;
	
	if (iface < 0) {
		return slimLoaderV4::ERR_SLV4_PARAM;
	}
	
	device_info* dev = get_device_info();
	
	if (dev == NULL) {
		return slimLoaderV4::ERR_SLV4_IFACE;
	}
	if (iface >= dev->configs->bNumInterfaces) {
		delete dev;
		return slimLoaderV4::ERR_SLV4_IFACE;
	}
	if (alt >= dev->configs->interface[iface].num_altsetting) {
		delete dev;
		return slimLoaderV4::ERR_SLV4_ALT;
	}
	
	for (n = 0; n < dev->configs->interface[iface].altsetting[alt].bNumEndpoints; n++) {
		ep = &dev->configs->interface[iface].altsetting[alt].endpoint[n];
		//
		// SlimloaderV4 actually has three bulk endpoints. 
		// 0x81 for bulk_in
		// 0x02 for bulk_out, which works
		// 0x03 for bulk_out, which does not work..
		//
		if ((ep->bmAttributes & 0x03) == LIBUSB_TRANSFER_TYPE_BULK) {
			if (ep->bEndpointAddress & 0x80 && m_bulkin < 0) {
				// LIBUSB_ENDPOINT_IN = 0x80
				m_bulkin = ep->bEndpointAddress;
#ifdef SLV4_DEBUG
				std::cerr << "bulk_in: " << static_cast<int>(m_bulkin) << std::endl;
#endif			
			
			} else if (m_bulkout < 0) {
				// LIBUSB_ENDPOINT_OUT = 0x00
				m_bulkout = ep->bEndpointAddress;
#ifdef SLV4_DEBUG
				std::cerr << "bulk_out: " << static_cast<int>(m_bulkout) << std::endl;
#endif
			}
		}
	}
	
	delete dev;

	// claim the interface.
	
	if (libusb_claim_interface(m_hndl, iface)) {
		return slimLoaderV4::ERR_SLV4_IFACE;
	}

	m_iface = iface;
	
	
	return slimLoaderV4::ERR_SLV4_OK;
}



void slimLoaderV4::done() throw()
{
	if (m_hndl) {
		libusb_release_interface(m_hndl,m_iface);
		libusb_close(m_hndl); 
		m_hndl = NULL;
	}
}

slimLoaderV4::~slimLoaderV4()
{
	done();

	if (m_ctx) {
		libusb_exit(m_ctx);
		m_ctx = NULL;
	}
}
 
 
device_info* slimLoaderV4::get_device_info() throw ()
{
	libusb_device* dev;
	device_info* nfo;

	dev = libusb_get_device(m_hndl);
	
	if (dev == NULL) {
		return NULL;
	}
	
	nfo = new (std::nothrow) device_info;

	if (nfo == NULL) {
		return NULL;
	}
	
	// read device descriptor..
	
	if (libusb_get_device_descriptor(dev, &nfo->desc) != LIBUSB_SUCCESS) {
		delete nfo;
		return NULL;
	}
	
	// read interfaces and all configs
	
	if (libusb_get_active_config_descriptor(dev, &nfo->configs) != LIBUSB_SUCCESS) {
		delete nfo;
		return NULL;
	}
	
	return nfo;
}


//=================================================================


int slimLoaderV4::recv_bulk_bytes_to_do( int bulktype ) throw ()
{
	uint8_t rb[8];
	int n;
	int hdr = bulktype == SLV4_CTRL ? 0x04fb0000 : 0x00ff0000;

	n = libusb_control_transfer( m_hndl,
								LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_VENDOR,
								0xa2,0x01,0x00,rb,sizeof(rb),m_ttl );
	
	if (n < 0) {
		m_usberror = n;
		return slimLoaderV4::ERR_SLV4_CTRL_IN;
	}
	if (GET4U(&rb[0]) != hdr) {
		m_usberror = 0;
		return slimLoaderV4::ERR_SLV4_CTRL_HDR;		
	}

	// The data amount is in little endian..
	//
	// When writing OUT to SL4 you can read back the amount to write using
	// this method.
	//
	// When reading, this method return 0 as a number of bytes to do..
	//
	//
	
	return LGET4U(&rb[4]);
}

int slimLoaderV4::m_wait_bulk( int times ) throw ()
{
	int n;

	do {
		n = recv_bulk_bytes_to_do(SLV4_DATA);

		if (n >= 0) {
#ifdef SLV4_DEBUG
			std::cerr << "m_wait_bulk() " << n << std::endl;
#endif
			return ERR_SLV4_OK;
		}
	} while (--times > 0);
	
	return ERR_SLV4_BULK_WAIT;
}


int slimLoaderV4::send_ctrl_msg_bulk_len( int l, int inout ) throw ()
{
	uint8_t wb[4];
	int n;
	uint16_t idx;

	//
	// idx = 1 when writing bulk data to the sl4 flasher
	// idx = 2 when reading bulk data from the sl4 flasher 
	// idx = 0 when writing bulk data to select a bank
	//
	
	switch (inout) {
		case SLV4_IN:
			idx = 2;
			break;
		case SLV4_OUT:
			idx = 1;
			break;
		case SLV4_OUT_BANK:
		default:
			idx = 0;
			break;
	}
	
	//
	
	LPUT4U(&wb[0],l);
	
	n = libusb_control_transfer( m_hndl,
								LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR,
								0xaa,0x01,idx,wb,sizeof(wb),m_ttl );
	if (n < 0) {
		m_usberror = n;
		return slimLoaderV4::ERR_SLV4_CTRL_OUT;
	}

	return n;
}


int slimLoaderV4::send_data_bulk_len( int sta, int len, int inout ) throw ()
{
	uint8_t wb[12+4];
	int n;
	int wl = inout == SLV4_OUT ? 16 : 12;
	int cmd = inout == SLV4_OUT ? 0xa1 : 0xa0;
	
	::memset(wb,0,16);
	
	// When constructing the actual command..
	// Start position is in unit of 16 bits -> divide by 2
	// Length is in unit of 32 bits -> divide by 4
	
	if (inout == SLV4_IN) {
		LPUT4U(&wb[4],sta>>1);
		LPUT4U(&wb[8],len>>2);
	} else {
		LPUT4U(&wb[8],sta>>1);
		LPUT4U(&wb[12],len>>2);
	}

	n = libusb_control_transfer( m_hndl,
								LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_VENDOR,
								cmd,0x01,0,wb,wl,m_ttl );
	if (n < 0) {
		m_usberror = n;
		return slimLoaderV4::ERR_SLV4_CTRL_OUT;
	}

	return n;
}



int slimLoaderV4::m_get_id_string( bool setup, uint8_t* idbuf ) throw(USBError)
{
	int n, m;

	if (setup) {
		if ((n = send_ctrl_msg_bulk_len(sizeof(sl4_id_query_block), SLV4_OUT)) < 0) {
			throw USBWriteCtrlError(__LINE__,__FILE__,n,
									"send_ctrl_msg_bulk_len(sl4_id_query_block,SLV4_OUT)");
		}
		if ((n = recv_bulk_bytes_to_do(SLV4_CTRL)) != sizeof(sl4_id_query_block)) {
			throw USBReadCtrlError(__LINE__,__FILE__,n,
								   "recv_bulk_bytes_to_do(SLV4_CTRL)");
		}
		if ((n = libusb_bulk_transfer(m_hndl, m_bulkout, &sl4_id_query_block[0],
								  sizeof(sl4_id_query_block), &m, m_ttl)) != 0) {
			m_usberror = n;
			throw USBWriteBulkError(__LINE__,__FILE__,ERR_SLV4_BULK_OUT,
									"libusb_bulk_transfer(out,sl4_id_query_block)");
		}
		if ((n = m_wait_bulk(BULK_WAIT_TIMES)) < 0) {
			throw USBWaitError(__LINE__,__FILE__,n,
							   "m_wait_bulk()");		
		}
	}

	// skip here if no init string is not sent..
	
	if ((n = send_ctrl_msg_bulk_len(16, SLV4_IN)) < 0) {
		throw USBWriteCtrlError(__LINE__,__FILE__,n,
								"send_ctrl_msg_bulk_len(16, SLV4_IN)");
	}

	// for reading.. todo returns 0..
	
	if ((n = recv_bulk_bytes_to_do(SLV4_CTRL)) != 0) {
		throw USBReadCtrlError(__LINE__,__FILE__,n,
							   "recv_bulk_bytes_to_do(SLV4_CTRL)");
	}
	if ((n = libusb_bulk_transfer(m_hndl,m_bulkin,idbuf,16,&m,m_ttl)) != 0) {
		m_usberror = n;
		throw USBReadBulkError(__LINE__,__FILE__,ERR_SLV4_BULK_IN,
								"libusb_bulk_transfer(16,in)");
	}
	if ((n = m_wait_bulk(BULK_WAIT_TIMES)) < 0) {
		throw USBWaitError(__LINE__,__FILE__,n,
						   "m_wait_bulk()");		
	}
			
	// we have now the id string.. output it..
	
	return ERR_SLV4_OK;
}

int slimLoaderV4::m_get_id_string2( uint8_t* idbuf24 ) throw(USBError)
{
	int n, m;
		
	if ((n = send_ctrl_msg_bulk_len( sizeof(sl4_unknown_id_block), SLV4_OUT)) < 0) {
		throw USBWriteCtrlError(__LINE__,__FILE__,n,
								"send_ctrl_msg_bulk_len(sl4_unknown_id_block,SLV4_OUT)");
	}
	if ((n = recv_bulk_bytes_to_do(SLV4_CTRL)) < 0) {
		throw USBReadCtrlError(__LINE__,__FILE__,n,
							   "recv_bulk_bytes_to_do(SLV4_CTRL)");
	}
	if ((n = libusb_bulk_transfer(m_hndl, m_bulkout, &sl4_unknown_id_block[0],
								  sizeof(sl4_unknown_id_block), &m, m_ttl)) != 0) {
		m_usberror = n;
		throw USBWriteBulkError(__LINE__,__FILE__,ERR_SLV4_BULK_OUT,
							   "libusb_bulk_transfer(444,out)");
	}
	if ((n = m_wait_bulk(BULK_WAIT_TIMES)) < 0) {
		throw USBWaitError(__LINE__,__FILE__,n,
						   "m_wait_bulk()");		
	}
	
	
	// skip here if no init string is not sent..
	
	if ((n = send_ctrl_msg_bulk_len(24, SLV4_IN)) < 0) {
		throw USBWriteCtrlError(__LINE__,__FILE__,n,
								"send_ctrl_msg_bulk_len(24, SLV4_IN)");
	}
	
	// for reading.. todo returns 0..
	
	if ((n = recv_bulk_bytes_to_do(SLV4_CTRL)) != 0) {
		throw USBReadCtrlError(__LINE__,__FILE__,n,
							   "recv_bulk_bytes_to_do(SLV4_CTRL)");
	}
	if ((n = libusb_bulk_transfer(m_hndl,m_bulkin,idbuf24,24,&m,m_ttl)) != 0) {
		m_usberror = n;
		throw USBReadBulkError(__LINE__,__FILE__,ERR_SLV4_BULK_IN,
								"libusb_bulk_transfer(24,in)");
	}
	if ((n = m_wait_bulk(BULK_WAIT_TIMES)) < 0) {
		throw USBWaitError(__LINE__,__FILE__,n,
						   "m_wait_bulk()");		
	}
	
	//
	//
	//
	//
	
	if ((n = send_ctrl_msg_bulk_len(sizeof(sl4_unknown_sequence1), SLV4_OUT)) < 0) {
		throw USBWriteCtrlError(__LINE__,__FILE__,n,
								"send_ctrl_msg_bulk_len(SLV4_OUT)");
	}
	
	// for reading.. todo returns 0..
	
	if ((n = recv_bulk_bytes_to_do(SLV4_CTRL)) != sizeof(sl4_unknown_sequence1)) {
		throw USBReadCtrlError(__LINE__,__FILE__,n,
							   "recv_bulk_bytes_to_do(SLV4_CTRL)");
	}
	if ((n = libusb_bulk_transfer(m_hndl,m_bulkout,sl4_unknown_sequence1,
								  sizeof(sl4_unknown_sequence1),&m,m_ttl)) != 0) {
		m_usberror = n;
		throw USBWriteBulkError(__LINE__,__FILE__,ERR_SLV4_BULK_OUT,
								"libusb_bulk_transfer(sl4_unknown_sequence1,out)");
	}
	if ((n = m_wait_bulk(BULK_WAIT_TIMES)) < 0) {
		throw USBWaitError(__LINE__,__FILE__,n,
						   "m_wait_bulk()");		
	}
	
	return ERR_SLV4_OK;
}


int slimLoaderV4::m_switch_bank( int bank, bool read ) throw(USBError)
{
	int m,n, len;
	uint8_t local_sl4_bank_select_block[sizeof(sl4_bank_select34)];
	
	// no idea how to select the bank actually ;)

	if (read == true) {
		len = sizeof(sl4_bank_select34);
		::memcpy(local_sl4_bank_select_block,sl4_bank_select34,len);
	} else {
		len = sizeof(sl4_bank_select24);
		::memcpy(local_sl4_bank_select_block,sl4_bank_select24,len);
	}
		
	local_sl4_bank_select_block[27] = bank;
	
	//
	
	if ((n = send_ctrl_msg_bulk_len( len, read ? SLV4_OUT : SLV4_OUT_BANK)) < 0) {
		throw USBWriteCtrlError(__LINE__,__FILE__,n,
								"send_ctrl_msg_bulk_len(SLV4_OUT/SLV4_OUT_BANK)");
	}
	if ((n = recv_bulk_bytes_to_do(SLV4_CTRL)) < 0) {
		throw USBReadCtrlError(__LINE__,__FILE__,n,
							   "recv_bulk_bytes_to_do(SLV4_CTRL)");
	}
	if ((n = libusb_bulk_transfer(m_hndl, m_bulkout, &local_sl4_bank_select_block[0],
								  len, &m, m_ttl)) < 0) {
		m_usberror = n;
		throw USBWriteBulkError(__LINE__,__FILE__,ERR_SLV4_BULK_OUT,
								"libusb_bulk_transfer(sl4_bank_select_block,out)");
	}
	if ((n = m_wait_bulk(BULK_WAIT_TIMES)) < 0) {
		throw USBWaitError(__LINE__,__FILE__,n,
						   "m_wait_bulk()");		
	}
	
	return ERR_SLV4_OK;
}



int slimLoaderV4::cart_query( buffercallback& cb ) throw ()
{
	uint8_t idbuf1[16];
	uint8_t idbuf2[16];
	uint8_t idbuf24[24];
	
	try {
		m_get_id_string( true, &idbuf1[0] );
		cb.recv(idbuf1, 16);
		m_switch_bank(0,true);
		m_get_id_string( false, &idbuf2[0] );
		cb.recv(idbuf2, 16);
		m_get_id_string2( &idbuf24[0] );
		cb.recv(idbuf24, 24);
	}
	catch (USBError& e) {
		return e.error;		
	}

	
	return ERR_SLV4_OK;
}


int slimLoaderV4::m_read_bulk_init( int sta, int len ) throw(USBError)
{
	int n;
	
	if ((n = send_data_bulk_len(sta,len,SLV4_IN)) < 0) {
		throw USBWriteCtrlError(__LINE__,__FILE__,n,
								"send_data_bulk_len(SLV4_IN)");
	}
	if ((n = recv_bulk_bytes_to_do(SLV4_CTRL)) < 0) {
		throw USBReadCtrlError(__LINE__,__FILE__,n,
							   "recv_bulk_bytes_to_do(SLV4_CTRL)");
	}
	return ERR_SLV4_OK;
}


int slimLoaderV4::m_write_bulk_init( int sta, int len ) throw(USBError)
{
	int n;
	
	if ((n = send_data_bulk_len(sta,len,SLV4_OUT)) < 0) {
		throw USBWriteCtrlError(__LINE__,__FILE__,n,
								"send_data_bulk_len(SLV4_OUT)");
	}
	if ((n = recv_bulk_bytes_to_do(SLV4_CTRL)) < 0) {
		throw USBReadCtrlError(__LINE__,__FILE__,n,
							   "recv_bulk_bytes_to_do(SLV4_CTRL)");
	}	

	return ERR_SLV4_OK;
}


int slimLoaderV4::cart_dump( int sta, int len, buffercallback& cb, int sze ) throw ()
{
	// DO NOT use other sze values than 131072 if you do not know
	// what you are really doing!
	
	int n,m, rlen;
	uint8_t* buf;
	
	if (sta & 0x1ffff) {
		// Must have 1Mbits alignment.
		// Technically, 32768bits alignment works when reading/writing
		// but it seems that cart rom start has minimum 1Mbits alignment.
		return ERR_SLV4_ALIGNMENT;
	}
	if (len & 3) {
		// round up to next 4 octets.
		len = (len + 3) & ~3;
	}
	if (sze & 3) {
		// round up to next 4 octets.
		sze = (sze + 3) & ~3;
	}
	if (sze < 4096) {
		sze = 4096;
	} else if (sze > 131072) {
		sze = 131072;
	}

	try {
		m_switch_bank(0,true);
	}
	catch (USBError& e) {
		return ERR_SLV4_USBERR;
	}
	
	if ((buf = new (std::nothrow) uint8_t[sze]) == NULL) {
		return ERR_SLV4_NOMEM;
	}

	rlen = 0;
	n = 0;

	while (cb.recv(buf,rlen) && len > 0) {
		if (len > sze) {
			m = sze;
		} else {
			m = len;
		}
		if (m & 3) {
			m = (m + 3) & ~3;
		}

		try {
			m_read_bulk_init(sta,m);
		}
		catch (USBError& e) {
			return ERR_SLV4_USBERR;
		}

		if ((n = libusb_bulk_transfer(m_hndl, m_bulkin, buf,
									  m, &rlen, m_ttl)) < 0) {
			m_usberror = n;
			return ERR_SLV4_USBERR;
		}
		
		len -= rlen;
		sta += rlen;

		if ((n = m_wait_bulk(BULK_WAIT_TIMES)) < 0) {
			break;
		}
	}

	cb.recv(buf,-1);	// we are done..
	delete[] buf;

	return n;
}

int slimLoaderV4::cart_flash( int sta, int len, buffercallback& cb, int sze ) throw ()
{
	// DO NOT use other sze values than 131072 if you do not know
	// what you are really doing!

	int n,m,wlen;
	uint8_t* buf;
	
	if (sta & 0x1ffff) {
		// Must have 1Mbits alignment.
		// Technically, 32768bits alignment works when reading/writing
		// but it seems that cart rom start has minimum 1Mbits alignment.
		return ERR_SLV4_ALIGNMENT;
	}
	if (len & 3) {
		// round up to next 4 octets.
		len = (len + 3) & ~3;
	}
	if (sze & 3) {
		// round up to next 4 octets.
		sze = (sze + 3) & ~3;
	}
	if (sze < 4096) {
		sze = 4096;
	} else if (sze > 131072) {
		sze = 131072;
	}
	if (sze > len) {
		sze = len;
	}

	try {
		m_switch_bank(0,false);
	}
	catch (USBError& e) {
		return ERR_SLV4_USBERR;
	}

	if ((buf = new (std::nothrow) uint8_t[sze]) == NULL) {
		return ERR_SLV4_NOMEM;
	}
	
	n = 0;
	
	while (len > 0 && ((m = cb.send(buf,sze)) > 0)) {
		if (m & 3) {
			m = (m + 3) & ~3;
		}
		
		try {
			m_write_bulk_init(sta,m);
		}catch (USBError& e) {
			return ERR_SLV4_USBERR;
		}
		
		if ((n = libusb_bulk_transfer(m_hndl, m_bulkout, buf,
									  m, &wlen, m_ttl)) < 0) {
			m_usberror = n;
			return ERR_SLV4_USBERR;
		}
		
		len -= wlen;
		sta += wlen;

		if ((n = m_wait_bulk(BULK_WAIT_TIMES)) < 0) {
			break;
		}
	}
	
	cb.send(buf,-1);	// we are done
	delete[] buf;
	
	return n;
}



int slimLoaderV4::cart_nop( int times ) throw()
{
	return m_wait_bulk(times);
}

int slimLoaderV4::re_init() throw() {
	return ERR_SLV4_OK;
}



//=================================================================

const char* slimLoaderV4::str_error( int err ) throw()
{
	switch (err) {
		case ERR_SLV4_OK:
			return "No error";
		case ERR_SLV4_NOTFOUND:
			return "SlimLoader IV not found";
		case ERR_SLV4_INIT:
			return "Failed to initialize libusb";
		case ERR_SLV4_IFACE:
			return "SlimLoader IV USB interface parameter error";
		case ERR_SLV4_NOMEM:
			return "Out or memory";
		case ERR_SLV4_PARAM:
			return "Parameter error";
		case ERR_SLV4_ALT:
			return "Wrong SlimLoader IV alternative interface";
		case ERR_SLV4_USBERR:
			return "libusb error";
		case ERR_SLV4_ALIGNMENT:
			return "Wrong ROM start address alignment";
		case ERR_SLV4_BULK_IN:
			return "Error reading bulk data";
		case ERR_SLV4_BULK_OUT:
			return "Error writing bulk data";
		case ERR_SLV4_CTRL_IN:
			return "Error reading control message";
		case ERR_SLV4_CTRL_OUT:
			return "Error writing control message";
		case ERR_SLV4_CTRL_HDR:
			return "Wrong header type received";
		case ERR_SLV4_BULK_WAIT:
			return "Timeout while waiting bulk transfer to complete";
		case ERR_SLV4_FILEIO:
			return "Error reading or writing a disk file";
		case ERR_SLV4_VERIFY:
			return "Error during verifying the flash cart";
		default:
			return "Unknown error code";
	}
	
	return NULL;
}


//=================================================================

