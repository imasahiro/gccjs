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

#ifndef __JS_H_
#define __JS_H_

extern void jstree_write_globals(void);
extern int js_enable_dump( const char * );
extern void js_set_prefix( const char * );

extern void js_preserve_from_gc( tree );

extern void js_add_search_path( const char * );
extern void js_parse_input_files( const char **, unsigned int );

extern tree js_type_for_size( unsigned int, int );
extern tree js_type_for_mode( enum machine_mode, int );

extern int js_lex_parse( const char * );

extern void __js_debug__( const char *, unsigned int,
			   const char *, ... )
  __attribute__ ((format (printf, 3, 4))) ;

#define debug( ... ) __js_debug__( __FILE__, __LINE__, __VA_ARGS__ );
#define TODO() debug("TODO");error("TODO")


#endif /* __JS_H_ */
