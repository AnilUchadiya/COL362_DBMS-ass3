#include <unistd.h>
#include <iostream>
#include "buffer_manager.h"
#include "file_manager.h"
#include "rtree.h"
#include "errors.h"
#include <algorithm>
#include <cstdio>
#include <fcntl.h>
#include <cstring>
#include <cstdlib>
#include <bits/stdc++.h>

using namespace std;

int fin_getval(ifstream* fin){
    string num_str;
    *fin>>num_str;
    return stoi(num_str);
}
pair<int,int> get_farPoints(int** points,int maxCap,int dim,int newPoint[]){
    int max_distance = INT32_MIN;
    int I,J;
    for (int i = 0; i < maxCap; i++){
        for (int j = i+1; j < maxCap; j++){
            int squr_distance = 0;
            for (int k = 0; k < dim; k++){
                squr_distance += (points[i][2*k] - points[j][2*k])*(points[i][2*k] - points[j][2*k]);
            }
            if(max_distance <= squr_distance){
                max_distance = squr_distance;
                I = i; J = j;
            }
        }
        // for new entry
        int squr_distance = 0;
        for (int k = 0; k < dim; k++){
            squr_distance += (points[i][2*k] - newPoint[2*k])*(points[i][2*k] - newPoint[2*k]);
        }
        if(max_distance <= squr_distance){
            max_distance = squr_distance;
            I = i; J = maxCap;
        }
    }
    pair<int,int> p(I,J);
    return p;
}

void print_tree(FileHandler *fh, int nodeid, int dim, int maxCap){
    print_page(fh,nodeid,dim,maxCap);
    node* printing_node = get_node(fh, nodeid, dim, maxCap);
    bool isleaf = (printing_node->children_ids[0] == -1);
    if(!isleaf){
        for (int i = 0; i < maxCap; i++){
            if(printing_node->children_ids[i] == -2)break;
            print_tree(fh,printing_node->children_ids[i],dim,maxCap);
        }
    }
    return;
}

bool pointQuery(FileHandler *fh, int* point, int nodeid, int dim, int maxCap){
    node* search_node = get_node(fh, nodeid, dim, maxCap);
    bool isleaf = (search_node->children_ids[0] == -1);
    if(isleaf){
        // cout<<" leaf id >> "<<search_node->id<<endl;
        for (int i = 0; i < maxCap; i++){
            if(search_node->children_ids[i] == -2) return false;
            int* point_rectangle = search_node->mbr[i];
            bool flag = true;
            for (int j = 0; j < dim; j++){
                if(point_rectangle[2*j] != point[2*j]){
                    flag = false;
                    break;
                }
            }
            if(flag)
                return true;
        }
        return false;
    }
    for (int i = 0; i < maxCap; i++){
        if(search_node->children_ids[i] == -2) return false;
        int* rectangle = search_node->mbr[i];
        bool ispersent = true;
        for (int j = 0; j < dim; j++){
            bool f = (rectangle[2*j] <= point[2*j]) && (point[2*j+1] <= rectangle[2*j+1]);
            if(!f){
                ispersent = false;
                break;
            }
        }
        if(ispersent){
            // cout<<"id -> "<<search_node->children_ids[i]<<endl;
            bool ret_flag = pointQuery(fh, point, search_node->children_ids[i], dim, maxCap);
            if(ret_flag) return true;
            // else{cout<<search_node->children_ids[i]<<" not found"<<endl;}
        }
    }
    return false;
}

int main(int argc, char *argv[])
{   
    // cout<<"In main"<<endl;
    ifstream fin;
    ofstream fout;
    try{
        fin.open(argv[1]); //input file
        fout.open(argv[4]);//output file
    }
    catch(const std::exception& e){
        cout<<"error while opening file"<<endl;
        std::cerr << e.what() << '\n';
    }
    ifstream fin_ans;
    // fin_ans.open("./Testcases/TC_7/answer_2_10_100_100_insert.txt");
    cout<<"file loaded"<<endl;
    // model parameter
    int dim = stoi(argv[3]);
    int maxCap = stoi(argv[2]);
    // local std variable
    int root_id = 0;
    int nd_size = 8 + maxCap*4 + maxCap*((2*dim)*4);
    int nodeCap_perPage = (PAGE_SIZE / nd_size);
    FileManager fm;
    FileHandler fh = fm.CreateFile("./rtree.txt");
    cout<<"file handler has been set"<<endl;
    // local incremantel varible
    int last_nid = -1;
    // working
    string instr;
    int instr_counter = 1;
    // string ans;
    while(fin >> instr){
        if(instr == "BULKLOAD"){
            string val;
            fin>>val;
            fin>>val;
            cout<<"BULKLOAD"<<endl<<endl;
            // fout << "BULKLOAD\n\n\n";
            continue;
        }
        // cout<<"============================================================================================================================================"<<endl;
        // if(instr == "QUERY") cout<<"#"<<instr_counter<<" "<<instr<<" :> ";
        instr_counter++;
        int newEntryPoint[2*dim];   //newEntryPoint : coordinate array
        for(int i=0;i<dim;i++){
            string val;
            fin >> val;
            // if(instr == "QUERY") cout<<val<<" ";
            newEntryPoint[2*i] = stoi(val);
            newEntryPoint[2*i+1] = stoi(val);
        }
        // cout<<endl;
        if(instr == "INSERT"){
            // fin_ans>>ans;
            // THE FIRST NODE
            if(last_nid < 0){
                node* root_node = new node(maxCap,dim,root_id,-2,newEntryPoint);
                PageHandler ph = fh.NewPage();
                load_node(&fh,root_node,dim,maxCap);
                // print_page(&fh,0,dim,maxCap);
                free_mem(root_node); // free new allocated memory
                last_nid = root_id;
                // cout<<"root formed"<<endl;
            }else{
                int current_nodeId = root_id;
                while(true){
                    // LOADING NODE
                    int page_num = current_nodeId/nodeCap_perPage;
                    int page_offset = (current_nodeId % nodeCap_perPage)*nd_size;
                    node* working_node = get_node(&fh, current_nodeId, dim, maxCap);
                    // cout<<"working node fatched id = "<<working_node->id<<endl;
                    // print_node(working_node,maxCap,dim);
                    // print_page(&fh,working_node->id,dim,maxCap);
                    bool isleaf = (working_node->children_ids[0] == -1);
                    int filled_portion = 0;
                    for (int i = 0; i < maxCap; i++){
                        if(working_node->children_ids[i] == -2) break;
                        filled_portion += 1;
                    }
                    // cout<<"filled_portion : "<<filled_portion<<endl;
                    // break;
                    if(isleaf && filled_portion < maxCap){//----------------------------------------------------------------------------------------------// NODE IS LEAF with filled_portion
                        for (int i = 0; i < maxCap; i++){
                            if(working_node->children_ids[i] == -2){
                                working_node->children_ids[i] = -1;
                                for (int j = 0; j < dim; j++){
                                    working_node->mbr[i][2*j] = newEntryPoint[2*j];
                                    working_node->mbr[i][2*j+1] = newEntryPoint[2*j+1];
                                }
                                break;
                            }
                        }
                        load_node(&fh,working_node,dim,maxCap);
                        // print_page(&fh,working_node->id,dim,maxCap);
                        update_back2root(&fh, working_node, dim, maxCap);
                        // cout<<"Working node modified "<<endl;
                        // print_page(&fh,working_node->id,dim,maxCap);
                        free_mem(working_node);
                        break;
                    }else if(isleaf && filled_portion == maxCap){ //------------------------------------------------------------------// NODE IS LEAF without filled_portion
                        int newid = (++last_nid); //post increment.

                        // getting furthest points
                        pair<int,int> p = get_bigest_rectangle(working_node->mbr,maxCap,dim,newEntryPoint);
                        // cout<<"leaf Node of id <"<<working_node->id<<"> to be split across child index number "<<p.first<<" , "<<p.second<<endl;
                        // cout<<"----"<<endl;
                        // cout<<"|"<<endl;
                        // initialized L1 and L2 node with two entries returned by get_farPoints.
                        node* L1 = new node(maxCap, dim, working_node->id, working_node->pid, working_node->mbr[p.first]);
                        node* L2;
                        if(p.second == maxCap){
                            L2 = new node(maxCap, dim,  newid, working_node->pid, newEntryPoint);
                        }else{
                            L2 = new node(maxCap, dim,  newid, working_node->pid, working_node->mbr[p.second]);
                        }

                        // creating empty rectangle for L1 and L2
                        int* L1_rectangle = new int[2*dim];
                        int* L2_rectangle = new int[2*dim];

                        // spliting all points between L1 and L2 
                        splitNode_ij( working_node, L1, L2, L1_rectangle, L2_rectangle, newEntryPoint, -1, dim, maxCap, p.first, p.second);
                        // overwrite working_node by L1
                        load_node(&fh,L1,dim,maxCap);
                        // insert L2 to memory
                        load_node(&fh,L2,dim,maxCap);

                        // update upward
                        update_upward(&fh, L1, L2, L1_rectangle, L2_rectangle, dim, maxCap, &last_nid, &root_id);
                        // cout<<"|"<<endl;
                        // cout<<"----"<<endl;
                        // cout<<"finishes spliting above leaf node id <"<<L1->id<<"> with || child creation of id <"<<L2->id<<">"<<endl;

                        // print_page(&fh,L1->id,dim,maxCap);
                        // print_page(&fh,L2->id,dim,maxCap);

                        // free used memory
                        free_mem(L1);
                        free_mem(L2);
                        delete L1_rectangle;
                        delete L2_rectangle;
                        break;
                    }else{//----------------------------------------------------------------------------------------------------// INTERNAL NODE
                        int bestfit = 0;
                        double minAreaEnlargment = INT32_MAX;
                        double bestfit_area = 1;
                        for (int i = 0; i < maxCap; i++){
                            if(working_node->children_ids[i] == -2) break;
                            int* rectangle = working_node->mbr[i];
                            double new_area = 1;
                            double initial_area = 1;
                            for (int j = 0; j < dim; j++){
                                initial_area =(rectangle[2*j+1] == rectangle[2*j])? initial_area:initial_area*(rectangle[2*j+1]-rectangle[2*j]);
                                int minx = min(min(rectangle[2*j],rectangle[2*j+1]) ,min(newEntryPoint[2*j],newEntryPoint[2*j+1]));
                                int maxx = max(max(rectangle[2*j],rectangle[2*j+1]) ,max(newEntryPoint[2*j],newEntryPoint[2*j+1]));
                                new_area = new_area*(maxx-minx);
                            }
                            double area_enlarge = new_area - initial_area;
                            // cout<<i<<" initial area = "<<initial_area<<" area enlarge = "<<area_enlarge<<endl;
                            if(area_enlarge < minAreaEnlargment){
                                minAreaEnlargment = area_enlarge;
                                bestfit = i;
                                bestfit_area = initial_area;
                            }else if(area_enlarge == minAreaEnlargment){
                                if(bestfit_area > initial_area){
                                    minAreaEnlargment = area_enlarge;
                                    bestfit = i;
                                    bestfit_area = initial_area;
                                }
                            }
                        }
                        // cout<<"best fit node id : "<<bestfit<<endl;
                        current_nodeId = working_node->children_ids[bestfit];
                    }
                }
            }
            // cout<<"INSERT complete. node count "<<last_nid+1<<endl;
            // cout<<"current root >>"<<endl;
            // print_page(&fh,root_id,dim,maxCap);
            // cout<<"tree : "<<endl;
            // print_tree(&fh,root_id,dim,maxCap);
            fout << "INSERT\n\n\n";
        }
        else if(instr == "QUERY"){
            // fin_ans>>ans;
            if(pointQuery(&fh, newEntryPoint, root_id, dim, maxCap)){ 
                fout << "TRUE\n\n\n";
                // cout<<"TRUE ";
                // if(ans == "TRUE") cout<<" (Passed)"<<endl;
                // else cout<<" (Failed)"<<endl;
            }
            else{
                fout << "FALSE\n\n\n";
                // cout<<"FALSE ";
                // if(ans == "FALSE") cout<<" (Passed)"<<endl;
                // else cout<<" (Failed)"<<endl;
            }
        }
    }
    // print_tree(&fh,root_id,dim,maxCap);
    // cout<<"total node created "<<last_nid+1<<endl;
    // cout<<" total instruction "<<instr_counter<<endl;
    // fm.PrintBuffer();
    fm.DestroyFile ("./rtree.txt");
    return 0;
}
