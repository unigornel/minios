package main

import "C"
import "fmt"

func main() {}

//export Main
func Main(unused int) {
	fmt.Println("Hello World!")
}
