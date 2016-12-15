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

/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_flakeid_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_flakeid_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "flakeid", arg);
	RETURN_STRINGL(strg, len, 0);
}
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
PHP_MINIT_FUNCTION(flakeid)
{
	REGISTER_INI_ENTRIES();
	unsigned char mac[6];

	if (FLAKEID_G(if_name) && !get_mac(FLAKEID_G(if_name), mac)) {
		zend_printf("shit");
		FLAKEID_G(flakeid_ctx) = flakeid_ctx_create(mac, 6);
	} else if (FLAKEID_G(enable_spoof)) {
		FLAKEID_G(flakeid_ctx) = flakeid_ctx_create_with_spoof();
	} else {
		FLAKEID_G(flakeid_ctx) = NULL;
		return FAILURE;
	}

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(flakeid)
{
	UNREGISTER_INI_ENTRIES();

	if (FLAKEID_G(flakeid_ctx)) {
		flake_ctx_destroy(FLAKEID_G(flakeid_ctx));
		FLAKEID_G(flakeid_ctx) = NULL;
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
	char *delimiter_str;
	int delimiter_strlen;

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
		RETURN_STRINGL(id, 16, 1);
	} else {
		char delimiter = '-';
		int len = 35;

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
		RETURN_STRINGL(hexstr, len , 1);
	}
}

/* {{{ flakeid_functions[]
 *
 * Every user visible function must have an entry in flakeid_functions[].
 */
const zend_function_entry flakeid_functions[] = {
	PHP_FE(confirm_flakeid_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE(flakeid_generate,	NULL)		/* For testing, remove later. */
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
