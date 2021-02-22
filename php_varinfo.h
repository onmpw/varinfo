
#ifndef PHP_VARINFO_H
#define PHP_VARINFO_H

extern zend_module_entry varinfo_module_entry;
#define phpext_varinfo_ptr &varinfo_module_entry

#define PHP_VARINFO_VERSION "0.1.0" /* Replace with version number for your extension */

#ifdef PHP_WIN32
#	define PHP_VARINFO_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_VARINFO_API __attribute__ ((visibility("default")))
#else
#	define PHP_VARINFO_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(varinfo)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(varinfo)
*/

/* In every utility function you add that needs to use variables 
   in php_varinfo_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as VARINFO_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define VARINFO_G(v) TSRMG(varinfo_globals_id, zend_varinfo_globals *, v)
#else
#define VARINFO_G(v) (varinfo_globals.v)
#endif

#define VAR_STR_LEN     1024

#if PHP_VERSION_ID >= 70000

#define RVAL_STRL(s,l,d)     RETVAL_STR(s)
#define RET_STRL(s,l,d)      { RVAL_STRL(s,l,d); return;}
#define V_ZVAL_STRINGL(z,s,l,d)   ZVAL_STRINGL(z,s,l)

#else

#define RVAL_STRL(s,l,d)    RETVAL_STRINGL(s,l,d)
#define RET_STRL(s,l,d)     { RVAL_STRL(r,s,d); return; }
#define V_ZVAL_STRINGL(z,s,l,d)   ZVAL_STRINGL(z,s,l,d)

#endif

typedef struct _varinfo_string {
    int len;
    int alloc;

    char *str;
} var_str;


#endif	/* PHP_VARINFO_H */

