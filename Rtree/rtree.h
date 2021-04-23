#include <unistd.h>
#include <iostream>
#include "buffer_manager.h"
#include "file_manager.h"
#include "errors.h"
#include <algorithm>
#include <cstdio>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include "vector"

using namespace std;

// for a given node 
// pid = -2 -> it's a root node
// all children ids = -2 
// if(coresponding child mbr data persent) -> children_ids[:] = -1
// all mbr = INT_MINs :: -2147483648
int current_page_number = 0;  // page number which can be used for inserting new node.
class node{
    public:
    int id;
    int pid;
    int* children_ids;
    int** mbr;
    node(int maxCap, int dim, int id_, int pid_, int* coordinates){
        id = id_;
        pid = pid_;
        mbr = new int*[maxCap];
        children_ids = new int[maxCap];
        for (int i = 0; i < maxCap; i++){
            children_ids[i] = -2;
            mbr[i] = new int[2*dim];
            for (int j = 0; j < dim; j++){
                mbr[i][2*j] = INT32_MIN;
                mbr[i][2*j+1] = INT32_MIN;
            }   
        }
        children_ids[0] = -1;
        for(int j=0; j<2*dim; j++){
            mbr[0][j] = coordinates[j];
        }
    }
};

void free_mem(node* n){
    delete n->children_ids;
    delete n->mbr;
    return;
}
void print_node(node* n, int maxCap, int dim){
    cout<<"id "<<n->id<<" pid "<<n->pid<<endl;
    for (int i = 0; i < maxCap; i++){
        cout<<i<<" | "<<n->children_ids[i]<<": ";
        for (int j = 0; j < dim; j++){
            cout<<"("<<n->mbr[i][2*j]<<" <-> "<<n->mbr[i][2*j+1]<<"),";
        }
        cout<<endl;
    }
}
void print_page(FileHandler *ptr_fh, int id,int dim,int maxCap){
    int nd_size = 4*(2 + maxCap + maxCap*(2*dim));
    int nodeCap_perPage = (PAGE_SIZE / nd_size);
    int page_num = id/nodeCap_perPage;
    int page_offset = (id % nodeCap_perPage)*nd_size;
    PageHandler ph;
    try{
        cout<<"page number "<<page_num<<endl;
        ph = ptr_fh->PageAt(page_num);
    }
    catch(const std::exception& e)
    {
        cout<<"cureent global page number "<<current_page_number<<endl;
        cout<<"page number "<<page_num<<" caused exception in print_page"<<endl;
        PageHandler ph_last = ptr_fh->LastPage();
        cout<<"last page of this *ptr_fh "<<ph_last.GetPageNum()<<endl;
        std::cerr << e.what() << '\n';
    }
    
    char *data = ph.GetData ();
    cout<<"------------------------------------------------------------------"<<endl;
    cout<<"page num "<<ph.GetPageNum()<<"| page offset "<<page_offset<<"| node size "<<nd_size<<"| node id "<<id<<endl;
    int four = 0;
    for (int i = 0; i < nd_size/4; i++){
        int dt;
        memcpy (&dt, &(data[page_offset]), sizeof(int));
        if(i==0)                cout<<"id : "<<dt; 
        else if(i==1)           cout<<" pid : "<<dt;
        else if(i == 2)         cout<<" children : ["<<dt<<",";
        else if(i<maxCap+1)     cout<<dt<<",";
        else if(i == maxCap+1)  {cout<<dt<<"] mbr : [(";}
        else{
            int ind = i - 1 - maxCap;
            if(ind % (2*dim) == 0) cout<<dt<<")][(";
            else if(ind%2 == 0) cout<<dt<<")(";
            else cout<<dt<<",";
        }
        page_offset += 4;
    }
    cout<<endl<<"------------------------------------------------------------------"<<endl;
    ptr_fh->UnpinPage(page_num);
    ptr_fh->FlushPage(page_num);
    return;
}
// to load while node data in page from current page offset. retrun true if load is succesfull else false if failed to do so.
bool load_node(FileHandler *ptr_fh, node* nd, int dim, int maxCap){
    int nd_size = 8 + maxCap*4 + maxCap*((2*dim)*4);
    int nodeCap_perPage = (PAGE_SIZE / nd_size);
    int page_num = nd->id/nodeCap_perPage;
    PageHandler ph;
    int page_offset;
    if(current_page_number == page_num-1){
        // cout<<"New page formation on demand of id"<<nd->id<<" , creating page of page number "<<page_num<<endl;
        ph = ptr_fh->NewPage();
        // cout<<"created new page of page number "<<ph.GetPageNum()<<endl;
        try{
            ph = ptr_fh->PageAt(page_num);
        }
        catch(const std::exception& e){
            // cout<<"page number "<<page_num<<" cause exception for new page"<<endl;
            std::cerr << e.what() << '\n';
        }
        page_offset = 0;
        current_page_number++;
    }else{
        ph = ptr_fh->PageAt(page_num);
        try{
            ph = ptr_fh->PageAt(page_num);
        }
        catch(const std::exception& e){
            // cout<<"page number "<<page_num<<" cause exception"<<endl;
            std::cerr << e.what() << '\n';
        }
        page_offset = (nd->id % nodeCap_perPage)*nd_size;
    }
    char *data = ph.GetData ();

    memcpy (&(data[page_offset]), &(nd->id), sizeof(int));
    page_offset+=4;
    memcpy (&(data[page_offset]), &(nd->pid), sizeof(int));
    page_offset+=4;
    for (int i = 0; i < maxCap; i++){
        memcpy (&(data[page_offset]), &(nd->children_ids[i]), sizeof(int));
        page_offset+=4;
    }
    for (int i = 0; i < maxCap; i++){
        for (int j = 0; j < 2*dim; j++){
            memcpy (&(data[page_offset]), &(nd->mbr[i][j]), sizeof(int));
            page_offset+=4;
        }
    }
    ptr_fh->MarkDirty(page_num);
    ptr_fh->UnpinPage(page_num);
    ptr_fh->FlushPages();
    // print_page(ptr_fh,nd->id,dim,maxCap);
    return true;
}

node* retrive_node(char *data,int page_offset,int dim,int maxCap){
    int id,pid;
    int coordinates[2*dim];
    for (int i = 0; i < 2*dim; i++){
        coordinates[i] = INT32_MIN;
    }
    node* ret_node = new node(maxCap,dim,-100,-100,coordinates);
    memcpy (&(ret_node->id), &data[page_offset], sizeof(int));
    page_offset+=4;
    memcpy (&(ret_node->pid), &data[page_offset], sizeof(int));
    page_offset+=4;
    for (int i = 0; i < maxCap; i++){
        memcpy (&(ret_node->children_ids[i]), &data[page_offset], sizeof(int));
        page_offset+=4;
    }
    for (int i = 0; i < maxCap; i++){
        for (int j = 0; j < 2*dim; j++){
            memcpy (&(ret_node->mbr[i][j]), &data[page_offset], sizeof(int));
            page_offset+=4;
        }
    }
    return ret_node;
}

node* get_node(FileHandler *ptr_fh, int current_nodeId,int dim, int maxCap){
    int nd_size = 8 + maxCap*4 + maxCap*((2*dim)*4);
    int nodeCap_perPage = (PAGE_SIZE / nd_size);
    int page_num = current_nodeId/nodeCap_perPage;
    PageHandler ph;
    try{
        ph = ptr_fh->PageAt(page_num);
    }
    catch(const std::exception& e){
        // cout<<"page number "<<page_num<<" cause exception"<<endl;
        std::cerr << e.what() << '\n';
    }
    
    char *data = ph.GetData ();
    int page_offset = (current_nodeId % nodeCap_perPage)*nd_size;
    node* nd = retrive_node(data,page_offset,dim,maxCap);
    ptr_fh->UnpinPage(page_num);
    ptr_fh->FlushPages();
    return nd;
}

void update_back2root(FileHandler *ptr_fh, node* cnode, int dim, int maxCap){
    // cout<<"In updateBack2Root, from node id "<<cnode->id<<" to update parent "<<cnode->pid<<endl;
    if(cnode->pid == -2){
        return;
    }
    int* mbr_limits = new int[2*dim];
    for (int j = 0; j < dim; j++){
        int left_min = INT32_MAX;
        int right_max = INT32_MIN;
        for (int i = 0; i < maxCap; i++){
            if(cnode->children_ids[i] == -2) break;
            left_min = (cnode->mbr[i][2*j] <= left_min)? cnode->mbr[i][2*j]:left_min;
            right_max = (cnode->mbr[i][2*j+1] >= right_max)? cnode->mbr[i][2*j+1]:right_max;
        }
        mbr_limits[2*j] = left_min;
        mbr_limits[2*j+1] = right_max;
    } 
    // getting parent node data from buffer memory
    int pid = cnode->pid;
    node* pnode = get_node(ptr_fh,pid,dim,maxCap);
    int node_index = -1;
    for (int i = 0; i < maxCap; i++){
        if(pnode->children_ids[i] == cnode->id){
            node_index = i;
            break;
        }
    } 
    // cout<<"checking 198"<<endl;
    // cout<<node_index<<endl;
    // pnode->mbr[node_index] = mbr_limits;

    for (int i = 0; i < dim; i++){
        pnode->mbr[node_index][2*i] = mbr_limits[2*i];
        pnode->mbr[node_index][2*i+1] = mbr_limits[2*i+1];
    }
    // cout<<"checking 202"<<endl;
    load_node(ptr_fh,pnode,dim,maxCap);
    delete mbr_limits;
    // cout<<"checking line 202"<<endl;
    update_back2root(ptr_fh, pnode, dim, maxCap);
    // cout<<"Freeing up memory of node "<<pnode->id<<endl;
    free_mem(pnode);
    // cout<<" memory freed"<<endl;
    return;
}

pair<int,int> get_bigest_rectangle(int** points,int maxCap,int dim,int* new_rectangle){
    double max_Area = INT32_MIN;
    int I,J;
    for (int i = 0; i < maxCap; i++){
        for (int j = i+1; j < maxCap; j++){
            double Area = 1;
            for (int k = 0; k < dim; k++){
                int minx = min(min(points[i][2*k],points[i][2*k+1]) ,min(points[j][2*k],points[j][2*k+1]));
                int maxx = max(max(points[i][2*k],points[i][2*k+1]) ,max(points[j][2*k],points[j][2*k+1]));
                Area = Area*(maxx-minx);
            }
            if(max_Area <= Area){
                max_Area = Area;
                I = i; J = j;
            }
        }
        // for new rectangle
        double Area = 1;
        for (int k = 0; k < dim; k++){
            int minx = min(min(points[i][2*k],points[i][2*k+1]) ,min(new_rectangle[2*k],new_rectangle[2*k+1]));
            int maxx = max(max(points[i][2*k],points[i][2*k+1]) ,max(new_rectangle[2*k],new_rectangle[2*k+1]));
            Area = Area*(maxx-minx);
        }
        if(max_Area <= Area){
            max_Area = Area;
            I = i; J = maxCap;
        }
    }
    pair<int,int> p(I,J);
    return p;
}
void splitNode_ij(node* spliting_node, node* L1, node* L2, int* L1_rectangle, int* L2_rectangle, int* newRect_Entry, int newRect_id, int dim, int maxCap, int I, int J){
    // working parameters
    // cout<<"In splitNode_ij"<<endl;
    int m = maxCap/2;
    int L1_count = 1, L2_count = 1;
    // composit rectangle so far
    for (int i = 0; i < 2*dim; i++){
        L1_rectangle[i] = L1->mbr[0][i];
        L2_rectangle[i] = L2->mbr[0][i];
    }
    
    for (int i = 0; i < maxCap+1; i++){
        if(i== I || i== J){
            continue;
        }
        int* current_point = (i == maxCap)? newRect_Entry:spliting_node->mbr[i];
        // cout<<"point "<<"("<<current_point[0]<<","<<current_point[2]<<")"<<endl;

        // getting flag of insertion
        bool L1_insert_flag = false;
        bool L2_insert_flag = false;
        
        if(L1_count > m){  L2_insert_flag = true;}
        else if(L2_count > m){ L1_insert_flag  = true;}
        else{
            double L1_initial_area = 1;
            double L2_initial_area = 1;
            double L1_extendedArea = 1;
            double L2_extendedArea = 1;
            int minx,maxx;
            for (int k = 0; k < dim; k++){
                L1_initial_area =(L1_rectangle[2*k+1] == L1_rectangle[2*k])? L1_initial_area : L1_initial_area*(L1_rectangle[2*k+1]-L1_rectangle[2*k]);
                minx = min(min(L1_rectangle[2*k],L1_rectangle[2*k+1]) ,min(current_point[2*k],current_point[2*k+1]));
                maxx = max(max(L1_rectangle[2*k],L1_rectangle[2*k+1]) ,max(current_point[2*k],current_point[2*k+1]));
                L1_extendedArea = (maxx == minx)? L1_extendedArea : L1_extendedArea *(maxx-minx);
                
                L2_initial_area =(L2_rectangle[2*k+1] == L2_rectangle[2*k])? L2_initial_area : L2_initial_area*(L2_rectangle[2*k+1]-L2_rectangle[2*k]);
                minx = min(min(L2_rectangle[2*k],L2_rectangle[2*k+1]) ,min(current_point[2*k],current_point[2*k+1]));
                maxx = max(max(L2_rectangle[2*k],L2_rectangle[2*k+1]) ,max(current_point[2*k],current_point[2*k+1]));
                L2_extendedArea = (maxx == minx)? L2_extendedArea : L2_extendedArea *(maxx-minx);
            }
            double L1_enlargement = L1_extendedArea - L1_initial_area;
            double L2_enlargement = L2_extendedArea - L2_initial_area;
            // cout<<L1_extendedArea<<", "<<L2_extendedArea<<endl;
            // cout<<L1_enlargement<<", "<<L2_enlargement<<endl;
            // cout<<L1_initial_area<<", "<<L2_initial_area<<endl;

            if(L1_enlargement < L2_enlargement) {L1_insert_flag = true;}
            else if(L1_enlargement > L2_enlargement) {L2_insert_flag = true;}
            else{
                if(L1_initial_area < L2_initial_area) {L1_insert_flag = true;}
                else if(L1_initial_area > L2_initial_area) {L2_insert_flag = true;}
                else{
                    if(L1_count < L2_count) {L1_insert_flag = true;}
                    else  {L2_insert_flag = true;}
                }
            }
        }
        // add point to L1
        if(L1_insert_flag){
            // cout<<"In L1"<<endl;
            if(newRect_id == -1) L1->children_ids[L1_count] = -1;   //spliting is happening for leaf node
            else{
                if(i==maxCap) L1->children_ids[L1_count] = newRect_id;
                else L1->children_ids[L1_count] = spliting_node->children_ids[i];
            }

            for (int j = 0; j < dim; j++){
                L1->mbr[L1_count][2*j] = current_point[2*j];
                L1->mbr[L1_count][2*j+1] = current_point[2*j+1];
                // L1_rectangle[2*j] = min(L1_rectangle[2*j],current_point[2*j]);
                // L1_rectangle[2*j+1] = max(L1_rectangle[2*j+1],current_point[2*j+1]);
                L1_rectangle[2*j]  = min(min(L1_rectangle[2*j],L1_rectangle[2*j+1]) ,min(current_point[2*j],current_point[2*j+1]));
                L1_rectangle[2*j+1] = max(max(L1_rectangle[2*j],L1_rectangle[2*j+1]) ,max(current_point[2*j],current_point[2*j+1]));
            }
            L1_count++;
        }else{ // add point to L2
            // cout<<"In L2"<<endl;
            if(newRect_id == -1) L2->children_ids[L2_count] = -1;
            else{
                if(i==maxCap) L2->children_ids[L2_count] = newRect_id;
                else L2->children_ids[L2_count] = spliting_node->children_ids[i];
            }
            for (int j = 0; j < dim; j++){
                L2->mbr[L2_count][2*j] = current_point[2*j];
                L2->mbr[L2_count][2*j+1] = current_point[2*j+1];
                // L2_rectangle[2*j] = min(L2_rectangle[2*j],current_point[2*j]);
                // L2_rectangle[2*j+1] = max(L2_rectangle[2*j+1],current_point[2*j+1]);
                L2_rectangle[2*j]  = min(min(L2_rectangle[2*j],L2_rectangle[2*j+1]) ,min(current_point[2*j],current_point[2*j+1]));
                L2_rectangle[2*j+1]  = max(max(L2_rectangle[2*j],L2_rectangle[2*j+1]) ,max(current_point[2*j],current_point[2*j+1]));
            }
            L2_count++;
        }
    }
    return;
}

void update_upward(FileHandler *ptr_fh, node* L10, node* L20, int* L1_rectangle0, int* L2_rectangle0,int dim, int maxCap, int* lastid_ptr, int* rootid_ptr){
    // cout<<"In update upward"<<endl;
    // if root has been splited
    if(L10->pid == -2){
        int newrootid = (++*lastid_ptr);
        *rootid_ptr = newrootid;
        node* newroot = new node(maxCap,dim,newrootid,-2,L1_rectangle0);
        newroot->children_ids[0] = L10->id;
        newroot->mbr[0] = L1_rectangle0;
        newroot->children_ids[1] = L20->id;
        newroot->mbr[1] = L2_rectangle0;
        // 
        int nd_size = 8 + maxCap*4 + maxCap*((2*dim)*4);
        int nodeCap_perPage = (PAGE_SIZE / nd_size);
        for (int i = 0; i < 2; i++){
            // directly modifying pid's of children
            int cid = newroot->children_ids[i];
            int page_num = cid/nodeCap_perPage;
            PageHandler ph = ptr_fh->PageAt(page_num);
            char *data = ph.GetData ();
            int pbase_offset = (cid % nodeCap_perPage)*nd_size;
            int page_offset = pbase_offset + 4;
            memcpy (&data[page_offset], &(newroot->id), sizeof(int));
            ptr_fh->MarkDirty(page_num);
            ptr_fh->UnpinPage(page_num);
            ptr_fh->FlushPage(page_num);
        }
        load_node(ptr_fh,newroot,dim,maxCap);
        // print_node(newroot,maxCap,dim);
        free_mem(newroot);
        return;
    }
    node* pnode = get_node(ptr_fh, L10->pid, dim, maxCap);
    // print_node(pnode,maxCap,dim);
    int capacity = 0;
    for (int i = 0; i < maxCap; i++){
        if(pnode->children_ids[i] == L10->id){   //loading correct data of L1 to pnode
            pnode->mbr[i] = L1_rectangle0;
        }
        else if(pnode->children_ids[i] == -2) break;
        capacity += 1;
    }
    if(capacity < maxCap){
        // Inserting L20 info in pnode
        int L2_index;
        for (int i = 0; i < maxCap; i++){
            if(pnode->children_ids[i] == -2){
                pnode->children_ids[i] = L20->id;
                L2_index = i;
                break;
            }
        }
        pnode->mbr[L2_index] = L2_rectangle0;
        load_node(ptr_fh,pnode,dim,maxCap);
        free_mem(pnode);
        return;
    }else{
        int newid = (++*lastid_ptr);
        pair<int,int> p = get_bigest_rectangle(pnode->mbr,maxCap,dim,L2_rectangle0);
        // cout<<"Node of id <"<<pnode->id<<"> to be split across child number "<<p.first<<" , "<<p.second<<endl;
        // cout<<"----"<<endl;
        // cout<<"|"<<endl;
        // initializing new node L1 and L2 ( L1 will be replacement of pnode)
        node* L1 = new node(maxCap, dim, pnode->id, pnode->pid, pnode->mbr[p.first]);
        L1->children_ids[0] = pnode->children_ids[p.first];
        node* L2;
        if(p.second == maxCap){
            L2 = new node(maxCap, dim,  newid, pnode->pid, L2_rectangle0);
            L2->children_ids[0] = L20->id;
        }
        else{
            L2 = new node(maxCap, dim,  newid, pnode->pid, pnode->mbr[p.second]);
            L2->children_ids[0] = pnode->children_ids[p.second];
        }

        // initializing empty rectangle for L1 and L2 complete span.
        int* L1_rectangle = new int[2*dim];
        int* L2_rectangle = new int[2*dim];

        // spliting up node in L1 and L2 using quadratic split.
        splitNode_ij(pnode, L1, L2, L1_rectangle, L2_rectangle, L2_rectangle0, L20->id, dim, maxCap, p.first, p.second);

        // updating pid's of all children which has changed now to L2.
        int nd_size = 8 + maxCap*4 + maxCap*((2*dim)*4);
        int nodeCap_perPage = (PAGE_SIZE / nd_size);
        for (int i = 0; i < maxCap; i++){
            int cid = L2->children_ids[i];
            if(cid == -2) break;
            // directly modifying pid's of children
            int page_num = cid/nodeCap_perPage;
            PageHandler ph = ptr_fh->PageAt(page_num);
            char *data = ph.GetData ();
            int pbase_offset = (cid % nodeCap_perPage)*nd_size;
            int page_offset = pbase_offset + 4;
            memcpy (&data[page_offset], &(L2->id), sizeof(int));
            ptr_fh->MarkDirty(page_num);
            ptr_fh->UnpinPage(page_num);
            ptr_fh->FlushPage(page_num);
        }
        // updating all the way up to a stable node.
        load_node(ptr_fh,L1,dim,maxCap);
        load_node(ptr_fh,L2,dim,maxCap);

        update_upward(ptr_fh, L1, L2, L1_rectangle, L2_rectangle, dim, maxCap, lastid_ptr, rootid_ptr);

        // overwrite working_node by L1
        // cout<<"|"<<endl;
        // cout<<"----"<<endl;
        // cout<<"finishes spliting above id <"<<L1->id<<"> with creation of new child of id <"<<L2->id<<">"<<endl;
        // print_page(ptr_fh,L1->id,dim,maxCap);
        // print_page(ptr_fh,L2->id,dim,maxCap);

        // freeing up memory all allocated memory
        free_mem(L1);
        free_mem(L2);
        delete L1_rectangle;
        delete L2_rectangle;
        return;
    }
}