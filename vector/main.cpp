#include "vector.hpp"
#include <vector>
#include <iostream>
using namespace sjtu;
// using namespace std;
using std::cin;
using std::cout;
using std::endl;

int main() {
    printf("haha\n");
    vector <int> a;
    for (int i = 0; i < 100; ++i)
        a.push_back(i);

    for (auto it = a.begin(); it != a.end(); ++it) {
        cout << *it << ' ';
    }
    a.pop_back();
    cout << endl << a.size() << endl;
    
}