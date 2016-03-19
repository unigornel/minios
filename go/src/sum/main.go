package main

import "C"
import (
	"fmt"
	"time"
)

func main() {}

//export Main
func Main(unused int) {
	for {
		fmt.Printf("[%v] Hello World!\n", time.Now().UTC())
		time.Sleep(1 * time.Second)
	}
}
