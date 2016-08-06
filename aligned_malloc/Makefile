test:
	gcc -Wall -Werror -pedantic align_alloc_test.c -lalignalloc -L./ -o align_alloc_test.out
lib:
	gcc -g align_alloc.c -c -Wall -Werror -pedantic -fPIC -o alignalloc.o
	gcc -shared -o libalignalloc.so alignalloc.o
clean:
	rm -rf align_alloc_test.out libalignalloc.so alignalloc.o
