#include <iostream>
#include "BtreeIndex.h"

int main() {
    BtreeIndex<Record*> treeIndex(3);
//    treeIndex.insert("aback");
//    treeIndex.insert("abacterial");
//    treeIndex.insert("abacus");
//    treeIndex.insert("abacuses");
//    treeIndex.insert("abaft");
//    treeIndex.insert("abalone");
//    treeIndex.Execute();
//    treeIndex.print();
//    treeIndex.save();
    treeIndex.Find("aardvarks");
    return 0;
}
