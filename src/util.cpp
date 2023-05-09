#include<vector>
#include<iostream>
#include<string>
using namespace std;

template<typename T>
void print_vector(vector<T>& v)
{
    for(auto i : v)
    {
        cout<<i<<" ";
    }
    cout<<endl;
}

void print_info(string func,string msg)
{
    printf("In func %s, msg: %s",func.c_str(),msg.c_str());
}