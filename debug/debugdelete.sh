make createfile
make deletion
./createfile debuginput
echo "INPUT"
./showfile debuginput
./deletion debuginput debugquery.txt
echo "OUTPUT AFTER DELETION"
./showfile debuginput
make clean
