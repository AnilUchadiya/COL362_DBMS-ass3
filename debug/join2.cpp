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
        // cout << ph.GetPageNum() << " - " << num << endl;
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

pair<int, int> first_occ(int num,FileHandler *fh){
    bool found = false;
    PageHandler ph;
    PageHandler outPage;
    int total_Pages = totalpage(fh);
    int int_per_page = PAGE_SIZE / sizeof(int) - 1;
    int high = total_Pages - 1, low = 0;
    int middle;
    int findindex;
    int findpage;
    while (low <= high)
    {
        middle = (low + high) / 2;
        // cout << "Page requested: " <<  << endl;
        ph = fh->PageAt(middle);
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
        fh->UnpinPage(middle);
        fh->FlushPage(middle);
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
        findpage = -1;
        // cout << " Number not found : " << num << endl;
        return make_pair(-1,-1);
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
            ph = fh->PageAt(prev);
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
            fh->UnpinPage(prev);
            fh->FlushPage(prev);

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

    }
    return make_pair(findpage, findindex);
}


int main(int argc, char *argv[]){
    FileManager fm;
    FileHandler r1;
    FileHandler r2;
    FileHandler output;
    ifstream qfile;
    int pinnedPage =1;
    try
    {
        r1 = fm.OpenFile(argv[1]);
        r2 = fm.OpenFile(argv[2]);
        fm.DestroyFile(argv[3]);
        output = fm.CreateFile(argv[3]);
        // cout << "File opened : " << argv[1] << endl;
        // cout << "File Created : " << argv[3] << endl;
    }
    catch (const std::exception &e)
    {
        cout << "Error opening files " << endl;
    }
    string q;
    int output_currPage = -1;
    int output_currOffset = 0;
    int r1Pages = totalpage(&r1);
    int r2Pages = totalpage(&r2);
    int int_per_page = PAGE_SIZE / sizeof(int) - 1;
    // cout<<"here"<<endl;
    PageHandler outPage;
    for(int r1p = 0;r1p<r1Pages;r1p++){
        // cout<<"here1"<<endl;
        PageHandler r1ph = r1.PageAt(r1p);
        pinnedPage +=1;
        char * r1data = r1ph.GetData();
        for(int i=0;i< int_per_page; i++){
            // cout<<"here2"<<endl;
            int num ;
            memcpy(&num,&r1data[i* sizeof(int)],sizeof(int));
            int q= -1;
            int offset;
            pair<int,int> occ = first_occ(num,&r2);
            q = occ.first;
            offset = occ.second;
            // cout<<"found num "<<num<< "pno = "<<q<<" index "<<offset<<endl;
            if(q == -1){
                // cout <<"Not found num : "<<num<<endl;
                continue;
            }
            else{

                while(q < r2Pages ){
                    // cout<<"here3"<<endl;
                    int temp;
                    PageHandler r2ph = r2.PageAt(q);
                    pinnedPage +=1;
                    char * r2data = r2ph.GetData();
                    memcpy(&temp,&r2data[offset* sizeof(int)], sizeof(int));
                    if(temp ==num){
                        if (output_currPage == -1)
                        {
                            outPage = output.NewPage();
                            output_currPage = 0;
                        }
                        else
                        {
                            outPage = output.PageAt(output_currPage);
                        }
                        char *output_buffer = outPage.GetData();
                        if (output_currOffset == PAGE_SIZE - sizeof(int))
                        {
                            output.MarkDirty(output_currPage);
                            output.UnpinPage(output_currPage);
                            output.FlushPage(output_currPage);
                            output_currPage += 1;
                            output.NewPage();
                            output_currOffset = 0;
                        }
                        memcpy(&output_buffer[output_currOffset], &num, sizeof(int));
                        output_currOffset += sizeof(int);
                    }
                    if(offset == int_per_page){
                        r2.UnpinPage(q);
                        r2.FlushPage(q);
                        pinnedPage -=1;
                        q++;
                        offset =0;
                    }
                    else{
                        offset ++;
                    }

                }

            }
        }
        if(pinnedPage == BUFFER_SIZE ){
                r1.UnpinPage(r1p);
                r1.FlushPage(r1p);
                pinnedPage -=1;
        }
    }

    if(output_currPage == -1){
      return 0;
    }

    if(output_currOffset != PAGE_SIZE - sizeof(int)){
        PageHandler term;
        term = output.LastPage();
        char * data = term.GetData();
        int term_offset = output_currOffset ;
        int min = INT32_MIN;
        while(term_offset != PAGE_SIZE - sizeof(int)){
            memcpy(&data[term_offset],&min, sizeof(int));
            term_offset += sizeof(int);
        }
        output.MarkDirty(output_currPage);
        output.UnpinPage(output_currPage);
        output.FlushPage(output_currPage);
    }
    else{
      output.MarkDirty(output_currPage);
      output.UnpinPage(output_currPage);
      output.FlushPage(output_currPage);
    }

    //  FileHandler ans;
    // ans = fm.OpenFile("TestCases/TC_join2/output_join2");
    // for (int i = 0; i < r1Pages; i++)
    // {
    //     print_page(&r1, i);
    // }
    // for (int i = 0; i < r2Pages; i++)
    // {
    //     print_page(&r2, i);
    // }
    // for (int i = 0;i<output_currPage+1; i++)
    // {
    //     print_page(&output, i);
    //     print_page(&ans, i);
    //
    // }

}
