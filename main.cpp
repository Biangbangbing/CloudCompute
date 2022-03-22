#include<string.h>
#include<iostream>
#include<fstream>
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "sudoku.h"
#include "sudoku_basic.cc"
#include "neighbor.cc"
using namespace std;
void get_puzzle() {
    int total = 0;
    //open the file
    ifstream ifs;
    bool (*solve)(int) = solve_sudoku_basic;
    ifs.open("input",ios::in);
    if(!ifs.is_open()) {
        cout << "read fail." << endl;
    }
    //get input from the input file("input")
    string buf;
    while(getline(ifs,buf)) {
        //get the name of file and output the txt in the file
        ifstream ifs1;
        FILE* fp = fopen(buf.substr(2).data(),"r");
        char* puzzle = new char[128];
        while(fgets(puzzle,sizeof puzzle,fp)!=NULL) {
            if(strlen(puzzle) >= N) {
                ++total;
                input(puzzle);
                init_cache();
            if (solve(0)) {
                if (!solved())
                    assert(0);
                cout << puzzle << endl;
            }
            else {
                printf("No: %s", puzzle);
                }
            }
        }
    }
    
    
}

int main() {
    get_puzzle();
}
