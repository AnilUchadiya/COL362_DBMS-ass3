sampleobjects = buffer_manager.o file_manager.o rtree.o

rtree : $(sampleobjects)
	     g++ -std=c++11 -o rtree $(sampleobjects)

rtree.o : rtree.cpp
	g++ -std=c++11 -c rtree.cpp

buffer_manager.o : buffer_manager.cpp
	g++ -std=c++11 -c buffer_manager.cpp

file_manager.o : file_manager.cpp
	g++ -std=c++11 -c file_manager.cpp

clean :
	rm -f rtree.o
	rm -f rtree
	rm -f rtree.txt
