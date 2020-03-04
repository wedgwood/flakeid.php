/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "flakeid.h"
#include "system.h"

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_flakeid.h"

ZEND_DECLARE_MODULE_GLOBALS(flakeid)

/* True global resources - no need for thread safety here */
static int le_flakeid;
static char if_name[64];

/* {{{ PHP_INI
 */
/*
 * Remove comments and fill if you need to have entries in php.ini
 */
PHP_INI_BEGIN()
	STD_PHP_INI_ENTRY("flakeid.if_name", "eth0", PHP_INI_ALL, OnUpdateStringUnempty, if_name, zend_flakeid_globals, flakeid_globals)
	STD_PHP_INI_ENTRY("flakeid.enable_spoof", "on", PHP_INI_ALL, OnUpdateBool, enable_spoof, zend_flakeid_globals, flakeid_globals)
PHP_INI_END()
/* }}} */

/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ php_flakeid_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_flakeid_init_globals(zend_flakeid_globals *flakeid_globals)
{
	flakeid_globals->global_value = 0;
	flakeid_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
static inline uint64_t __now_ms() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000 + (tv.tv_usec / 1000);
}

static inline void __make_hexstr(unsigned char *out, const unsigned char *raw, int len) {
	static const char *hex = "0123456789abcdef";
	int i;

	for (i = 0; i < len; i++) {
		out[i * 2]       = hex[raw[i] >> 4];
		out[(i * 2) + 1] = hex[raw[i] & 0x0F];
	}
}
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(flakeid)
{
	REGISTER_INI_ENTRIES();
	unsigned char addr[6];
	FLAKEID_G(last_flush_key) = 0;

	if (FLAKEID_G(if_name) && !get_ifaddr(FLAKEID_G(if_name), addr)) {
		FLAKEID_G(flakeid_ctx) = flakeid_ctx_create(addr, 6);
		get_ipv4(FLAKEID_G(if_name), &FLAKEID_G(ipv4));
		memcpy(FLAKEID_G(addr), addr, 6);
	} else if (!get_first_ifaddr(addr, if_name, sizeof(if_name))) {
		FLAKEID_G(flakeid_ctx) = flakeid_ctx_create(addr, 6);
		get_ipv4(if_name, &FLAKEID_G(ipv4));
		memcpy(FLAKEID_G(addr), addr, 6);
	} else if (FLAKEID_G(enable_spoof)) {
		FLAKEID_G(flakeid_ctx) = flakeid_ctx_create_with_spoof(addr);
		FLAKEID_G(ipv4) = *(uint32_t *)(addr + 2);
		memcpy(FLAKEID_G(addr), addr, 6);
	} else {
		FLAKEID_G(flakeid_ctx) = NULL;
		return FAILURE;
	}


	FLAKEID_G(flakeid64_ctx) = flakeid64_ctx_create_with_spoof(NULL);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(flakeid)
{
	UNREGISTER_INI_ENTRIES();

	if (FLAKEID_G(flakeid_ctx)) {
		flakeid_ctx_destroy(FLAKEID_G(flakeid_ctx));
		FLAKEID_G(flakeid_ctx) = NULL;
	}

	if (FLAKEID_G(flakeid64_ctx)) {
		flakeid64_ctx_destroy(FLAKEID_G(flakeid64_ctx));
		FLAKEID_G(flakeid64_ctx) = NULL;
	}

	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(flakeid)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(flakeid)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(flakeid)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "flakeid support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

ZEND_FUNCTION(flakeid_generate)
{
	zend_bool raw_output = 0;
	char *delimiter_str = NULL;
	size_t delimiter_strlen = 0;

	if (zend_parse_parameters(
		ZEND_NUM_ARGS() TSRMLS_CC,
		"|bs",
		&raw_output,
		&delimiter_str,
		&delimiter_strlen) == FAILURE
	) {
		RETURN_NULL();
	}

	unsigned char id[16];
	flakeid_get(FLAKEID_G(flakeid_ctx), id);

	if (raw_output) {
		RETURN_STRINGL(id, 16);
	} else {
		char delimiter = '-';
		size_t len = 35;

		if (delimiter_str) {
			if (delimiter_strlen > 0) {
				delimiter = delimiter_str[0];
			} else {
				delimiter = 0;
				len = 32;
			}
		}

		unsigned char hexstr[len];
		flakeid_hexdump(id, delimiter, hexstr);

		RETURN_STRINGL(hexstr, len);
	}
}

ZEND_FUNCTION(flakeid_generate64)
{
	zend_bool raw_output = 0;

	if (zend_parse_parameters(
		ZEND_NUM_ARGS() TSRMLS_CC,
		"|b",
		&raw_output) == FAILURE
	) {
		RETURN_NULL();
	}

	int64_t id;
	flakeid64_get(FLAKEID_G(flakeid64_ctx), &id);

	if (raw_output) {
		RETURN_STRINGL((char *)&id, 8);
	} else {
		unsigned char hexstr[16];
		flakeid64_hexdump(id, hexstr);
		RETURN_STRINGL(hexstr, 16);
	}
}

ZEND_FUNCTION(flakeid_get_ipv4)
{
	zend_bool raw_output = 0;

	if (zend_parse_parameters(
		ZEND_NUM_ARGS() TSRMLS_CC,
		"|b",
		&raw_output) == FAILURE
	) {
		RETURN_NULL();
	}

	if (raw_output) {
		RETURN_STRINGL((char *)&FLAKEID_G(ipv4), 4);
	} else {
		unsigned char hexstr[8];
		__make_hexstr(hexstr, (char *)&FLAKEID_G(ipv4), 4);
		RETURN_STRINGL(hexstr, 8);
	}
}

ZEND_FUNCTION(flakeid_get_ifaddr)
{
	zend_bool raw_output = 0;

	if (zend_parse_parameters(
		ZEND_NUM_ARGS() TSRMLS_CC,
		"|b",
		&raw_output) == FAILURE
	) {
		RETURN_NULL();
	}

	if (raw_output) {
		RETURN_STRINGL(FLAKEID_G(addr), 6);
	} else {
		unsigned char hexstr[12];
		__make_hexstr(hexstr, (char *)&FLAKEID_G(addr), 6);
		RETURN_STRINGL(hexstr, 12);
	}
}

ZEND_FUNCTION(flakeid_next_seq)
{
	long flush_key;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &flush_key) == FAILURE
	) {
		RETURN_NULL();
	}

	if (flush_key != FLAKEID_G(last_flush_key)) {
		FLAKEID_G(last_flush_key) = flush_key;
		FLAKEID_G(seq) = 0;
	}

	RETURN_LONG(FLAKEID_G(seq)++);
}

/* {{{ flakeid_functions[]
 *
 * Every user visible function must have an entry in flakeid_functions[].
 */
const zend_function_entry flakeid_functions[] = {
	PHP_FE(flakeid_generate,	NULL)
	PHP_FE(flakeid_generate64,	NULL)
	PHP_FE(flakeid_get_ipv4,	NULL)
	PHP_FE(flakeid_get_ifaddr,	NULL)
	PHP_FE(flakeid_next_seq,	NULL)
	PHP_FE_END	/* Must be the last line in flakeid_functions[] */
};
/* }}} */

/* {{{ flakeid_module_entry
 */
zend_module_entry flakeid_module_entry = {
	STANDARD_MODULE_HEADER,
	"flakeid",
	flakeid_functions,
	PHP_MINIT(flakeid),
	PHP_MSHUTDOWN(flakeid),
	PHP_RINIT(flakeid),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(flakeid),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(flakeid),
	PHP_FLAKEID_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_FLAKEID
ZEND_GET_MODULE(flakeid)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
