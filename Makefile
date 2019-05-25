all:
	gcc -shared -fPIC -o myrand.so myrand.c -ldl
	gcc -shared -fPIC -o myfinder.so myfinder.c -ldl
