#ifndef RANDOM_CHOOSER_2_HPP
#define RANDOM_CHOOSER_2_HPP

#include <cassert>
#include <random>
#include <vector>

#define INNER 0
#define LEAF 1

// TODO unit tests

template <typename T>
class SumTree {
private:
    std::default_random_engine gen;
    typedef struct _node {
        _node * parent;
        double weight;
        union {
            struct {
                _node * left;
                _node * right;
            } inner;
            struct {
                _node * next;
                double coef;
                T data;
            } leaf;
        } u;
        int type;
    } Node;
    Node * _root;
    Node * _last_leaf;
    void update(Node * leaf, double value);
    Node * find(Node * node, double rest);
    double reset_node(Node * node);
    void free_node(Node * node);

public:
    SumTree() : _root(nullptr), _last_leaf(nullptr) {}
    SumTree(int seed) : _root(nullptr), _last_leaf(nullptr) { gen.seed(seed); }
    ~SumTree() {
        if(isEmpty()) return;
        free_node(_root);
        _root = nullptr;
    }
    int isEmpty() { return _root == nullptr; }
    int canPick() { return isEmpty() ? false : (_root->weight > 0.0); }
    void add(T val, double coef);
    T pick_and_reset();
    T pick();
    void reset();

    void print_node(Node * node, int depth);
    void print();
};

template <typename T>
void SumTree<T>::update(Node * leaf, double value) {
    leaf->weight += value;
    for(Node * node = leaf->parent; node != nullptr; node = node->parent)
        node->weight += value;
}

template <typename T>
void SumTree<T>::add(T item, double coef) {
    if(coef == 0.0) return;

    Node * new_leaf = new Node;
    new_leaf->weight = 0;
    new_leaf->u.leaf.coef = coef;
    new_leaf->u.leaf.data = item;
    new_leaf->type = LEAF;

    if(isEmpty()) {
        new_leaf->parent = nullptr;
        _root = _last_leaf = new_leaf->u.leaf.next = new_leaf;
        update(new_leaf, coef);
        return;
    }

    Node * new_inner = new Node;
    if(_last_leaf->parent == nullptr)
        _root = new_inner;
    else {
        if(_last_leaf == _last_leaf->parent->u.inner.left)
            _last_leaf->parent->u.inner.left = new_inner;
        else
            _last_leaf->parent->u.inner.right = new_inner;
    }

    new_inner->parent = _last_leaf->parent;
    new_inner->weight = _last_leaf->weight;
    new_inner->u.inner.left = _last_leaf;
    _last_leaf->parent = new_inner;
    new_inner->u.inner.right = new_leaf;
    new_leaf->parent = new_inner;
    new_inner->type = INNER;

    new_leaf->u.leaf.next = _last_leaf->u.leaf.next;
    _last_leaf->u.leaf.next = new_leaf;

    _last_leaf = new_leaf->u.leaf.next;

    update(new_leaf, coef);
}

template <typename T>
SumTree<T>::Node * SumTree<T>::find(Node * node, double rest) {
    if(node->type == LEAF) return node;
    const double left_weight = node->u.inner.left->weight;
    if(rest < left_weight) return find(node->u.inner.left, rest);
    return find(node->u.inner.right, rest - left_weight);
}

template <typename T>
T SumTree<T>::pick_and_reset() {
    assert(canPick());
    std::uniform_real_distribution<> dis(0, _root->weight);
    double val = dis(gen);
    return find(_root, val)->u.leaf.data;
}

template <typename T>
T SumTree<T>::pick() {
    assert(canPick());
    std::uniform_real_distribution<> dis(0, _root->weight);
    double val = dis(gen);

    Node * picked_node = find(_root, val);
    update(picked_node, -picked_node->weight);
    return picked_node->u.leaf.data;
}

template <typename T>
double SumTree<T>::reset_node(Node * node) {
    if(node->type == LEAF) return node->weight = node->u.leaf.coef;
    const double sum_left = reset_node(node->u.inner.left);
    const double sum_right = reset_node(node->u.inner.right);
    return node->weight = sum_left + sum_right;
}

template <typename T>
void SumTree<T>::reset() {
    if(isEmpty()) return;
    reset_node(_root);
}

template <typename T>
void SumTree<T>::free_node(Node * node) {
    if(node->type == INNER) {
        free_node(node->u.inner.left);
        free_node(node->u.inner.right);
    }
    delete node;
}

template <typename T>
void SumTree<T>::print_node(Node * node, int depth) {
    if(node->type == INNER) {
        std::cout << "(";
        print_node(node->u.inner.left, depth + 1);
        std::cout << ",";
        print_node(node->u.inner.right, depth + 1);
        std::cout << ")";
        return;
    }
    std::cout << node->u.leaf.data;
}
template <typename T>
void SumTree<T>::print() {
    if(isEmpty()) return;
    print_node(_root, 0);
    std::cout << std::endl;
}

#endif  // RANDOM_CHOOSER_2_HPP