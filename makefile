all:
	gcc -Wall -Wno-pointer-sign -o monolit monolit.c -ltomcrypt -lsodium -lm -s
	#cp monolit /usr/bin/monolit
