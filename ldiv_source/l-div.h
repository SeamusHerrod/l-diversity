

#ifndef L_DIV_H
#define L_DIV_H
#include <iostream>
#include <vector>
#include <string>
int from_l_div();
static std::string FILEPATH = "../data/adult.data";
using namespace std;

// Enumerations used by Record
enum education { 
    bachelors,
    some_college,
    eleventh,
    hs_grad,
    prof_school,
    assoc_acdm,
    assoc_voc,
    ninth,
    seventh_eighth,
    twelveth,
    masters,
    first_fourth,
    tenth,
    doctorate,
    fifth_sixth,
    preschool,
    null_edu
};
enum marital_status { 
    married_civ_spouse,
    divorced,
    never_married,
    separated,
    widowed,
    married_spouse_absent,
    married_af_spouse,
    null_marital
};
enum races { 
    white,
    asian_pac_islander,
    amer_indian_eskimo,
    other,
    black, 
    null_r
};

// helpers
std::string to_string(education e);
std::string to_string(marital_status m);
std::string to_string(races r);
int assign_rand_k();

class Record {
    public:
        float age;
        int k;
        education edu;
        marital_status marriage;
        races race;
        Record(float a, int k_anon, education e, marital_status m, races r);
};

class Dataset {
    public:
        std::vector<Record> records;
        Dataset();
};
#endif // L_DIV_H

