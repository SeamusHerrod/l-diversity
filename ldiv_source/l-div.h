

#ifndef L_DIV_H
#define L_DIV_H
#include <iostream>
#include <vector>
#include <string>
#include <map>
int from_l_div();
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
enum occupations {
    Tech_support,
    Craft_repair,
    Other_service,
    Sales,
    Exec_managerial,
    Prof_specialty,
    Handlers_cleaners,
    Machine_op_inspct,
    Adm_clerical,
    Farming_fishing,
    Transport_moving,
    Priv_house_serv,
    Protective_serv,
    Armed_Forces,
    null_occ
};

// helpers
std::string to_string(education e);
std::string to_string(marital_status m);
std::string to_string(races r);
std::string to_string(occupations o);
// generalization maps: map from enum to vector of string levels (level 0..N)
extern std::map<education, std::vector<std::string>> EDU_HIER;
extern std::map<marital_status, std::vector<std::string>> MARITAL_HIER;
extern std::map<races, std::vector<std::string>> RACE_HIER;

// age generalizer: returns string for given level (0=exact, higher=more general)
std::string generalize_age(int age, int level);
int assign_rand_k(int k1, int k2, int k3); // assign random k from given options

class Record {
    public:
        int age;
        int k;
        education edu;
        marital_status marriage;
        races race;
        occupations occ;
        Record(int a, int k_anon, education e, marital_status m, races r, occupations o);
};

class Dataset {
    public:
        std::vector<Record> records;
        Dataset();
};

// Personalized k-anonymity: groups records by current generalization levels
// and iteratively generalizes attributes until every equivalence class
// has size >= max k required among its members.
// Returns final levels as tuple: (ageL, eduL, marL, raceL)
std::tuple<int,int,int,int> personalized_anonymize(Dataset &ds, int maxLevel = 3);

// Achieve uniform k-anonymity (default k=6) together with t-diversity (default t=3) on the sensitive
// attribute `occupation` using global generalization plus suppression. The function searches over
// generalization level combinations and returns a tuple: (ageL, eduL, marL, raceL, suppressed_count).
// suppressed_count is the number of records that must be suppressed (removed) under that combination
// to ensure every remaining equivalence class has size >= k and at least `diversity` distinct occupations.
std::tuple<int,int,int,int,int,float> achieve_k_and_l_diversity(Dataset &ds, int k = 6, int diversity = 3, int maxLevel = 3);

// helper get_qi is implemented in the cpp
std::tuple<std::string,std::string,std::string,std::string> get_qi(const Record &r, int ageL, int eduL, int marL, int raceL);

/*
    calculate distortion:
        Distortion Algorithm:
            Distortion, D = 
                \Sigma_atr-i (current level of generalization for attribut i / max level of generalization for attribute i) / N_attr
        Input: current level of generalization for each attribute
        Output: distortion value D
*/
float calculate_distortion(int ageL, int eduL, int marL, int raceL);

// Note: templated helper `get_level` is defined in the implementation file.
// Templates must be visible at instantiation; to keep changes minimal we
// declare only the typed generalization helper calls via the maps above.
#endif // L_DIV_H

