#ifndef BTREEDISK_BTREEINDEX_H
#define BTREEDISK_BTREEINDEX_H
#include "Page.h"

template <typename T>
class BtreeIndex{
private:
    Page<T>* root;
    unsigned int minDegree;    /* maxRecords Capacity = (2 * minDegree) - 1  */
public:
    BtreeIndex(unsigned int minDegree) : minDegree(minDegree), root(nullptr) {};

    void insert(T keyValue){
        cout << "** Insert value: " << keyValue << "\n";
        if(root == nullptr){
            root = new Page<T>(minDegree, true);
            root->keys[0] = keyValue;
            root->currentKeys = 1;
        }
        else {
            // if root is full, create new root Node and split full node
            if(root->currentKeys == 2 * minDegree - 1){
                auto ptrRoot = root;
                auto newRoot = new Page<T>(minDegree, false);
                root = newRoot;
                // New Root node is now parent of the original old Root node
                newRoot->children[0] = ptrRoot;
                // Execute split staying from the New Root node
                newRoot->splitChild(ptrRoot, 0);
                // Select 1 of 2 children of root to have new key
                int idx = 0;
                if(keyValue > root->keys[0]){
                    idx++;
                }
                newRoot->children[idx]->insertNonFullNode(keyValue);
            }
            else {
                root->insertNonFullNode(keyValue);
            }
        }
    }

    void print() {
        cout << "** Print BTree Index **\n";
        if(root) root->recorrerPages();
        cout << '\n';
    }
};

#endif //BTREEDISK_BTREEINDEX_H
