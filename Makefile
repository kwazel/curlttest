GCCFLAGS = -g -O0 -std=gnu2x -Wall
# gcc main.c -o curltest $(GCCFLAGS) -lcurl -lnghttp2 -lcrypto -lssl -ldl -lm -lz
curltest:	main.c credentials.h
	gcc main.c -o curltest $(GCCFLAGS) -lcurl

run:	curltest
	./curltest

clean:
	rm -f curltest