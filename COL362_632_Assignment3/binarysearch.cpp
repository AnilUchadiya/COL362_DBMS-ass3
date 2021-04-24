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
            int num;
            qfile >> num;
            cout << "+++++++++++++++++++++++++++++++++++++++++++++++++++++" << endl;
            // cout << "SEARCH -> " << num << endl;
            bool found = false;
            PageHandler ph;
            PageHandler outPage;

            int high = total_Pages - 1, low = 0;
            int middle;
            int findindex;
            int findpage;
            // for (int i = 0; i < total_Pages; i++)
            while (low <= high)
            {
                middle = (low + high) / 2;
                // cout << "Page requested: " <<  << endl;
                ph = input.PageAt(middle);
                char *data = ph.GetData();
                int plow;
                int phigh;
                memcpy(&plow, &data[0], sizeof(int));

                for (int in = 0; in < int_per_page; in++)
                {
                    int temp;
                    memcpy(&temp, &data[in * sizeof(int)], sizeof(int));
                    if (temp == INT32_MIN)
                    {
                        break;
                    }
                    phigh = temp;

                    if (temp == num)
                    {
                        found = true;
                        findindex = in;
                        findpage = middle;
                        // cout << "Found num : " << num << " pno = " << middle << " offset = " << in << endl;
                        break;
                    }
                }
                input.UnpinPage(middle);
                input.FlushPage(middle);
                if (found)
                    break;
                if (plow < num && phigh > num)
                {
                    break;
                }
                if (phigh < num)
                {
                    low = middle + 1;
                }
                if (plow > num)
                {
                    high = middle - 1;
                }
            }
            // cout << "DONE " << endl;

            if (!found)
            {
                // cout << " Number not found : " << num << endl;
            }
            else
            {
                // cout << "page num: " << findpage << ", index: " << findindex << endl;

                while (findpage > 0)
                {
                    if (findindex > 0)
                    {
                        break;
                    }
                    int prev = findpage - 1;
                    ph = input.PageAt(prev);
                    char *data = ph.GetData();

                    int cind = int_per_page - 1;
                    while (cind > -1)
                    {
                        int temp;
                        memcpy(&temp, &data[cind * sizeof(int)], sizeof(int));

                        if (temp != num)
                        {
                            // cind++;
                            break;
                        }
                        else
                        {
                            cind--;
                        }
                    }
                    input.UnpinPage(prev);
                    input.FlushPage(prev);

                    cind++;

                    // if(cind == -1){
                    //   cind == 0;
                    // }

                    if (cind == int_per_page)
                    {
                        break;
                    }
                    else
                    {
                        findpage = prev;
                        findindex = cind;
                    }
                }

                // cout << "page num: " << findpage << ", index: " << findindex << endl;

                // test for 2 pages of same value, check till 0 index too

                // bool stp = false;
                while (findpage < total_Pages)
                {

                    ph = input.PageAt(findpage);
                    char *data = ph.GetData();

                    while (findindex < int_per_page)
                    {

                        int temp;
                        memcpy(&temp, &data[findindex * sizeof(int)], sizeof(int));

                        if (temp != num)
                        {
                            // stp = true;
                            break;
                        }
                        cout << "Found num : " << num << " pno = " << findpage << " offset = " << findindex << endl;

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
                        memcpy(&output_buffer[currOffset], &findpage, sizeof(int));
                        currOffset += sizeof(int);
                        memcpy(&output_buffer[currOffset], &findindex, sizeof(int));
                        currOffset += sizeof(int);

                        findindex++;
                    }

                    input.UnpinPage(findpage);
                    input.FlushPage(findpage);

                    // if(stp){
                    //   break;
                    // }
                    if (findindex == int_per_page)
                    {
                        findindex = 0;
                        findpage++;
                    }
                    else
                    {
                        break;
                    }
                }
            }
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
            int terminate = -1;
            memcpy(&output_buffer[currOffset], &terminate, sizeof(int));
            memcpy(&output_buffer[currOffset + sizeof(int)], &terminate, sizeof(int));
            currOffset += 2 * sizeof(int);
        }
    }

    if (currOffset != PAGE_SIZE - sizeof(int))
    {
        PageHandler term;
        term = output.LastPage();
        char *data = term.GetData();
        int term_offset = currOffset;
        int min = INT32_MIN;
        while (term_offset != PAGE_SIZE - sizeof(int))
        {
            memcpy(&data[term_offset], &min, sizeof(int));
            term_offset += sizeof(int);
        }
        output.MarkDirty(currPage);
        output.UnpinPage(currPage);
        output.FlushPage(currPage);
    }
    else
    {
        output.MarkDirty(currPage);
        output.UnpinPage(currPage);
        output.FlushPage(currPage);
    }

    FileHandler ans;
    ans = fm.OpenFile(argv[3]);
    std::cout << "numpages " << currPage + 1 << '\n';
    for (int i = 0; i < currPage + 1; i++)
    {
        print_page(&ans, i);
        // print_page(&output, i);
    }

    return 0;
}

/*
./search TestCases/TC_search/sorted_input TestCases/TC_search/query_search.txt output.txt
make binary_search
/binarysearch TestCases/TC_search/sorted_input TestCases/TC_search/query_search.txt output.txt
*/
