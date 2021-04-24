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
    PageHandler phl = fh->LastPage();
    // cout << "last page of this *ptr_fh " << phl.GetPageNum() << endl;
    int lp = phl.GetPageNum();
    if (lp == -1)
    {
        cout << " Empty file " << endl;
        return;
    }
    fh->UnpinPage(lp);
    fh->FlushPage(lp);
    try
    {
        cout << "Printing  page no : " << pid << endl;
        ph = fh->PageAt(pid);
    }
    catch (const std::exception &e)
    {
        // cout << "Error opening page no : " << pid << endl;
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
        cout << ph.GetPageNum() << " - " << num << endl;
        // if (num == INT32_MIN)
        //     break;
    }
    fh->UnpinPage(pid);
    fh->FlushPage(pid);
}
int totalpage(FileHandler *fh)
{
    PageHandler lastpage = fh->LastPage();
    int total_Pages = lastpage.GetPageNum() + 1;
    fh->UnpinPage(total_Pages - 1);
    fh->FlushPage(total_Pages - 1);
    return total_Pages;
}
int clean_file(FileHandler *fh)
{

    int pno = 0;
    PageHandler ph;
    int deletecount = 0;
    ph = fh->LastPage();
    pno = ph.GetPageNum();
    fh->UnpinPage(pno);
    fh->FlushPage(pno);
    // cout << "clenup iu[ = " << pno << endl;
    if (pno == -1)
        return 0;

    for (int pn = pno; pn >= 0; pn--)
    {
        ph = fh->PageAt(pn);
        char *data = ph.GetData();
        bool destroy = true;
        for (int i = 0; i < 6; i++)
        {
            int temp;
            memcpy(&temp, &data[i * sizeof(int)], sizeof(int));
            if (temp != INT32_MIN)
            {
                destroy = false;
                fh->UnpinPage(pn);
                fh->FlushPage(pn);
                // cout << "temp = " << temp << endl;
                // cout << "clenup - " << pn << " *** " << i << deletecount << endl;
                return deletecount;
                break;
            }
        }
        if (destroy)
        {
            fh->UnpinPage(pn);
            fh->FlushPage(pn);
            fh->DisposePage(pn);
            fh->FlushPages();
            deletecount++;
        }
    }
    // cout << "clenup - " << deletecount << endl;
    return deletecount;
}

struct empty_pos
{
    int startp;
    int startoff;
    int lastp;
    int lastoff;
    bool edited = false;
};

struct empty_pos swap_up(FileHandler *fh)
{
    int tp = totalpage(fh);
    struct empty_pos ans;
    // cout<<"in swap"<<tp<<endl;
    if (tp == 0)
        return ans;
    // cout<<"swap"<<tp<<endl;
    PageHandler ph;
    int start_page;
    int start_offset;
    int last_page;
    int last_offset;
    bool empty = false;
    int currPage = 0;
    int currOffset = 0;
    int min = INT32_MIN;
    bool eof = false;
    bool interval = false;
    for (int i = 0; i < tp; i++)
    {
        ph = fh->PageAt(i);
        char *data = ph.GetData();
        for (int j = 0; j < 6; j++)
        {
            int temp;
            memcpy(&temp, &data[j * sizeof(int)], sizeof(int));
            // cout<<"temp = "<<temp<<endl;
            if (temp == INT32_MIN && !empty)
            {
                start_page = i;
                start_offset = j;
                empty = true;
                // printf("start_page = %d,start_offset = %d  \n", start_page, start_offset);
            }
            if (empty && temp != INT32_MIN)
            {
                last_page = i;
                last_offset = j;
                // printf("start_page = %d, last_page = %d start_offset = %d last_offset = %d \n", start_page, last_page, start_offset, last_offset);
                interval = true;
                ans.edited = true;
                break;
            }
        }
        fh->UnpinPage(i);
        fh->FlushPage(i);
        if (empty && interval)
            break;
    }
    ans.startp = start_page;
    ans.startoff = start_offset;
    ans.lastp = last_page;
    ans.lastoff = last_offset;
    return ans;
}

int main(int argc, char *argv[])
{
    FileManager fm;
    FileHandler input;

    FileHandler output;
    // cout << "okk" << endl;
    ifstream qfile;
    try
    {
        input = fm.OpenFile(argv[1]);
        qfile.open(argv[2]);
        // cout << "File opened : " << argv[1] << endl;
        // cout << "File Created : " << argv[3] << endl;
    }
    catch (const std::exception &e)
    {
        // cout << "Error opening files " << endl;
    }
    PageHandler lastpage = input.LastPage();
    int total_Pages = lastpage.GetPageNum() + 1;
    input.UnpinPage(total_Pages - 1);
    input.FlushPage(total_Pages - 1);
    // for (int i = 0; i < total_Pages; i++)
    // {
    //     print_page(&input, i);
    // }
    string q;
    int currPage = 0;
    int currOffset = 0;
    int min = INT32_MIN;

    int int_per_page = PAGE_SIZE / sizeof(int) - 1;
    while (qfile >> q)
    {
        // cout << " q = " << q << endl;
        if (q == "DELETE")
        {
            int num;
            qfile >> num;
            bool deleted = false;
            bool eof = false;
            PageHandler ph;
            PageHandler qh;
            currPage = 0;
            currOffset = 0;
            int page_p;
            int offset_p;
            int offset_q;
            int page_q;
            // cout << "num = " << num << endl;
            while (currPage <= total_Pages - 1)
            {

                try
                {
                    ph = input.PageAt(currPage);
                }
                catch (const std::exception &e)
                {
                    // cout << "Error opening page no : " << currPage << endl;
                    PageHandler phl = input.LastPage();
                    // cout << "last page of this *ptr_fh " << phl.GetPageNum() << endl;
                    std::cerr << e.what() << '\n';
                    input.UnpinPage(total_Pages - 1);
                    input.FlushPage(total_Pages - 1);
                }
                char *data_p = ph.GetData();
                int temp;
                memcpy(&temp, &data_p[currOffset], sizeof(int));

                // if (temp == INT32_MIN)
                //     break;
                if (temp == num)
                {
                    // input.UnpinPage(currPage);
                    // input.FlushPages();
                    // cout << "Found num : " << num << " pno = " << currPage << " offset = " << currOffset << endl;
                    memcpy(&data_p[currOffset], &min, sizeof(int));
                    currOffset += sizeof(int);
                }
                else
                {
                    currOffset += sizeof(int);
                }

                if (currOffset == PAGE_SIZE - sizeof(int))
                {
                    input.MarkDirty(currPage);
                    input.UnpinPage(currPage);
                    input.FlushPage(currPage);
                    currPage++;
                    currOffset = 0;
                }
            }
            input.MarkDirty(currPage);
            input.UnpinPage(currPage);
            input.FlushPage(currPage);
            struct empty_pos ans = swap_up(&input);
            if (!ans.edited)
            {
                // continue;
            }
            else
            {
                // for (int i = 0; i < totalpage(&input); i++)
                // {
                //     print_page(&input, i);
                // }
                page_p = ans.startp;
                offset_p = ans.startoff * sizeof(int);
                offset_q = ans.lastoff * sizeof(int);
                page_q = ans.lastp;
                if (currOffset == PAGE_SIZE - sizeof(int))
                {
                    page_q++;
                    offset_q = 0;
                }

                while (page_q < total_Pages)
                {
                    PageHandler page_ph;
                    PageHandler page_qh;
                    // cout<<"pq = "<<page_q<<"pp = "<<page_p<<endl;
                    if (offset_q == PAGE_SIZE - sizeof(int))
                    {
                        input.MarkDirty(page_q);
                        input.UnpinPage(page_q);
                        input.FlushPage(page_q);
                        page_q++;
                        offset_q = 0;
                        if (page_q == total_Pages)
                        {
                            // cout << "breaked here " << offset_q << "  " << offset_p << endl;
                            if (!eof && offset_p == PAGE_SIZE - 2 * sizeof(int))
                            {
                                // cout << "inserting intmin " << endl;
                                page_ph = input.PageAt(page_p);
                                char *data_p = page_ph.GetData();
                                memcpy(&data_p[offset_p], &min, sizeof(int));
                                offset_p += sizeof(int);
                                input.MarkDirty(page_p);
                                input.UnpinPage(page_p);
                                input.FlushPage(page_p);
                            }
                            break;
                        }
                    }
                    if (offset_p == PAGE_SIZE - sizeof(int))
                    {
                        input.MarkDirty(page_p);
                        input.UnpinPage(page_p);
                        input.FlushPage(page_p);
                        page_p++;
                        offset_p = 0;
                        if (page_q == total_Pages)
                        {
                            break;  
                        }
                    }
                    try
                    {
                        page_ph = input.PageAt(page_p);
                    }
                    catch (const std::exception &e)
                    {
                        // cout << "Error opening page_ph no : " << currPage << endl;
                        PageHandler phl = input.LastPage();
                        // cout << "last page of this  " << phl.GetPageNum() << endl;
                        std::cerr << e.what() << '\n';
                        input.UnpinPage(total_Pages - 1);
                        input.FlushPage(total_Pages - 1);
                    }
                    try
                    {
                        page_qh = input.PageAt(page_q);
                    }
                    catch (const std::exception &e)
                    {
                        // cout << "Error opening page_qh no : " << currPage << endl;
                        PageHandler phl = input.LastPage();
                        // cout << "last page of this  " << phl.GetPageNum() << endl;
                        std::cerr << e.what() << '\n';
                        input.UnpinPage(total_Pages - 1);
                        input.FlushPage(total_Pages - 1);
                    }
                    char *data_p = page_ph.GetData();
                    char *data_q = page_qh.GetData();

                    int temp;
                    memcpy(&temp, &data_q[offset_q], sizeof(int));
                    int temp2;
                    memcpy(&temp2, &data_p[offset_p], sizeof(int));
                    if (temp == INT32_MIN)
                    {
                        eof = true;
                        // cout<<"End of file found"<<endl;
                    }
                    // if(temp ==0){
                    //     printf("page_p = %d, page_q = %d offset_p = %d offset_q = %d \n",page_p,page_q,offset_p,offset_q);
                    // }
                    // cout << "copying " << temp << " to " << temp2 << endl;
                    if (!eof && page_p == total_Pages - 1 && offset_p == PAGE_SIZE - 2 * sizeof(int))
                    {
                        // cout << "Page is completly packed eof not found" << endl;
                        memcpy(&data_p[offset_p], &min, sizeof(int));
                        input.MarkDirty(page_p);
                        input.UnpinPage(page_p);
                        input.FlushPage(page_p);
                    }
                    else
                    {
                        // cout<<"here"<<endl;
                        memcpy(&data_p[offset_p], &data_q[offset_q], sizeof(int));
                        memcpy(&data_q[offset_q],&min, sizeof(int));
                    }

                    offset_p += sizeof(int);
                    offset_q += sizeof(int);
                }
                input.MarkDirty(page_q);
                input.UnpinPage(page_q);
                input.FlushPage(page_q);
                input.MarkDirty(page_p);
                input.UnpinPage(page_p);
                input.FlushPage(page_p);

                // for (int i = 0; i < totalpage(&input); i++)
                // {
                //     print_page(&input, i);
                // }
            }
            
            // swap_up(&input);
            // cout << "Found num : " << num << " pno = " << currPage << " offset = " << currOffset << endl;
            try
            {
                int dp = clean_file(&input);
                total_Pages -= dp;
                // if (dp > 0)
                // cout << "Deleted  Pages :  " << dp << endl;
            }
            catch (const std::exception &e)
            {
                // cout << "Error opening page_ph no : " << currPage << endl;
                PageHandler phl = input.LastPage();
                // cout << "last page of this  " << phl.GetPageNum() << endl;
                std::cerr << e.what() << '\n';
                input.UnpinPage(total_Pages - 1);
                input.FlushPage(total_Pages - 1);
            }
        }
        // for (int i = 0; i < totalpage(&input); i++)
        // {
        //     print_page(&input, i);
        // }

        // break;
    }
    input.FlushPages();
    // FileHandler ans;
    // ans = fm.OpenFile("TestCases/TC_delete/output_delete");
    for (int i = 0; i < totalpage(&input); i++)
    {
        print_page(&input, i);
    }
    // for (int i = 0; i < totalpage(&ans); i++)
    // {
    //     print_page(&ans, i);
    // }
    // cout << "okk" << endl;

    // cout<<i<<endl;
    return 0;
}