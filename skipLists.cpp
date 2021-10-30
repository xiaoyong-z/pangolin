#include <mutex>
#include <codec.h>
struct SkipNode {
    double score_;
    Entry elem_;
};

struct Link {
    SkipNode* skip_nodes_;  
};

class SkipList {
    SkipList() {

    }


    

    bool contains(char* key) {
        if (header == nullptr) {
            return false;
        }
        
    } 
    

    void insert(const Entry& elem) {

    }

private:
    Link* header;
};
