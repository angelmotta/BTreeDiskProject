#ifndef BTREEDISK_RECORD_H
#define BTREEDISK_RECORD_H

using namespace std;

class Record{
public:
    char key[50];
    unsigned long pdir[6];
    unsigned long offset[6] = {0, 0, 0, 0, 0, 0};

    friend bool operator> (const Record &r1, const Record &r2){
        string str1(r1.key);
        string str2(r2.key);
        //return string(r1.key) > string(r2.key);
        return str1 > str2;
    }
    friend bool operator< (const Record &r1, const Record &r2){
        string str1(r1.key);
        string str2(r2.key);
        //return string(r1.key) > string(r2.key);
        return str1 < str2;
    }
};

#endif //BTREEDISK_RECORD_H
