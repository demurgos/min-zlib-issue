#!/usr/bin/env bash
ocamlc extc_stubs.c
ocamlopt -a -safe-string -o IO.cmxa IO.ml
ocamlopt -a -safe-string -o extc.cmxa extc.ml
ocamlopt -safe-string -o main IO.cmxa extc.cmxa -cclib extc_stubs.o -cclib -lz  main.ml
