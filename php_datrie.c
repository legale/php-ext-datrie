/* file: php_datrie.c */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"

#include "php_datrie.h"

#include "trie.h"
#include "trie-private.h"
#include "fileutils.h"
#include "alpha-map.h"
#include "alpha-map-private.h"
#include "darray.h"
#include "tail.h"
#include "trie-string.h"


/* True global resources - no need for thread safety here */
static int le_datrie;
static int le_datrie_state;

PHP_MINIT_FUNCTION (datrie) {
    le_datrie = zend_register_list_destructors_ex(
            php_datrie_dtor,
            NULL, PHP_DATRIE_RES_NAME, module_number);

    le_datrie_state = zend_register_list_destructors_ex(
            php_datrie_state_dtop,
            NULL, PHP_DATRIE_STATE_RES_NAME, module_number);
    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION (datrie) {

    return SUCCESS;
}

PHP_RINIT_FUNCTION (datrie) {
#if defined(COMPILE_DL_DATRIE) && defined(ZTS)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif
    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION (datrie) {
    return SUCCESS;
}

PHP_MINFO_FUNCTION (datrie) {
    php_info_print_table_start();
    php_info_print_table_header(2, "datrie support", "enabled");
    php_info_print_table_end();
}

const zend_function_entry datrie_functions[] = {
        PHP_FE(confirm_datrie_compiled, NULL)
        PHP_FE(trie_new, NULL)
        PHP_FE(trie_state_new, NULL)
        PHP_FE(trie_store, NULL)
        PHP_FE(trie_load, NULL)
        PHP_FE(trie_save, NULL)
        PHP_FE(trie_free, NULL)
        PHP_FE(trie_state_free, NULL)
        PHP_FE(trie_state_walkable_chars, NULL)
        PHP_FE(trie_state_walk, NULL)
        PHP_FE(trie_state_rewind, NULL)
        PHP_FE(trie_state_is_walkable, NULL)
        PHP_FE(trie_state_is_single, NULL)
        PHP_FE(trie_state_is_leaf, NULL)
        PHP_FE(trie_state_is_terminal, NULL)
        PHP_FE(trie_enumerate, NULL)
        PHP_FE(trie_retrieve, NULL)
        PHP_FE(tail_walk_char, NULL)
        PHP_FE(tail_get_data, NULL)
        PHP_FE_END    /* Must be the last line in trie_functions[] */
};


zend_module_entry datrie_module_entry = {
        STANDARD_MODULE_HEADER,
        "datrie",
        datrie_functions,
        PHP_MINIT(datrie),
        PHP_MSHUTDOWN(datrie),
        PHP_RINIT(datrie),        /* Replace with NULL if there's nothing to do at request start */
        PHP_RSHUTDOWN(datrie),    /* Replace with NULL if there's nothing to do at request end */
        PHP_MINFO(datrie),
        PHP_DATRIE_VERSION,
        STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_DATRIE
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(datrie)
#endif

static void php_datrie_dtor(zend_resource *rsrc TSRMLS_DC) {
    Trie *trie = (Trie *) rsrc->ptr;
    trie_free(trie);
}

static void php_datrie_state_dtop(zend_resource *rsrc TSRMLS_DC) {
    TrieState *state = (TrieState *) rsrc->ptr;
    trie_state_free(state);
}


PHP_FUNCTION (confirm_datrie_compiled) {
    char *arg = NULL;
    size_t arg_len, len;
    zend_string *strg;

    if (zend_parse_parameters(ZEND_NUM_ARGS(), "s", &arg, &arg_len) == FAILURE) {
        return;
    }

    strg = strpprintf(0,
                      "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.",
                      "datrie", arg);

    RETURN_STR(strg);
}

PHP_FUNCTION (trie_new) {
    Trie *trie;
    AlphaMap *alpha_map;

    alpha_map = alpha_map_new();
    if (!alpha_map) {
        RETURN_NULL();
    }

    //we will parse string as 1-byte letter sequence
    if (alpha_map_add_range(alpha_map, 0x01, 0xff) != 0) {
        alpha_map_free(alpha_map);
        RETURN_NULL();
    }

    trie = trie_new(alpha_map);
    alpha_map_free(alpha_map);
    if (!trie) {
        RETURN_NULL();
    }

    ZVAL_RES(return_value, zend_register_resource(trie, le_datrie));
}

//max word letters allowed
#define KEYWORD_MAX_LEN 512

//max word length in chars. 3 bytes for each char
#define KEYWORD_CHAR_MAX_LEN KEYWORD_MAX_LEN * 3

PHP_FUNCTION (trie_store) {
    Trie *trie;
    zval *resource;
    zend_long data = -1;
    unsigned char *keyword;
    size_t keyword_len;
    int i;
    AlphaChar alpha_key[KEYWORD_MAX_LEN + 1];

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|l",
                              &resource, &keyword, &keyword_len, &data) == FAILURE) {
        RETURN_FALSE;
    }
    if (keyword_len > KEYWORD_MAX_LEN || keyword_len < 1) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "keyword should has [1, %d] bytes", KEYWORD_MAX_LEN);
        RETURN_FALSE;
    }
    trie = (Trie *)zend_fetch_resource( Z_RES_P(resource), PHP_DATRIE_RES_NAME, le_datrie);
    i = 0;
    while (*keyword && *keyword != '\n' && *keyword != '\r') {
        alpha_key[i++] = (AlphaChar) *keyword;
        ++keyword;
    }
    alpha_key[i] = TRIE_CHAR_TERM;
    if (!trie_store(trie, alpha_key, (TrieData) data)) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_FUNCTION (trie_free) {
    Trie *trie;
    zval *resource;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) == FAILURE) {
        RETURN_FALSE;
    }

    trie = (Trie *) zend_fetch_resource(Z_RES_P(resource), PHP_DATRIE_RES_NAME, le_datrie);


    if (zend_list_close(Z_RES_P(resource)) == SUCCESS) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

PHP_FUNCTION (trie_state_free) {

    zval *resource;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) == FAILURE) {
        RETURN_FALSE;
    }

    zend_fetch_resource(Z_RES_P(resource), PHP_DATRIE_STATE_RES_NAME, le_datrie_state);


    if (zend_list_close(Z_RES_P(resource)) == SUCCESS) {
        RETURN_TRUE;
    }
    RETURN_FALSE;
}

PHP_FUNCTION (trie_save) {
    Trie *trie;
    zval *resource;
    unsigned char *filename;
    size_t filename_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs",
                              &resource, &filename, &filename_len) == FAILURE) {
        RETURN_FALSE;
    }
    if (filename_len < 1 || strlen(filename) != filename_len) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "save path required");
        RETURN_FALSE;
    }
    trie = (Trie *) zend_fetch_resource(Z_RES_P(resource) , PHP_DATRIE_RES_NAME, le_datrie);
    if (trie_save(trie, filename)) {
        RETURN_FALSE;
    }
    RETURN_TRUE;
}

PHP_FUNCTION (trie_load) {
    Trie *trie;
    char *path;
    size_t path_len;

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
                              &path, &path_len) == FAILURE) {
        RETURN_NULL();
    }

    trie = trie_new_from_file(path);
    if (!trie) {
        RETURN_NULL();
    }

    ZVAL_RES(return_value, zend_register_resource(trie, le_datrie));
}

PHP_FUNCTION (trie_state_new) {
    Trie *trie;
    TrieState *state;
    zval *resource;

    //get and check function args
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) == FAILURE) {
        RETURN_FALSE;
    }

    trie = (Trie *) zend_fetch_resource(Z_RES_P(resource), PHP_DATRIE_RES_NAME, le_datrie);

    state = trie_root(trie);

    ZVAL_RES(return_value, zend_register_resource(state, le_datrie_state));
}

PHP_FUNCTION (trie_state_walkable_chars) {
    TrieState *state;
    zval *resource;
    AlphaChar chars[KEYWORD_MAX_LEN + 1];

    //get and check function args
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) == FAILURE) {
        RETURN_FALSE;
    }

    //get the resource from php
    state = (TrieState *) zend_fetch_resource(Z_RES_P(resource), PHP_DATRIE_STATE_RES_NAME, le_datrie_state);
    if (state == NULL) {
        RETURN_FALSE;
    }

    trie_state_walkable_chars(state, (AlphaChar *) &chars, 0xff - 0x1);


    //create php array for the results
    array_init(return_value);
    //fill the array
    for (int i = 0; chars[i] != '\0' && i < 255; ++i) {
        add_index_long(return_value, i, chars[i]);
    }

}

PHP_FUNCTION (trie_state_walk) {
    TrieState *state;
    zval *state_resource;
    AlphaChar *c;
    size_t c_len;

    //get and check function args
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &state_resource, &c, &c_len) == FAILURE) {
        RETURN_FALSE;
    }


    //get the resource from php
    state = (TrieState *) zend_fetch_resource(Z_RES_P(state_resource), PHP_DATRIE_STATE_RES_NAME, le_datrie_state);
    if (state == NULL) {
        RETURN_FALSE;
    }

    if (trie_state_walk(state, *c)) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }

}

PHP_FUNCTION (tail_get_data) {
    Trie *trie;
    zval *resource;
    int64_t tail_idx;
    TrieData data;

    //get and check function args
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rl", &resource, &tail_idx) ==
        FAILURE) {
        RETURN_NULL();
    }

    //get the resource from zend
    trie = (Trie *) zend_fetch_resource(Z_RES_P(resource), PHP_DATRIE_RES_NAME, le_datrie);
    if (trie == NULL) {
        RETURN_NULL();
    }
    data = tail_get_data(trie->tail, (TrieIndex)tail_idx);
    RETURN_LONG(data);
}

PHP_FUNCTION (tail_walk_char) {
    Trie *trie;

    zval *resource;
    int64_t tail_idx; //we use zend_long because zend_parse_parameters expects int64_t for the int args
    int64_t suffix_idx;
    char *c;
    size_t c_len; //zend_parse_parameters expects uint64_t for the string length


    //get and check function args
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rlls", &resource, &tail_idx, &suffix_idx, &c, &c_len) ==
        FAILURE) {
        RETURN_NULL();
    }

    //get the resource from zend
    trie = (Trie *) zend_fetch_resource(Z_RES_P(resource), PHP_DATRIE_RES_NAME, le_datrie);
    if (trie == NULL) {
        RETURN_NULL();
    }

    if (tail_walk_char(trie->tail, (TrieIndex) tail_idx, (short *) &suffix_idx, (TrieChar) *c)) {
        RETURN_LONG(suffix_idx);
    } else {
        RETURN_NULL();
    }

}

PHP_FUNCTION (trie_state_rewind) {
    TrieState *state;
    zval *resource;

    //get and check function args
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) == FAILURE) {
        RETURN_FALSE;
    }

    //get the resource from php
    state = (TrieState *) zend_fetch_resource(Z_RES_P(resource), PHP_DATRIE_STATE_RES_NAME, le_datrie_state);
    if (state == NULL) {
        RETURN_FALSE;
    }

    trie_state_rewind(state);
    RETURN_TRUE;
}

PHP_FUNCTION (trie_state_is_terminal) {
    TrieState *state;
    zval *resource;


    //get and check function args
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) == FAILURE) {
        RETURN_FALSE;
    }


    //get the resource from php
    state = (TrieState *) zend_fetch_resource(Z_RES_P(resource), PHP_DATRIE_STATE_RES_NAME, le_datrie_state);
    if (state == NULL) {
        RETURN_FALSE;
    }

    if (trie_state_is_terminal(state)) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }

}

PHP_FUNCTION (trie_state_is_leaf) {
    TrieState *state;
    zval *resource;


    //get and check function args
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) == FAILURE) {
        RETURN_FALSE;
    }


    //get the resource from php
    state = (TrieState *) zend_fetch_resource(Z_RES_P(resource), PHP_DATRIE_STATE_RES_NAME, le_datrie_state);
    if (state == NULL) {
        RETURN_FALSE;
    }

    if (trie_state_is_leaf(state)) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }

}

PHP_FUNCTION (trie_state_is_single) {
    TrieState *state;
    zval *resource;

    //get and check function args
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) == FAILURE) {
        RETURN_FALSE;
    }

    //get the resource from php
    state = (TrieState *) zend_fetch_resource(Z_RES_P(resource), PHP_DATRIE_STATE_RES_NAME, le_datrie_state);
    if (state == NULL) {
        RETURN_FALSE;
    }

    if (trie_state_is_single(state)) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }
}

PHP_FUNCTION (trie_state_is_walkable) {
    TrieState *state;
    zval *state_resource;
    AlphaChar *c;
    size_t c_len;

    //get and check function args
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &state_resource, &c, &c_len) == FAILURE) {
        RETURN_FALSE;
    }

    if (c_len != 1) {
        php_error_docref(NULL TSRMLS_CC, E_NOTICE, "only one character expected");
        RETURN_FALSE;
    }

    //get the resource from php
    state = (TrieState *) zend_fetch_resource(Z_RES_P(state_resource), PHP_DATRIE_STATE_RES_NAME, le_datrie_state);
    if (state == NULL) {
        RETURN_FALSE;
    }

    if (trie_state_is_walkable(state, *c)) {
        RETURN_TRUE;
    } else {
        RETURN_FALSE;
    }

}

PHP_FUNCTION (trie_enumerate) {
    Trie *trie;
    zval *resource;

    //get and check function args
    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &resource) == FAILURE) {
        RETURN_FALSE;
    }

    //get the resource from php
    trie = (Trie *) zend_fetch_resource(Z_RES_P(resource), PHP_DATRIE_RES_NAME, le_datrie);
    if (trie == NULL) {
        RETURN_FALSE;
    }

    array_init(return_value);
    //list all datrie content
    if (!trie_enumerate(trie, list_enum_func, return_value)) {
        RETURN_FALSE;
    }

}

PHP_FUNCTION (trie_retrieve) {
    Trie *trie;
    zval *resource;
    zend_long data;
    unsigned char *keyword;
    size_t keyword_len;
    int i;
    AlphaChar alpha_key[KEYWORD_MAX_LEN + 1];

    if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs|l",
                              &resource, &keyword, &keyword_len, &data) == FAILURE) {
        RETURN_FALSE;
    }
    if (keyword_len > KEYWORD_MAX_LEN || keyword_len < 1) {
        php_error_docref(NULL TSRMLS_CC, E_WARNING, "keyword should has [1, %d] bytes", KEYWORD_MAX_LEN);
        RETURN_FALSE;
    }
    trie = (Trie *) zend_fetch_resource(Z_RES_P(resource), PHP_DATRIE_RES_NAME, le_datrie);

    i = 0;
    while (*keyword && *keyword != '\n' && *keyword != '\r') {
        alpha_key[i++] = (AlphaChar) *keyword;
        ++keyword;
    }
    alpha_key[i] = TRIE_CHAR_TERM;

    if (!trie_retrieve(trie, alpha_key, (TrieData *) &data)) {
        RETURN_NULL();
    } else {
        RETURN_LONG(data);
    }

}

//callback function that expected by trie_enumerate()
static Bool list_enum_func(const AlphaChar *key, TrieData key_data, void *user_data) {
    zval word;
    zval *return_value = (zval *) user_data;
    //init string array + 1 byte for the tail terminator
    char string[KEYWORD_CHAR_MAX_LEN + 1];

    //print by one because AlphaChar is 4-byte length
    int i = 0;
    while (key[i]) {
        string[i] = (char) key[i];
        ++i;
    }
    //terminate string
    string[i] = '\0';

    //create array
    array_init_size(&word, 2);
    add_next_index_string(&word, (const char *) &string);
    add_next_index_long(&word, key_data);
    //insert created array to the return_value array
    add_next_index_zval(return_value, &word);
    return TRUE;
}
