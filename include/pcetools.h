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

#ifndef _pcetools_h_included
#define _pcetools_h_included

#include <stdint.h>


/*
 *  pcetools.h
 *  sl4lib
 *
 *  Created by Jouni Korhonen on 2/13/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#define NEOFLASH_MAXROMS	64

class pcetools {
	char m_map_2m[NEOFLASH_MAXROMS];
	char m_map_4m[NEOFLASH_MAXROMS >> 1];
	char m_map_8m[NEOFLASH_MAXROMS >> 2];
	int m_pos_2m, m_pos_4m, m_pos_8m;
	int m_flashSize;
public:
	pcetools() {};
	~pcetools() {};
	void init_rom_pos( int=128 );
	int get_rom_pos( int );
	static int patch_usa_rom( uint8_t*, int, uint8_t* =0, int=0 );
	static int remove_header_pos( uint8_t*, int, int );
	static void mirror_rom_bytes( uint8_t*, int );
	static int mirror_3mbits_rom( uint8_t*, int, int );


};

#endif
