/* file: php_datrie.h */

#ifndef PHP_DATRIE_H
#define PHP_DATRIE_H

#include "php.h"

#include "datrie/trie.h"


extern zend_module_entry datrie_module_entry;
#define phpext_datrie_ptr &datrie_module_entry

#define PHP_DATRIE_VERSION "0.0.1"
#define PHP_DATRIE_RES_NAME "Datrie structure"
#define PHP_DATRIE_STATE_RES_NAME "Datrie state structure"

#ifdef PHP_WIN32
#	define PHP_DATRIE_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_DATRIE_API __attribute__ ((visibility("default")))
#else
#	define PHP_DATRIE_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:

ZEND_BEGIN_MODULE_GLOBALS(datrie)
	zend_long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(datrie)
*/

/* Always refer to the globals in your function as DATRIE_G(variable).
   You are encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/
#define DATRIE_G(v) ZEND_MODULE_GLOBALS_ACCESSOR(datrie, v)

//previously (php5) used MACROS
#define ZEND_FETCH_RESOURCE(rsrc, rsrc_type, passed_id, default_id, resource_type_name, resource_type)        \
        (rsrc = (rsrc_type) zend_fetch_resource(Z_RES_P(*passed_id), resource_type_name, resource_type))
#define ZEND_REGISTER_RESOURCE(return_value, result, le_result)  ZVAL_RES(return_value,zend_register_resource(result, le_result))



PHP_FUNCTION (confirm_datrie_compiled);

PHP_FUNCTION (trie_new);

PHP_FUNCTION(trie_state_new);

PHP_FUNCTION(trie_state_rewind);

PHP_FUNCTION(trie_store);

PHP_FUNCTION(trie_load);

PHP_FUNCTION(trie_save);

PHP_FUNCTION(trie_free);

PHP_FUNCTION(trie_state_free);

PHP_FUNCTION(trie_state_walkable_chars);

PHP_FUNCTION(trie_state_walk);


PHP_FUNCTION(trie_state_rewind);

PHP_FUNCTION(trie_state_is_walkable);

PHP_FUNCTION(trie_state_is_single);

PHP_FUNCTION(trie_state_is_leaf);

PHP_FUNCTION(trie_state_is_terminal);

PHP_FUNCTION(trie_enumerate);

PHP_FUNCTION(trie_retrieve);

PHP_FUNCTION(tail_walk_char);

PHP_FUNCTION(tail_get_data);

static Bool list_enum_func (const AlphaChar *key, TrieData key_data, void *user_data);

static void php_datrie_dtor(zend_resource *rsrc TSRMLS_DC);

static void php_datrie_state_dtop(zend_resource *rsrc TSRMLS_DC);

#if defined(ZTS) && defined(COMPILE_DL_DATRIE)
ZEND_TSRMLS_CACHE_EXTERN()
#endif


#endif    /* PHP_DATRIE_H */

