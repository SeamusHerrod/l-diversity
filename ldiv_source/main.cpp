#include <iostream>
#include "l-div.h"


int main() {
    from_l_div(); // Call the function from l-div.h
    Dataset dataset; // reads entire data/adult.data
    for (size_t i = 0; i < dataset.records.size(); ++i) {
        std::cout << "Record " << i << ": Age = " << dataset.records[i].age << std::endl;
    }
    return 0;
}