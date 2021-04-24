make createfile
make linearsearch
make binarysearch
./createfile debuginput
./linearsearch debuginput debugquery.txt outputlinear
./binarysearch debuginput debugquery.txt outputbinary
echo "LINEAR SEARCH OUTPUT"
./showfile outputlinear
echo "BINARY SEARCH OUTPUT"
./showfile outputbinary
echo "DIFFERENCE"
diff outputlinear outputbinary
make clean
rm -f outputlinear
rm -f outputbinary
