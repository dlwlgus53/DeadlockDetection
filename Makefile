all:
	gcc -shared -fPIC -o Dmonitor.so Dmonitor.c -ldl
	gcc -shared -fPIC -o DDetector.so DDetector.c -ldl
	gcc -o Thread_Creation_example -g Thread_Creation_example.c -lpthread
	gcc -o Gate_Lock_example -g Gate_Lock_example.c -lpthread
	gcc -o Single_threaded_example -g Single_threaded_example.c -lpthread
	gcc -o dinning -g dinning.c -lpthread
	gcc -o abba -g abba.c -lpthread
	gcc DPredict.c -o DPredict -lpthread
clean:
	rm -rf Dmonitor.so DDetector.so Thread_Creation_example Single_threaded_example dinning abba DPredict	
