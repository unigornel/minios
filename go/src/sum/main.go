package main

import "C"
import (
	"fmt"
	"time"
)

func main() {}

//export Main
func Main(unused int) {
	fmt.Println("Hello World!")
	time.Sleep(5 * time.Second)
	fmt.Println("Hello World!")
	time.Sleep(5 * time.Second)
	fmt.Println("Hello World!")
	time.Sleep(5 * time.Second)
	fmt.Println("Hello World!")
	time.Sleep(5 * time.Second)
	fmt.Println("Hello World!")
}
