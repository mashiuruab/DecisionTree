#ifndef TREE_H
#define TREE_H

#include "splitter.h"
#include <climits>

class TreeNode {
public:
    int column_index; // column index [0......m-1] if there are m columns
    vector<size_t>::iterator start; // start iterator of the data with which the node is formed
    vector<size_t>::iterator end; ///  end iterator of the  data with which the node is formed

    TreeNode* left = nullptr; //  left child pointer
    TreeNode* right = nullptr; //  right child pointer
};

template <class T>
class OrdinalTreeNode : public TreeNode {
public:
    T split_value;
};

template <class T>
class CategoricalTreeNode : public TreeNode {
public:
    T label;// frequent y_label between [start, end)
    unordered_set<T> S;
    unordered_set<T> S1;
};

template <class T>
class LeafNode : public TreeNode {
public:
    T label;
};

// for debug
// [start, end) should've been sorted by Xi (recorded in SplitResult)
void print_internal_node_content(SplitResult* result, vector<size_t>::iterator &start,
                 vector<size_t>::iterator &end, Matrix &data_set, size_t y_index) {

    Column* Xi = data_set.get_column(result->column_idx);
    Column* Y = data_set.get_column(y_index);

    cout << "|D| = " << (end - start) << ", attr " << result->column_idx << " is the best" << endl;

    size_t count = 0;
    // printing index \t sample_index \t attribute_value  \t  y_value
    cout<< "No." << "\t" << "idx" << "\t" << "Xi" << "\t" << "y" << endl;
    for(auto it = start; it != end; it++){
        if(it == result->pos) {
            cout<< count << "*" << "\t" << *it << "*" << "\t" << Xi->string_value(*it) << "*" << "\t" << Y->string_value(*it) << "*" << endl;
        } else {
            cout<< count << "\t" << *it << "\t" << Xi->string_value(*it) << "\t" << Y->string_value(*it) << endl;
        }
        count++;
    }
}

// this function is to decide when to stop splitting given [start, end)
// param: start_iterator, end_iterator (exclusive), Column Y
// output: stop ?
template <class T>
bool stop_splitting(vector<size_t>::iterator start, vector<size_t>::iterator end,
                    Column* Y, T &leaf_label) { // checking whether all Y_values are same
    assert(start != end);

    map<T, size_t> freq; // freq[y_label] = frequency of elements with y_label in [start, end)

    for(auto it = start; it != end; it++) {
        T value;
        Y->get(*it, &value);

        auto it2 = freq.find(value);
        if(it2 == freq.end()) {
            freq[value] = 1;
        } else {
            it2->second++;
        }
    }

    size_t max_freq = 0;

    for(auto it = freq.begin(); it != freq.end(); it++) {
        if(max_freq < it->second) {
            leaf_label = it->first;
            max_freq = it->second;
        }
    }

    return freq.size() == 1; // stop splitting if there is only 1 item in Y[start, end)
}

template <class T>
TreeNode* create_leaf(vector<size_t>::iterator start, vector<size_t>::iterator end, T label)
{
    LeafNode<T>* node = new LeafNode<T>();
    node->start = start;
    node->end = end;
    node->column_index = -1; //-1 means leaf
    node->label = label;
    return node;
}

TreeNode* build_tree(vector<size_t>::iterator start, vector<size_t>::iterator end,
                Matrix &data_set, size_t y_index, size_t tree_depth) {

    bool end_of_path = (tree_depth == MAX_TREE_DEPTH);

    tree_depth++;

    Column* Y = data_set.get_column(y_index);
    int ytype = Y->data_type;
    bool stop;
    string node_label; // y value with highest frequency

    if(ytype == ELEM_BOOL) {
        bool label;
        stop = stop_splitting<bool>(start, end, Y, label);
        if(stop || end_of_path) {
            TreeNode* node = create_leaf<bool>(start, end, label);
            return node;
        }
        node_label = to_string(label);
    } else if(ytype == ELEM_SHORT) {
        short label;
        stop = stop_splitting<short>(start, end, Y, label);
        if(stop || end_of_path) {
            TreeNode* node = create_leaf<short>(start, end, label);
            return node;
        }
        node_label = to_string(label);
    } else if(ytype == ELEM_INT) {
        int label;
        stop = stop_splitting<int>(start, end, Y, label);
        if(stop || end_of_path) {
            TreeNode* node = create_leaf<int>(start, end, label);
            return node;
        }
        node_label = to_string(label);
    } else if(ytype == ELEM_FLOAT) {
        float label;
        stop = stop_splitting<float>(start, end, Y, label);
        if(stop || end_of_path) {
            TreeNode* node = create_leaf<float>(start, end, label);
            return node;
        }
        node_label = to_string(label);
    } else if(ytype == ELEM_DOUBLE) {
        double label;
        stop = stop_splitting<double>(start, end, Y, label);
        if(stop || end_of_path) {
            TreeNode* node = create_leaf<double>(start, end, label);
            return node;
        }
        node_label = to_string(label);
    } else if(ytype == ELEM_CHAR) {
        char label;
        stop = stop_splitting<char>(start, end, Y, label);
        if(stop || end_of_path) {
            TreeNode* node = create_leaf<char>(start, end, label);
            return node;
        }
        node_label = to_string(label);
    } else if(ytype == ELEM_STRING) {
        string label;
        stop = stop_splitting<string>(start, end, Y, label);
        if(stop || end_of_path) {
            TreeNode* node = create_leaf<string>(start, end, label);
            return node;
        }
        node_label = label;
    } else {
        cout<<"[ERROR] we don't consider any other attribute type"<<endl;
        exit(-1);
    }

    //find the best split among all the features (Xi's)
    SplitResult* result = node_split(start, end, data_set, y_index); //sort [start, end) by the best Xi

    print_internal_node_content(result, start, end, data_set, y_index); //###### debug ######

    vector<size_t>::iterator middle = result->pos; // split row index left <, right >=

    //create current node and recurse on children
    Column* Xi = data_set.get_column(result->column_idx);

    TreeNode* curNode;

    if(Xi->is_ordinal) {
        //-----
        if(Xi->data_type == ELEM_BOOL) {
            OrdinalTreeNode<bool>* ordinalTreeNode = new OrdinalTreeNode<bool>();
            curNode = ordinalTreeNode;
            Xi->get(*result->pos, &(ordinalTreeNode->split_value));

        } else if(Xi->data_type == ELEM_SHORT) {
            OrdinalTreeNode<short>* ordinalTreeNode = new OrdinalTreeNode<short>();
            curNode = ordinalTreeNode;
            Xi->get(*result->pos, &(ordinalTreeNode->split_value));

        } else if(Xi->data_type == ELEM_INT) {
            OrdinalTreeNode<int>* ordinalTreeNode = new OrdinalTreeNode<int>();
            curNode = ordinalTreeNode;
            Xi->get(*result->pos, &(ordinalTreeNode->split_value));

        } else if(Xi->data_type == ELEM_FLOAT) {
            OrdinalTreeNode<float>* ordinalTreeNode = new OrdinalTreeNode<float>();
            curNode = ordinalTreeNode;
            Xi->get(*result->pos, &(ordinalTreeNode->split_value));

        } else if(Xi->data_type == ELEM_DOUBLE) {
            OrdinalTreeNode<double>* ordinalTreeNode = new OrdinalTreeNode<double>();
            curNode = ordinalTreeNode;
            Xi->get(*result->pos, &(ordinalTreeNode->split_value));

        } else if(Xi->data_type == ELEM_CHAR) {
            OrdinalTreeNode<char>* ordinalTreeNode = new OrdinalTreeNode<char>();
            curNode = ordinalTreeNode;
            Xi->get(*result->pos, &(ordinalTreeNode->split_value));

        } else if(Xi->data_type == ELEM_STRING) {
            OrdinalTreeNode<string>* ordinalTreeNode = new OrdinalTreeNode<string>();
            curNode = ordinalTreeNode;
            Xi->get(*result->pos, &(ordinalTreeNode->split_value));
        } else {
            cout<<"[ERROR] we don't consider any other attribute type"<<endl;
            exit(-1);
        }
        //------
        curNode->column_index = result->column_idx;
        curNode->start = start;
        curNode->end = end;
        curNode->left = build_tree(start, middle, data_set, y_index, tree_depth); // < or less than attribute_val
        curNode->right = build_tree(middle, end, data_set, y_index, tree_depth); // >= or greater than or equal attribute_val
        return curNode;
    } else {
        // only considering string here
        // TODO:: might need to consider other types
        CategoricalSplitResult<string>* splitResult = (CategoricalSplitResult<string>*) result;
        CategoricalTreeNode<string>* treeNode = new CategoricalTreeNode<string>();

        curNode = treeNode;

        istringstream(node_label) >> treeNode->label;
        treeNode->S = splitResult->S;
        treeNode->S1 = splitResult->S1;
        treeNode->column_index = splitResult->column_idx;
        treeNode->start = start;
        treeNode->end = end;
        treeNode->left = build_tree(start, middle, data_set, y_index, tree_depth); // < or less than attribute_val
        treeNode->right = build_tree(middle, end, data_set, y_index, tree_depth); // >= or greater than or equal attribute_val
        return curNode;
    }
}

// output: root node of the tree
TreeNode* train(Matrix &data_set, size_t y_index, vector<size_t> &indices) {
    TreeNode* root = build_tree(indices.begin(), indices.end(), data_set, y_index, 0);
    return root;
}

// ================== da checked till here...

// printing the tree starting from root to each leaf
// recursive function called repeatedly until the end
// param  : TreeNode* root, Matrix& data_set and output_col_index
void print_tree(TreeNode* root,  Matrix &data_set, size_t y_index, size_t tree_depth) {
    Column* Y = data_set.get_column(y_index);

    for(size_t i = 0; i < tree_depth; i++) cout<< "--- ";

    size_t sample_size = root->end - root->start;

    if(root->column_index == -1) {
        cout<<"Leaf: |D| = "<<sample_size;

        if(Y->data_type == ELEM_BOOL) {
            LeafNode<bool>* leaf_node = (LeafNode<bool>*) root;
            cout<< ", label  = " << leaf_node->label;
        } else if(Y->data_type == ELEM_SHORT) {
            LeafNode<short>* leaf_node = (LeafNode<short>*) root;
            cout<< ", label  = " << leaf_node->label;
        } else if(Y->data_type == ELEM_INT) {
            LeafNode<int>* leaf_node = (LeafNode<int>*) root;
            cout<< ", label  = " << leaf_node->label;
        } else if(Y->data_type == ELEM_FLOAT) {
            LeafNode<float>* leaf_node = (LeafNode<float>*) root;
            cout<< ", label  = " << leaf_node->label;
        } else if(Y->data_type == ELEM_DOUBLE) {
            LeafNode<double>* leaf_node = (LeafNode<double>*) root;
            cout<< ", label  = " << leaf_node->label;
        } else if(Y->data_type == ELEM_CHAR) {
            LeafNode<char>* leaf_node = (LeafNode<char>*) root;
            cout<< ", label  = " << leaf_node->label;
        } else if(Y->data_type == ELEM_STRING) {
            LeafNode<string>* leaf_node = (LeafNode<string>*) root;
            cout<< ", label  = " << leaf_node->label;
        } else {
            cout<< "[ERROR] Unknown Attribute(y) of type " << Y->data_type << endl;
            exit(-1);
        }
    } else { // regular node other than leaf
        cout<<"Internal: |D| = "<<sample_size<<", attr-idx = "<<(root->column_index);

        Column* Xi = data_set.get_column(root->column_index);

        if(Xi->is_ordinal) {
            if(Xi->data_type == ELEM_BOOL) {
                OrdinalTreeNode<bool>* ordinalTreeNode = (OrdinalTreeNode<bool>*) root;
                cout<< ", split @ " << ordinalTreeNode->split_value;
            } else if(Xi->data_type == ELEM_SHORT) {
                OrdinalTreeNode<short>* ordinalTreeNode = (OrdinalTreeNode<short>*) root;
                cout<< ", split @ " << ordinalTreeNode->split_value;
            } else if(Xi->data_type == ELEM_INT) {
                OrdinalTreeNode<int>* ordinalTreeNode = (OrdinalTreeNode<int>*) root;
                cout<< ", split @ " << ordinalTreeNode->split_value;
            } else if(Xi->data_type == ELEM_FLOAT) {
                OrdinalTreeNode<float>* ordinalTreeNode = (OrdinalTreeNode<float>*) root;
                cout<< ", split @ " << ordinalTreeNode->split_value;
            } else if(Xi->data_type == ELEM_DOUBLE) {
                OrdinalTreeNode<double>* ordinalTreeNode = (OrdinalTreeNode<double>*) root;
                cout<< ", split @ " << ordinalTreeNode->split_value;
            } else if(Xi->data_type == ELEM_CHAR) {
                OrdinalTreeNode<char>* ordinalTreeNode = (OrdinalTreeNode<char>*) root;
                cout<< ", split @ " << ordinalTreeNode->split_value;
            } else if(Xi->data_type == ELEM_STRING) {
                OrdinalTreeNode<string>* ordinalTreeNode = (OrdinalTreeNode<string>*) root;
                cout<< ", split @ " << ordinalTreeNode->split_value;
            } else {
                cout<< "[ERROR] Unknown Attribute(Xi) of type " << Xi->data_type << endl;
                exit(-1);
            }
        } else {
            // TODO:: might need to consider types other than string
            CategoricalTreeNode<string>* treeNode = (CategoricalTreeNode<string>*) root;
            cout<< ", split @ S1 = ";

            for(auto it = treeNode->S1.begin(); it != treeNode->S1.end(); it++) {
                if(it != treeNode->S1.begin()) cout<<", ";
                cout<< *it;
            }
        }
    }
    cout<< endl;
    tree_depth++;
    if(root->left != nullptr) print_tree(root->left, data_set, y_index, tree_depth);
    if(root->right != nullptr) print_tree(root->right, data_set, y_index, tree_depth);
}

template <class T>
void free_internal_node(TreeNode* root, Column* Xi) {
    if (Xi->is_ordinal) {
        OrdinalTreeNode<T>* node = (OrdinalTreeNode<T>*) root;
        delete node;
    } else {
        CategoricalTreeNode<T>* node = (CategoricalTreeNode<T>*) root;
        delete node;
    }
}

void free_tree(TreeNode* root, Matrix &data_set, size_t y_index) {

    if(root->column_index == -1) { //current node is a leaf
        Column* Y = data_set.get_column(y_index);
        if(Y->data_type == ELEM_BOOL) {
            LeafNode<bool>* node = (LeafNode<bool>*) root;
            delete node;
        } else if(Y->data_type == ELEM_SHORT) {
            LeafNode<short>* node = (LeafNode<short>*) root;
            delete node;
        } else if(Y->data_type == ELEM_INT) {
            LeafNode<int>* node = (LeafNode<int>*) root;
            delete node;
        } else if(Y->data_type == ELEM_FLOAT) {
            LeafNode<float>* node = (LeafNode<float>*) root;
            delete node;
        } else if(Y->data_type == ELEM_DOUBLE) {
            LeafNode<double>* node = (LeafNode<double>*) root;
            delete node;
        } else if(Y->data_type == ELEM_CHAR) {
            LeafNode<char>* node = (LeafNode<char>*) root;
            delete node;
        } else if(Y->data_type == ELEM_STRING) {
            LeafNode<string>* node = (LeafNode<string> *) root;
            delete node;
        } else {
            cout<< "[ERROR] " << "y: Unknown data type " << Y->data_type << endl;
            exit(-1);
        }
    } else {
        free_tree(root->left, data_set, y_index);
        free_tree(root->right, data_set, y_index);

        Column* Xi = data_set.get_column(root->column_index);
        if(Xi->data_type == ELEM_BOOL) {
            free_internal_node<bool>(root, Xi);
        } else if(Xi->data_type == ELEM_SHORT) {
            free_internal_node<short>(root, Xi);
        } else if(Xi->data_type == ELEM_INT) {
            free_internal_node<int>(root, Xi);
        } else if(Xi->data_type == ELEM_FLOAT) {
            free_internal_node<float>(root, Xi);
        } else if(Xi->data_type == ELEM_DOUBLE) {
            free_internal_node<double>(root, Xi);
        } else if(Xi->data_type == ELEM_CHAR) {
            free_internal_node<char>(root, Xi);
        } else if(Xi->data_type == ELEM_STRING) {
            free_internal_node<string>(root, Xi);
        } else {
            cout<< "[ERROR] " << "Xi: Unknown data type " << Xi->data_type << endl;
            exit(-1);
        }
    }
}

// predicting y for row_idx in test_set
// input: root (Tree_Root), test_set, y, row_idx
// output: predicted_y
template <class T>
void predict(TreeNode *root, Matrix &test, size_t row_idx, T &predicted_y) {
    TreeNode* temp = root;

    while (temp->column_index != -1) { // column_index == -1 means leaf node, see create_leaf function
        Column* column = test.get_column(temp->column_index);

        if(column->is_ordinal) {
            if(column->data_type == ELEM_SHORT) {
                short value;
                column->get(row_idx, &value);

                OrdinalTreeNode<short>* node = (OrdinalTreeNode<short>*) temp;
                if(value >= node->split_value) {
                    temp = temp->right;
                } else {
                    temp = temp->left;
                }
            } else if(column->data_type == ELEM_INT) {
                int value;
                column->get(row_idx, &value);

                OrdinalTreeNode<int>* node = (OrdinalTreeNode<int>*) temp;
                if(value >= node->split_value) {
                    temp = temp->right;
                } else {
                    temp = temp->left;
                }
            } else if(column->data_type == ELEM_FLOAT) {
                float value;
                column->get(row_idx, &value);

                OrdinalTreeNode<float>* node = (OrdinalTreeNode<float>*) temp;
                if(value >= node->split_value) {
                    temp = temp->right;
                } else {
                    temp = temp->left;
                }
            } else if(column->data_type == ELEM_DOUBLE) {
                double value;
                column->get(row_idx, &value);

                OrdinalTreeNode<double>* node = (OrdinalTreeNode<double>*) temp;
                if(value >= node->split_value) {
                    temp = temp->right;
                } else {
                    temp = temp->left;
                }
            } else if(column->data_type == ELEM_CHAR) {
                char value;
                column->get(row_idx, &value);

                OrdinalTreeNode<char>* node = (OrdinalTreeNode<char>*) temp;
                if(value >= node->split_value) {
                    temp = temp->right;
                } else {
                    temp = temp->left;
                }
            } else if(column->data_type == ELEM_STRING) {
                string value;
                column->get(row_idx, &value);

                OrdinalTreeNode<string>* node = (OrdinalTreeNode<string>*) temp;
                if(value >= node->split_value) {
                    temp = temp->right;
                } else {
                    temp = temp->left;
                }
            } else {
                cout<< "Line : " << __LINE__ << "[Error] Unknown data type : "
                    << column->data_type << endl;
                exit(-1);
            }
        } else {
            // TODO:: only considered string categorical value for now
            string value = column->string_value(row_idx);
            CategoricalTreeNode<string>* node = (CategoricalTreeNode<string>*) temp;

            if(node->S.find(value) == node->S.end()) {
                istringstream(node->label) >> predicted_y;
                cout<< "[DEBUG] value = " << value << " not found, return  max freq_label = "
                    << node->label << endl;
                return;
            }

            if(node->S1.find(value) == node->S1.end()) {
                temp = temp->right;
            } else {
                temp = temp->left;
            }

        }
    }

    assert(temp->column_index == -1);

    LeafNode<T>* leafNode = (LeafNode<T>*) temp;
    predicted_y = leafNode->label;
}

// input: Tree_Root, test_set, y
// output: predicted_y values
template <class T>
void predict(TreeNode *root, Matrix &test, vector<T> &predicted_ys) {
    int n_sample = test.get_column(0)->size;
    for(size_t i = 0; i < n_sample; i++) {
        T predicted_y;
        predict<T>(root, test, i, predicted_y);
        predicted_ys.push_back(predicted_y);
    }
}

// function to get accuracy
double get_accuracy(TreeNode* root, Matrix &test, int y_index) {
    Column* y = test.get_column(y_index);
    int total = y->size;
    int correct = 0;

    if(y->data_type == ELEM_BOOL) {
        vector<bool> predicted_ys;
        predict<bool>(root, test, predicted_ys);

        for(size_t i = 0; i < y->size; i++) {
            bool y_value;
            y->get(i, &y_value);
            if(predicted_ys[i] == y_value) {
                correct++;
            }
        }
    } else if(y->data_type == ELEM_SHORT) {
        vector<short> predicted_ys;
        predict<short>(root, test, predicted_ys);

        for(size_t i = 0; i < y->size; i++) {
            short y_value;
            y->get(i, &y_value);
            if(predicted_ys[i] == y_value) {
                correct++;
            }
        }
    } else if(y->data_type == ELEM_INT) {
        vector<int> predicted_ys;
        predict<int>(root, test, predicted_ys);

        for(size_t i = 0; i < y->size; i++) {
            int y_value;
            y->get(i, &y_value);
            if(predicted_ys[i] == y_value) {
                correct++;
            }
        }
    } else if(y->data_type == ELEM_FLOAT) {
        vector<float> predicted_ys;
        predict<float>(root, test, predicted_ys);

        for(size_t i = 0; i < y->size; i++) {
            float y_value;
            y->get(i, &y_value);
            if(predicted_ys[i] == y_value) {
                correct++;
            }
        }
    } else if(y->data_type == ELEM_DOUBLE) {
        vector<double> predicted_ys;
        predict<double>(root, test, predicted_ys);

        for(size_t i = 0; i < y->size; i++) {
            double y_value;
            y->get(i, &y_value);
            if(predicted_ys[i] == y_value) {
                correct++;
            }
        }
    } else if(y->data_type == ELEM_CHAR) {
        vector<char> predicted_ys;
        predict<char>(root, test, predicted_ys);

        for(size_t i = 0; i < y->size; i++) {
            bool y_value;
            y->get(i, &y_value);
            if(predicted_ys[i] == y_value) {
                correct++;
            }
        }
    } else if(y->data_type == ELEM_STRING) {
        vector<string> predicted_ys;
        predict<string>(root, test, predicted_ys);

        for(size_t i = 0; i < y->size; i++) {
            string y_value;
            y->get(i, &y_value);
            if(predicted_ys[i] == y_value) {
                correct++;
            }
        }
    }

    return (double)correct / y->size;
}

#endif