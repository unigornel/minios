Mini-OS with Go
===============

After cloning build `sum.go` into a c-archive to generate `sum.a` and `sum.h`

```
$ cd go/src/sum
$ make
```

Now compile minios with `go_main.c` and watch the linker errors!

```
$ make GOARCHIVE=go/src/sum/sum.a GOINCLUDE=go/src/sum
```
