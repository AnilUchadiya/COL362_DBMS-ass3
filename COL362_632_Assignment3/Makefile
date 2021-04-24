searchobjects = buffer_manager.o file_manager.o search.o
deleteobjects = buffer_manager.o file_manager.o delete.o
createfileobjects = buffer_manager.o file_manager.o createfile.o
binarysearchobjects = buffer_manager.o file_manager.o binarysearch.o

linearsearch : $(searchobjects)
	     g++ -std=c++11 -o search $(searchobjects)

search.o : search.cpp
	g++ -std=c++11 -c search.cpp

deletion : $(deleteobjects)
		g++ -std=c++11 -o delete $(deleteobjects)

delete_run.o : delete.cpp
	g++ -std=c++11 -c delete.cpp

binarysearch : $(binarysearchobjects)
	     g++ -std=c++11 -o binarysearch $(binarysearchobjects)

binarysearch.o : binarysearch.cpp
	g++ -std=c++11 -c binarysearch.cpp

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
