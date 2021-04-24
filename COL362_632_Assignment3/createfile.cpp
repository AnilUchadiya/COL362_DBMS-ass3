#include <iostream>
#include "file_manager.h"
#include "errors.h"
#include <cstring>
#include <bits/stdc++.h>
#include <vector>

using namespace std;

void print_page(FileHandler *fh, int pid)
{
    int int_per_page = PAGE_SIZE / sizeof(int) - 1;
    PageHandler ph;
    try
    {
        cout << "Printing  page no : " << pid << endl;
        ph = fh->PageAt(pid);
    }
    catch (const std::exception &e)
    {
        cout << "Error opening page no : " << pid << endl;
        PageHandler phl = fh->LastPage();
        cout << "last page of this *ptr_fh " << phl.GetPageNum() << endl;
        std::cerr << e.what() << '\n';
    }
    char *data;
    data = ph.GetData();
    int i = 0;
    for (i = 0; i < int_per_page; i++)
    {
        int num;
        memcpy(&num, &data[sizeof(int) * i], sizeof(int));
        // if (num == INT32_MIN)
        //     break;
        cout << ph.GetPageNum() << " - " << num << endl;
    }
    fh->UnpinPage(pid);
    fh->FlushPage(pid);
}

int main(int argc, char *argv[])
{
    FileManager fm;

    FileHandler output;
    try
    {
        fm.DestroyFile(argv[1]);
        output = fm.CreateFile(argv[1]);
        cout << "File created : " << argv[1] << endl;
    }
    catch (const std::exception &e)
    {
        cout << "Error opening files " << endl;
    }
    string q;


    PageHandler ph;
    PageHandler outPage;

    vector<int> vect{
    2, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3,
    3, 4, 4, 4, 4, 4,
    4, 5, 6, 7, 8, 9,
    9, 10, 10, 10, 10, 10,
    10, 10, 10, 12, 12, 12,
    12, 12, 12, 12, 12, 12
    };

    int currPage = 0;
    int currOffset = 0;
    outPage = output.NewPage();

    for(int x: vect ){
      char *output_buffer = outPage.GetData();
      if (currOffset == PAGE_SIZE - sizeof(int))
      {
        output.MarkDirty(currPage);
        output.UnpinPage(currPage);
        output.FlushPage(currPage);
        currPage += 1;
        output.NewPage();
        currOffset = 0;
      }
      memcpy(&output_buffer[currOffset], &x, sizeof(int));
      currOffset += sizeof(int);
    }

    if(currOffset != PAGE_SIZE - sizeof(int)){
        PageHandler term;
        term = output.LastPage();
        char * data = term.GetData();
        int term_offset = currOffset ;
        int min = INT32_MIN;
        while(term_offset != PAGE_SIZE - sizeof(int)){
            memcpy(&data[term_offset],&min, sizeof(int));
            term_offset += sizeof(int);
        }
        
    }
    output.MarkDirty(currPage);
    output.UnpinPage(currPage);
    output.FlushPage(currPage);

    FileHandler ans;
    ans = fm.OpenFile(argv[1]);
    for (int i = 0;i<currPage+1; i++)
    {
        print_page(&ans, i);
    }

    return 0;
}
