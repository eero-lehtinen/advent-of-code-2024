set windows-shell := ['nu', '-c']

run FILE:
	clang  -g -Wall {{FILE}}.c -o ./out
	./out

