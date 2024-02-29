all:
	gcc -o monolit monolit.c -ltomcrypt -lsodium -lm -s
