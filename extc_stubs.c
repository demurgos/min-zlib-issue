#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/fail.h>
#include <zlib.h>
#include <stdio.h>

value zlib_new_stream() {
    value z = alloc((sizeof(z_stream) + sizeof(value) - 1) / sizeof(value),Abstract_tag);
    z_stream *s = (z_streamp)(z);
    s->zalloc = NULL;
    s->zfree = NULL;
    s->opaque = NULL;
    s->next_in = NULL;
    s->next_out = NULL;
    return z;
}

CAMLprim value zlib_deflate_init2(value lvl, value wbits) {
    value z = zlib_new_stream();
    if(deflateInit2((z_streamp)(z), Int_val(lvl), Z_DEFLATED, Int_val(wbits), 8, Z_DEFAULT_STRATEGY) != Z_OK ) {
        failwith("zlib_deflate_init");
    }
    return z;
}

CAMLprim value zlib_deflate(value zv, value src, value spos, value slen, value dst, value dpos, value dlen, value flush) {
    z_streamp z = (z_streamp)(zv);
    value res;
    int r;

    z->next_in = (Bytef*)(String_val(src) + Int_val(spos));
    z->next_out = (Bytef*)(String_val(dst) + Int_val(dpos));
    z->avail_in = Int_val(slen);
    z->avail_out = Int_val(dlen);
    if((r = deflate(z, Int_val(flush))) < 0) {
        failwith("zlib_deflate");
    }

    z->next_in = NULL;
    z->next_out = NULL;

    res = alloc_small(3, 0);
    Field(res, 0) = Val_bool(r == Z_STREAM_END);
    Field(res, 1) = Val_int(Int_val(slen) - z->avail_in);
    Field(res, 2) = Val_int(Int_val(dlen) - z->avail_out);
    return res;
}

CAMLprim value zlib_deflate_end(value zv) {
    if(deflateEnd((z_streamp)(zv)) != 0) {
        failwith("zlib_deflate_end");
    }
    return Val_unit;
}
