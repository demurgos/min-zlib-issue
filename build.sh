#!/usr/bin/env bash
ocamlc extc_stubs.c
ocamlopt -a -o IO.cmxa IO.ml
ocamlopt -a -o extc.cmxa extc.ml
ocamlopt -o main IO.cmxa extc.cmxa -cclib extc_stubs.o -cclib -lz  main.ml
