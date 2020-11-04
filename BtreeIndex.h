#ifndef BTREEDISK_BTREEINDEX_H
#define BTREEDISK_BTREEINDEX_H
#include "Page.h"
#include "Record.h"

#include <iostream>
#include <list>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

template <typename T>
class BtreeIndex{
private:
    Page<T>* root;
    unsigned int minDegree;    /* maxRecords Capacity = (2 * minDegree) - 1  */
    vector<string> dataFileList = {"../database/testSpanish.txt"};
    unordered_map<string, Record*> mapWords;
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

    void Execute(){
        cout << "** Execute Indexing **\n";
        for(int i = 0; i < dataFileList.size(); i++){  // iterate for each Language
            ifstream file(dataFileList[i], ios::binary);   // open dataFile
            unordered_map<string, Record*> reps;    // new Map de repeticiones para cada Idioma
            string line;
            // Iterate over the file Language
            while(file.peek() != EOF){
                long pdirRecord = file.tellg();         /* Get start pdir of Record */
                getline(file, line, '\t');    /* Get Key */
                // If this word has more than one instance in this Language, Update Offset
                if(reps.find(line) != reps.end()){
                    auto rPtr = reps[line];    /* get pointer to existing Record */
                    getline(file, line);          /* Get Significado */
                    long end2 = file.tellg();
                    rPtr->offset[i] += (end2 - pdirRecord);   /* Update Offset for this Language */
                }
                else{ /* check first if keyword already exist in a Record */
                    Record* r;
                    bool isNewWord = false;
                    // check if this KeyWord already exist in BTree
                    if(mapWords.find(line) != mapWords.end()){
                        r = mapWords[line];         /* get Pointer of existing Record*/
                    }
                    else {
                        r = new Record();           /* Create Record */
                        isNewWord = true;
                        mapWords[line] = r;         /* include keyword in global Map */
                    }
                    r->pdir[i] = pdirRecord;        /* Set start pdir for Language 'i' in Record */
                    reps[line] = r;                 /* Create entry in Map of repetitions */
                    strcpy(r->key, line.c_str());   /* Set key value of Record */
                    getline(file, line);      /* Get significado, move forward file pointer */
                    unsigned long end = file.tellg();
                    r->offset[i] = end - pdirRecord;  /* Set offset for Language 'i' in Record */
                    // If is new key --> Insert Btree
                    if(isNewWord){
                        this->insert(r->key);    /* Pending Change this for Record*/
                    }
                }
            }
            cout << "Index Language finished " << dataFileList[i] << '\n';
            file.close();  // Close Language datafile
        }
    }

    void print() {
        cout << "** Print BTree Index **\n";
        if(root) root->recorrerPages();
        cout << '\n';
    }
};

#endif //BTREEDISK_BTREEINDEX_H
