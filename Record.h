#ifndef BTREEDISK_RECORD_H
#define BTREEDISK_RECORD_H

using namespace std;

class Record{
public:
    char key[50];
    unsigned long pdir[6];
    unsigned long offset[6] = {0, 0, 0, 0, 0, 0};

    friend bool operator> (const Record &r1, const Record &r2){
        return r1.key > r2.key;
    }
};

#endif //BTREEDISK_RECORD_H
