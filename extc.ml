type zstream

type zflush =
    | Z_NO_FLUSH
    | Z_PARTIAL_FLUSH
    | Z_SYNC_FLUSH
    | Z_FULL_FLUSH
    | Z_FINISH


type zresult = {
    z_finish : bool;
    z_read : int;
    z_wrote : int;
}

external zlib_deflate_init2 : int -> int -> zstream = "zlib_deflate_init2"
external zlib_deflate : zstream -> src:string -> spos:int -> slen:int -> dst:string -> dpos:int -> dlen:int -> zflush -> zresult = "zlib_deflate_bytecode" "zlib_deflate"
external zlib_deflate_end : zstream -> unit = "zlib_deflate_end"

let output_zip ?(bufsize=65536) ?(level=9) ch =
    let z = zlib_deflate_init2 level 15 in
    let out = Bytes.create bufsize in
    let tmp_out = Bytes.create bufsize in
    let p = ref 0 in
    let rec flush finish =
        let r = zlib_deflate z out 0 !p tmp_out 0 bufsize (if finish then Z_FINISH else Z_SYNC_FLUSH) in
        ignore(IO.really_output ch tmp_out 0 r.z_wrote);
        let remain = !p - r.z_read in
        String.blit out r.z_read out 0 remain;
        p := remain;
        if finish && not r.z_finish then flush true
    in
    let write c =
        if !p = bufsize then flush false;
        Bytes.unsafe_set out !p c;
        incr p
    in
    let rec output str pos len =
        let str = Bytes.of_string str in
        let b = bufsize - !p in
        if len <= b then begin
            Bytes.blit str pos out !p len;
            p := !p + len;
            len
        end else begin
            Bytes.blit str pos out !p b;
            p := !p + b;
            flush false;
            b + output str (pos + b) (len - b);
        end;
    in
    let close() =
        flush true;
        zlib_deflate_end z
    in
    IO.create_out ~write ~output ~flush:(fun() -> flush false; IO.flush ch) ~close
