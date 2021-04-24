#include <iostream>
#include "file_manager.h"
#include "errors.h"
#include <bits/stdc++.h>

using namespace std;


void print_page(FileHandler *fh, int pid){
    int int_per_page = PAGE_SIZE / sizeof(int) - 1;
    PageHandler ph;
    try{
        cout << "Printing page no: " << pid << endl;
        ph = fh->PageAt(pid);
    }
    catch(const std::exception &e){
        cout << "Error opening page no: " << pid << endl;
        PageHandler phl = fh->LastPage();
        cout << "Last page of this *ptr_fh " << phl.GetPageNum() << endl;
        cerr << e.what()  << '\n';
    }

    char *data;
    data = ph.GetData();
    int i=0;
    for(int i = 0;i < int_per_page;i++){
        int num;
        memcpy(&num, &data[sizeof(int)*i],sizeof(int));
        cout << i << " - " << num << endl;
    }
    fh->UnpinPage(pid);
    fh->FlushPage(pid);
}

int main(int argc, char *argv[]){
    FileManager fm;
    FileHandler r1;
    FileHandler r2;
    FileHandler output;
    ifstream rfile1;
    ifstream rfile2;
    int min =INT32_MIN;

    try{
        r1 = fm.OpenFile(argv[1]);
        r2 = fm.OpenFile(argv[2]);
        fm.DestroyFile(argv[3]);
        output = fm.CreateFile(argv[3]);
        // cout << "File Opened : " << argv[1] << " & " << argv[2] << endl;
        // cout << "File created: " << argv[3] << endl;
    }
    catch(const std::exception &e){
        cout << "Error opening files " << endl;
    }

    int int_per_page = PAGE_SIZE / sizeof(int) - 1;
    PageHandler ph1;
    PageHandler ph2;
    PageHandler outPage;
    int r1Pages = r1.LastPage().GetPageNum() + 1;
    r1.UnpinPage(r1Pages-1);
    r2.FlushPage(r1Pages-1);
    int r2Pages = r2.LastPage().GetPageNum() + 1;
    r2.UnpinPage(r2Pages-1);
    r2.FlushPage(r2Pages-1);

    int currOutputPage = -1;
    int currOutputPageOffset = 0;

    // for(int i=0;i<r1Pages;i++){
    //     print_page(&r1,i);
    // }
    // for(int i=0;i<r2Pages;i++){
    //     print_page(&r2,i);
    // }

    for(int i = 0; i < r1Pages; i++){
        ph1 = r1.PageAt(i);
        char *datar1 = ph1.GetData();
        for(int ii = 0; ii < r2Pages; ii++){
            ph2 = r2.PageAt(ii);
            char *datar2 = ph2.GetData();
            for(int ip = 0; ip < int_per_page; ip++){
                int pg1Num;
                memcpy(&pg1Num,&datar1[ip*sizeof(int)],sizeof(int));
                if(pg1Num == INT32_MIN){
                    break;
                }
                else{
                    for(int iip = 0; iip < int_per_page; iip++){
                        int pg2Num;
                        memcpy(&pg2Num, &datar2[iip*sizeof(int)],sizeof(int));
                        if(pg2Num == INT32_MIN){
                            break;
                        }
                        else if(pg1Num == pg2Num){
                            // cout << "Num found: " << pg1Num << " at ";
                            // cout << i << "."<< ip << " - " << ii <<"."<<iip << endl;
                            if(currOutputPage == -1){
                                outPage = output.NewPage();
                                currOutputPage = 0;
                            }
                            else{
                                outPage = output.PageAt(currOutputPage);
                            }

                            char *output_buffer = outPage.GetData();
                            if(currOutputPageOffset == PAGE_SIZE - sizeof(int)){
                                output.MarkDirty(currOutputPage);
                                output.UnpinPage(currOutputPage);
                                output.FlushPage(currOutputPage);
                                currOutputPage += 1;
                                currOutputPageOffset = 0;
                                output.NewPage();
                            }
                            // cout << currOutputPageOffset << " Page: "<<currOutputPage << endl;
                            memcpy(&output_buffer[currOutputPageOffset], &pg1Num, sizeof(int));
                            currOutputPageOffset += sizeof(int);

                        }

                    }
                }

            }
            r2.UnpinPage(ii);
            r2.FlushPage(ii);
        }
        r1.UnpinPage(i);
        r1.FlushPage(i);
    }
    // cout << currOutputPage << endl;
    if(currOutputPage == -1){
      return 0;
    }
    outPage = output.PageAt(currOutputPage);
    char *output_buffer = outPage.GetData();
    while(currOutputPageOffset != PAGE_SIZE - sizeof(int)){
        memcpy(&output_buffer[currOutputPageOffset],&min,sizeof(int));
        currOutputPageOffset += sizeof(int);
    }
    output.MarkDirty(currOutputPage);
    output.UnpinPage(currOutputPage);
    output.FlushPage(currOutputPage);

    // for(int i=0;i<currOutputPage+1;i++){
    //     print_page(&output,i);
    // }

}
