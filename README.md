Mini-OS with Go
===============

[Wiki](https://unigornel.org/doku.php?id=minios:link_with_go)

After cloning build `sum.go` into a c-archive to generate `sum.a` and `sum.h`

```
$ cd go/src/sum
$ make
```

Now compile minios with `go_main.c` and watch the linker errors!

```
$ make GOARCHIVE=go/src/sum/sum.a GOINCLUDE=go/src/sum
```
