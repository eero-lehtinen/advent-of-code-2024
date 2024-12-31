run FILE:
	clang -g -Wall {{FILE}}.c -o ./out
	./out

run-opt FILE:
	clang -O3 -Wall {{FILE}}.c -o ./out
	./out
