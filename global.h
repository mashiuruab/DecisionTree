#ifndef GLOBAL_H
#define GLOBAL_H

#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdio>
#include <unordered_set>

using namespace std;

//values that Column::data_type can take
const int ELEM_BOOL = 1; //binary
const int ELEM_SHORT = 2;
const int ELEM_INT = 3;
const int ELEM_FLOAT = 4;
const int ELEM_DOUBLE = 5;
const int ELEM_CHAR = 6;
const int ELEM_STRING = 7;

// constants related to impurity functions
const int IMPURITY_ENTROPY = 8;
const int IMPURITY_GINI = 9;
const int IMPURITY_CLASSIFICATION_ERROR = 10;
const int IMPURITY_VARIANCE = 11;

int IMPURITY_FUNC = -1;
int MAX_TREE_DEPTH = INT_MAX;
int BRUTE_FORCE_MAX_ITEM = INT_MAX;

#endif
