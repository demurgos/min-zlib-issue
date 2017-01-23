/*
 *  Extc : C common OCaml bindings
 *  Copyright (c)2004 Nicolas Cannasse
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA
 */

#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/mlvalues.h>
#include <caml/fail.h>
#include <zlib.h>
#ifdef _WIN32
#    include <windows.h>
#    include <conio.h>
#else
#    include <dlfcn.h>
#    include <limits.h>
#    include <unistd.h>
#    include <string.h>
#    include <termios.h>
#    include <stdio.h>
#    include <sys/time.h>
#    include <sys/times.h>
#    include <sys/stat.h>
#    include <caml/memory.h>
#endif
#ifdef __APPLE__
#    include <sys/param.h>
#    include <sys/syslimits.h>
#    include <mach-o/dyld.h>
#endif
#ifdef __FreeBSD__
#    include <sys/param.h>
#    include <sys/sysctl.h>
#    include <sys/user.h>
#endif

#ifndef CLK_TCK
#    define CLK_TCK    100
#endif


#define zval(z)        ((z_streamp)(z))

value zlib_new_stream() {
    value z = alloc((sizeof(z_stream) + sizeof(value) - 1) / sizeof(value),Abstract_tag);
    z_stream *s = zval(z);
    s->zalloc = NULL;
    s->zfree = NULL;
    s->opaque = NULL;
    s->next_in = NULL;
    s->next_out = NULL;
    return z;
}

CAMLprim value zlib_deflate_init2(value lvl,value wbits) {
    value z = zlib_new_stream();
    if( deflateInit2(zval(z),Int_val(lvl),Z_DEFLATED,Int_val(wbits),8,Z_DEFAULT_STRATEGY) != Z_OK )
        failwith("zlib_deflate_init");
    return z;
}

CAMLprim value zlib_deflate( value zv, value src, value spos, value slen, value dst, value dpos, value dlen, value flush ) {
    z_streamp z = zval(zv);
    value res;
    int r;

    z->next_in = (Bytef*)(String_val(src) + Int_val(spos));
    z->next_out = (Bytef*)(String_val(dst) + Int_val(dpos));
    z->avail_in = Int_val(slen);
    z->avail_out = Int_val(dlen);
    if( (r = deflate(z,Int_val(flush))) < 0 )
        failwith("zlib_deflate");

    z->next_in = NULL;
    z->next_out = NULL;

    res = alloc_small(3, 0);
    Field(res, 0) = Val_bool(r == Z_STREAM_END);
    Field(res, 1) = Val_int(Int_val(slen) - z->avail_in);
    Field(res, 2) = Val_int(Int_val(dlen) - z->avail_out);
    return res;
}

CAMLprim value zlib_deflate_end(value zv) {
    if( deflateEnd(zval(zv)) != 0 )
        failwith("zlib_deflate_end");
    return Val_unit;
}
