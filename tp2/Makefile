FLAGS = -std=gnu11 -Wall -Werror -pedantic -Wextra -Wconversion -lm -fopenmp -O2 -g
make:
	cppcheck --enable=all ./
	gcc $(FLAGS) src/bmp_editor.c src/simple_bmp.c -o bin/bmp_editor
clean:
	rm -r bin/*
