type 'a output = {
    mutable out_write : char -> unit;
    mutable out_output : string -> int -> int -> int;
    mutable out_close : unit -> 'a;
    mutable out_flush : unit -> unit;
}

exception Output_closed

let create_out ~write ~output ~flush ~close =
    {
        out_write = write;
        out_output = output;
        out_close = close;
        out_flush = flush;
    }

let really_output o s p l' =
    let sl = String.length s in
    if p + l' > sl || p < 0 || l' < 0 then invalid_arg "IO.really_output";
       let l = ref l' in
    let p = ref p in
    while !l > 0 do
        let w = o.out_output s !p !l in
        if w = 0 then raise Sys_blocked_io;
        p := !p + w;
        l := !l - w;
    done;
    l'

let nwrite o s =
    let p = ref 0 in
    let l = ref (String.length s) in
    while !l > 0 do
        let w = o.out_output s !p !l in
        if w = 0 then raise Sys_blocked_io;
        p := !p + w;
        l := !l - w;
    done

let flush o = o.out_flush()

let close_out o =
    let f _ = raise Output_closed in
    let r = o.out_close() in
    o.out_write <- f;
    o.out_output <- f;
    o.out_close <- f;
    o.out_flush <- f;
    r


let output_channel ch =
    {
        out_write = (fun c -> output_char ch c);
        out_output = (fun s p l -> Pervasives.output ch (Bytes.of_string s) p l; l);
        out_close = (fun () -> Pervasives.close_out ch);
        out_flush = (fun () -> Pervasives.flush ch);
    }
