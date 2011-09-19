/* This file is part of GCC.

GCC is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3, or (at your option)
any later version.

GCC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with GCC; see the file COPYING3.  If not see
<http://www.gnu.org/licenses/>. */

#include "config.h"
#include "system.h"
#include "ansidecl.h"
#include "coretypes.h"
#include "tm.h"
#include "opts.h"
#include "tree.h"
#include "tree-iterator.h"
#include "tree-pass.h"
#include "gimple.h"
#include "toplev.h"
#include "debug.h"
#include "options.h"
#include "flags.h"
#include "convert.h"
#include "diagnostic-core.h"
#include "langhooks.h"
#include "langhooks-def.h"
#include "target.h"
#include "cgraph.h"

#include "gmp.h"
#include "mpfr.h"

#include "gjs.h"
#include "js-tree.h"
#include "js-op.h"

/* Language-dependent contents of a type.  */
struct GTY(()) lang_type {
  char dummy;
} ;

/* Language-dependent contents of a decl.  */
struct GTY(()) lang_decl {
  char dummy;
} ;

/* Language-dependent contents of an identifier.  This must include a
   tree_identifier.
*/
struct GTY(()) lang_identifier {
  struct tree_identifier common;
} ;

/* The resulting tree type.  */
union GTY((desc ("TREE_CODE (&%h.generic) == IDENTIFIER_NODE"),
	   chain_next ("(union lang_tree_node *) TREE_CHAIN (&%h.generic)")))
lang_tree_node
{
  union tree_node GTY((tag ("0"),
		       desc ("tree_node_structure (&%h)"))) generic;
  struct lang_identifier GTY((tag ("1"))) identifier;
};

/* We don't use language_function.  */
struct GTY(()) language_function {
  int dummy;
};

/* Language hooks.  */
static
bool js_langhook_init( void )
{
  build_common_tree_nodes( false );
  build_common_tree_nodes_2( 0 );

  void_list_node = build_tree_list( NULL_TREE, void_type_node );

  return true;
}

/* Initialize before parsing options.  */
static void
js_langhook_init_options( unsigned int decoded_options_count,
			   struct cl_decoded_option *decoded_options )
{
  return;
}

/* Handle js specific options.  Return 0 if we didn't do anything.  */
static bool
js_langhook_handle_option( size_t scode, const char *arg, int value, int kind,
			    const struct cl_option_handlers *handlers )
{
  enum opt_code code = (enum opt_code) scode;
  int retval = 1;

  debug("inside handle option!");
  switch( code )
    {
    default:
      /* Just return 1 to indicate that the option is valid.  */
      break;
    }

  return retval;
}

/* Run after parsing options.  */
static bool
js_langhook_post_options( const char **pfilename ATTRIBUTE_UNUSED )
{
  debug("post options!");
  gcc_assert( num_in_fnames > 0 );

  if( flag_excess_precision_cmdline == EXCESS_PRECISION_DEFAULT )
    {
      flag_excess_precision_cmdline = EXCESS_PRECISION_STANDARD;
    }

  /* Returning false means that the backend should be used.  */
  return false;
}

static void
js_langhook_parse_file( int set_yy_debug ATTRIBUTE_UNUSED )
{
  unsigned int idx = 0;
  debug("parse file!");

  for( ; idx<num_in_fnames; ++idx )
    {
      const char * t = in_fnames[idx];
      debug("<%i> input = <%s>!", idx, t);
      js_lex_parse(t);
    }
}

static tree
js_langhook_type_for_size( unsigned int bits ATTRIBUTE_UNUSED,
			    int unsignedp ATTRIBUTE_UNUSED )
{
  return NULL;
}

static tree
js_langhook_type_for_mode( enum machine_mode mode ATTRIBUTE_UNUSED,
			    int unsignedp ATTRIBUTE_UNUSED )
{
  return NULL_TREE;
}

/* Record a builtin function.  We just ignore builtin functions.  */
static tree
js_langhook_builtin_function( tree decl ATTRIBUTE_UNUSED )
{
  debug("builtin function!");
  return decl;
}

static int
js_langhook_global_bindings_p( void )
{
  return 1;
}

static tree
js_langhook_pushdecl( tree decl ATTRIBUTE_UNUSED )
{
  debug("pushdecl!\n");
  return NULL;
}

static tree
js_langhook_getdecls( void )
{
  debug("get decls!\n");
  return NULL;
}

extern void jstree_write_globals(void);
/* Write out globals.  */
static void js_langhook_write_globals( void )
{
  debug("write globals!\n");
  jstree_write_globals();
}

static int
js_langhook_gimplify_expr( tree *expr_p ATTRIBUTE_UNUSED,
			    gimple_seq *pre_p ATTRIBUTE_UNUSED,
			    gimple_seq *post_p ATTRIBUTE_UNUSED )
{
  /* debug_tree( (*expr_p) ); */

  enum tree_code code = TREE_CODE (*expr_p);

  /* This is handled mostly by gimplify.c, but we have to deal with
     not warning about int x = x; as it is a GCC extension to turn off
     this warning but only if warn_init_self is zero.  */
  if (code == DECL_EXPR
      && TREE_CODE (DECL_EXPR_DECL (*expr_p)) == VAR_DECL
      && !DECL_EXTERNAL (DECL_EXPR_DECL (*expr_p))
      && !TREE_STATIC (DECL_EXPR_DECL (*expr_p))
      && (DECL_INITIAL (DECL_EXPR_DECL (*expr_p)) == DECL_EXPR_DECL (*expr_p))
      && !warn_init_self)
    TREE_NO_WARNING (DECL_EXPR_DECL (*expr_p)) = 1;

  return GS_UNHANDLED;
}

/* Functions called directly by the generic backend.  */
tree convert( tree type ATTRIBUTE_UNUSED,
	      tree expr ATTRIBUTE_UNUSED )
{
  gcc_unreachable( );
}

static GTY(()) tree js_gc_root;

void
js_preserve_from_gc( tree t ATTRIBUTE_UNUSED )
{
  js_gc_root = tree_cons( NULL_TREE, t, js_gc_root );
  debug("preserve from gc!\n");
}

void __js_debug__(const char * file, unsigned int lineno,
                  const char * fmt, ... )
{
  va_list args;
  fprintf( stderr, "debug:<%s:%i> -> ", file, lineno );
  va_start( args, fmt );
  vfprintf( stderr, fmt, args );
  va_end( args );
  fprintf(stderr, "\n");

}

/* The attribute table might be used for the GCC attribute syntax, e.g.
 * __attribute__((unused)), but this feature isn't yet used in gcalc
 */ 
const struct attribute_spec js_attribute_table[] = {
  { NULL, 0, 0, false, false, false, NULL }
};


/* The language hooks data structure. This is the main interface between the GCC front-end
 * and the GCC middle-end/back-end. A list of language hooks could be found in
 * <gcc>/langhooks.h
 */
#undef LANG_HOOKS_NAME
#undef LANG_HOOKS_INIT 
#undef LANG_HOOKS_INIT_OPTIONS
#undef LANG_HOOKS_HANDLE_OPTION
#undef LANG_HOOKS_POST_OPTIONS
#undef LANG_HOOKS_PARSE_FILE
#undef LANG_HOOKS_TYPE_FOR_MODE
#undef LANG_HOOKS_TYPE_FOR_SIZE
#undef LANG_HOOKS_BUILTIN_FUNCTION
#undef LANG_HOOKS_GLOBAL_BINDINGS_P
#undef LANG_HOOKS_PUSHDECL
#undef LANG_HOOKS_GETDECLS
#undef LANG_HOOKS_WRITE_GLOBALS
#undef LANG_HOOKS_GIMPLIFY_EXPR

#define LANG_HOOKS_NAME			"GNU JavaScript"
#define LANG_HOOKS_INIT			js_langhook_init
#define LANG_HOOKS_INIT_OPTIONS		js_langhook_init_options
#define LANG_HOOKS_HANDLE_OPTION	js_langhook_handle_option
#define LANG_HOOKS_POST_OPTIONS		js_langhook_post_options
#define LANG_HOOKS_PARSE_FILE		js_langhook_parse_file
#define LANG_HOOKS_TYPE_FOR_MODE	js_langhook_type_for_mode
#define LANG_HOOKS_TYPE_FOR_SIZE	js_langhook_type_for_size
#define LANG_HOOKS_BUILTIN_FUNCTION	js_langhook_builtin_function
#define LANG_HOOKS_GLOBAL_BINDINGS_P	js_langhook_global_bindings_p
#define LANG_HOOKS_PUSHDECL		js_langhook_pushdecl
#define LANG_HOOKS_GETDECLS		js_langhook_getdecls
#define LANG_HOOKS_WRITE_GLOBALS	js_langhook_write_globals
#define LANG_HOOKS_GIMPLIFY_EXPR	js_langhook_gimplify_expr

struct lang_hooks lang_hooks = LANG_HOOKS_INITIALIZER;

#include "gt-js-js-lang.h"
#include "gtype-js.h"
