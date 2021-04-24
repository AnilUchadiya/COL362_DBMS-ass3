#include <iostream>
#include "file_manager.h"
#include "errors.h"
#include <cstring>
#include <bits/stdc++.h>

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
    FileHandler input;

    FileHandler output;
    ifstream qfile;
    try
    {
        input = fm.OpenFile(argv[1]);
        qfile.open(argv[2]);
        fm.DestroyFile(argv[3]);
        output = fm.CreateFile(argv[3]);
        cout << "File opened : " << argv[1] << endl;
        cout << "File Created : " << argv[3] << endl;
    }
    catch (const std::exception &e)
    {
        cout << "Error opening files " << endl;
    }
    string q;
    int currPage = -1;
    int currOffset = 0;

    PageHandler lastpage = input.LastPage();
    int total_Pages = lastpage.GetPageNum() + 1;
    int int_per_page = PAGE_SIZE / sizeof(int) - 1;
    input.UnpinPage(total_Pages - 1);
    input.FlushPage(total_Pages - 1);
    while (qfile >> q)
    {
        if (q == "SEARCH")
        {
          // cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
            int num;
            qfile >> num;
            bool found = false;
            PageHandler ph;
            PageHandler outPage;
            for (int i = 0; i < total_Pages; i++)
            {
                ph = input.PageAt(i);
                char *data = ph.GetData();
                for (int in = 0; in < int_per_page; in++)
                {
                    int temp;
                    memcpy(&temp, &data[in * sizeof(int)], sizeof(int));
                    if (temp == INT32_MIN)
                        break;
                    if (temp == num)
                    {
                        // cout << "Found num : " << num << " pno = " << i << " offset = " << in << endl;
                        found = true;
                        if (currPage == -1)
                        {
                            outPage = output.NewPage();
                            currPage = 0;
                        }
                        else
                        {
                            outPage = output.PageAt(currPage);
                        }
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
                        // cout <<currOffset<<" page  = "<<currPage<<endl;
                        memcpy(&output_buffer[currOffset], &i, sizeof(int));
                        currOffset += sizeof(int);
                        memcpy(&output_buffer[currOffset], &in, sizeof(int));
                        currOffset += sizeof(int);

                    }
                }
                input.UnpinPage(i);
                input.FlushPage(i);
            }


            if (!found)
            {
                // cout << " Number not found : " << num << endl;
            }
            if (currPage == -1)
            {
                outPage = output.NewPage();
                currPage = 0;
            }
            else
                outPage = output.PageAt(currPage);
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
            int terminate = -1;
            memcpy(&output_buffer[currOffset], &terminate, sizeof(int));
            memcpy(&output_buffer[currOffset + sizeof(int)], &terminate, sizeof(int));
            currOffset += 2 * sizeof(int);
        }
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
        output.MarkDirty(currPage);
        output.UnpinPage(currPage);
        output.FlushPage(currPage);
    }
    // int intpPage = (PAGE_SIZE / sizeof(int));
    // PageHandler ph;
    // ph = input.FirstPage();
    // char *data ;
    // data = ph.GetData();
    // int i=0;
    FileHandler ans;
    ans = fm.OpenFile("TestCases/TC_search/output_search");
    for (int i = 0;i<currPage+1; i++)
    {
        // print_page(&ans, i);
        print_page(&output, i);
    }

    // cout<<i<<endl;
    return 0;
}
