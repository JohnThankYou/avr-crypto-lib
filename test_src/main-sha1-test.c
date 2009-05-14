/* main-sha1-test.c */
/*
    This file is part of the AVR-Crypto-Lib.
    Copyright (C) 2008  Daniel Otte (daniel.otte@rub.de)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
 * SHA-1 test-suit
 * 
*/

#include "config.h"
#include "serial-tools.h"
#include "uart.h"
#include "debug.h"

#include "sha1.h"
#include "nessie_hash_test.h"
#include "hfal_sha1.h"
#include "hfal-performance.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "cli.h"
#include "shavs.h"
#include "hfal_sha1.h"
#include "dump.h"

char* algo_name = "SHA-1";

const hfdesc_t* algolist[] PROGMEM = {
	(hfdesc_t*)&sha1_desc,
	NULL
};
/*****************************************************************************
 *  additional validation-functions											 *
 *****************************************************************************/

void testrun_nessie_sha1(void){
	nessie_hash_ctx.hashsize_b  = 160;
	nessie_hash_ctx.blocksize_B = 512/8;
	nessie_hash_ctx.ctx_size_B  = sizeof(sha1_ctx_t);
	nessie_hash_ctx.name = algo_name;
	nessie_hash_ctx.hash_init = (nessie_hash_init_fpt)sha1_init;
	nessie_hash_ctx.hash_next = (nessie_hash_next_fpt)sha1_nextBlock;
	nessie_hash_ctx.hash_last = (nessie_hash_last_fpt)sha1_lastBlock;
	nessie_hash_ctx.hash_conv = (nessie_hash_conv_fpt)sha1_ctx2hash;
	
	nessie_hash_run();
}

/*****************************************************************************
 *  self tests																 *
 *****************************************************************************/

void sha1_ctx_dump(sha1_ctx_t *s){
	uint8_t i;
	cli_putstr("\r\n==== sha1_ctx_dump ====");
	for(i=0;i<5;++i){
		cli_putstr("\r\na["); cli_hexdump(&i, 1); cli_putstr("]: ");
		cli_hexdump(&(s->h[i]), 4);
	}
	cli_putstr("\r\nlength"); cli_hexdump(&i, 8);
} 

void testrun_sha1(void){
	sha1_hash_t hash;
	sha1(&hash,"abc",3*8);
	cli_putstr("\r\nsha1(\"abc\") = \r\n\t");
	cli_hexdump(hash,SHA1_HASH_BITS/8);
	
	sha1(&hash,"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",448);
	cli_putstr("\r\nsha1(\"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq\") = \r\n\t");
	cli_hexdump(hash,SHA1_HASH_BITS/8);
	
	cli_putstr("\r\nsha1(1,000,000 * 'a') = \r\n\t");
	{
		uint8_t block[SHA1_BLOCK_BITS/8];
		uint16_t i;
		sha1_ctx_t s;
		memset(block,'a',SHA1_BLOCK_BITS/8);
		sha1_init(&s);
		for(i=0;i<15625; ++i){ /* (1000000/(SHA1_BLOCK_BITS/8)) */
			sha1_nextBlock(&s, block);
		}
		sha1_lastBlock(&s,block,0);
		sha1_ctx2hash(&hash, &s);
	}
	cli_hexdump(hash,SHA1_HASH_BITS/8);
	

	cli_putstr("\r\nx");
}


void testrun_sha1_2(void){
	sha1_ctx_t ctx;
	sha1_hash_t hash;
	sha1(&hash,"",0);
	cli_putstr("\r\nsha1(NULL) = \r\n\t");
	cli_hexdump(hash,SHA1_HASH_BYTES);

	memset(hash, 0, SHA1_HASH_BYTES);

	sha1_init(&ctx);
	sha1_lastBlock(&ctx, "", 0);
	sha1_ctx2hash(&hash, &ctx); 
	cli_putstr("\r\nsha1(NULL) = \r\n\t");
	cli_hexdump(hash,SHA1_HASH_BYTES);
}


void testrun_performance_sha1(void){
	hfal_performance_multiple(algolist);
}


/*****************************************************************************
 *  main																	 *
 *****************************************************************************/

const char nessie_str[]      PROGMEM = "nessie";
const char test_str[]        PROGMEM = "test";
const char test2_str[]       PROGMEM = "test2";
const char performance_str[] PROGMEM = "performance";
const char echo_str[]        PROGMEM = "echo";
const char shavs_list_str[]  PROGMEM = "shavs_list";
const char shavs_set_str[]   PROGMEM = "shavs_set";
const char shavs_test1_str[] PROGMEM = "shavs_test1";
const char dump_str[]        PROGMEM = "dump";


cmdlist_entry_t cmdlist[] PROGMEM = {
	{ nessie_str,          NULL, testrun_nessie_sha1},
	{ test_str,            NULL, testrun_sha1},
	{ test2_str,           NULL, testrun_sha1_2},
	{ performance_str,     NULL, testrun_performance_sha1},
	{ echo_str,        (void*)1, (void_fpt)echo_ctrl},
	{ shavs_list_str,      NULL, shavs_listalgos},
	{ shavs_set_str,   (void*)1, (void_fpt)shavs_setalgo},
	{ shavs_test1_str,     NULL, shavs_test1},
	{ dump_str,        (void*)1, (void_fpt)dump},
	{ NULL,                NULL, NULL}
};

int main (void){
	DEBUG_INIT();
	
	cli_rx = uart_getc;
	cli_tx = uart_putc;	 
	shavs_algolist=(hfdesc_t**)algolist;
	shavs_algo=(hfdesc_t*)&sha1_desc;	
	for(;;){
		cli_putstr_P(PSTR("\r\n\r\nCrypto-VS ("));
		cli_putstr(algo_name);
		cli_putstr_P(PSTR(")\r\nloaded and running\r\n"));
		cmd_interface(cmdlist);
	}
}
