#include <iostream>
#include <fstream>
#include "Constraint.h"
#include "DissatRate.h"
using namespace std;

int main(int argc,char* argv[]){
    // string filename = "../testcases/sample.txt";
    if(argc == 1){
        cout<<"No input file!"<<endl;
        return 0;
    }

    ifstream infile(argv[1],ios::in);
    if(!infile){
        cout<<"Failed to open the file: "<<argv[1]<<endl;
        return 0;
    }
    string line;

    vector<Constraint> constraints;
    
    while(getline(infile,line)){
        constraints.push_back(Constraint(line));
    }
    /*
    for(auto cons : constraints)
        cout<< cons<<endl;
    */

    DissatRateCalculator calculator(constraints);
    RACOSOptimizer racos(calculator);
    cout<<racos.optimize()<<endl;
    auto sol = racos.getSolution();
    cout<<sol<<endl;
    return 0;
}