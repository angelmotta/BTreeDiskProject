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
    unsigned long int root_disk;
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
                        this->insert(r);
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

    void save(){
        cout << "** Save Index to Disk\n";
        if(root){
            root_disk = root->write();
        }
        cout << "root disk: " << root_disk << '\n';
        cout << '\n';
    }

    void Find(string key) {
        cout << "** Find **\n";
        cout << "";
        bool isFindFinished = false;
        ifstream file("../index.dat", ios::binary);
//        file.seekg(1642);
        file.seekg(0, ios::end);
        unsigned long int rootDirDisk = file.tellg();
        rootDirDisk = rootDirDisk - 821;   // sizeof(Page in Disk) = 821
        Page<T> pageLoad(minDegree, true);
        // START WHILE
        while(!isFindFinished){
            file.seekg(rootDirDisk);
            file.read((char *)&pageLoad.t, sizeof(pageLoad.t));
            file.read((char *)&pageLoad.currentKeys, sizeof(pageLoad.currentKeys));
            int recordsSizeVector = -1;
            file.read((char *)&recordsSizeVector, sizeof(recordsSizeVector));
            for (int i = 0; i < recordsSizeVector; i++) {
                auto recObj = new Record();
                file.read((char *)&(*recObj), sizeof(*recObj));
                pageLoad.keys[i] = recObj;
            }
            for(int i = 0; i < recordsSizeVector+1; i++){
                unsigned long int pdirPageChild;
                file.read((char *)(&(pdirPageChild)), sizeof(pdirPageChild));
                pageLoad.children_pDisk[i] = pdirPageChild;
            }
            file.read((char *)&pageLoad.isLeaf, sizeof(pageLoad.isLeaf));
//            file.close();

            // Read keys from Page in memory
            cout << "Read Keys from Page\n";
            for (int i = 0; i < pageLoad.currentKeys; i++) {
                cout << pageLoad.keys[i]->key << " ";
                // Check if keys are equals
                if(pageLoad.keys[i]->key == key){
                    cout << "\nKeyword Found!!\n";
                    // Read Definition from Disk using index
                    for(int idx = 0; idx < dataFileList.size(); idx++){    // for each Language
                        if(pageLoad.keys[i]->offset[idx] == 0) continue;     //  if word not have entry in that Language continue to next Language
                        cout << getIdioma(idx) << ": "<< '\n';
                        ifstream file(dataFileList[idx], ios::binary);
                        file.seekg(pageLoad.keys[i]->pdir[idx]);
                        string buf;
                        int of;
                        int temp = pageLoad.keys[i]->offset[idx];
                        int temp2;
                        int numResult = 1;
                        while(temp){    // read offset for this language
                            of = file.tellg();  //posicion antes del key
                            getline(file,buf,'\t');
                            temp2 = file.tellg(); //posicion despues del key == inicio del significado
                            temp -= (temp2 - of);
                            getline(file,buf);    // leer toda la segunda parte linea (significado)
                            of = file.tellg();  // posicion despues del significado
                            cout << numResult++ << ") " << buf << endl;
                            temp -=  (of-temp2);
                        }
                    }
                    isFindFinished = true;
                    break;
                }
                else if(key < pageLoad.keys[i]->key){
                    cout << "\nRead from disk Child Page[" << i << "]" << "\n";
                    // TODO
                    if(!pageLoad.isLeaf){
                        // READ child Page from Disk
                        rootDirDisk = pageLoad.children_pDisk[0];
                    }
                    else{
                        isFindFinished = true;
                    }
                    break;
                }
            } // End For Loop for this Page
        }
        cout << '\n';
    }

    string getIdioma(int idx){
        vector<string> languageArr = {"Spanish", "Portuguese", "French", "Latin", "German", "Italian"};
        return languageArr[idx];
    }
};

#endif //BTREEDISK_BTREEINDEX_H
