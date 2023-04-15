# Переменная CC указывает компилятор, используемый для сборки
CC=g++
#В переменной CFLAGS лежат флаги, которые передаются компилятору
CFLAGS = -mavx2 -c -DDRAW
#Флаг оптимизации
OPTFLAG = -O1

all: alpha_blending execute clean	

alpha_blending: alpha_blending.o
	$(CC) alpha_blending.o -o alpha_blending -lsfml-graphics -lsfml-window -lsfml-system

alpha_blending.o:
	$(CC) $(CFLAGS) $(OPTFLAG) alpha_blending.cpp -o alpha_blending.o

clean:
	rm *.o

execute:
	./alpha_blending