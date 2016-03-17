.PHONY: all clean

ifeq ($(GOOS),)
GOOS=netbsd
endif

ifeq ($(GOARCH),)
GOARCH=amd64
endif

CGO_CFLAGS+=-I$(abspath ./include)

HEADERS=go_pthread.h runtime.h syscalls.h types.h waittypes.h list.h
HEADERS_SOURCE=$(patsubst %,../../../include/%,$(HEADERS))
HEADERS_LOCAL=$(patsubst %,include/%,$(HEADERS))

all: include/mini-os
	GOPATH=${PWD}/../.. CGO_ENABLED=1 CGO_CFLAGS="$(CGO_CFLAGS)" GOOS=$(GOOS) GOARCH=$(GOARCH) ${GOROOT}/bin/go build -buildmode=c-archive -a -x -o sum.a main.go
	objcopy --globalize-symbol=_rt0_amd64_netbsd_lib sum.a

include/%.h: ../../../include/%.h include
	cp $< $@

include:
	mkdir -p include
	
include/mini-os: $(HEADERS_LOCAL)
	rm -f include/mini-os
	ln -s ./ include/mini-os

clean:
	rm -vf *.a *.o *.h
