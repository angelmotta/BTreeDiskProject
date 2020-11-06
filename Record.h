#ifndef BTREEDISK_RECORD_H
#define BTREEDISK_RECORD_H

using namespace std;

class Record{
public:
    char key[50];
    unsigned long pdir[6];
    unsigned long offset[6] = {0, 0, 0, 0, 0, 0};

    bool operator <(Record other){
        return this->key < other.key;
    }

    bool operator >(Record other){
        return this->key > other.key;
    }

};

#endif //BTREEDISK_RECORD_H
