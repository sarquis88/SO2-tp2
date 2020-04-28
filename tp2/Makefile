FLAGS= -std=gnu11 -Wall -Werror -pedantic -Wextra -Wconversion -g -lm

make:
	cppcheck ./
	gcc $(FLAGS) src/bmp_editor.c src/simple_bmp.c -o bin/bmp_editor
clean:
	rm -r bin/*
