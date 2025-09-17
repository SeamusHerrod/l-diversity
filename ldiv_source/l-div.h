

#ifndef L_DIV_H
#define L_DIV_H
#include <iostream>
#include <vector>
#include <string>
#include <map>
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
// generalization maps: map from enum to vector of string levels (level 0..N)
extern std::map<education, std::vector<std::string>> EDU_HIER;
extern std::map<marital_status, std::vector<std::string>> MARITAL_HIER;
extern std::map<races, std::vector<std::string>> RACE_HIER;

// age generalizer: returns string for given level (0=exact, higher=more general)
std::string generalize_age(int age, int level);
int assign_rand_k();

class Record {
    public:
        int age;
        int k;
        education edu;
        marital_status marriage;
        races race;
        Record(int a, int k_anon, education e, marital_status m, races r);
};

class Dataset {
    public:
        std::vector<Record> records;
        Dataset();
};
#endif // L_DIV_H

