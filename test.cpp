#include <iostream>
#include <climits>
#include <chrono>
#include <algorithm>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include "matrix.h"
#include "csv.h"
#include "criterion.h"
#include "splitter.h"
#include "tree.h"

using namespace std;

/*
//====== testing dense matrix printing
int main(int argc, char** argv){
	//--- create matrix:
	//1, a, 1.1
	//2, b, 2.2
	//3, c, 3.3
	//assume all columns are ordinal
	//in actual implementation later
	//- one can read metadata and data from a CSV file
	//- one can also export columns (even in binary format) for external-memory streaming
    Matrix mat;
    //first column:
    Column * col1 = new DenseColumn<int>(ELEM_INT, true);
    int temp1 = 1;
    col1->append(0, &temp1);
    temp1 = 2;
	col1->append(1, &temp1);
	temp1 = 3;
	col1->append(2, &temp1);
	//second column:
    Column * col2 = new DenseColumn<char>(ELEM_CHAR, true);
	char temp2 = 'a';
	col2->append(0, &temp2);
	temp2 = 'b';
	col2->append(1, &temp2);
	temp2 = 'c';
	col2->append(2, &temp2);
	//third column:
    Column * col3 = new DenseColumn<float>(ELEM_FLOAT, true);
	float temp3 = 1.1;
	col3->append(0, &temp3);
	temp3 = 2.2;
	col3->append(1, &temp3);
	temp3 = 3.3;
	col3->append(2, &temp3);

    Matrix matrix;
    matrix.append(col1);
    matrix.append(col2);
    matrix.append(col3);
	cout<< "matrix printing: " << endl;
    matrix.print();

    return 0;
}
//*/

/*
//====== testing sparse matrix printing
int main(int argc, char** argv){
	//--- create matrix:
	//1, '0', 0.0
	//0, b, 0.0
	//0, '0', 3.3
	//assume all columns are ordinal
	//in actual implementation later
	//- one can read metadata and data from a CSV file
	//- one can also export columns (even in binary format) for external-memory streaming
	//first column:

	Column * col1 = new SparseColumn<int>(ELEM_INT, true, 0);
    int temp1 = 1;
    col1->append(0, &temp1);
    col1->finish(3);
    //second column:
    Column * col2 = new SparseColumn<char>(ELEM_CHAR, true, '0');
    char temp2 = 'b';
    col2->append(1, &temp2);
    col2->finish(3);
    //third column:
    Column * col3 = new SparseColumn<float>(ELEM_FLOAT, true, 0.0);
    float temp3 = 3.3;
    col3->append(2, &temp3);
    col3->finish(3);

    Matrix matrix;
    matrix.append(col1);
    matrix.append(col2);
    matrix.append(col3);
	cout<< "matrix printing: " << endl;
    matrix.print();

    return 0;
}
//*/

/*
// impurity, variance calculation related test code
int main(int argc, char** argv) {
    //ref: http://people.revoledu.com/kardi/tutorial/DecisionTree/how-to-measure-impurity.htm    
    cout<< "----------------------"<<endl;
    cout<< "Test with Dense Column:" <<endl;
    Column* target_column = new DenseColumn<string>(ELEM_STRING, false);
    string str = "Bus";
    target_column->append(0, &str);
    target_column->append(1, &str);
    target_column->append(2, &str);
    target_column->append(3, &str);
    
    str = "Car";
    target_column->append(4, &str);
    target_column->append(5, &str);
    target_column->append(6, &str);
    
    str = "Train";
    target_column->append(7, &str);
    target_column->append(8, &str);
    target_column->append(9, &str);

    vector<size_t> indices;

    for(size_t i = 0; i < 10; i++){
        indices.push_back(i);
    }
    
    cout<< "entropy = " << classification_impurity(indices.begin(), indices.end(),
                                     target_column, entropy) << endl;
    cout<< "gini = " << classification_impurity(indices.begin(), indices.end(),
                                  target_column, gini) << endl;
    cout<< "classification error = " << classification_impurity(indices.begin(), indices.end(),
                                                  target_column, classification_error) << endl;
    cout<< "----------------------"<<endl<<endl;
    
    cout<< "----------------------"<<endl;
    cout<< "Test with Sparse Column:" <<endl;
    Column* sparse_column = new SparseColumn<string>(ELEM_STRING, false, "Bus");
    str = "Car";
    sparse_column->append(4, &str);
    sparse_column->append(5, &str);
    sparse_column->append(6, &str);
    
    str = "Train";
    sparse_column->append(7, &str);
    sparse_column->append(8, &str);
    sparse_column->append(9, &str);
    
    sparse_column->finish(10);// if it is set to number of non_default element then dense and sparse result should be same
    
    cout<< "entropy = " << classification_impurity(indices.begin(), indices.end(),
                                     sparse_column, entropy) << endl;
    cout<< "gini = " << classification_impurity(indices.begin(), indices.end(),
                                                 sparse_column, gini) << endl;
    cout<< "classification error = " << classification_impurity(indices.begin(), indices.end(),
                                                  sparse_column, classification_error) << endl;
    cout<< "----------------------"<<endl<<endl;
    
    //Ref: https://www.mathsisfun.com/data/standard-deviation.html
    cout<< "----------------------"<<endl;
    cout<< "Dense Column Variance Calculation:" <<endl;
    
    Column* dense_variance = new DenseColumn<int>(ELEM_INT, true);
    int value = 600;
    dense_variance->append(0, &value);
    value = 470;
    dense_variance->append(1, &value);
    value = 170;
    dense_variance->append(2, &value);
    value = 430;
    dense_variance->append(3, &value);
    value = 300;
    dense_variance->append(4, &value);

    indices.clear();

    for(size_t i = 0;i < 5; i++) {
        indices.push_back(i);
    }
    
    cout<< "variance = " << var(indices.begin(), indices.end(),
                                                dense_variance)<< endl;

    cout<< "----------------------"<<endl<<endl;
    
    cout<< "----------------------"<<endl;
    cout<< "Sparse Column Variance Calculation:" <<endl;
    
    Column* sparse_variance = new SparseColumn<int>(ELEM_INT, true, 600);
    value = 470;
    sparse_variance->append(1, &value);
    value = 170;
    sparse_variance->append(2, &value);
    value = 430;
    sparse_variance->append(3, &value);
    value = 300;
    sparse_variance->append(4, &value);
    sparse_variance->finish(5);
    
    cout<< "variance = " << var(indices.begin(), indices.end(),
                                sparse_variance)<< endl;
    cout<< "----------------------"<<endl<<endl;
    
    return 0;
}
//*/

/*
// impurity calculation related test code
int main(int argc, char** argv) {
    //ref: http://people.revoledu.com/kardi/tutorial/DecisionTree/how-to-measure-impurity.htm
    cout<< "----------------------"<<endl;
    cout<< "Test with Dense Column:" <<endl;
    Column* target_column = new DenseColumn<string>(ELEM_STRING, false);
    string str = "Bus";
    target_column->append(0, &str);
    target_column->append(1, &str);
    target_column->append(2, &str);
    target_column->append(3, &str);

    str = "Car";
    target_column->append(4, &str);
    target_column->append(5, &str);
    target_column->append(6, &str);

    str = "Train";
    target_column->append(7, &str);
    target_column->append(8, &str);
    target_column->append(9, &str);

    str = "Bus";
    target_column->append(10, &str);
    target_column->append(11, &str);
    target_column->append(12, &str);
    target_column->append(13, &str);


    str = "Train";
    target_column->append(14, &str);
    target_column->append(15, &str);
    target_column->append(16, &str);

    vector<size_t> indices;

    for(size_t i = 0; i < 17; i++){
        indices.push_back(i);
    }

    cout<< "entropy = " << classification_impurity(indices.begin() + 4, indices.end() - 3,
                                     target_column, entropy) << endl;
    cout<< "gini = " << classification_impurity(indices.begin() + 4, indices.end() - 3,
                                  target_column, gini) << endl;
    cout<< "classification error = " << classification_impurity(indices.begin() + 4, indices.end() - 3,
                                                  target_column, classification_error) << endl;
    cout<< "----------------------"<<endl<<endl;

    cout<< "----------------------"<<endl;
    cout<< "Test with Sparse Column:" <<endl;
    Column* sparse_column = new SparseColumn<string>(ELEM_STRING, false, "Bus");
    str = "Car";
    sparse_column->append(4, &str);
    sparse_column->append(5, &str);
    sparse_column->append(6, &str);

    str = "Train";
    sparse_column->append(7, &str);
    sparse_column->append(8, &str);
    sparse_column->append(9, &str);

    str = "Train";
    sparse_column->append(14, &str);
    sparse_column->append(15, &str);
    sparse_column->append(16, &str);

    sparse_column->finish(17);// if it is set to number of non_default element then dense and sparse result should be same

    cout<< "entropy = " << classification_impurity(indices.begin() + 4, indices.end() - 3,
                                     sparse_column, entropy) << endl;
    cout<< "gini = " << classification_impurity(indices.begin() + 4, indices.end() - 3,
                                  sparse_column, gini) << endl;
    cout<< "classification error = " << classification_impurity(indices.begin() + 4, indices.end() - 3,
                                                  sparse_column, classification_error) << endl;
    cout<< "----------------------"<<endl<<endl;

    //Ref: https://www.mathsisfun.com/data/standard-deviation.html
    cout<< "----------------------"<<endl;
    cout<< "Dense Column Variance Calculation:" <<endl;

    Column* dense_variance = new DenseColumn<int>(ELEM_INT, true);
    int value = 600;
    dense_variance->append(0, &value);
    value = 470;
    dense_variance->append(1, &value);
    value = 170;
    dense_variance->append(2, &value);
    value = 430;
    dense_variance->append(3, &value);
    value = 300;
    dense_variance->append(4, &value);
    value = 600;
    dense_variance->append(5, &value);

    value = 300;
    dense_variance->append(6, &value);

    indices.clear();

    for(size_t i = 0;i < 7; i++) {
        indices.push_back(i);
    }

    cout<< "variance = " << var(indices.begin() + 1, indices.end() - 1,
                                dense_variance)<< endl;

    cout<< "----------------------"<<endl<<endl;

    cout<< "----------------------"<<endl;
    cout<< "Sparse Column Variance Calculation:" <<endl;

    Column* sparse_variance = new SparseColumn<int>(ELEM_INT, true, 600);
    value = 470;
    sparse_variance->append(1, &value);
    value = 170;
    sparse_variance->append(2, &value);
    value = 430;
    sparse_variance->append(3, &value);
    value = 300;
    sparse_variance->append(4, &value);
    sparse_variance->finish(7);

    cout<< "variance = " << var(indices.begin() + 1, indices.end() - 1,
                                sparse_variance)<< endl;
    cout<< "----------------------"<<endl<<endl;

    return 0;
}
//*/

/*
// integer column sorting
int main(int argc, char** argv) {
    vector<size_t> elements; //Xi
    vector<size_t> indices; //idx array

    int num = 6;

    for(size_t i = 0; i < num; i++) {
        elements.push_back(i);
        indices.push_back(i);
    }

    std::random_shuffle(elements.begin(), elements.end());
    Column* column = new DenseColumn<int>(ELEM_INT, true);
    for(int i = 0; i < num; i++) {
        column->append(i, &elements[i]);
    }

    cout<<"idx\tXi"<<endl;
    for(size_t i = 0; i < num; i++) cout << indices[i] << "\t" << elements[i] <<endl;

    cout<<"sorting ..."<<endl<<endl;
    column->row_sort(indices.begin(), indices.end());

    cout<<"idx\tXi"<<endl;
    for(size_t i = 0; i < num; i++) cout << indices[i] << "\t" << elements[indices[i]] <<endl;

    size_t mid = num / 2;

    cout<<"reinit ..."<<endl<<endl;

    indices.clear();
    for(size_t i = 0; i < num; i++) indices.push_back(i);

    cout<<"idx\tXi"<<endl;
    for(size_t i = 0; i < num; i++) cout << indices[i] << "\t" << elements[i] <<endl;

    cout<<"sorting ..."<<endl<<endl;
    column->row_sort(indices.begin(), indices.begin() + mid);

    cout<<"idx\tXi"<<endl;
    for(size_t i = 0; i < num; i++) cout << indices[i] << "\t" << elements[indices[i]] <<endl;

    return 0;
}
//*/

/*
// string sorting
int main(int argc, char** argv) {
    vector<string> elements; //Xi
    vector<size_t> indices; //idx array

    int num = 6;

    for(size_t i = 0; i < num; i++) {
        elements.push_back("str_" + to_string(i));
        indices.push_back(i);
    }

    std::random_shuffle(elements.begin(), elements.end());
    Column* column = new DenseColumn<string>(ELEM_STRING, true);
    for(int i = 0; i < num; i++) {
        column->append(i, &elements[i]);
    }

    cout<<"idx\tXi"<<endl;
    for(size_t i = 0; i < num; i++) cout << indices[i] << "\t" << elements[i] <<endl;

    cout<<"sorting ..."<<endl<<endl;
    column->row_sort(indices.begin(), indices.end());

    cout<<"idx\tXi"<<endl;
    for(size_t i = 0; i < num; i++) cout << indices[i] << "\t" << elements[indices[i]] <<endl;

    size_t mid = num / 2;

    cout<<"reinit ..."<<endl<<endl;

    indices.clear();
    for(size_t i = 0; i < num; i++) indices.push_back(i);

    cout<<"idx\tXi"<<endl;
    for(size_t i = 0; i < num; i++) cout << indices[i] << "\t" << elements[i] <<endl;

    cout<<"sorting ..."<<endl<<endl;
    column->row_sort(indices.begin(), indices.begin() + mid);

    cout<<"idx\tXi"<<endl;
    for(size_t i = 0; i < num; i++) cout << indices[i] << "\t" << elements[indices[i]] <<endl;

    return 0;
}
//*/

/*
// test related to sketch variance calculation
int main(int argc, char** argv){
    // Ref : https://www.mathsisfun.com/data/standard-deviation.html
    Column* y = new DenseColumn<int>(ELEM_INT, true);
    int value = 600;
    y->append(0, &value);
    value = 470;
    y->append(1, &value);
    value = 170;
    y->append(2, &value);
    value = 430;
    y->append(3, &value);
    value = 300;
    y->append(4, &value);

    vector<size_t> indices;

    for(size_t i = 0;i < 5; i++) {
        indices.push_back(i);
    }

    var_sketch left; //for left child
    var_sketch right; //for right child

    set_sketch<int>(right, indices.begin(), indices.end(), y);

    double variance = var(right);

    cout<< "Initially right variance = " << variance << endl;

    // update sketches example

    // this is to test:
    for(size_t i = 0; i < y->size - 1; i++)
    {
        //note, you cannot move last element to L, or R has nothing
        int value;
        y->get(i, &value);
        update_sketches<int>(left, right, value);
        double var_L = var(left);
        double var_R = var(right);
        cout << "i = " << i << endl;
        cout << "var_L = " << var_L << endl;
        cout << "var_R = " << var_R << endl;
        cout << endl;
    }

    cout << "Ground Truth: "<<endl;
    auto begin = indices.begin();
    auto end = indices.end();
    for(size_t i = 0; i < y->size - 1; i++)
    {
        //note, you cannot move last element to L, or R has nothing
        cout << "i = " << i << endl;
        cout << "var_L = " << var(begin, begin + i + 1, y) << endl;
        cout << "var_R = " << var(begin + i + 1, end, y) << endl;
        cout << endl;
    }
}
//*/


/*
// sketch impurity, variance calculation related test code
int main(int argc, char** argv) {
    //ref: http://people.revoledu.com/kardi/tutorial/DecisionTree/how-to-measure-impurity.htm
    Column* target_column = new DenseColumn<string>(ELEM_STRING, false);
    string str = "Bus";
    target_column->append(0, &str);
    target_column->append(1, &str);
    target_column->append(2, &str);
    target_column->append(3, &str);

    str = "Car";
    target_column->append(4, &str);
    target_column->append(5, &str);
    target_column->append(6, &str);

    str = "Train";
    target_column->append(7, &str);
    target_column->append(8, &str);
    target_column->append(9, &str);

    vector<size_t> indices;

    for(size_t i = 0; i < 10; i++){
        indices.push_back(i);
    }

    classify_sketch<string> left;
    classify_sketch<string> right;

    set_sketch<string>(right, indices.begin(), indices.end(), target_column);

    vector<double> target_ratios;
    right.set_ratio(target_ratios);

    cout<< "node impurity:" << endl;
    cout<< "- entropy  = " << entropy(target_ratios) << endl;
    cout<< "- gini  = " << gini(target_ratios) << endl;
    cout<< "- classification error  = " << classification_error(target_ratios) << endl << endl;

    cout<< "------------ update test (U = Update Results, G = Ground Truth) ----------"<<endl<<endl;

    auto begin = indices.begin();
    auto end = indices.end();

    for(size_t i = 0; i < target_column->size - 1; i++) {
        string value;
        target_column->get(i, &value);

        //update sketch
        update_sketches<string>(left, right, value);

        //sketches => ratios
        vector<double> left_tgt_ratio;
        vector<double> right_tgt_ratio;
        left.set_ratio(left_tgt_ratio);
        right.set_ratio(right_tgt_ratio);

        //impurity(ratios)
        cout << "=========== i = " << i << " ===========" << endl;

        cout << "U: entropy_L = " << entropy(left_tgt_ratio)
             << ", entropy_R = " << entropy(right_tgt_ratio) << endl;

        cout << "G: entropy_L = " << classification_impurity(begin, begin + i + 1, target_column, entropy)
             << ", entropy_R = " << classification_impurity(begin + i + 1, end, target_column, entropy)
             << endl << endl;

        cout << "U: gini_L = " << gini(left_tgt_ratio)
             << ", gini_R = " << gini(right_tgt_ratio) << endl;

        cout << "G: gini_L = " << classification_impurity(begin, begin + i + 1, target_column, gini)
             << ", gini_R = " << classification_impurity(begin + i + 1, end, target_column, gini)
             << endl << endl;

        cout << "U: classification_error_L = " << classification_error(left_tgt_ratio)
             << ", classification_error_R = " << classification_error(right_tgt_ratio) << endl;

        cout << "G: classification_error_L = " << classification_impurity(begin, begin + i + 1, target_column, classification_error)
             << ", classification_error_R = " << classification_impurity(begin + i + 1, end, target_column, classification_error)
             << endl << endl;

        cout << endl;
    }

    return 0;
}
//*/

/*
// test meta csv and data.csv and load to matrix
int main(int argc, char** argv){
    Matrix X;
    load_csv("dataset/soybean.csv", "dataset/meta_soybean_latest.csv", X);
    cout<< "Meta Info ....................." << endl;
    X.print_meta();
    cout<< "Data Set ......................" << endl;
    X.print();

    return 0;
}
//*/

/*
// test split_node classification data (Y)
int main(int argc, char** argv){
    // test case is from:
    // http://dataaspirant.com/2017/01/30/how-decision-tree-algorithm-works/
    Matrix X;
    load_csv("dataset/ig_data.csv", "dataset/ig_meta.csv", X);
    X.print();

    size_t y_index = 4;
    Column* Y = X.get_column(y_index);
    size_t n_row = X.get_column(0)->size;
    vector<size_t> indices;

    for(size_t i = 0; i < n_row; i++) {
        indices.push_back(i);
    }

    vector<size_t>::iterator start = indices.begin();
    vector<size_t>::iterator end = indices.end();


    cout<< "@@@@@@@@@@@@@@@@@@@@ Entropy Impurity Test " <<endl;

    IMPURITY_FUNC = IMPURITY_ENTROPY;

    SplitResult* result = node_split(start, end, X, y_index);

    print_internal_node_content(result, start, end, X, y_index);

    cout<< "@@@@@@@@@@@@@@@@@@@@ GINI Impurity Test " <<endl;

    IMPURITY_FUNC = IMPURITY_GINI;

    result = node_split(start, end, X, y_index);

    print_internal_node_content(result, start, end, X, y_index);

    cout<< "@@@@@@@@@@@@@@@@@@@@ Classification Impurity Test " <<endl;

    IMPURITY_FUNC = IMPURITY_CLASSIFICATION_ERROR;

    result = node_split(start, end, X, y_index);

    print_internal_node_content(result, start, end, X, y_index);

    return 0;
}
//*/

/*
// test split node regression data (Y)
int main(int argc, char** argv){
    // test case is from:
    // http://dataaspirant.com/2017/01/30/how-decision-tree-algorithm-works/
    Matrix X;
    load_csv("dataset/ig_data1.csv", "dataset/ig_meta1.csv", X);

    size_t y_index = 4;
    Column* Y = X.get_column(y_index);
    size_t n_row = X.get_column(0)->size;
    vector<size_t> indices;

    for(size_t i = 0; i < n_row; i++) {
        indices.push_back(i);
    }


    cout<< "@@@@@@@@@@@@@@@@@@@@ Variance Impurity Test " <<endl;

    IMPURITY_FUNC = IMPURITY_VARIANCE;

    vector<size_t>::iterator start = indices.begin();
    vector<size_t>::iterator end = indices.end();

    SplitResult* result = node_split(start, end, X, y_index);

    print_internal_node_content(result, start, end, X, y_index);

    return 0;
}
//*/

/*
// test build_tree classification data (Y), X ordinal
// TODO:: need to check IMPURITY_CLASSIFICATION_ERRORS, there might be some issue, need to discuss
int main(int argc, char** argv){
    // test case is from:
    // http://dataaspirant.com/2017/01/30/how-decision-tree-algorithm-works/
    //Matrix X;
    //load_csv("dataset/ig_data.csv", "dataset/ig_meta.csv", X);
    //load_csv("dataset/soybean.csv", "dataset/meta_soybean_latest.csv", X);

    // Ref : http://archive.ics.uci.edu/ml/datasets/Iris
    //load_csv("dataset/classification/iris.data.csv", "dataset/classification/iris_meta.csv", X);
    //X.print();

    size_t sample_size;
    size_t n_split = 2;
    vector<Matrix> split_set;

    load_data("dataset/soybean.csv", "dataset/meta_soybean_latest.csv", n_split, split_set, sample_size);

    Matrix &train_set = split_set[0];
    Matrix &test_set = split_set[1];

    //size_t y_index = 4; // for both ig_data and iris_data
    size_t y_index = 35; // for soybean train_set
    vector<size_t> indices;

    size_t n_row = train_set.get_column(0)->size;

    for(size_t i = 0; i < n_row; i++) {
        indices.push_back(i);
    }

    cout<< "@@@@@@@@@@@@@@@@@@@@ Entropy Impurity Test " <<endl;

    IMPURITY_FUNC = IMPURITY_ENTROPY;

    TreeNode* root = train(train_set, y_index, indices);

    cout<< "######################" << endl;
    cout<< "printing the tree " << endl;
    print_tree(root, train_set, y_index, 0);

    double accuracy_entropy = get_accuracy(root, test_set, y_index);

    free_tree(root, train_set, y_index);
    cout<< "############# deleted tree properly #################" << endl;

    indices.clear();

    for(size_t i = 0; i < n_row; i++) {
        indices.push_back(i);
    }

    cout<< "@@@@@@@@@@@@@@@@@@@@ Gini Impurity Test " <<endl;

    IMPURITY_FUNC = IMPURITY_GINI;

    TreeNode* root2 = train(train_set, y_index, indices);

    cout<< "######################" << endl;
    cout<< "printing the tree " << endl;
    print_tree(root2, train_set, y_index, 0);

    double accuracy_gini = get_accuracy(root2, test_set, y_index);

    free_tree(root2, train_set, y_index);
    cout<< "############# deleted tree properly #################" << endl;

    indices.clear();

    for(size_t i = 0; i < n_row; i++) {
        indices.push_back(i);
    }

    cout<< "@@@@@@@@@@@@@@@@@@@@ Classification Error Impurity Test " <<endl;

    IMPURITY_FUNC = IMPURITY_CLASSIFICATION_ERROR;

    TreeNode* root3 = train(train_set, y_index, indices);

    cout<< "######################" << endl;
    cout<< "printing the tree " << endl;
    print_tree(root3, train_set, y_index, 0);

    double accuracy_cls_error = get_accuracy(root3, test_set, y_index);

    free_tree(root3, train_set, y_index);
    cout<< "############# deleted tree properly #################" << endl;

    cout<< "|sample| = " << sample_size << ", |train| = " << train_set.get_column(0)->size
        << ", |test| = " << test_set.get_column(0)->size << endl;
    cout << "Accuracy Entropy : " << accuracy_entropy << endl;
    cout << "Accuracy Gini : " << accuracy_gini << endl;
    cout << "Accuracy Classification Error : " << accuracy_cls_error << endl;

    return 0;
}
//*/

/*
// test Breiman's algorithm categorical data sorting example
int main(int argc, char** argv){
    int num_item = 10;

    DenseColumn<string>* categorical_column = new DenseColumn<string>(ELEM_STRING, false);
    string value = "green";
    categorical_column->append(0, &value);
    value = "red";
    categorical_column->append(1, &value);
    value = "green";
    categorical_column->append(2, &value);
    value = "green";
    categorical_column->append(3, &value);
    value = "blue";
    categorical_column->append(4, &value);
    value = "red";
    categorical_column->append(5, &value);
    value = "blue";
    categorical_column->append(6, &value);

    value = "green";
    categorical_column->append(7, &value);
    value = "red";
    categorical_column->append(8, &value);
    value = "green";
    categorical_column->append(9, &value);

    DenseColumn<int>* Y = new DenseColumn<int>(ELEM_INT, true);
    int value1 = 24;
    Y->append(0, &value1);
    value1 = 56;
    Y->append(1, &value1);
    value1 = 29;
    Y->append(2, &value1);
    value1 = 13;
    Y->append(3, &value1);
    value1 = 120;
    Y->append(4, &value1);
    value1 = 45;
    Y->append(5, &value1);
    value1 = 100;
    Y->append(6, &value1);

    value1 = 24;
    Y->append(7, &value1);
    value1 = 56;
    Y->append(8, &value1);
    value1 = 29;
    Y->append(9, &value1);

    vector<size_t> indices;

    for(size_t i = 0; i < num_item; i++){
        indices.push_back(i);
    }

    vector<size_t>::iterator start = indices.begin();
    vector<size_t>::iterator end = indices.end() - 3;

    vector<category_item> category_items;

    row_sort_categorical(start, end, categorical_column, Y, category_items);

    cout<< "#################sorted result 0 ---- 6, total = " << end  - start << endl;

    cout<< "index" << "\t" << "Column " << "\t" << "Y" << endl;
    for(auto it = start; it < end; it++){
        cout<< *it << "\t" << categorical_column->string_value(*it) << "\t" << Y->string_value(*it) << endl;
    }

    indices.clear();

    for(size_t i = 0; i < num_item; i++){
        indices.push_back(i);
    }

    start = indices.begin() + 3;
    end = indices.end();

    category_items.clear();
    row_sort_categorical(start, end, categorical_column, Y, category_items);

    cout<< "##################sorted result 3 ---- 10, total =  " << end - start << endl;

    cout<< "index" << "\t" << "Column " << "\t" << "Y" << endl;
    for(auto it = start; it < end; it++){
        cout<< *it << "\t" << categorical_column->string_value(*it) << "\t" << Y->string_value(*it) << endl;
    }


    return 0;
}
//*/

/*
// test Breiman's algorithm categorical data (Xi), Y continuous variance sketch update (batch update) test case
int main(int argc, char** argv){
    // Ref : http://www.dcc.fc.up.pt/~ltorgo/PhD/th3.pdf (Page - 72)
    int num_item = 7;

    DenseColumn<string>* Xi = new DenseColumn<string>(ELEM_STRING, false);
    string value = "green";
    Xi->append(0, &value);
    value = "red";
    Xi->append(1, &value);
    value = "green";
    Xi->append(2, &value);
    value = "green";
    Xi->append(3, &value);
    value = "blue";
    Xi->append(4, &value);
    value = "red";
    Xi->append(5, &value);
    value = "blue";
    Xi->append(6, &value);

    DenseColumn<int>* Y = new DenseColumn<int>(ELEM_INT, true);
    int value1 = 24;
    Y->append(0, &value1);
    value1 = 56;
    Y->append(1, &value1);
    value1 = 29;
    Y->append(2, &value1);
    value1 = 13;
    Y->append(3, &value1);
    value1 = 120;
    Y->append(4, &value1);
    value1 = 45;
    Y->append(5, &value1);
    value1 = 100;
    Y->append(6, &value1);

    vector<size_t> indices;

    for(size_t i = 0; i < num_item; i++) {
        indices.push_back(i);
    }

    vector<size_t>::iterator start = indices.begin();
    vector<size_t>::iterator end = indices.end();

    vector<category_item> category_items;
    row_sort_categorical(start, end, Xi, Y, category_items);

    cout << "start = " << *start << ", end = " << *(end - 1) << endl;

    cout<< "#################sorted result 0 ---- 6, total = " << end  - start << endl;

    cout<< "index" << "\t" << "Column " << "\t" << "Y" << endl;

    for(auto it = start; it < end; it++){
        cout<< *it << "\t" << Xi->string_value(*it) << "\t" << Y->string_value(*it) << endl;
    }

    var_sketch left;
    var_sketch right;
    set_sketch<int>(right, start, end, Y); // template type of Y

    double varLeft = var(left);
    double varRight = var(right);

    int total_element = 0;

    for(size_t i = 0; i < category_items.size() - 1; i++) {
        update_sketches(left, right, category_items[i]);
        total_element += category_items[i].indices.size();

        //compute pi
        double varL = var(left);
        double varR = var(right);

        //impurity(ratios)
        cout << "start = " << *start << " =========== middle = "
             << *(start + total_element) << " ===========" << ", end = " << * (end - 1) << endl;

        cout << "U: var_L = " << varL << ", var_R = " << varR << endl;

        cout << "G: var_L = " << var(start, start + total_element, Y)
             << ", var_R = " << var(start + total_element, end, Y) << endl;

        cout << endl;
    }

    return 0;
}
//*/

///// ############################## Xi categorical #############################################

/*
// test build_tree (X,Y) continuous regression problem
int main(int argc, char** argv){
    // test case is from:
    // https://data.world/uci/wine-quality, (red wine)
    // normal regression problem with all numeric data

    size_t sample_size;
    size_t n_split = 2;
    vector<Matrix> split_set;

    load_data("dataset/regression/wq_clean.csv",
             "dataset/regression/wq_clean_meta.csv", n_split, split_set, sample_size);

    Matrix &train_set = split_set[0];
    Matrix &test_set = split_set[1];
    IMPURITY_FUNC = IMPURITY_VARIANCE;
    //MAX_TREE_DEPTH = 5;

    size_t y_index = 11;
    size_t n_row = train_set.get_column(0)->size;
    vector<size_t> indices;

    for(size_t i = 0; i < n_row; i++){
        indices.push_back(i);
    }

    TreeNode* root = train(train_set, y_index, indices);

    print_tree(root, train_set, y_index, 0);

    double accuracy_variance = get_accuracy(root, test_set, y_index);

    free_tree(root, train_set, y_index);
    cout<< "deleted the tree properly"<<endl;

    cout<< "sample_size = " << sample_size << ", |train| = " << train_set.get_column(0)->size
        << ", |test| = " << test_set.get_column(0)->size << endl;
    cout << "Accuracy = " <<  accuracy_variance << endl;

    return 0;
}
//*/

/*
// test build_tree X might be categorical, Y continuous regression problem
int main(int argc, char** argv){
    // test case is from:
    // https://data.world/uci/forest-fires/workspace/project-summary
    //Matrix data_set;
    //load_csv("dataset/breiman/regression/forestfires_clean.csv", "dataset/breiman/regression/forestfires_meta_clean.csv", data_set); // binary classification
    //data_set.print();
    size_t sample_size;
    vector<Matrix> split_set;
    size_t n_split = 2;

    load_data("dataset/breiman/regression/forestfires_clean.csv",
             "dataset/breiman/regression/forestfires_meta_clean.csv", n_split, split_set, sample_size);

    IMPURITY_FUNC = IMPURITY_VARIANCE;
    Matrix &train_set = split_set[0];
    Matrix &test_set = split_set[1];
    //MAX_TREE_DEPTH = 10;

    size_t y_index = 12;
    size_t n_row = train_set.get_column(0)->size;
    vector<size_t> indices;

    for(size_t i = 0; i < n_row; i++){
        indices.push_back(i);
    }

    TreeNode* root = train(train_set, y_index, indices);
    print_tree(root, train_set, y_index, 0);

    double accuracy_variance = get_accuracy(root, test_set, y_index);

    free_tree(root, train_set, y_index);
    cout<< "deleted the tree properly" << endl;

    cout<< "|sample| = " << sample_size << ", |train| = " << train_set.get_column(0)->size
        << ", |test| = " << test_set.get_column(0)->size << endl;
    cout<< "Accuracy : " << accuracy_variance << endl;

    return 0;
}
//*/

/*
// test Brute-force categorical data ordering (lexical order) example
int main(int argc, char** argv){
    // Ref : https://medium.com/@rishabhjain_22692/decision-trees-it-begins-here-93ff54ef134

    int num_item = 18;

    // Outlook column of the above example
    DenseColumn<string>* Xi = new DenseColumn<string>(ELEM_STRING, false);
    string value = "Rainy";
    Xi->append(0, &value);
    value = "Rainy";
    Xi->append(1, &value);
    value = "Overcast";
    Xi->append(2, &value);
    value = "Sunny";
    Xi->append(3, &value);
    value = "Sunny";
    Xi->append(4, &value);
    value = "Sunny";
    Xi->append(5, &value);
    value = "Overcast";
    Xi->append(6, &value);

    value = "Rainy";
    Xi->append(7, &value);
    value = "Rainy";
    Xi->append(8, &value);
    value = "Sunny";
    Xi->append(9, &value);
    value = "Rainy";
    Xi->append(10, &value);
    value = "Overcast";
    Xi->append(11, &value);
    value = "Overcast";
    Xi->append(12, &value);
    value = "Sunny";
    Xi->append(13, &value);

    value = "Rainy";
    Xi->append(14, &value);
    value = "Rainy";
    Xi->append(15, &value);
    value = "Overcast";
    Xi->append(16, &value);
    value = "Sunny";
    Xi->append(17, &value);


    DenseColumn<string>* Y = new DenseColumn<string>(ELEM_STRING, false);
    value = "no";
    Y->append(0, &value);
    value = "no";
    Y->append(1, &value);
    value = "yes";
    Y->append(2, &value);
    value = "yes";
    Y->append(3, &value);
    value = "yes";
    Y->append(4, &value);
    value = "no";
    Y->append(5, &value);
    value = "yes";
    Y->append(6, &value);
    value = "no";
    Y->append(7, &value);
    value = "yes";
    Y->append(8, &value);
    value = "yes";
    Y->append(9, &value);
    value = "yes";
    Y->append(10, &value);
    value = "yes";
    Y->append(11, &value);
    value = "yes";
    Y->append(12, &value);
    value = "no";
    Y->append(13, &value);

    value = "no";
    Y->append(14, &value);
    value = "no";
    Y->append(15, &value);
    value = "yes";
    Y->append(16, &value);
    value = "yes";
    Y->append(17, &value);

    vector<size_t> indices;

    for(size_t i = 0; i < num_item; i++){
        indices.push_back(i);
    }

    vector<size_t>::iterator start = indices.begin();
    vector<size_t>::iterator end = indices.end() - 4;

    vector<Item> sorted_items;

    set_grouped_item(start, end, Xi, Y, sorted_items);


    cout<< "#################sorted result 0 ---- 13, total = " << end  - start << endl;

    cout<< "index" << "\t" << "Column " << "\t" << "Y" << endl;
    for(auto it = start; it < end; it++){
        cout<< *it << "\t" << Xi->string_value(*it) << "\t" << Y->string_value(*it) << endl;
    }

    indices.clear();

    for(size_t i = 0; i < num_item; i++){
        indices.push_back(i);
    }

    start = indices.begin() + 4;
    end = indices.end();

    sorted_items.clear();
    set_grouped_item(start, end, Xi, Y, sorted_items);

    cout<< "##################sorted result 4 ---- 17, total =  " << end - start << endl;

    cout<< "index" << "\t" << "Column " << "\t" << "Y" << endl;
    for(auto it = start; it < end; it++){
        cout<< *it << "\t" << Xi->string_value(*it) << "\t" << Y->string_value(*it) << endl;
    }

    return 0;
}
//*/

// helper function of set_enumeration()
void do_enumerate(size_t N, size_t curr_val, vector<size_t> &temp_result, vector<vector<size_t>> &list_s1) {
    // k = 1 .. floor(n/2), n is odd (no duplicate), n even (duplicate)
    size_t k_range = N / 2;

    if(temp_result.size() > 0) {
        if((N % 2 == 0) && (temp_result.size() == k_range - 1) && (temp_result[0] > 0)) {
            return;
        }
    }

    temp_result.push_back(curr_val);

    list_s1.push_back(temp_result);

    if(temp_result.size() < k_range) {
        for(int i = curr_val + 1; i < N; i++) {
            do_enumerate(N, i, temp_result, list_s1);
        }
    }

    temp_result.pop_back();
}

// param: number of unique item in vector<Item>
// param: list of index of vector<Item> (S1)
void set_enumeration(size_t N, vector<vector<size_t>> &list_s1) {
    if (N == 0) {
        return;
    } else if(N == 1) {
        vector<size_t> temp;
        temp.push_back(0);
        list_s1.push_back(temp);
        return;
    }

    vector<size_t> temp_result;

    for(size_t i = 0; i < N; i++) {
        do_enumerate(N, i, temp_result, list_s1);
    }
}

/*
// test Brute-force categorical data split algorithm, combination of items
int main(int argc, char** argv){
    cout << "For N even, suppose P = floor(N /2), then total = Nc1 + Nc2 + .....+ (NcP)/2" << endl;
    cout << "For N odd, suppose P = floor(N /2), then total = Nc1 + Nc2 + .....+ NcP" << endl;

    size_t N = 5;

    vector<vector<size_t>> list_s1;
    set_enumeration(N, list_s1);

    cout<< "Enumeration when number of unique items are " << N << endl;

    for(size_t i = 0; i < list_s1.size(); i++) {
        for(size_t j = 0; j < list_s1[i].size(); j++) {
            cout<< list_s1[i][j] << ", ";
        }
        cout<< endl;
    }

    cout << "N = " << N << ", total =  " << list_s1.size() << endl; //

    list_s1.clear();
    N = 6;
    set_enumeration(N, list_s1);

    cout<< "Enumeration when number of unique items are " << N << endl;

    for(size_t i = 0; i < list_s1.size(); i++) {
        for(size_t j = 0; j < list_s1[i].size(); j++) {
            cout<< list_s1[i][j] << ", ";
        }
        cout<< endl;
    }

    cout << "N = " << N << ", total = " << list_s1.size() << endl;


    list_s1.clear();
    N = 2;
    set_enumeration(N, list_s1);

    cout<< "Enumeration when number of unique items are " << N << endl;

    for(size_t i = 0; i < list_s1.size(); i++) {
        for(size_t j = 0; j < list_s1[i].size(); j++) {
            cout<< list_s1[i][j] << ", ";
        }
        cout<< endl;
    }

    cout << "N = " << N << ", total = " << list_s1.size() << endl;


    list_s1.clear();
    N = 0;
    set_enumeration(N, list_s1);

    cout<< "Enumeration when number of unique items are " << N << endl;

    for(size_t i = 0; i < list_s1.size(); i++) {
        for(size_t j = 0; j < list_s1[i].size(); j++) {
            cout<< list_s1[i][j] << ", ";
        }
        cout<< endl;
    }

    cout << "N = " << N << ", total = " << list_s1.size() << endl;


    list_s1.clear();
    N = 10;

    auto start = std::chrono::system_clock::now();
    set_enumeration(N, list_s1);
    auto end = std::chrono::system_clock::now();

    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    cout<< "time to do enumeraton for N = " << N << " is = " << elapsed.count() << " microseconds"<< endl;

    cout<< "Enumeration when number of unique items are " << N << endl;

    for(size_t i = 0; i < list_s1.size(); i++) {
        for(size_t j = 0; j < list_s1[i].size(); j++) {
            cout<< list_s1[i][j] << ", ";
        }
        cout<< endl;
    }

    cout << "N = " << N << ", total = " << list_s1.size() << endl;


    return 0;
}
//*/


/*
// test Brute-force categorical data batch_sketch update test case
int main(int argc, char** argv){
    // Ref : https://medium.com/@rishabhjain_22692/decision-trees-it-begins-here-93ff54ef134

    Matrix data_set;
    load_csv("dataset/brute_force/weather_data.csv", "dataset/brute_force/weather_meta.csv", data_set);
    //data_set.print();

    size_t y_index = 4;
    Column* Xi = data_set.get_column(0);
    Column* Y = data_set.get_column(y_index);

    vector<size_t> indices;

    for(size_t i = 0; i < Xi->size; i++) {
        indices.push_back(i);
    }

    vector<size_t>::iterator start = indices.begin();
    vector<size_t>::iterator end = indices.end();

    vector<Item> sorted_items;
    set_grouped_item(start, end, Xi, Y, sorted_items);

    cout<< "index" << "\t" << "Xi_value"<<endl;

    for(size_t i = 0; i < sorted_items.size(); i++) {
        for(size_t j = 0; j < sorted_items[i].indices.size(); j++) {
            cout<< sorted_items[i].indices[j] << "\t"
                << Xi->string_value(sorted_items[i].indices[j]) << endl;
        }
    }

    cout<<endl<<endl;

    IMPURITY_FUNC = IMPURITY_ENTROPY;

    double impurity_improvement;
    size_t best_offset;
    vector<size_t> S1;

    find_best_split_enum(sorted_items, Y, impurity_improvement, best_offset, S1);
    update_iterator(start, sorted_items);

    cout << "best impurity found = " << impurity_improvement << endl;
    cout << "and best enumeration is : " << endl;

    for(size_t i = 0; i < S1.size(); i++) {
        cout << "enum index = " << S1[i] << ", item name = "
             << sorted_items[S1[i]].category
             << ", |sample| = " << sorted_items[S1[i]].indices.size()
             << ", start_idx = " << *(start)
             << ", end_idx = " <<   *(start + best_offset) << endl;
    }

    cout << endl;

    cout<< "index" << "\t" << "Xi_value"<<endl;

    for(auto it = start; it != end; it++) {
        if(it == (start + best_offset)) {
            cout<< *it << "*"  << "\t" << Xi->string_value(*it) << "*" << endl;
        } else {
            cout<< *it << "\t" << Xi->string_value(*it) << endl;
        }
    }

    return 0;
}
//*/

/*
// test Brute-force categorical data node_split test case
int main(int argc, char** argv){
    // Ref : https://medium.com/@rishabhjain_22692/decision-trees-it-begins-here-93ff54ef134

    Matrix data_set;
    load_csv("dataset/brute_force/weather_data.csv", "dataset/brute_force/weather_meta.csv", data_set);
    data_set.print();

    size_t y_index = 4;
    Column* Xi = data_set.get_column(0);
    Column* Y = data_set.get_column(y_index);

    vector<size_t> indices;

    for(size_t i = 0; i < Xi->size; i++) {
        indices.push_back(i);
    }

    vector<size_t>::iterator start = indices.begin();
    vector<size_t>::iterator end = indices.end();


    cout<< "@@@@@@@@@@@@@@@@@@@@ Entropy Impurity Test " <<endl;

    IMPURITY_FUNC = IMPURITY_ENTROPY;

    SplitResult* result = node_split(start, end, data_set, y_index);

    print_internal_node_content(result, start, end, data_set, y_index);

    cout<< "@@@@@@@@@@@@@@@@@@@@ GINI Impurity Test " <<endl;

    IMPURITY_FUNC = IMPURITY_GINI;

    indices.clear();

    for(size_t i = 0; i < Xi->size; i++) {
        indices.push_back(i);
    }

    start = indices.begin();
    end = indices.end();

    result = node_split(start, end, data_set, y_index);

    print_internal_node_content(result, start, end, data_set, y_index);

    cout<< "@@@@@@@@@@@@@@@@@@@@ Classification Impurity Test " <<endl;

    IMPURITY_FUNC = IMPURITY_CLASSIFICATION_ERROR;

    indices.clear();

    for(size_t i = 0; i < Xi->size; i++) {
        indices.push_back(i);
    }

    start = indices.begin();
    end = indices.end();

    result = node_split(start, end, data_set, y_index);

    print_internal_node_content(result, start, end, data_set, y_index);

    return 0;
}

//*/


///// ############################### Train_Test Data ###########################################

/*
// test train and test data load to matrix
int main(int argc, char** argv){
    size_t sample_size;
    int n_split = 2;
    vector<Matrix> split_set;

    load_data("dataset/soybean.csv", "dataset/meta_soybean_latest.csv", n_split, split_set, sample_size);

    for(size_t i = 0; i < split_set.size(); i++) {
        cout << "Set = " << i;
        Matrix &data = split_set[i];
        cout << ", |N| = " << data.get_column(0)->size << endl;
        data.print();
    }

    return 0;
}
//*/

/*
// test build_tree X might be categorical, Y Categorical Classification problem
int main(int argc, char** argv){
    size_t sample_size;
    size_t n_split = 2;
    vector<Matrix> split_set;
    // Ref : https://medium.com/@rishabhjain_22692/decision-trees-it-begins-here-93ff54ef134
    //load_data("dataset/brute_force/weather_data.csv", "dataset/brute_force/weather_meta.csv", data_set, n_split, split_set);

    // Ref : http://archive.ics.uci.edu/ml/datasets/Bank+Marketing
    load_data("dataset/brute_force/bank.csv", "dataset/brute_force/bank_meta.csv", n_split, split_set, sample_size);
    //data_set.print();
    Matrix &train_set = split_set[0];
    Matrix &test_set = split_set[1];

    //size_t y_index = 4;// for weather data
    size_t y_index = 16;// for bank data
    Column* Xi = train_set.get_column(0);
    Column* Y = train_set.get_column(y_index);

    vector<size_t> indices;
    TreeNode* root;

    cout<< "@@@@@@@@@@@ Entropy Impurity Function Tree" << endl;
    IMPURITY_FUNC = IMPURITY_ENTROPY;

    for(size_t i = 0; i < Xi->size; i++) {
        indices.push_back(i);
    }

    vector<size_t>::iterator start = indices.begin();
    vector<size_t>::iterator end = indices.end();

    root = train(train_set, y_index, indices);
    cout<< "@@@@@@@@@@@Training done properly, now would print the tree" << endl;
    print_tree(root, train_set, y_index, 0);

    double accuracy_entropy = get_accuracy(root, test_set, y_index);

    free_tree(root, train_set, y_index);
    cout<< "deleted the tree properly" << endl;

    cout<< "@@@@@@@@@@@ Gini Impurity Function Tree" << endl;

    IMPURITY_FUNC = IMPURITY_GINI;

    indices.clear();

    for(size_t i = 0; i < Xi->size; i++) {
        indices.push_back(i);
    }

    root = train(train_set, y_index, indices);
    cout<< "@@@@@@@@@@@Training done properly, now would print the tree" << endl;
    print_tree(root, train_set, y_index, 0);

    double accuracy_gini = get_accuracy(root, test_set, y_index);
    free_tree(root, train_set, y_index);
    cout<< "deleted the tree properly" << endl;

    cout<< "@@@@@@@@@@@ Classification Error Impurity Function Tree" << endl;

    IMPURITY_FUNC = IMPURITY_CLASSIFICATION_ERROR;

    indices.clear();

    for(size_t i = 0; i < Xi->size; i++) {
        indices.push_back(i);
    }

    root = train(train_set, y_index, indices);
    cout<< "@@@@@@@@@@@Training done properly, now would print the tree" << endl;
    print_tree(root, train_set, y_index, 0);

    double accuracy_cls_error = get_accuracy(root, test_set, y_index);

    free_tree(root, train_set, y_index);
    cout<< "deleted the tree properly" << endl;

    cout<< "|sample| = " << sample_size << ", |train| = " << train_set.get_column(0)->size
        << ", |test| = " << test_set.get_column(0)->size << endl;
    cout << "Entropy Impurity :: Accuracy = " << accuracy_entropy << endl;
    cout << "Gini Impurity :: Accuracy = " << accuracy_gini << endl;
    cout << "Classification Error Impurity :: Accuracy = " << accuracy_cls_error << endl;

    //delete_matrix(split_set);

    return 0;
}
//*/

/*
// test train and test data load to matrix
int main(int argc, char** argv){
    // test code how to run (./run -f "dataset/brute_force/bank.csv" -m "dataset/brute_force/bank_meta.csv" -s 2 -y 16)
    cout<< "Usage : " << endl;
    cout<< "-f = csv_file path, -m = meta file path, -D = Max Tree Depth, -U = Max Categorical Item, ";
    cout<< "-s = number of split, -i = Impurity Function, -y = y_index " << endl;

    char* file_path;
    char* meta_file;
    int option;
    int n_split = 2;// default
    size_t y_index = -1;
    IMPURITY_FUNC = IMPURITY_ENTROPY;

    while((option = getopt(argc, argv,"s:D:U:f:m:i:y:")) != -1) {
        switch(option) {
            case 's':
                n_split = atoi(optarg);
                break;
            case 'D':
                MAX_TREE_DEPTH = atoi(optarg);
                break;
            case 'U':
                BRUTE_FORCE_MAX_ITEM = atoi(optarg);
                break;
            case 'f':
                file_path = optarg;
                break;
            case 'm':
                meta_file = optarg;
                break;
            case 'i':
                IMPURITY_FUNC = atoi(optarg);
                assert(IMPURITY_FUNC >= IMPURITY_ENTROPY && IMPURITY_FUNC <= IMPURITY_VARIANCE);
                break;
            case 'y':
                y_index = atoi(optarg);
                break;
            default:
                break;
        }
    }

    cout << "[DEBUG] f_path = " << file_path << ", meta file = " << meta_file
         << ", MAX_TREE_DEPTH = " << MAX_TREE_DEPTH << ", n_split = " << n_split
         << ", BRUTE_FORCE_MAX_ITEM = " << BRUTE_FORCE_MAX_ITEM
         << ", y_index = " << y_index << endl;


    size_t sample_size;
    vector<Matrix> split_set;

    // Ref : http://archive.ics.uci.edu/ml/datasets/Bank+Marketing
    load_data(file_path, meta_file, n_split, split_set, sample_size);
    //data_set.print();
    Matrix &train_set = split_set[0];
    Matrix &test_set = split_set[1];

    //size_t y_index = 4;// for weather data
    //size_t y_index = 16;// for bank data
    Column* Xi = train_set.get_column(0);
    Column* Y = train_set.get_column(y_index);

    vector<size_t> indices;
    TreeNode* root;

    cout<< "@@@@@@@@@@@ Entropy Impurity Function Tree" << endl;
    IMPURITY_FUNC = IMPURITY_ENTROPY;

    for(size_t i = 0; i < Xi->size; i++) {
        indices.push_back(i);
    }

    vector<size_t>::iterator start = indices.begin();
    vector<size_t>::iterator end = indices.end();

    root = train(train_set, y_index, indices);
    cout<< "@@@@@@@@@@@Training done properly, now would print the tree" << endl;
    print_tree(root, train_set, y_index, 0);

    double accuracy_entropy = get_accuracy(root, test_set, y_index);

    free_tree(root, train_set, y_index);
    cout<< "deleted the tree properly" << endl;

    return 0;
}
//*/

//*
// test build_tree X might be categorical, Y Categorical Classification problem
int main(int argc, char** argv){
    size_t sample_size;

    // Ref : http://archive.ics.uci.edu/ml/datasets/Bank+Marketing
    Matrix train_set;
    Matrix test_set;
    load_csv("dataset/brute_force/missing_D/bank_train.csv",
             "dataset/brute_force/missing_D/bank_meta.csv", train_set);
    load_csv("dataset/brute_force/missing_D/bank_test.csv",
             "dataset/brute_force/missing_D/bank_meta.csv", test_set);

    //size_t y_index = 4;// for weather data
    size_t y_index = 16;// for bank data
    Column* Xi = train_set.get_column(0);
    Column* Y = train_set.get_column(y_index);
    sample_size = Xi->size;

    vector<size_t> indices;
    TreeNode* root;

    cout<< "@@@@@@@@@@@ Entropy Impurity Function Tree" << endl;
    IMPURITY_FUNC = IMPURITY_ENTROPY;

    for(size_t i = 0; i < Xi->size; i++) {
        indices.push_back(i);
    }

    vector<size_t>::iterator start = indices.begin();
    vector<size_t>::iterator end = indices.end();

    root = train(train_set, y_index, indices);
    cout<< "@@@@@@@@@@@Training done properly, now would print the tree" << endl;
    print_tree(root, train_set, y_index, 0);

    double accuracy_entropy = get_accuracy(root, test_set, y_index);

    free_tree(root, train_set, y_index);
    cout<< "deleted the tree properly" << endl;

    cout<< "@@@@@@@@@@@ Gini Impurity Function Tree" << endl;

    IMPURITY_FUNC = IMPURITY_GINI;

    indices.clear();

    for(size_t i = 0; i < Xi->size; i++) {
        indices.push_back(i);
    }

    root = train(train_set, y_index, indices);
    cout<< "@@@@@@@@@@@Training done properly, now would print the tree" << endl;
    print_tree(root, train_set, y_index, 0);

    double accuracy_gini = get_accuracy(root, test_set, y_index);
    free_tree(root, train_set, y_index);
    cout<< "deleted the tree properly" << endl;

    cout<< "@@@@@@@@@@@ Classification Error Impurity Function Tree" << endl;

    IMPURITY_FUNC = IMPURITY_CLASSIFICATION_ERROR;

    indices.clear();

    for(size_t i = 0; i < Xi->size; i++) {
        indices.push_back(i);
    }

    root = train(train_set, y_index, indices);
    cout<< "@@@@@@@@@@@Training done properly, now would print the tree" << endl;
    print_tree(root, train_set, y_index, 0);

    double accuracy_cls_error = get_accuracy(root, test_set, y_index);

    free_tree(root, train_set, y_index);
    cout<< "deleted the tree properly" << endl;

    cout<< "|sample| = " << sample_size << ", |train| = " << train_set.get_column(0)->size
        << ", |test| = " << test_set.get_column(0)->size << endl;
    cout << "Entropy Impurity :: Accuracy = " << accuracy_entropy << endl;
    cout << "Gini Impurity :: Accuracy = " << accuracy_gini << endl;
    cout << "Classification Error Impurity :: Accuracy = " << accuracy_cls_error << endl;

    return 0;
}
//*/