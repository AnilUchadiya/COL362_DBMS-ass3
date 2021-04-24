searchobjects = buffer_manager.o file_manager.o linearsearch.o
deleteobjects = buffer_manager.o file_manager.o deletion.o
createfileobjects = buffer_manager.o file_manager.o createfile.o
binarysearchobjects = buffer_manager.o file_manager.o binarysearch.o
join1objects = buffer_manager.o file_manager.o join1.o
join2objects = buffer_manager.o file_manager.o join2.o


linearsearch : $(searchobjects)
	     g++ -std=c++11 -o linearsearch $(searchobjects)

linearsearch.o : linearsearch.cpp
	g++ -std=c++11 -c linearsearch.cpp

deletion : $(deleteobjects)
		g++ -std=c++11 -o deletion $(deleteobjects)

deletion.o : deletion.cpp
	g++ -std=c++11 -c deletion.cpp

binarysearch : $(binarysearchobjects)
	     g++ -std=c++11 -o binarysearch $(binarysearchobjects)

binarysearch.o : binarysearch.cpp
	g++ -std=c++11 -c binarysearch.cpp

join1 : $(join1objects)
		g++ -std=c++11 -o join1 $(join1objects)

join1.o : join1.cpp
		g++ -std=c++11 -c join1.cpp

join2 : $(join2objects)
		g++ -std=c++11 -o join2 $(join2objects)

join2.o : join2.cpp
		g++ -std=c++11 -c join2.cpp

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
