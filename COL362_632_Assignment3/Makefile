searchobjects = buffer_manager.o file_manager.o search.o
deleteobjects = buffer_manager.o file_manager.o delete.o
createfileobjects = buffer_manager.o file_manager.o createfile.o

search : $(searchobjects)
	     g++ -std=c++11 -o search $(searchobjects)

delete : $(deleteobjects)
		g++ -std=c++11 -o delete $(deleteobjects)

search.o : search.cpp
	g++ -std=c++11 -c search.cpp

delete_run.o : delete.cpp
	g++ -std=c++11 -c delete.cpp

createfile : $(createfileobjects)
	     g++ -std=c++11 -o createfile $(createfileobjects)

createfile.o : createfile.cpp
	g++ -std=c++11 -c createfile.cpp

buffer_manager.o : buffer_manager.cpp
	g++ -std=c++11 -c buffer_manager.cpp

file_manager.o : file_manager.cpp
	g++ -std=c++11 -c file_manager.cpp

clean :
	rm -f *.o
	rm -f sample_run
