object:general_plugin.cpp
	g++ -c -fPIC general_plugin.cpp -o general_plugin.o -I ../

shared_lib:general_plugin.o
	g++ -shared general_plugin.o -o general_plugin.so

frontend:frontend.cpp
	g++ -g3 frontend.cpp -o frontend.x -l GL

all:object shared_lib

clean:
	rm *.o