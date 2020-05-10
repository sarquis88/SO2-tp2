FLAGS = -std=gnu11 -Wall -Werror -pedantic -Wextra -Wconversion -fopenmp -O3
make:
	cppcheck --enable=all ./
	gcc $(FLAGS) src/bmp_editor.c src/simple_bmp.c -o bin/bmp_editor
docs:
	doxygen Doxyfile
clean:
	rm -r bin/*
