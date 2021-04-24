linearsearchobjects = buffer_manager.o file_manager.o linearsearch.o
binarysearchobjects = buffer_manager.o file_manager.o binarysearch.o
deletionobjects = buffer_manager.o file_manager.o deletion.o
createfileobjects = buffer_manager.o file_manager.o createfile.o

linearsearch : $(linearsearchobjects)
	     g++ -std=c++11 -o linearsearch $(linearsearchobjects)

binarysearch : $(binarysearchobjects)
	     g++ -std=c++11 -o binarysearch $(binarysearchobjects)

deletion : $(deletionobjects)
	     g++ -std=c++11 -o deletion $(deletionobjects)

createfile : $(createfileobjects)
	     g++ -std=c++11 -o createfile $(createfileobjects)

linearsearch.o : linearsearch.cpp
		 	 g++ -std=c++11 -c linearsearch.cpp

binarysearch.o : binarysearch.cpp
	g++ -std=c++11 -c binarysearch.cpp

deletion.o : deletion.cpp
	g++ -std=c++11 -c deletion.cpp

createfile.o : createfile.cpp
	g++ -std=c++11 -c createfile.cpp

buffer_manager.o : buffer_manager.cpp
	g++ -std=c++11 -c buffer_manager.cpp

file_manager.o : file_manager.cpp
	g++ -std=c++11 -c file_manager.cpp

clean :
	rm -f *.o
	rm -f binarysearch
	rm -f linearsearch
	rm -f deletion
	rm -f createfile
