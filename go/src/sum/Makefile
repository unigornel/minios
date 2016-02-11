.PHONY: all clean

all:
	GOPATH=${PWD}/../.. go build -buildmode=c-archive -o sum.a main.go

clean:
	rm -vf *.a *.o *.h
