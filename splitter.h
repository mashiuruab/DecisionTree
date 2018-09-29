#ifndef SPLITTER_H
#define SPLITTER_H

#include "criterion.h"

using namespace std;

// 1. design sketch
// 2. impurity(sketch) = score
// 3. split based on best score

// #################### column-split #######################
// -------- for regression --------

double impurity_improvement_fn(double left, double right, size_t n_l, size_t n_r);

struct category_item { //each X-label
    string category; //string used for any Xi-category type
    double sum_1 = 0.0; // this is like sum_1(var_sketch)
    double sum = 0.0; //double for any y-numerical type, like sum_y (var_sketch)
    double average; //!!! needs to be manually set after sum is finalized
    double sum_y2 = 0.0; // calculating (sum * sum)  for variance calculation later
    vector<size_t> indices; //  row indices of "category"
};

struct Category_comparator //for sorting X-labels by y-average
{
    bool operator()(const category_item item1, const category_item item2)
    {
        return item1.average < item2.average;
    }
};


struct var_sketch //sketch for variance
{//motivated by Google's PLANET paper
    double sum_y = 0.0;
    double sum_y2 = 0.0;
    double sum_1 = 0.0;
};

double var(var_sketch & sketch)
{//compute variance from sketch
    double MSS = (double) sketch.sum_y2 / sketch.sum_1; //mean sum of square: E[x^2]
    double mean = (double) sketch.sum_y / sketch.sum_1; //E[x]

    double mean2 = mean * mean; //square of mean

    return MSS - mean2;
}

template<class T>
void set_sketch(var_sketch & sketch, vector<size_t>::iterator start,
                vector<size_t>::iterator end, Column* y)
{//compute sketch over rows [start, end), using labels y[row_idx]
    sketch.sum_y = 0.0;
    sketch.sum_y2 = 0.0;
    sketch.sum_1 = 0.0;

    for(auto it = start; it != end; it++) {
        T value;
        y->get(*it, &value);

        sketch.sum_y += value;
        sketch.sum_y2 += (value * value);
        sketch.sum_1 += 1;
    }

}

template <class T>
void update_sketches(var_sketch & sketch_L, var_sketch & sketch_R, T value) //T should be numeric
{//incrementally update left and right child-nodes' sketches,
    //after moving "value" from right child to left child
    sketch_L.sum_y += value;
    sketch_L.sum_y2 += (value * value);
    sketch_L.sum_1 += 1;

    sketch_R.sum_y -= value;
    sketch_R.sum_y2 -= (value * value);
    sketch_R.sum_1 -= 1;
}

// for batch sketch update for categorical Xi
void update_sketches(var_sketch & sketch_L, var_sketch & sketch_R, category_item value)
{//incrementally update left and right child-nodes' batch sketches for  categorical data,
    //after moving "value" from right child to left child
    sketch_L.sum_y += value.sum;
    sketch_L.sum_y2 += value.sum_y2;
    sketch_L.sum_1 += value.sum_1;

    sketch_R.sum_y -= value.sum;
    sketch_R.sum_y2 -= value.sum_y2;
    sketch_R.sum_1 -= value.sum_1;
}

// #################### column-split for classification #######################

template <class T>
class classify_sketch {
public:
    map<T, size_t> freq; //freq[class] = # of times the class occurred
    size_t N = 0; //total number of elements
    //prob_class = freq[class] / N, used for computing impurity

    void add(T key) {
        auto it = freq.find(key);

        if(it == freq.end()) freq[key] = 1;
        else it->second++;

        N++;
    }

    void remove(T key) {
        auto it = freq.find(key);

        assert(it != freq.end());

        assert(it->second > 0);
        it->second--;

        if(it->second == 0) freq.erase(it);
        N--;
    }

    void add(map<string, size_t> &f_y) {
        for(auto it = f_y.begin(); it != f_y.end(); it++) {
            T key;
            istringstream(it->first) >> key;
            auto it2 = freq.find(key);

            if(it2 == freq.end()) {
                freq[key] = it->second;
            } else {
                it2->second += it->second;
            }

            N += it->second;
        }
    }

    void remove(map<string, size_t> &f_y) {
        for(auto it = f_y.begin(); it != f_y.end(); it++) {
            T key;
            istringstream(it->first) >> key;
            auto it2 = freq.find(key);

            assert(it2 != freq.end());
            it2->second -= it->second;

            assert(it2->second >= 0);
            if(it2->second == 0) {
                freq.erase(it2);
            }

            N -= it->second;
        }
    }

    void set_ratio(vector<double> &target_ratios) {
        for(auto it = freq.begin(); it != freq.end(); it++) {
            double ratio = (double) it->second / N;
            target_ratios.push_back(ratio);
        }
    }

};

template <class T>
void set_sketch(classify_sketch<T> &sketch, vector<size_t>::iterator start,
                vector<size_t>::iterator end, Column* y)
{//compute sketch over rows [start, end), using labels y[row_idx]
    for(auto it = start; it != end; it++) {
        T value;
        y->get(*it, &value);
        sketch.add(value);
    }
}

template <class T>
void update_sketches(classify_sketch<T> &sketch_L, classify_sketch<T> &sketch_R, T value) //T should be numeric
{//incrementally update left and right child-nodes' sketches,
    //after moving "value" from right child to left child
    sketch_L.add(value);
    sketch_R.remove(value);
}

// #################### brute-force #######################

// category item for brute force algorithm
struct Item {
    string category;
    vector<size_t> indices; // row indices
    map<string, size_t> freq_y; // y frequency
};


// updating the iterator [start, end) in order vector<Item> &items
// helper function to update original iterator indices [start, end)
void update_iterator(vector<size_t>::iterator start, vector<Item> &items) {
    // updating indices from [start, end) based on sorted order
    int offset = 0;

    for(int i = 0; i < items.size(); i++) {
        vector<size_t> &indices = items[i].indices;
        std::copy(indices.begin(), indices.end(), start + offset);
        offset += indices.size(); //  (end  - begin)
    }
}

// scan [start, end) to group items by Xi-label
// return a item vector "ordered_items" for enumerating S1
// "ordered_items" can be a vector of labels of any order: (Apple, Banana, ..) or (Banana, Apple,...)
bool set_grouped_item(vector<size_t>::iterator start, vector<size_t>::iterator end,
             Column* Xi, Column* Y, vector<Item> &ordered_items) {
    // return is_skip? true to skip splitting Xi
    map<string, Item> category_map; // collecting unique categories

    for(auto it = start; it != end; it++) {
        string x_value = Xi->string_value(*it);
        string y_value = Y->string_value(*it);

        auto it2 = category_map.find(x_value);

        if(it2 == category_map.end()) {
            if(category_map.size() >= BRUTE_FORCE_MAX_ITEM) {
                cout<< "[Warning] (Xi Categorical, Y Categorical), |category_item| = "
                    << category_map.size() << " exceeds the limit " << BRUTE_FORCE_MAX_ITEM << endl;
                return true;
            }

            Item item;
            item.category = x_value;
            item.indices.push_back(*it);
            item.freq_y[y_value] = 1;
            category_map[x_value] = item;
        } else {
            map<string, size_t> &freq_y = it2->second.freq_y;

            // creating y_freq map for each category
            auto it3 = freq_y.find(y_value);
            if(it3 == freq_y.end()) {
                freq_y[y_value] = 1;
            } else {
                it3->second++;
            }

            it2->second.indices.push_back(*it);
        }
    }

    // collecting category items in a vector
    for(auto it = category_map.begin(); it != category_map.end(); it++) {
        ordered_items.push_back(it->second);
    }

    return false;
}

// helper function called from get_impurity_improvement(..)
// to calculate impurity based on Y data-type
template <class T>
double impurity_improvement(vector<Item> &ordered_items,
                                vector<size_t> &s1, Column* Y) {
    //output:
    //impurity_improvement

    classify_sketch<T> left;
    classify_sketch<T> right;

    vector<bool> isS2(ordered_items.size(), true);

    for(size_t i = 0; i < s1.size(); i++) {
        left.add(ordered_items[s1[i]].freq_y);
        isS2[s1[i]] = false;
    }

    for(size_t i = 0; i < isS2.size(); i++) {
        if(isS2[i]) {
            right.add(ordered_items[i].freq_y);
        }
    }

    //compute pi
    vector<double> left_tgt_ratio;
    vector<double> right_tgt_ratio;
    left.set_ratio(left_tgt_ratio);
    right.set_ratio(right_tgt_ratio);

    double left_impurity;
    double right_impurity;
    if (IMPURITY_FUNC == IMPURITY_ENTROPY) {
        left_impurity = entropy(left_tgt_ratio);
        right_impurity = entropy(right_tgt_ratio);
    } else if (IMPURITY_FUNC == IMPURITY_GINI) {
        left_impurity = gini(left_tgt_ratio);
        right_impurity = gini(right_tgt_ratio);
    } else if (IMPURITY_FUNC == IMPURITY_CLASSIFICATION_ERROR) {
        left_impurity = classification_error(left_tgt_ratio);
        right_impurity = classification_error(right_tgt_ratio);
    } else {
        cout<< "[ERROR] for classification we only consider entropy, "
                "gini and classification error as impurity function " << endl;
        exit(-1);
    }

    return impurity_improvement_fn(left_impurity, right_impurity, left.N, right.N);
}

// calculate impurity improvement for a particular combination |S1| and |S-S1|
double get_impurity_improvement(vector<Item> &ordered_items,
                                vector<size_t> &s1, Column* Y) {
    int type = Y->data_type;

    if(type == ELEM_BOOL) {
        return impurity_improvement<bool>(ordered_items, s1, Y);
    } else if(type == ELEM_SHORT) {
        return impurity_improvement<short>(ordered_items, s1, Y);
    } else if(type == ELEM_INT) {
        return impurity_improvement<int>(ordered_items, s1, Y);
    } else if(type == ELEM_FLOAT) {
        return impurity_improvement<float>(ordered_items, s1, Y);
    } else if(type == ELEM_DOUBLE) {
        return impurity_improvement<double>(ordered_items, s1, Y);
    } else if(type == ELEM_CHAR) {
        return impurity_improvement<char>(ordered_items, s1, Y);
    } else if(type == ELEM_STRING) {
        return impurity_improvement<string>(ordered_items, s1, Y);
    }
}

// helper function to calculate the offset from ordered_items and S1
size_t get_offset(vector<Item> &ordered_items, vector<size_t> &S1) {
    size_t offset = 0;

    for(size_t i = 0; i < S1.size(); i++) {
        offset += ordered_items[S1[i]].indices.size();
    }

    return offset;
}

// helper function of find_best_split_enum()
void do_enumerate(size_t curr_val, vector<size_t> &temp_s1,
                  vector<Item> &ordered_items, Column* Y,
                  double &best_impurity_improvement, size_t &best_offset, vector<size_t> &best_S1) {
    //N: # of different Xi values
    //curr_val: latest (largest) element in enumerating set S1
    //temp_s1: current elements in S1

    // k = 1 .. floor(n/2), n is odd (no duplicate), n even (duplicate)
    size_t k_range = ordered_items.size() / 2;

    if(temp_s1.size() > 0)
        if((ordered_items.size() % 2 == 0) && (temp_s1.size() == k_range - 1) && (temp_s1[0] > 0))
            return;

    temp_s1.push_back(curr_val);

    double impurity = get_impurity_improvement(ordered_items, temp_s1, Y);
    if(impurity > best_impurity_improvement) {
        best_impurity_improvement = impurity;
        best_S1 = temp_s1;
        best_offset = get_offset(ordered_items, temp_s1);
    }

    if(temp_s1.size() < k_range) {
        for(int i = curr_val + 1; i < ordered_items.size(); i++) {
            do_enumerate(i, temp_s1, ordered_items, Y, best_impurity_improvement, best_offset, best_S1);
        }
    }

    temp_s1.pop_back();
}

// param: number of unique item in vector<Item>
// param: Column y
// output: best_impurity_improvement, best_offset, best_S1
void find_best_split_enum(vector<Item> &ordered_items, Column* Y,
                                   double &best_impurity_improvement, size_t &best_offset,
                                   vector<size_t>  &best_S1) {
    int N = ordered_items.size();

    if(N <= 1) {
        cout << "[ERROR] Line : " << __LINE__ << " N should be greater than 1, current N = " << N << endl;
        exit(-1);
    }

    best_impurity_improvement = -DBL_MAX;

    vector<size_t> temp_s1;// tree path stack

    for(size_t i = 0; i < N; i++) {
        do_enumerate(i, temp_s1, ordered_items, Y, best_impurity_improvement,
                     best_offset, best_S1); //i-root prefix tree enumeration: i must be in S1
    }
}

// updating the vector<Item> &ordered_item with the new S1
// the best S1 found while doing brute force impurity calculation
void update_category_order(vector<size_t>::iterator start, vector<Item> &ordered_item,
                           vector<size_t> &S1) {

    vector<Item> new_order;
    vector<bool> isS2(ordered_item.size(), true);

    for(size_t i = 0; i < S1.size(); i++) {
        Item &item = ordered_item[S1[i]];
        new_order.push_back(item);
        isS2[S1[i]] = false;
    }

    for(size_t i = 0; i < isS2.size(); i++) {
        if(isS2[i]) {
            Item &item = ordered_item[i];
            new_order.push_back(item);
        }
    }

    update_iterator(start, new_order);
}

// #################### node-split #######################

class SplitResult {
public:
    // column (attribute) index to split on
    size_t column_idx;
    // position (+ 1) in the sample index array to split for column_idx
    vector<size_t>::iterator pos;
};

template <class T>
class OrdinalSplitResult : public SplitResult {
public:
    // attribute value on which to split column left and right
    T attribute_value;
    OrdinalSplitResult(){}
};

template <class T>
class CategoricalSplitResult : public SplitResult {
public:
    unordered_set<T> S; // unique categories between [start, end)
    unordered_set<T> S1;
};

void free(SplitResult* sr, Matrix &data_set) {
    Column* column = data_set.get_column(sr->column_idx);
    int type = column->data_type;
    bool is_ordinal = column->is_ordinal;

    if(is_ordinal) {
        if(type == ELEM_BOOL) {
            OrdinalSplitResult<bool>* result = (OrdinalSplitResult<bool>*) sr;
            delete result;
        } else if(type == ELEM_SHORT) {
            OrdinalSplitResult<short>* result = (OrdinalSplitResult<short>*) sr;
            delete result;
        } else if(type == ELEM_INT) {
            OrdinalSplitResult<int>* result = (OrdinalSplitResult<int>*) sr;
            delete result;
        } else if(type == ELEM_FLOAT) {
            OrdinalSplitResult<float>* result = (OrdinalSplitResult<float>*) sr;
            delete result;
        } else if(type == ELEM_DOUBLE) {
            OrdinalSplitResult<double>* result = (OrdinalSplitResult<double>*) sr;
            delete result;
        } else if(type == ELEM_CHAR) {
            OrdinalSplitResult<char>* result = (OrdinalSplitResult<char>*) sr;
            delete result;
        } else if(type == ELEM_STRING) {
            OrdinalSplitResult<string>* result = (OrdinalSplitResult<string>*) sr;
            delete result;
        }
    } else {
        cout<< "[ERROR] we don't consider categorical X data for node_split now " << endl;
        exit(-1);
    }
}

double impurity_improvement_fn(double left, double right, size_t n_l, size_t n_r) {
    return (- left * n_l - right * n_r);
}

// find the best position for a column
// !!! sort [start, end) first !!! X,Y both continuous (ordinal)
template <class T>
void set_best_pos_classification(vector<size_t>::iterator start, vector<size_t>::iterator end, //row index range
                                       Column* Y, double &best_impurity_improvement, size_t &best_offset) {
    //output:
    //best_impurity_improvement
    //best offest: the chosen split position from "start"

    best_impurity_improvement = -DBL_MAX;

    classify_sketch<T> left;
    classify_sketch<T> right;

    set_sketch<T>(right, start, end, Y);

    for (auto it = start; it < end - 1; it++) {
        T value;
        Y->get(*it, &value);
        update_sketches<T>(left, right, value); //update with the current element

        //compute pi
        vector<double> left_tgt_ratio;
        vector<double> right_tgt_ratio;
        left.set_ratio(left_tgt_ratio);
        right.set_ratio(right_tgt_ratio);

        double left_impurity;
        double right_impurity;
        if (IMPURITY_FUNC == IMPURITY_ENTROPY) {
            left_impurity = entropy(left_tgt_ratio);
            right_impurity = entropy(right_tgt_ratio);
        } else if (IMPURITY_FUNC == IMPURITY_GINI) {
            left_impurity = gini(left_tgt_ratio);
            right_impurity = gini(right_tgt_ratio);
        } else if (IMPURITY_FUNC == IMPURITY_CLASSIFICATION_ERROR) {
            left_impurity = classification_error(left_tgt_ratio);
            right_impurity = classification_error(right_tgt_ratio);
        } else {
            cout<< "[ERROR] for classification we only consider entropy, "
                    "gini and classification error as impurity function " << endl;
            exit(-1);
        }

        double impurity_improvement = impurity_improvement_fn(left_impurity, right_impurity, left.N, right.N);

        if (best_impurity_improvement < impurity_improvement) {
            best_impurity_improvement = impurity_improvement;
            best_offset = it - start;
        }
    }

}

// find the best position for a column
// !!! sort [start, end) first !!! X,Y both continuous (ordinal)
template <class T>
void set_best_pos_regression(vector<size_t>::iterator start, vector<size_t>::iterator end, //row index range
                                 Column* Y, double &best_impurity_improvement, size_t &best_offset) {
    //output:
    //best_impurity_improvement
    //best offest: the chosen split position from "start"
    best_impurity_improvement = -DBL_MAX;

    var_sketch left;
    var_sketch right;

    set_sketch<T>(right, start, end, Y);

    for (auto it = start; it < end - 1; it++) {
        T value;
        Y->get(*it, &value);
        update_sketches<T>(left, right, value); //update with the current element

        if(IMPURITY_FUNC == IMPURITY_VARIANCE){
            double var_L = var(left);
            double var_R = var(right);

            double impurity_improvement = impurity_improvement_fn(var_L, var_R, left.sum_1, right.sum_1);

            if (best_impurity_improvement < impurity_improvement) {
                best_impurity_improvement = impurity_improvement;
                best_offset = it - start;
            }
        } else {
            cout<< "[ERROR] Currently for regression we don't consider any impurity func other than variance" << endl;
            exit(-1);
        }

    }

}


// find the best position for a column (categorical value)
// !!! sort [start, end) first !!! Xi categorical, Y continuous (ordinal)
// need to update sketch on batch
template <class T>
void set_best_pos_regression_breiman(vector<size_t>::iterator start, vector<size_t>::iterator end, //row index range
                             Column* Xi, Column* Y, vector<category_item> &category_items,
                                     double &best_impurity_improvement, size_t &best_offset,
                                     vector<size_t> &best_S1) {
    //output:
    //best_impurity_improvement
    //best offest: the chosen split position from "start"

    best_impurity_improvement = -DBL_MAX;

    var_sketch left;
    var_sketch right;
    set_sketch<T>(right, start, end, Y); // template type of Y

    int total_indices = 0;
    vector<size_t> S1;

    for(size_t i = 0; i < category_items.size(); i++) {
        S1.push_back(i);
        update_sketches(left, right, category_items[i]);
        total_indices += category_items[i].indices.size();

        if(IMPURITY_FUNC == IMPURITY_VARIANCE){
            double var_L = var(left);
            double var_R = var(right);

            double impurity_improvement = impurity_improvement_fn(var_L, var_R, left.sum_1, right.sum_1);

            if (best_impurity_improvement < impurity_improvement) {
                best_impurity_improvement = impurity_improvement;
                best_offset = total_indices;// (end - start)
                best_S1 = S1;
            }
        } else {
            cout<< "[ERROR:Breiman] Currently for regression we don't consider any impurity func other than variance" << endl;
            exit(-1);
        }
    }

}

void set_sorted_category_item(vector<size_t>::iterator start,
                              vector<size_t>::iterator end, Column* Xi, Column* Y,
                              vector<category_item> &category_items) {

    map<string, category_item> item_map;

    for(auto it = start; it < end; it++) { //scan data to get X-label groups and their y-avg's accumulating values
        string attr_val = Xi->string_value(*it);
        double y_value;

        if(Y->data_type == ELEM_SHORT) {
            short value;
            Y->get(*it, &value);
            y_value = value;
        } else if(Y->data_type == ELEM_INT) {
            int value;
            Y->get(*it, &value);
            y_value = value;
        } else if(Y->data_type == ELEM_FLOAT) {
            float value;
            Y->get(*it, &value);
            y_value = value;
        } else if(Y->data_type == ELEM_DOUBLE) {
            Y->get(*it, &y_value);
        }

        auto it2 = item_map.find(attr_val);

        if(it2 == item_map.end()) {
            category_item ct;
            ct.category = attr_val;
            ct.sum_1 += 1;
            ct.sum += y_value;
            ct.sum_y2 += y_value * y_value;
            ct.indices.push_back(*it);
            item_map[attr_val] = ct;

        } else {
            it2->second.sum_1++;
            it2->second.sum += y_value;
            it2->second.sum_y2 += y_value * y_value;
            it2->second.indices.push_back(*it);
        }

    }
    //compute y-avg for each x-group
    for(auto it = item_map.begin(); it != item_map.end(); it++){
        it->second.average = it->second.sum / it->second.sum_1;
        //cout<< "key = " << it->second.category << ", avg = " << it->second.average << endl;
        category_items.push_back(it->second);
    }
    //sort the X-groups by y-avg
    Category_comparator ctg_comparator;
    std::sort(category_items.begin(), category_items.end(), ctg_comparator);
    //=================================
}

// setting the sorted indices (sorted based on category sort) from  [start, end)
void set_sorted_indices(vector<size_t>::iterator start, vector<category_item> &category_items) {
    int offset = 0;

    for(int i = 0; i < category_items.size(); i++) {
        vector<size_t> &indices = category_items[i].indices;
        std::copy(indices.begin(), indices.end(), start + offset);
        offset += indices.size(); //  (end  - begin)
    }

}

// wrapper function to do the sorting
// we need vector<category_item> &category_items in other parts of breiman's algorithm
void row_sort_categorical(vector<size_t>::iterator start, vector<size_t>::iterator end,
                          Column* Xi, Column* Y, vector<category_item> &category_items){
    set_sorted_category_item(start, end, Xi, Y, category_items);
    set_sorted_indices(start, category_items);
}

//form the final result after all columns are checked
//!!! return value needs to be deleted outside !!!
template <class T>
SplitResult* create_split_result(size_t best_col_idx, Column* best_Xi, Column* Y,
                                 vector<size_t>::iterator start, vector<size_t>::iterator end,
                                 size_t best_offset, vector<size_t> &best_S1) {
    //shuffle back row_idx array for recursive split later

    if(best_Xi->is_ordinal) {
        best_Xi->row_sort(start, end);
        OrdinalSplitResult<T>* ordinalSplitResult = new OrdinalSplitResult<T>();
        ordinalSplitResult->pos = start + best_offset + 1;
        ordinalSplitResult->column_idx = best_col_idx;

        T value;
        best_Xi->get(*(ordinalSplitResult->pos), &value);
        ordinalSplitResult->attribute_value = value;

        return ordinalSplitResult;
    } else if (Y->is_ordinal) { // breiman's algorithm Xi categorical, Y ordinal (continuous)
        vector<category_item> category_items;
        row_sort_categorical(start, end, best_Xi, Y, category_items);
        CategoricalSplitResult<T>* splitResult = new CategoricalSplitResult<T>();
        splitResult->pos = start + best_offset;
        splitResult->column_idx = best_col_idx;

        for(size_t i = 0; i < best_S1.size(); i++) {
            T value;
            istringstream(category_items[best_S1[i]].category) >> value;
            splitResult->S1.insert(value);
        }

        for(size_t i = 0; i < category_items.size(); i++) {
            T value;
            istringstream(category_items[i].category) >> value;
            splitResult->S.insert(value);
        }

        return splitResult;
    } else if (!Y->is_ordinal) { // best_Xi categorical and Y categorical brute force algorithm
        // the following 3 lines are part of
        // ordering the data in best_S1
        vector<Item> ordered_items;
        // here we don't need to consider return value, it is taken care off before coming here
        set_grouped_item(start, end, best_Xi, Y, ordered_items);
        update_category_order(start, ordered_items, best_S1);

        // now we can use best offset
        CategoricalSplitResult<T>* splitResult = new CategoricalSplitResult<T>();
        splitResult->pos = start + best_offset;
        splitResult->column_idx = best_col_idx;

        for(size_t i = 0; i < best_S1.size(); i++) {
            T value;
            istringstream(ordered_items[best_S1[i]].category) >> value;
            splitResult->S1.insert(value);
        }

        for(size_t i = 0; i < ordered_items.size(); i++) {
            T value;
            istringstream(ordered_items[i].category) >> value;
            splitResult->S.insert(value);
        }

        return splitResult;
    }
}

SplitResult* node_split(vector<size_t>::iterator start, vector<size_t>::iterator end,
                 Matrix &data_set, size_t y_index) {

    Column* y = data_set.get_column(y_index);

    if((IMPURITY_FUNC == IMPURITY_VARIANCE)
       && !(y->data_type >= ELEM_SHORT && y->data_type <= ELEM_DOUBLE)) {
        cout<< "[ERROR] output column should be numeric for regression " << endl;
        exit(-1);
    }

    double best_impurity_improvement = -DBL_MAX;
    size_t best_offset;
    size_t best_col_idx;
    vector<size_t> best_s1;// Only need at X categorical


    for(size_t col_idx = 0; col_idx < data_set.size - 1; col_idx++) {
        if(col_idx == y_index) continue; // skipping target_column from splitting

        //sort row-idx array based on Xi
        Column* Xi = data_set.get_column(col_idx);
        int type = y->data_type;

        double impurity_improvement = -DBL_MAX;
        vector<size_t> S1;//// X categorical
        size_t offset;

        if(Xi->is_ordinal)
        {
            Xi->row_sort(start, end);

            if((IMPURITY_FUNC == IMPURITY_ENTROPY) || (IMPURITY_FUNC == IMPURITY_GINI) || (IMPURITY_FUNC == IMPURITY_CLASSIFICATION_ERROR)) {
                //---
                if(type == ELEM_BOOL) {
                    set_best_pos_classification<bool>(start, end, y, impurity_improvement, offset);
                } else if(type == ELEM_SHORT) {
                    set_best_pos_classification<short>(start, end, y, impurity_improvement, offset);
                } else if(type == ELEM_INT) {
                    set_best_pos_classification<int>(start, end, y, impurity_improvement, offset);
                } else if(type == ELEM_FLOAT) {
                    set_best_pos_classification<float>(start, end, y, impurity_improvement, offset);
                } else if(type == ELEM_DOUBLE) {
                    set_best_pos_classification<double>(start, end, y, impurity_improvement, offset);
                } else if(type == ELEM_CHAR) {
                    set_best_pos_classification<char>(start, end, y, impurity_improvement, offset);
                } else if(type == ELEM_STRING) {
                    set_best_pos_classification<string>(start, end, y, impurity_improvement, offset);
                }

            } else if(IMPURITY_FUNC == IMPURITY_VARIANCE) {

                if(type == ELEM_SHORT) {
                    set_best_pos_regression<short>(start, end, y, impurity_improvement, offset);
                } else if(type == ELEM_INT) {
                    set_best_pos_regression<int>(start, end, y, impurity_improvement, offset);
                } else if(type == ELEM_FLOAT) {
                    set_best_pos_regression<float>(start, end, y, impurity_improvement, offset);
                } else if(type == ELEM_DOUBLE) {
                    set_best_pos_regression<double>(start, end, y, impurity_improvement, offset);
                }

            } else {
                cout<<"Line "<<__LINE__<<": [ERROR] Currently we have only Entropy, Gini, Classification Error (for classification) and Variance (for regression) as impurity function" << endl;
                exit(-1);
            }

            //---
            if(impurity_improvement > best_impurity_improvement)
            {
                best_impurity_improvement = impurity_improvement;
                best_col_idx = col_idx;
                best_offset = offset;
            }

        } else if (y->is_ordinal) { // is Xi is categorical, Y ordinal, Breiman's alogrithm

            if(IMPURITY_FUNC == IMPURITY_VARIANCE) {

                vector<category_item> category_items; // sorted category items (of Xi) with indices
                row_sort_categorical(start, end, Xi, y, category_items);

                if(type == ELEM_SHORT) {
                    set_best_pos_regression_breiman<short>(start, end, Xi, y, category_items,
                                                           impurity_improvement, offset, S1);
                } else if(type == ELEM_INT) {
                    set_best_pos_regression_breiman<int>(start, end, Xi, y, category_items,
                                                         impurity_improvement, offset, S1);
                } else if(type == ELEM_FLOAT) {
                    set_best_pos_regression_breiman<float>(start, end, Xi, y, category_items,
                                                           impurity_improvement, offset, S1);
                } else if(type == ELEM_DOUBLE) {
                    set_best_pos_regression_breiman<double>(start, end, Xi, y, category_items,
                                                            impurity_improvement, offset, S1);
                }

                //---
                if(impurity_improvement > best_impurity_improvement)
                {
                    best_impurity_improvement = impurity_improvement;
                    best_col_idx = col_idx;
                    best_offset = offset;
                    best_s1 = S1;
                }


            } else {
                cout<<"[ERROR:Breiman] Currently we have only Variance (for regression) as impurity function" << endl;
                exit(-1);
            }

        } else if (!y->is_ordinal) { // Xi is categorical, Y categorical, Brute force classification
            vector<Item> ordered_items;
            bool skip_Xi = set_grouped_item(start, end, Xi, y, ordered_items);

            if(skip_Xi) {
                cout<< "[Warning] Skip the categorical feature with index = " << col_idx << " (too many items to enumerate)" << endl;
                continue;
            }

            if(ordered_items.size() == 1) {
                cout<< "[Warning] Skip the categorical feature with index = " << col_idx << " (only 1 items, nothing to split)" << endl;
                continue;
            }
            if((IMPURITY_FUNC == IMPURITY_ENTROPY) || (IMPURITY_FUNC == IMPURITY_GINI) || (IMPURITY_FUNC == IMPURITY_CLASSIFICATION_ERROR)) {
                //---
                find_best_split_enum(ordered_items, y, impurity_improvement, offset, S1);

                if(impurity_improvement > best_impurity_improvement) {
                    best_impurity_improvement = impurity_improvement;
                    best_col_idx = col_idx;
                    best_offset = offset;
                    best_s1 = S1;
                }

            } else {
                cout<< "Line " << __LINE__<< ": [ERROR] Currently we have only Entropy, Gini, Classification Error as impurity function for classification (Xi is categorical)" << endl;
                exit(-1);
            }
        }

    }


    Column* best_Xi = data_set.get_column(best_col_idx);
    int data_type = best_Xi->data_type;

    if(data_type == ELEM_BOOL) {
        return create_split_result<bool>(best_col_idx, best_Xi, y, start, end,
                                         best_offset, best_s1);
    } else if(data_type == ELEM_SHORT) {
        return create_split_result<short>(best_col_idx, best_Xi, y, start, end,
                                          best_offset, best_s1);
    } else if(data_type == ELEM_INT) {
        return create_split_result<int>(best_col_idx, best_Xi, y, start, end,
                                        best_offset, best_s1);
    } else if(data_type == ELEM_FLOAT) {
        return create_split_result<float>(best_col_idx, best_Xi, y, start, end,
                                          best_offset, best_s1);
    } else if(data_type == ELEM_DOUBLE) {
        return create_split_result<double>(best_col_idx, best_Xi, y, start, end,
                                           best_offset, best_s1);
    } else if(data_type == ELEM_CHAR) {
        return create_split_result<char>(best_col_idx, best_Xi, y, start, end,
                                         best_offset, best_s1);
    } else if(data_type == ELEM_STRING) {
        return create_split_result<string>(best_col_idx, best_Xi, y, start, end,
                                           best_offset, best_s1);
    }
}

#endif