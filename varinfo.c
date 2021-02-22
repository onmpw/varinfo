
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_varinfo.h"

/* If you declare any globals in php_varinfo.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(varinfo)
*/

/* True global resources - no need for thread safety here */
static int le_varinfo;

static int var_string_add(var_str *ds,char *str,...)
{
    char *strg;
    int len;
    va_list args;

    va_start(args,str);
    len = vspprintf(&strg,0,str,args);
    va_end(args);

    if(ds->len + len > ds->alloc - 1) {
        ds->str = realloc(ds->str,ds->alloc + len + VAR_STR_LEN);

        ds->alloc = ds->alloc + len + VAR_STR_LEN;
    }

    memcpy(ds->str + ds->len,strg,len);

    ds->str[ds->len + len] = '\0';

    ds->len = ds->len + len;

    efree(strg);
}

static int hash_find(zval **res,HashTable *ht, char *name,uint name_len TSRMLS_DC)
{
#if PHP_VERSION_ID >= 70000
    zend_string *z_name;
    zval *val;
#else
    zval **val;
#endif

#if PHP_VERSION_ID >= 70000
    z_name = zend_string_init(name,name_len,0);

    val = zend_hash_find(ht,z_name);
    if(val == NULL) {
        return 0;
    }

    if(Z_TYPE_P(val) == IS_INDIRECT) {
        val = val->value.zv;
    }

    *res = val;

    zend_string_free(z_name);
#else
    if(SUCCESS != zend_hash_find(ht,name,name_len+1,(void **)&val)) {
        return 0;
    }
    *res = *val;
#endif

    return 1;
}

static int get_zval_from_bucket(void **val,Bucket *bt)
{
#if PHP_VERSION_ID >= 70000
#else
    if(bt != NULL) {
        *val = bt->pData;
        return 1;
    }
#endif
    return 0;
}

static char * get_string(void *p)
{
    char *s;
#if PHP_VERSION_ID >= 70000
    zend_string *zs = (zend_string *)p;
    s = zs->val;
#else
    s = (char *)p;
#endif

    return s;
}

static inline char * get_class_name(zend_class_entry *ce)
{
    return get_string(ce->name);
}


static inline void * convert_str(void **new_str,char *p,int *len)
{
#if PHP_VERSION_ID >= 70000
    zend_string  *strg;
    strg = strpprintf(0, "%s", p);
#else
    char *strg;
    *len = spprintf(&strg,0,"%s",p);
#endif
    *new_str = (void *)strg;
    return NULL;
}


char * get_var_info(zval *var , int *len TSRMLS_DC)
{
    zval *p;
    var_str info = {0,0,NULL};
    int l;
    Bucket *bt;
    HashTable *ht;
#if PHP_VERSION_ID >= 70000
    zend_object *zobj;
    zend_string *key;
    int i;
#else
    zend_object_value zobjv;
    zend_object *zobj;
    char *key;
    zval **val;
#endif

    p = var;

    var_string_add(&info,"(");
#if PHP_VERSION_ID >= 70000

    if(Z_TYPE_P(p) == IS_INDIRECT) {
        p = p->value.zv;
    }

    if(Z_TYPE_P(p) == IS_REFERENCE) {
        p = &p->value.ref->val;
    }
#endif
    switch (Z_TYPE_P(p)) {
        case IS_ARRAY:
#if PHP_VERSION_ID >= 70000
            ht = p->value.arr;
#else
            ht = p->value.ht;
#endif
            // 上面可以直接统一使用 ht = Z_ARRVAL_P(p）， 使用原生获取方式加深对存储结构的印象。

            var_string_add(&info, "addr=> %p, type=> array", p);
            if(ht->nNumOfElements == 0) {
                break;
            }

#if PHP_VERSION_ID >= 70000
        for(i=0;i<ht->nNumOfElements;i++) {
                bt = &ht->arData[i];
                if(bt->key != NULL) {
                    key = bt->key;
                    var_string_add(&info,", %s", ZSTR_VAL(key));
                }else{
                    var_string_add(&info, ", %d", bt->h);
                }

                var_string_add(&info, "=> %s",get_var_info(&bt->val,&l TSRMLS_CC));
            }
#else
            bt = ht->pListHead;
            while(bt) {
                if(bt->arKey != NULL) {
                    key = bt->arKey;
                    var_string_add(&info,", %s",key);
                }else{
                    var_string_add(&info,", %d", bt->h);
                }
                if(get_zval_from_bucket((void **)&val,bt)) {
                    var_string_add(&info, "=> %s",get_var_info(*val,&l TSRMLS_CC));
                }
                bt = bt->pListNext;
            }
#endif
            break;
        case IS_STRING:
            var_string_add(&info,"addr=> %p, type=> string, value=> %s",p,Z_STRVAL_P(p));
            break;
        case IS_LONG:
            var_string_add(&info,"addr=> %p, type=> int, value=> %d",p, Z_LVAL_P(p));
            break;
        case IS_DOUBLE:
            var_string_add(&info,"addr=> %p, type=> double, value=> %.6f",p, Z_DVAL_P(p));
            break;
#if PHP_VERSION_ID >= 70000
        case IS_TRUE:
            var_string_add(&info,"addr=> %p, type=> bool, value=> %s",p, "true");
            break;
        case IS_FALSE:
            var_string_add(&info,"addr=> %p, type=> bool, value=> %s",p, "false");
            break;
#else
        case IS_BOOL:
            var_string_add(&info,"addr=> %p, type=> bool, value=> %s",p, p->value.lval == 1 ? "true":"false");
            break;
#endif
        case IS_OBJECT:
#if PHP_VERSION_ID >= 70000
            zobj = Z_OBJ_P(p);
            zend_class_entry *ce = Z_OBJCE_P(p);
            HashTable *properties = zobj->handlers->get_properties(p);
            zval *prop;
            zend_string *c_key,*prop_name;
            zend_ulong unum_index;
#else
            zobjv = Z_OBJVAL_P(p);
            zobj = zend_objects_get_address(p TSRMLS_CC);
            zend_class_entry *ce = zobjv.handlers->get_class_entry(&zobjv TSRMLS_CC);
            HashTable *properties = zobjv.handlers->get_properties(&zobjv TSRMLS_CC);
            zval **prop;
            char *c_key,*prop_name;
            uint key_len,prop_len;
            ulong unum_index;
#endif
            HashPosition pos;


            var_string_add(&info,"addr=> %p, type=> object, value=> class %s {",p,get_class_name(ce));
            zend_hash_internal_pointer_reset_ex(properties,&pos);
#if PHP_VERSION_ID >= 70000
        while((prop = zend_hash_get_current_data_ex(properties,&pos)) != NULL ) {
                if(zend_hash_get_current_key_ex(properties,&c_key,&unum_index,&pos) == HASH_KEY_IS_STRING) {
                    if(zend_check_property_access(zobj,c_key) == SUCCESS) {
#else

            while(zend_hash_get_current_data_ex(properties,(void **)&prop, &pos) == SUCCESS) {
                if(zend_hash_get_current_key_ex(properties, &c_key, &key_len,&unum_index,0,&pos) == HASH_KEY_IS_STRING) {
                    if (zend_check_property_access(zobj, c_key, key_len-1 TSRMLS_CC) == SUCCESS) {
#endif
                        var_string_add(&info, " public $%s=>%s", get_string((void *)c_key),get_var_info(prop,&l TSRMLS_CC));
                    }
                }
                zend_hash_move_forward_ex(properties, &pos);
            }
            var_string_add(&info,"}");
            break;
        default:
            var_string_add(&info,"NULL");
            break;
    }

#if PHP_VERSION_ID >= 70000
    if(Z_TYPE_P(p) >= IS_STRING) {
        var_string_add(&info, ", refcount=> %d, is_ref=> %d", var->value.counted->gc.refcount,
                       Z_TYPE_P(var) == IS_REFERENCE ? 1 : 0);
    }else{
        var_string_add(&info, ", refcount=>0, is_ref=>0");
    }
#else
    var_string_add(&info,", refcount=> %d, is_ref=> %d",p->refcount__gc,p->is_ref__gc == 1 ? 1 : 0);
#endif

    var_string_add(&info," )");
    *len = info.len;

    return info.str;
}

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("varinfo.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_varinfo_globals, varinfo_globals)
    STD_PHP_INI_ENTRY("varinfo.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_varinfo_globals, varinfo_globals)
PHP_INI_END()
*/
/* }}} */

PHP_FUNCTION(varinfo)
{
        char *var_name,*p;
#if PHP_VERSION_ID >= 70000
        size_t var_name_len,len;
        zend_string *strg;
#else
        int var_name_len, len;
        char *strg;
#endif

        zval *var;
        int res;
        HashTable *vars;


#if PHP_VERSION_ID >= 70000
        if(zend_parse_parameters(ZEND_NUM_ARGS(),"s",&var_name,&var_name_len) == FAILURE) {
            return ;
        }
        vars = &EG(symbol_table);
#else
        if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&var_name,&var_name_len) == FAILURE) {
            return ;
        }
        vars = EG(active_symbol_table);
#endif

        if(hash_find(&var,vars,var_name,var_name_len TSRMLS_CC)) {
            php_printf("%s: ", var_name);

            p = get_var_info(var,&len TSRMLS_CC);

        }else{
            PHPWRITE("no such symbol.\n",16);
        }
        convert_str(&strg,p,&len);
        ZVAL_STRINGL(return_value,get_string(strg),len,1);
        free(p);
        p = NULL;
        efree(strg);
        return;
}



/* {{{ php_varinfo_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_varinfo_init_globals(zend_varinfo_globals *varinfo_globals)
{
	varinfo_globals->global_value = 0;
	varinfo_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(varinfo)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(varinfo)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(varinfo)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(varinfo)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(varinfo)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "varinfo support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

/* {{{ varinfo_functions[]
 *
 * Every user visible function must have an entry in varinfo_functions[].
 */
const zend_function_entry varinfo_functions[] = {
    PHP_FE(varinfo,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in varinfo_functions[] */
};
/* }}} */

/* {{{ varinfo_module_entry
 */
zend_module_entry varinfo_module_entry = {
	STANDARD_MODULE_HEADER,
	"varinfo",
	varinfo_functions,
	PHP_MINIT(varinfo),
	PHP_MSHUTDOWN(varinfo),
	PHP_RINIT(varinfo),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(varinfo),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(varinfo),
	PHP_VARINFO_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_VARINFO
ZEND_GET_MODULE(varinfo)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
