make createfile1
make createfile2
make join1
make join2
./createfile1 debuginput1
./createfile2 debuginput2
echo "INPUT1"
./showfile debuginput1
echo "INPUT2"
./showfile debuginput2
./join1 debuginput1 debuginput2 output1
./join2 debuginput1 debuginput2 output2
echo "OUTPUT AFTER JOIN1"
./showfile output1
echo "OUTPUT AFTER JOIN2"
./showfile output2
echo "DIFFERENCE"
diff output1 output2
make clean
rm -f output1
rm -f output2
rm -f debuginput1
rm -f debuginput2
