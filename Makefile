
test: main.c image.c filters.c
	gcc -o test main.c image.c filters.c io_png.c -lpng

