CFLAGS = -Wall -g
LDLIBS = -lcrypto -lssl -lpthread

cache: url_cache.o url_include.h url_cache.h url_cache.c main.c
#cache: url_cache.o url_include.h url_cache.c main.c
	cc -g -o url_cache url_cache.o main.c $(LDLIBS)

#target_link_libraries(url_cache crypto lssl)
cache_clean:
	rm  url_cache.o url_cache
