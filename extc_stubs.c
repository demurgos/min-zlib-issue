#include <caml/alloc.h>
#include <caml/callback.h>
#include <caml/custom.h>
#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/fail.h>
#include <zlib.h>
#include <stdio.h>

#define ZStreamPointer_val(v) (*((z_streamp *) Data_custom_val(v)))

/**
 * Free the pointer contained in the supplied caml value.
 */
void haxe_free_z_stream(value camlZStream) {
  free(ZStreamPointer_val(camlZStream));
  ZStreamPointer_val(camlZStream) = NULL;
}

static struct custom_operations haxe_z_stream_ops = {
  "z_stream_ops", &haxe_free_z_stream, NULL, NULL, NULL, NULL
};

value zlib_new_stream() {
    value camlZStream = caml_alloc_custom(&haxe_z_stream_ops, sizeof(z_streamp), 0, 1);
    ZStreamPointer_val(camlZStream) = malloc(sizeof(z_stream));
    ZStreamPointer_val(camlZStream)->zalloc = NULL;
    ZStreamPointer_val(camlZStream)->zfree = NULL;
    ZStreamPointer_val(camlZStream)->opaque = NULL;
    ZStreamPointer_val(camlZStream)->next_in = NULL;
    ZStreamPointer_val(camlZStream)->next_out = NULL;
    return camlZStream;
}

CAMLprim value zlib_deflate_init2(value lvl, value wbits) {
    printf("zlib_deflate_init2\n");
    int cLvl = Int_val(lvl);
    int cWbits = Int_val(wbits);
    printf("zlib_deflate_init2: lvl: %i\n", cLvl);
    printf("zlib_deflate_init2: wbits: %i\n", cWbits);
    value z = zlib_new_stream();
    z_streamp zPtr = ZStreamPointer_val(z);
    printf("zlib_deflate_init2: (z_streamp)(z): %p\n", zPtr);
    int deflateInit2Result = deflateInit2(zPtr, cLvl, Z_DEFLATED, cWbits, 8, Z_DEFAULT_STRATEGY);
    if(deflateInit2Result != Z_OK) {
        switch (deflateInit2Result) {
            case Z_MEM_ERROR:
                failwith("Error in `zlib_deflate_init2` (extc_stubs.c), call to `deflateInit2` failed: Z_MEM_ERROR");
                break;
            case Z_STREAM_ERROR:
                failwith("Error in `zlib_deflate_init2` (extc_stubs.c), call to `deflateInit2` failed: Z_STREAM_ERROR");
                break;
            case Z_VERSION_ERROR:
                failwith("Error in `zlib_deflate_init2` (extc_stubs.c), call to `deflateInit2` failed: Z_VERSION_ERROR");
                break;
            default:
                failwith("Error in `zlib_deflate_init2` (extc_stubs.c), call to `deflateInit2` failed with unknown error code");
        }
    }
    return z;
}

CAMLprim value zlib_deflate(value zv, value src, value spos, value slen, value dst, value dpos, value dlen, value flush) {
    printf("zlib_deflate\n");
    z_streamp z = ZStreamPointer_val(zv); // z_stream pointer
    char * cSrc = String_val(src);
    int cSpos = Int_val(spos);
    int cSlen = Int_val(slen);
    char * cDst = String_val(dst);
    int cDpos = Int_val(dpos);
    int cDlen = Int_val(dlen);
    int cFlush = Int_val(flush);
    printf("zlib_deflate: (z_streamp)(zv): %p\n", z);
    printf("zlib_deflate: spos: %i\n", cSpos);
    printf("zlib_deflate: slen: %i\n", cSlen);
    printf("zlib_deflate: dpos: %i\n", cDpos);
    printf("zlib_deflate: dlen: %i\n", cDlen);
    printf("zlib_deflate: flush: %i\n", cFlush);

    value res;

    z->next_in = (Bytef*)(cSrc + cSpos);
    z->next_out = (Bytef*)(cDst + cDpos);
    z->avail_in = cSlen;
    z->avail_out = cDlen;

    int deflateResult = deflate(z, cFlush);
    // Possible return codes:
    // https://github.com/madler/zlib/blob/cacf7f1d4e3d44d871b605da3b647f07d718623f/zlib.h#L177
    if(deflateResult != Z_OK) {
        switch (deflateResult) {
            case Z_STREAM_END:
                // This return code is handled later
                break;
            case Z_NEED_DICT:
                // TODO: handle this ?
                break;
            case Z_BUF_ERROR:
                // TODO: use ERR_MSG from zlib
                failwith("Error in `zlib_deflate` (extc_stubs.c), call to `deflate` failed: Z_BUF_ERROR");
                break;
            case Z_DATA_ERROR:
                failwith("Error in `zlib_deflate` (extc_stubs.c), call to `deflate` failed: Z_DATA_ERROR");
                break;
            case Z_ERRNO:
                failwith("Error in `zlib_deflate` (extc_stubs.c), call to `deflate` failed: Z_ERRNO");
                break;
            case Z_MEM_ERROR:
                failwith("Error in `zlib_deflate` (extc_stubs.c), call to `deflate` failed: Z_MEM_ERROR");
                break;
            case Z_STREAM_ERROR:
                failwith("Error in `zlib_deflate` (extc_stubs.c), call to `deflate` failed: Z_STREAM_ERROR");
                break;
            case Z_VERSION_ERROR:
                failwith("Error in `zlib_deflate` (extc_stubs.c), call to `deflate` failed: Z_VERSION_ERROR");
                break;
            default:
                failwith("Error in `zlib_deflate` (extc_stubs.c), call to `deflate` failed with unknown error code");
        }
    }

    z->next_in = NULL;
    z->next_out = NULL;

    res = alloc_small(3, 0);
    Field(res, 0) = Val_bool(deflateResult == Z_STREAM_END);
    Field(res, 1) = Val_int(Int_val(slen) - z->avail_in);
    Field(res, 2) = Val_int(Int_val(dlen) - z->avail_out);
    return res;
}

CAMLprim value zlib_deflate_end(value zv) {
    printf("zlib_deflate_end\n");
    z_streamp zPtr = ZStreamPointer_val(zv);
    printf("zlib_deflate: (z_streamp)(zv): %p\n", zPtr);
    if(deflateEnd(zPtr) != Z_OK) {
        failwith("zlib_deflate_end");
    }
    return Val_unit;
}
