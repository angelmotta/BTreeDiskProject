#ifndef BTREEDISK_PAGE_H
#define BTREEDISK_PAGE_H
#include <vector>
#include <queue>
#include <iostream>
#include <utility>
#include <fstream>
#include "Record.h"

using namespace std;

template<typename T>
class BtreeIndex;

template <typename T>
class Page{
private:
    unsigned int t;     /* minDegree */
    unsigned int currentKeys;
    vector<T> keys;    /* Vector of Records* MaxCapacity = (2 * t -1) */
    vector<Page<T>*> children;
    vector<unsigned long int> children_pDisk;
    bool isLeaf;
    template <typename TT>
    friend std::ostream& operator <<(std::ostream & stream, Page<TT> & record);
    template <typename TT>
    friend std::istream& operator >>(std::istream & stream, Page<TT> & record);
public:
    Page(unsigned int t, bool isLeaf) : t(t), isLeaf(isLeaf) {
        keys.resize(2 * t - 1);    /* Capacity = (2*t - 1)*/
        children.resize(2 * t);
        children_pDisk.resize(2 * t);
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
            cout << keys[i]->key << " ";
        }
    }

    unsigned long int write(){
        for (int i = 0; i <= currentKeys; i++) {
            if(children[i] != nullptr){
                this->children_pDisk[i] = children[i]->write();
            }
        }
        // Write Page
        ofstream file("../index.dat", ios::out | ios::binary | ios::app);
        unsigned long int start = file.tellp();
        cout << "Page Start byte disk: " << start << '\n';
        //file.write((char*)&(*this), sizeof(*this));    /* Check this */
        file.write((char*)&(*this).t, sizeof((*this).t));
        file.write((char*)&(*this).currentKeys, sizeof((*this).currentKeys));
        int recordsSize = this->keys.size();
        file.write((char*)&recordsSize, sizeof(recordsSize));
        for (int i = 0; i < this->keys.size(); i++) {
            if(keys[i]){
                file.write((char *)(&(*keys[i])), sizeof(*keys[i]));
            }
            else {
                auto recObj = Record();
                file.write((char *)(&recObj), sizeof(recObj));
            }
        }
        for (int i = 0; i < this->children_pDisk.size(); i++) {
            file.write((char *)(&(children_pDisk[i])), sizeof(children_pDisk[i]));
        }

        /*Page<T> pageObj = *this;
        file << pageObj;
         */
        file.close();

        return start;
        /*cout << "Write Page to Disk\n";
        for (int i = 0; i < currentKeys; i++) {
            cout << keys[i]->key << " ";
        }
        cout << '\n';
         */
    }

    friend class BtreeIndex<T>;
};

// Overload operator | Write file Operation << object
template <typename T>
std::ostream & operator << (std::ostream & stream, Page<T> & pageObj) {
    stream.write((char *)(&pageObj.t), sizeof(pageObj.t));
    stream.write((char *)(&pageObj.currentKeys),  pageObj.currentKeys);
    int recordsSize = pageObj.keys.size();
    stream.write((char *)(&recordsSize), sizeof(recordsSize));  // number of Vector size Records
    /*
    for(auto record : pageObj.keys){
        if(record){
            stream.write((char *)(&(*record)), sizeof(*record));    // dereference and save Record Object
        }
        else{
            Record r = Record();
            stream.write((char *)(&(r)), sizeof(r));    // dereference and save Record Object
        }
    }
    int childrenSize = pageObj.children_pDisk.size();
    stream.write((char *)(&childrenSize), sizeof(childrenSize));  // number of Vector Size
    for(auto pdirDisk : pageObj.children_pDisk){
        stream.write((char *)(&(pdirDisk)), sizeof(pdirDisk));
    }
    */
    stream << std::flush;
    return stream;
}


// Overload operator | Read file operation >> object
template <typename T>
std::istream& operator >> (std::istream& stream, Page<T> & pageObj) {
    stream.read(reinterpret_cast<char *>(&pageObj.t), sizeof(pageObj.t));   // read Metadata: get size
    stream.read(reinterpret_cast<char *>(&pageObj.currentKeys), sizeof(pageObj.currentKeys));   // use this size to read real value
    int recordsSize;
    stream.read(reinterpret_cast<char *>(&recordsSize), sizeof(recordsSize));
    /*
    auto r = new Record();
    for(int i = 0; i < recordsSize; i++){ //pageObj.keys[i]
        stream.read(reinterpret_cast<char *>(&(*r)), sizeof(*r));
        pageObj.keys[i] = r;
    }
    int childrenSize;
    stream.read(reinterpret_cast<char *>(&childrenSize), sizeof(childrenSize));
    unsigned long int pdirChildPage;
    for(int i = 0; i < childrenSize; i++){
        stream.read((char *)(&(pdirChildPage)), sizeof(pdirChildPage));
        pageObj.children_pDisk[i] = pdirChildPage;
    }
    */
    return stream;
}

#endif //BTREEDISK_PAGE_H
