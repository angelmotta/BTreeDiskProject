#ifndef BTREEDISK_PAGE_H
#define BTREEDISK_PAGE_H
#include <vector>
#include <queue>
#include <iostream>
#include <utility>
#include "Record.h"
using namespace std;

template<typename T>
class BtreeIndex;

template <typename T>
class Page{
private:
    unsigned int t;     /* minDegree */
    unsigned int currentKeys;
    /* Pending change for Vector of Records Objects */
    vector<T> keys;    /* MaxCapacity = (2 * t -1) */
    vector<Record*> m_Records;
    /* Pending change this for */
    vector<Page<T>*> children;
    bool isLeaf;
public:
    Page(unsigned int t, bool isLeaf) : t(t), isLeaf(isLeaf) {
        keys.resize(2 * t - 1);    /* Capacity = (2*t - 1)*/
        children.resize(2 * t);
        currentKeys = 0;
    }

    // This->node is parent of child Y. This function Split child Y
    void splitChild(Page<T>* nodeY, int idx){
        auto nodeZ = new Page(nodeY->t, nodeY->isLeaf);
        nodeZ->currentKeys = t - 1;
        for(int i = 0; i < t - 1; i++){
            nodeZ->keys[i] = nodeY->keys[i+t];
        }
        if(!nodeY->isLeaf){
            for(int i = 0; i < t; i++){
                nodeZ->children[i] = nodeY->children[i+t];
            }
        }
        nodeY->currentKeys = t - 1;
        // Update child pointers of this->node (parent of Y)
        for(int i = currentKeys; i >= idx + 1; i--){
            this->children[i + 1] = this->children[i];
        }
        this->children[idx + 1] = nodeZ;
        // Update keys indexes of this->node
        for(int i = currentKeys - 1; i >= idx; i--){
            this->keys[i + 1] = this->keys[i];
        }
        // Move up the midterm key of y to this node
        this->keys[idx] = nodeY->keys[t-1];
        this->currentKeys++;
    }

    void insertNonFullNode(T keyValue){
        int keyIdx = currentKeys - 1;   // index of rightmost key in Node
        // If this node is Leaf, execute insertion
        if(this->isLeaf){
            while(keyIdx >= 0 && this->keys[keyIdx] > keyValue){
                this->keys[keyIdx + 1] = this->keys[keyIdx];
                keyIdx--;
            }
            this->keys[keyIdx + 1] = keyValue;
            this->currentKeys++;
        }
        else { // if not, go down (recursion) to the the leaf node
            while(keyIdx >= 0 && this->keys[keyIdx] > keyValue){
                keyIdx--;
            }
            // check if childNode found (keyIdx + 1) is Full
            if(this->children[keyIdx + 1]->currentKeys == 2 * t - 1 ){
                splitChild(children[keyIdx + 1], keyIdx + 1);
                // After split of child, check where go down
                if(keyValue > this->keys[keyIdx + 1]){
                    keyIdx++;
                }
            }
            this->children[keyIdx + 1]->insertNonFullNode(keyValue);
        }
    }

    void recorrerPages() {
        queue<pair<Page<T>*, int>> nextQueue;
        nextQueue.push(pair<Page<T>*, int>(this, 0));
        while (!nextQueue.empty()) {
            auto temp = nextQueue.front();
            nextQueue.pop();
            if(!temp.first) return; // If Page is nullptr return
            temp.first->printKeys(temp.second);
            for (int i = 0; i <= temp.first->currentKeys; i++) {
                nextQueue.push(pair<Page<T>*, int>(temp.first->children[i], temp.second + 1));
            }
            cout << endl;
        }
    }

    void printKeys(int level) {
        cout << "Tree level " << level << ": ";
        for (int i = 0; i < currentKeys; i++) {
            cout << keys[i] << " ";
        }
    }



    friend class BtreeIndex<T>;
};

#endif //BTREEDISK_PAGE_H
