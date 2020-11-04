#include <iostream>
#include "BtreeIndex.h"

int main() {
    BtreeIndex<string> treeIndex(3);
//    treeIndex.insert("aback");
//    treeIndex.insert("abacterial");
//    treeIndex.insert("abacus");
//    treeIndex.insert("abacuses");
//    treeIndex.insert("abaft");
//    treeIndex.insert("abalone");
    treeIndex.Execute();
    treeIndex.print();
    return 0;
}
