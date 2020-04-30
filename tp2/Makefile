FLAGS0 = -std=gnu11 -Wall -Werror -pedantic -Wextra -Wconversion -g -lm -fopenmp -O3
FLAGS1 = -std=gnu11 -Wall -Werror -pedantic -Wextra -Wconversion -g -lm -fopenmp -O3 -pg
make:
	cppcheck ./
	gcc $(FLAGS0) src/bmp_editor.c src/simple_bmp.c -o bin/bmp_editor
profiling:
	cppcheck ./
	gcc $(FLAGS1) src/bmp_editor.c src/simple_bmp.c -o bin/bmp_editor
clean:
	rm -r bin/*
