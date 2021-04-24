linearsearchobjects = buffer_manager.o file_manager.o linearsearch.o
binarysearchobjects = buffer_manager.o file_manager.o binarysearch.o
deletionobjects = buffer_manager.o file_manager.o deletion.o
join1objects = buffer_manager.o file_manager.o join1.o
join2objects = buffer_manager.o file_manager.o join2.o
createfileobjects = buffer_manager.o file_manager.o createfile.o
createfile1objects = buffer_manager.o file_manager.o createfile1.o
createfile2objects = buffer_manager.o file_manager.o createfile2.o

linearsearch : $(linearsearchobjects)
	     g++ -std=c++11 -o linearsearch $(linearsearchobjects)

binarysearch : $(binarysearchobjects)
	     g++ -std=c++11 -o binarysearch $(binarysearchobjects)

deletion : $(deletionobjects)
	     g++ -std=c++11 -o deletion $(deletionobjects)

join1 : $(join1objects)
	     g++ -std=c++11 -o join1 $(join1objects)

join2 : $(join2objects)
	     g++ -std=c++11 -o join2 $(join2objects)

createfile : $(createfileobjects)
	     g++ -std=c++11 -o createfile $(createfileobjects)

createfile1 : $(createfile1objects)
	     g++ -std=c++11 -o createfile1 $(createfile1objects)

createfile2 : $(createfile2objects)
	     g++ -std=c++11 -o createfile2 $(createfile2objects)

linearsearch.o : linearsearch.cpp
		 	 g++ -std=c++11 -c linearsearch.cpp

binarysearch.o : binarysearch.cpp
	g++ -std=c++11 -c binarysearch.cpp

deletion.o : deletion.cpp
	g++ -std=c++11 -c deletion.cpp

join1.o : join1.cpp
	g++ -std=c++11 -c join1.cpp

join2.o : join2.cpp
	g++ -std=c++11 -c join2.cpp

createfile.o : createfile.cpp
	g++ -std=c++11 -c createfile.cpp

createfile1.o : createfile1.cpp
	g++ -std=c++11 -c createfile1.cpp

createfile2.o : createfile2.cpp
	g++ -std=c++11 -c createfile2.cpp

buffer_manager.o : buffer_manager.cpp
	g++ -std=c++11 -c buffer_manager.cpp

file_manager.o : file_manager.cpp
	g++ -std=c++11 -c file_manager.cpp

clean :
	rm -f *.o
	rm -f binarysearch
	rm -f linearsearch
	rm -f deletion
	rm -f join1
	rm -f join2
	rm -f createfile
	rm -f createfile1
	rm -f createfile2
