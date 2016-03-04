package main

import "C"

func main() {}

//export Sum
func Sum(a, b int) int {
	return a + b
}
