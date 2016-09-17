// =======================================================================
//
// v0.2 (c) 2011 Jouni 'Mr.Spiv' Korhonen
// v0.3 (c) 2016 fixed a precedence error
//
// This is not GPL.
//
//
//
//
// =======================================================================

#include <string.h>
#include "pcetools.h"

// Merry and Happy !!   whatever happened to the fplooy? - The hattifnatters ate it.


static char alignment_table[32] = {
	0x02, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
	0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08
};

static uint8_t mtab[256]= 
{
	0x00,0x80,0x40,0xc0,0x20,0xa0,0x60,0xe0,0x10,0x90,0x50,0xd0,0x30,0xb0,0x70,0xf0,
	0x08,0x88,0x48,0xc8,0x28,0xa8,0x68,0xe8,0x18,0x98,0x58,0xd8,0x38,0xb8,0x78,0xf8,
	0x04,0x84,0x44,0xc4,0x24,0xa4,0x64,0xe4,0x14,0x94,0x54,0xd4,0x34,0xb4,0x74,0xf4,
	0x0c,0x8c,0x4c,0xcc,0x2c,0xac,0x6c,0xec,0x1c,0x9c,0x5c,0xdc,0x3c,0xbc,0x7c,0xfc,
	0x02,0x82,0x42,0xc2,0x22,0xa2,0x62,0xe2,0x12,0x92,0x52,0xd2,0x32,0xb2,0x72,0xf2,
	0x0a,0x8a,0x4a,0xca,0x2a,0xaa,0x6a,0xea,0x1a,0x9a,0x5a,0xda,0x3a,0xba,0x7a,0xfa,
	0x06,0x86,0x46,0xc6,0x26,0xa6,0x66,0xe6,0x16,0x96,0x56,0xd6,0x36,0xb6,0x76,0xf6,
	0x0e,0x8e,0x4e,0xce,0x2e,0xae,0x6e,0xee,0x1e,0x9e,0x5e,0xde,0x3e,0xbe,0x7e,0xfe,
	0x01,0x81,0x41,0xc1,0x21,0xa1,0x61,0xe1,0x11,0x91,0x51,0xd1,0x31,0xb1,0x71,0xf1,
	0x09,0x89,0x49,0xc9,0x29,0xa9,0x69,0xe9,0x19,0x99,0x59,0xd9,0x39,0xb9,0x79,0xf9,
	0x05,0x85,0x45,0xc5,0x25,0xa5,0x65,0xe5,0x15,0x95,0x55,0xd5,0x35,0xb5,0x75,0xf5,
	0x0d,0x8d,0x4d,0xcd,0x2d,0xad,0x6d,0xed,0x1d,0x9d,0x5d,0xdd,0x3d,0xbd,0x7d,0xfd,
	0x03,0x83,0x43,0xc3,0x23,0xa3,0x63,0xe3,0x13,0x93,0x53,0xd3,0x33,0xb3,0x73,0xf3,
	0x0b,0x8b,0x4b,0xcb,0x2b,0xab,0x6b,0xeb,0x1b,0x9b,0x5b,0xdb,0x3b,0xbb,0x7b,0xfb,
	0x07,0x87,0x47,0xc7,0x27,0xa7,0x67,0xe7,0x17,0x97,0x57,0xd7,0x37,0xb7,0x77,0xf7,
	0x0f,0x8f,0x4f,0xcf,0x2f,0xaf,0x6f,0xef,0x1f,0x9f,0x5f,0xdf,0x3f,0xbf,0x7f,0xff
};


void pcetools::mirror_rom_bytes( uint8_t* rom, int romsize )
{
	uint8_t* dst = rom;
	
	while (romsize-- > 0) {
		*dst++ = mtab[*rom++];
	}
	 
}



int pcetools::remove_header_pos( uint8_t* rom, int buflen, int romsize )
{
	int shft = romsize & 0x1fff;

	if (romsize < 0x2000 || buflen < 0x2000) {
		// Cannot handle ROMs smaller than one page..
		return -2;
	}
	if ((romsize & 0x1ffff) == 0) {
		// 1Mbits even..
		return 0;
	}
	if (rom[shft + 0x1fff] >= 0xe0) {
		return shft;
	}
	return -1;
}



void pcetools::init_rom_pos( int szmbits )
{
	
	// the size is in mbits.. no sanity checks but
	// so far 64Mbits and 128Mbits exist
	m_flashSize = szmbits * 1024 * 1024 / 8;
	
	//
	::memset(m_map_2m,0,NEOFLASH_MAXROMS);
	::memset(m_map_4m,0,NEOFLASH_MAXROMS >> 1);
	::memset(m_map_8m,0,NEOFLASH_MAXROMS >> 2);
	m_pos_2m = 0;
	m_pos_4m = 0;
	m_pos_8m = 0;	
}


int pcetools::get_rom_pos( int romsize ) {
	int alignment;
	int pos;
	int n;

	if (romsize > 24) { // largest known PCE ROM
		return -1;
	}
	
	alignment = alignment_table[romsize];
	
	switch (alignment) {
		case 2:
			pos = m_pos_2m;
			break;
			
		case 4:
			pos = m_pos_4m << 1;
			break;
			
		case 8:
			pos = m_pos_8m << 2;
			break;
			
		default:
			return -1;
	}
	
	if ((pos + romsize) * 0x40000 > m_flashSize) {
		return -2;
	}
	
	// fill in..
	
	n = pos;
	
	while (romsize > 0) {
		m_map_4m[n >> 1] = 1;
		m_map_8m[n >> 2] = 1;
		m_map_2m[n++] = 1;
		romsize -= 2;
	}
	
	while (m_pos_2m < NEOFLASH_MAXROMS && m_map_2m[m_pos_2m]) { m_pos_2m++; }
	while (m_pos_4m < (NEOFLASH_MAXROMS >> 1) && m_map_4m[m_pos_4m]) { m_pos_4m++; }
	while (m_pos_8m < (NEOFLASH_MAXROMS >> 2) && m_map_8m[m_pos_8m]) { m_pos_8m++; }
	
	return pos * 0x40000;	// 2Mbits steps..
}


//
// chksum is supposed to be the MD5 hash over the rom image.. however
// this stuff is unimplemented yet
//

static uint8_t patchCode[6] = {0xad,0x00,0x10,0x29,0x40,0xf0};

// TG-16 pacthing stuff.. very basic..

int pcetools::patch_usa_rom( uint8_t* rom, int romlen, uint8_t* md5, int offs )
{
	int n, m;
	
	// search only the first 8K of the rom for the following code:
	//
	//  LDA $1000	; AD 00 10
	//  AND #$40    ; 29 40
	//  BEQ ...     ; F0
	//
	// which is the most common USA region check..
	
	n = 0;
	
	while (n < 8192 && n+offs < romlen) {
		m = 1;
		if (rom[offs+n] == patchCode[0]) {
			for (; m < 6; m++) {
				if (rom[offs+n+m] != patchCode[m]) {
					break;
				}
			}
			if (m == 6) {
				rom[offs+n+5] = 0x80;
				return offs+n;
			}
		}
		n += m;		// this optimization can be done as 0xAD does not
		// appear anywhere in the patchCode array.
	}
	
	return -1;
}

//

int pcetools::mirror_3mbits_rom( uint8_t* rom, int romsize, int bufsize ) {



	return -1;
}