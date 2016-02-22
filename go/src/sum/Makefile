.PHONY: all clean

ifeq ($(GOOS),)
GOOS=netbsd
endif

ifeq ($(GOARCH),)
GOARCH=amd64
endif

all:
	GOPATH=${PWD}/../.. CGO_ENABLED=1 GOOS=$(GOOS) GOARCH=$(GOARCH) ${GOROOT}/bin/go build -buildmode=c-archive -o sum.a main.go

clean:
	rm -vf *.a *.o *.h
