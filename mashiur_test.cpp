#include <iostream>
#include <vector>
#include <map>

using namespace std;

void std_copy() {
    vector<int> num1 = {1,2,3,4,5};
    vector<int> num2 = {6,7,8,9,10};
    vector<int> num3 = {11,12,13,14,15};

    vector<int> result;
    result.resize(15);

    std::copy(num1.begin(), num1.end(), result.begin());

    /*for(size_t i = 0; i < 5; i++) {
        cout << result[i] << " ";
    }

    cout << endl;*/

    int offset = num1.end() - num1.begin();

    //cout << "first offset = " << offset << endl;

    std::copy(num2.begin(), num2.end(), result.begin() + offset);

    /*for(size_t i = 5; i < 10; i++) {
        cout << result[i] << " ";
    }

    cout << endl;*/

    offset += (num2.end() - num2.begin());

    //cout << "second offset = " << offset << endl;

    std::copy(num3.begin(), num3.end(), result.begin() + offset);


    /*for(size_t i = 10; i < result.size(); i++) {
        cout << result[i] << " ";
    }

    cout << endl;

    cout << "size found = " << result.size() << endl;*/

    for(size_t i = 0; i < 15; i++) {
        cout << result[i] << " ";
    }

    cout<<endl;
}

vector<vector<string>> result;

void do_compute(vector<string> &items, int curr_idx, int k, vector<string> temp_result) {
    temp_result.push_back(items[curr_idx]);

    if(temp_result.size() == k) {
        result.push_back(temp_result);
        return;
    }

    for(int i = curr_idx + 1; i < items.size(); i++) {
        do_compute(items, i, k, temp_result);
    }
}

vector<vector<int>> result_int;

void do_compute(vector<int> &items, int curr_idx, int k, vector<int> temp_result) {
    temp_result.push_back(items[curr_idx]);

    if(temp_result.size() == k) {
        result_int.push_back(temp_result);
        return;
    }

    for(int i = curr_idx + 1; i < items.size(); i++) {
        do_compute(items, i, k, temp_result);
    }
}

void do_compute2(int N, int curr_idx, int k, vector<int> temp_result) {
    temp_result.push_back(curr_idx);

    if(temp_result.size() == k) {
        result_int.push_back(temp_result);
        return;
    }

    for(int i = curr_idx + 1; i < N; i++) {
        do_compute2(N, i, k, temp_result);
    }
}

void combination_str() {
    vector<string> items = {"A", "B", "C", "D"};
    //vector<string> items = {"AL", "BK", "CJ", "DI"};
    //vector<string> items = {"A", "B", "C", "D", "E"};
    //vector<string> items = {"A", "B", "C", "D", "E", "F"};

    cout<< "Items to enumerate are  : ";

    for(int i = 0; i < items.size(); i++) {
        cout<< items[i] << ", ";
    }

    cout<<endl;

    int k_range = items.size() / 2; // k = 1 .. floor(n/2), n is odd (no duplicate), n even (duplicate)

    for(int k = 1; k <= k_range; k++) {
        for(int i = 0; i < items.size(); i++) {
            vector<string> temp_result;
            do_compute(items, i,  k, temp_result);
        }
    }

    cout << "results string " << endl;

    for(int i = 0; i < result.size(); i++) {
        map<string, int> k_map;
        for(int j = 0; j < result[i].size(); j++) {
            cout<< result[i][j] << ", ";
            k_map[result[i][j]] = 1;
        }

        cout<< "\t";
        bool flag = true;

        if((items.size() % 2 == 0) && (result[i].size() == (items.size() / 2))) {
            flag = false;
        }

        for(int c = 0; c < items.size(); c++) {
            if(k_map.find(items[c]) == k_map.end()) {
                if(result[i][0] > items[c] && !flag) { //
                    flag = true;
                    cout<< "this values can be avoided " << " ";
                }

                cout << items[c] << ", ";
            }
        }

        cout<<endl;
    }
}

void combination_int() {
    vector<int> items = {0,1,2,3};
    //vector<int> items = {0,1,2,3,4};
    //vector<int> items = {0,1,2,3,4,5};

    cout<< "Items to enumerate are  : ";

    for(int i = 0; i < items.size(); i++) {
        cout<< items[i] << ", ";
    }

    cout<<endl;

    int k_range = items.size() / 2; // k = 1 .. floor(n/2), n is odd (no duplicate), n even (duplicate)

    for(int k = 1; k <= k_range; k++) {
        for(int i = 0; i < items.size(); i++) {
            vector<int> temp_result;
            do_compute(items, i,  k, temp_result);
        }
    }

    cout << "results int " << endl;

    for(int i = 0; i < result_int.size(); i++) {
        map<int, int> k_map;
        for(int j = 0; j < result_int[i].size(); j++) {
            cout<< result_int[i][j] << ", ";
            k_map[result_int[i][j]] = 1;
        }

        cout<< "\t";
        bool flag = true;

        if((items.size() % 2 == 0) && (result_int[i].size() == (items.size() / 2))) {
            flag = false;
        }

        for(int c = 0; c < items.size(); c++) {
            if(k_map.find(items[c]) == k_map.end()) {
                if(result_int[i][0] > items[c] && !flag) { //
                    flag = true;
                    cout<< "this values can be avoided " << " ";
                }

                cout << items[c] << ", ";
            }
        }

        cout<<endl;
    }
}

void combination_int2() {
    int N = 6;
    //vector<int> items = {0,1,2,3,4};
    //vector<int> items = {0,1,2,3,4,5};

    cout<< "Items to enumerate are  : ";

    for(int i = 0; i < N; i++) {
        cout<< i << ", ";
    }

    cout<<endl;

    int k_range = N / 2; // k = 1 .. floor(n/2), n is odd (no duplicate), n even (duplicate)

    for(int k = 1; k <= k_range; k++) {
        if((N % 2 == 0) && (k == k_range)) {
            vector<int> temp_result;
            do_compute2(N, 0, k, temp_result);
        } else {
            for(int i = 0; i < N; i++) {
                vector<int> temp_result;
                do_compute2(N, i,  k, temp_result);
            }
        }
    }

    cout << "results int " << endl;

    for(int i = 0; i < result_int.size(); i++) {
        map<int, int> k_map;
        for(int j = 0; j < result_int[i].size(); j++) {
            cout<< result_int[i][j] << ", ";
            k_map[result_int[i][j]] = 1;
        }

        cout<< "\t";
        bool flag = true;

        if((N % 2 == 0) && (result_int[i].size() == (N / 2))) {
            flag = false;
        }

        for(int c = 0; c < N; c++) {
            if(k_map.find(c) == k_map.end()) {
                if(result_int[i][0] > c && !flag) { //
                    flag = true;
                    cout<< "this values can be avoided " << " ";
                }

                cout << c << ", ";
            }
        }

        cout<<endl;
    }
}

bool str_cmp(string s1, string s2) {
    bool isGreater = s1 > s2;

    if(isGreater) {
        cout<< s1 << " is greater than " << s2 << endl;
    } else {
        cout<< s1 << " is less than or equal " << s2 << endl;
    }
}

int main(int argc, char** argv) {
    /*result_int.clear();
    combination_int2();*/

    /*result.clear();
    combination_str();*/

    string s1 = "D4";
    string s2 = "D5";

    str_cmp(s1, s2);

    return 0;
}