.PHONY: all clean

ifeq ($(GOOS),)
GOOS=netbsd
endif

ifeq ($(GOARCH),)
GOARCH=amd64
endif

all:
	GOPATH=${PWD}/../.. CGO_ENABLED=1 GOOS=$(GOOS) GOARCH=$(GOARCH) ${GOROOT}/bin/go build -buildmode=c-archive -a -x -o sum.a main.go
	objcopy --globalize-symbol=_rt0_amd64_netbsd_lib sum.a

clean:
	rm -vf *.a *.o *.h
