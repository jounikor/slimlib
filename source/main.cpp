// =======================================================================
//
// v0.2 (c) 2011 Jouni 'Mr.Spiv' Korhonen
//
// This is not GPL.
//
//
//
//
// =======================================================================


#include <ios>
#include <iostream>
#include <cstdio>
#include <cerrno>
#include <csignal>
#include <cstdlib>

#include "slimusb.h"
#include "slimexc.h"
#include "getopt.h"
#include "pcetools.h"

// =======================================================================
//
// Static configuration variables..
//

static slimLoaderV4* sl4 = NULL;
static int rom_start = 0;
static int rom_length = -1;
static bool mirror_tg16 = false;
static bool remove_header = false;
static bool patch_tg16 = false;
static char command = 0;
static bool aborted = false;	// was ctrl-c pressed?
static int rom_size = 128;
static bool mirror_3mbits = false;

// =======================================================================
//
// A callback class to read/write files.. There is no class definition of
// this outside this source file..
//

class rwcb: public buffercallback {
	bool m_tg16;
	FILE* m_fh;
	int m_total;
	bool m_first;
	bool m_patch;
	int m_position;
	bool m_3mbits;
public:
	rwcb( bool tg16=false, bool patch=false, bool m3=false ) {
		m_fh = NULL;
		m_tg16 = tg16;
		m_total = 0;
		m_first = true;
		m_patch = patch;
		m_position = 0;
		m_3mbits = m3;
	}
	~rwcb() {}
	bool recv(uint8_t* b, int l ) throw() {
		int n;

		if (aborted) {
			return false;
		}
		if (l < 0) {
			::printf("\rDumped total: %#-8x bytes at %#-8x\n",
					 m_total,m_position);
			::fflush(stdout);
			return true;
		}
		if (l == 0) {
			::printf("Dumping cart: %#-8x bytes at %#-8x",
					 m_total,m_position);
			::fflush(stdout);
			return true;
		}
		if (m_tg16) {
			pcetools::mirror_rom_bytes(b,l);
		}
		if ((n = ::fwrite(b, 1, l, m_fh)) < l) {
			::printf("\n");
			::fflush(stdout);
			return false;
		}

		m_total += n;
		::printf("\rDumping cart: %#-8x",m_total);
		::fflush(stdout);
		return true;
	}
	
	int send(uint8_t* b, int l) throw() {
		int n;

		if (aborted) {
			return -1;
		}		
		if (l <= 0) {
			::printf("\rFlashed total: %#-8x bytes at %#-8x\n",
					 m_total,m_position);
			::fflush(stdout);
			return true;
		}
		if ((n = ::fread(b,1,l,m_fh)) < 0) {
			::printf("\n");
			::fflush(stdout);
			return n;
		}
		if (m_first) {
			m_first = false;
			
			if (m_patch) {
				if (pcetools::patch_usa_rom(b,n,NULL,0) > 0) {
					::printf("USA ROM region code found and patched away\n");
				}
			}
		}
		if (m_tg16) {
			pcetools::mirror_rom_bytes(b,n);
		}
		
		n = (n + 3) & ~3;	// Rounding should only happen to the last
							// possible block..
		m_total += n;
		::printf("\rFlashing cart: %#-8x bytes at %#-8x",
				 m_total,m_position);
		::fflush(stdout);
		
		return n;
	}

	void set_file( FILE* fh ) {
		m_fh = fh;
		m_total = 0;
		m_first = true;
	}

	void set_position( int pos ) {
		m_position = pos;
	}
};

// =======================================================================
// For outputting the info strings read off the cart..

class infocb: public buffercallback {
public:
	infocb() {}
	~infocb() {}
	bool recv(uint8_t* b, int l ) throw() {
		if (l <= 0) {
			return true;
		}
		for (int n=0; n<l; n++) {
			::printf("%02X ",b[n]);
		}
		::printf("\n");
		
		return true;
	}
	
	int send(uint8_t* b, int l) throw() {
		return 0;
	}
	
};

// =======================================================================

static void ctrlc( int sig ) {
	aborted = true;
	::printf("\n**CTRL-C.. exiting..\n");
}

static void cleanup( void ) {
	if (sl4) {
		sl4->delInstance();
	}
}

// =======================================================================

static void usage( char* cmd ) {
	::printf("SL4lib flash tool v0.2 (c) 2011 Jouni 'Mr.Spiv' Korhonen\n\n"
	         "Usage: %s command [options] [files..]\n",cmd);
	::printf("Commands:\n"
			 "  i  Read cart ID information strings\n"
			 "  d  Dump cart to a file\n"
			 "  f  Flash file(s) to a cart\n"
			 "  v  Format and verify a cart (not implemented)\n"
			 "Options:\n"
			 "  -h        Display help\n"
			 "  -t        Mirror every byte (for TG16 compatibility)\n"
			 "  -p        Try patching away TG16 region check code\n"
			 "  -r        Remove 'ROM header'\n"
			 "  -s start  Set cart start position (in Mbits, default 0)\n"
			 "  -l length Set cart dump length (in Mbits, defaults to cart size)\n"
			 "  -m size   Set  cart size (in Mbits, default 128)\n"
                         "  -M        Mirroring fix for 3Mbits ROMs\n"
			 "Files..:\n"
			 "  For dumping only one file name is used.\n"
			 "  When flashing multiple ROM, then multiple files can be defined,\n"
			 "  unless the start position of the ROM is forced using the -s option.\n"
			 "  When flashing multiple ROMs, the first ROM always gets positioned\n"
			 "  at the beginning of the cart. The rest will be placed into an\n"
			 "  optimal place depending on the ROM size and available space.\n");
	
	exit(0);
}
					 
// =======================================================================

#define TMPLEN 16384

static int flash_rom( rwcb& rw, pcetools& pcetls, FILE* fh ) {
	int romlen, tmplen;
	int pos;
	uint8_t* tmpbuf;
	
	if ((tmpbuf = new (std::nothrow) uint8_t[16384]) == NULL) {
		return slimLoaderV4::ERR_SLV4_NOMEM;
	}
	
	rw.set_file(fh);

	::fseek(fh, 0, SEEK_END);
	romlen = ftell(fh);
	::fseek(fh, 0, SEEK_SET);

	if (remove_header) {
		// read a bit into tmp buffer
		pos = 0;
		
		if ((tmplen = ::fread(tmpbuf, 1, 16384, fh)) < 0) {
			delete[] tmpbuf;
			return slimLoaderV4::ERR_SLV4_FILEIO;
		}
		if (remove_header) {
			pos = pcetools::remove_header_pos(tmpbuf, tmplen, romlen);
		}
		if (pos > 0) {
			::printf("ROM header found (%d bytes).. removing..\n",pos);
			::fseek(fh, pos, SEEK_SET);
			romlen -= pos;
		} else {
			::fseek(fh, 0, SEEK_SET);
		}
	}

	delete[] tmpbuf;
	
	// Now we have the ROM length in bytes. Convert that to megabits
	// and ask for a proper position in the flash cart. Note that the
	// first rom will _always_ be located at 0x00000 in the cart. The
	// rest are placed into an optimal position based on the ROM
	// size and free space in the flash cart.
		
	romlen = (romlen +3 ) & ~3;	// align to 4
	tmplen = romlen * 8 / 1024 / 1024;

	if (rom_start > 0) {
		// It is possible to fix the rom pos.. be carefull
		pos = rom_start * 1024 * 1024 / 8;
	} else {
		if ((pos = pcetls.get_rom_pos(tmplen)) < 0) {
			return slimLoaderV4::ERR_SLV4_ALIGNMENT;
		}		
	}

	rw.set_position(pos);

	return sl4->cart_flash(pos, romlen, rw );
}

static int format_verify( rwcb& rw ) {
	return 0;
}


// =======================================================================
//
//
//
//
//
//
//
//
//
// =======================================================================
					 
int main (int argc, char * const argv[]) {
	int m,n;
	char ch;

	
	if (argc < 2) {
		usage(argv[0]);
	}
	
	// =======================================================================

	switch (argv[1][0]) {
		case 'd':
			command = 'd';
			break;
		case 'f':
			command = 'f';
			break;
		case 'i':
			command = 'i';
			break;
		case 'v':
			command = 'v';
			break;
		default:
			usage(argv[0]);
	}

	// =======================================================================

	optind = 2;             // start from second param..
					 
	while ((ch = getopt(argc,argv,"htprs:l:m:M")) != -1) {
		switch (ch) {
			case 't':
				mirror_tg16 = true;
				break;
			case 'p':
				patch_tg16 = true;
				break;
			case 'r':
				remove_header = true;
				break;
			case 's':
				rom_start = ::strtol(optarg,NULL,10);

				if (rom_start == 0 && (errno == EINVAL || errno == ERANGE)) {
					usage(argv[0]);
				}
				if (rom_start < 0) {
					usage(argv[0]);
				}
				if (rom_start > 127) {
					usage(argv[0]);
				}
				break;
			case 'l':
				rom_length = ::strtol(optarg,NULL,10);

				if (rom_length == 0 && (errno == EINVAL || errno == ERANGE)) {
					usage(argv[0]);
				}
				if (rom_length < 1) {
					usage(argv[0]);
				}
				if (rom_length > 128) {
					usage(argv[0]);
				}
				break;

			case 'm':
				rom_size = ::strtol(optarg,NULL,10);
				
				if (rom_size == 0 && (errno == EINVAL || errno == ERANGE)) {
					usage(argv[0]);
				}
				if (!(rom_size == 64 || rom_size == 128)) {
					usage(argv[0]);
				}
				break;
			case 'M':
				mirror_3mbits = true;
				break;	
				
			case 'h':
			default:
				usage(argv[0]);
		}
	}
	
	// =======================================================================
	
	try {
		sl4 = slimLoaderV4::getInstance();
	}
	catch (USBError& e) {
		::printf("**Error: %s\n", e.what());
		::printf("  %s:%d\n", e.file,e.line);
		exit(0);
	}
	catch (...) {
		::printf("**Error: slimLoaderV4::getInstance()\n");
		exit(0);
	}

	atexit( cleanup );
	
	if (sl4->claim_for_use(0,0) != 0) {
		::printf("**Error: claiming slimLoaderV4 interface failed\n");
		exit(0);
	}

	if (signal(SIGINT,ctrlc)) {
		::printf("**Warning: signal(SIGINT) failed.\n"); 
		exit(0);
	}

	// The following sanity check would be more meaningfull if we actually
	// knew the cart size.. currently the code does not provide such facility
	
	if (rom_length < 0) {
		rom_length = rom_size;
	}
	if (rom_start + rom_length > rom_size) {
		::printf("**Error: Boundaries out of %dbits cart\n",rom_size);
		exit(0);
	}
	
	// =======================================================================

	rwcb rw(mirror_tg16,patch_tg16,mirror_3mbits);
	infocb cb;
	FILE* fh;
	pcetools tools;
	n = 0;
	
	switch (command) {
		case 'i':  // Info
			::printf("Querying Neoflash card ID strings:\n");
			n = sl4->cart_query(cb);
			break;

		case 'd':  // dump cart
			// Do the sanity checks.
			if (rom_length < 0) {
				usage(argv[0]);
			}
			if (optind >= argc) {
				usage(argv[0]);
			}
			if ((fh = ::fopen(argv[optind],"w")) == NULL) {
				::printf("**Error: fopen(%s)\n",argv[optind]);
				exit(0);
			}
			
			rw.set_file(fh);
			rw.set_position(rom_start * 0x20000);
			n = sl4->cart_dump(rom_start * 0x20000, rom_length * 0x20000, rw);
			::fclose(fh);
			break;

		case 'f':	// flash cart
			m = argc - optind;
			tools.init_rom_pos(rom_size);
			
			if (rom_start > 0 && m > 1) {
				m = 1;
			}
			
			while (!aborted && m-- > 0) {
				::printf("Opening '%s' for flashing\n",argv[optind]);

				if ((fh = ::fopen(argv[optind],"r")) == NULL) {
					::printf("**Error: fopen(%s)\n",argv[optind]);
					exit(0);
				}
				if ((n = flash_rom(rw, tools, fh)) < 0) {
					m = 0;	// this exits the while() loop..
				}
				
				++optind;
				::fclose(fh);
			}
			
			break;
		
		case 'v':
			n = format_verify(rw);
			break;
			
		default:
			::printf("?? Unknown command\n");
			n = 0;
			break;
	}
	
	if (n < 0) {
		::printf("**Error '%s'\n",sl4->str_error(n));
	}
	
	
	// =======================================================================
    // =======================================================================
		
	sl4->delInstance();
	sl4 = NULL;
	
	return 0;
}
