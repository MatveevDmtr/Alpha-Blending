all:
	g++ -mavx2 -c  alpha-blending.cpp -o alpha-blending.o
	g++ alpha-blending.o -o sfml-app -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
	./sfml-app

clear:
	rm -f *.o
