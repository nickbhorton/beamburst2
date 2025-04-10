all: bb2

bb2: src/beamburst2.cpp
	g++ $< -g -O3 -Wall -Werror -Wextra -o bb2 -lpng

clean:
	rm -f *.png
	rm -f bb2
