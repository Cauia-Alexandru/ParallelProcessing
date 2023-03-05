build:
	gcc main.cpp -o tema1 -lstdc++ -lpthread -Werror -Wall -lm
build_debug:
	gcc main.cpp -o tema1 -lstdc++ -lpthread -DDEBUG -g3 -O0 -Werror -Wall
