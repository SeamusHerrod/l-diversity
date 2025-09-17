#include "l-div.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <map>
#include <cmath>
#include <cstdlib>
#include <set>
#include <tuple>

int from_l_div() {
    std::cout << "Function from_l_div called." << std::endl;
    return 0;
}
// safe getter used earlier in discussion
template<typename Enum, typename MapT>
static std::string get_level(const MapT &map, Enum key, int level) {
    auto it = map.find(key);
    if (it == map.end()) return "*";
    const auto &vec = it->second;
    if (vec.empty()) return "*";
    if (level < 0) level = 0;
    if (level >= static_cast<int>(vec.size())) return vec.back();
    return vec[level];
}

static std::tuple<std::string, std::string, std::string, std::string>
get_qi(const Record &r, int ageL, int eduL, int marL, int raceL) {
    return std::make_tuple(
        generalize_age(r.age, ageL),
        get_level(EDU_HIER, r.edu, eduL),
        get_level(MARITAL_HIER, r.marriage, marL),
        get_level(RACE_HIER, r.race, raceL)
    );
}


std::tuple<int,int,int,int> personalized_anonymize(Dataset &ds, int maxLevel) {
    if (ds.records.empty()) return std::make_tuple(0,0,0,0);

    // starting levels
    int ageL = 0, eduL = 0, marL = 0, raceL = 0;

    bool changed = true;
    while (changed) {
        changed = false;
        // group records by QI tuple
        std::map<std::tuple<std::string,std::string,std::string,std::string>, std::vector<int>> groups;
        for (size_t i = 0; i < ds.records.size(); ++i) {
            auto key = get_qi(ds.records[i], ageL, eduL, marL, raceL);
            groups[key].push_back(static_cast<int>(i));
        }

        // check groups: any group failing must be generalized; use strictest k among members
        bool needGeneralize = false;
        for (auto &p : groups) {
            auto &idxs = p.second;
            int strict_k = 1;
            for (int idx : idxs) strict_k = std::max(strict_k, ds.records[idx].k);
            if (static_cast<int>(idxs.size()) < strict_k) { needGeneralize = true; break; }
        }

        if (!needGeneralize) break; // done

        // Choose attribute to generalize: pick attribute with largest number of distinct values currently
        std::set<std::string> ageVals, eduVals, marVals, raceVals;
        for (const auto &rec : ds.records) {
            ageVals.insert(generalize_age(rec.age, ageL));
            eduVals.insert(get_level(EDU_HIER, rec.edu, eduL));
            marVals.insert(get_level(MARITAL_HIER, rec.marriage, marL));
            raceVals.insert(get_level(RACE_HIER, rec.race, raceL));
        }

        // compute sizes
        int aN = static_cast<int>(ageVals.size());
        int eN = static_cast<int>(eduVals.size());
        int mN = static_cast<int>(marVals.size());
        int rN = static_cast<int>(raceVals.size());

        // pick attribute with max distinct
        if (ageL < maxLevel && aN >= eN && aN >= mN && aN >= rN) { ageL++; changed = true; }
        else if (eduL < maxLevel && eN >= aN && eN >= mN && eN >= rN) { eduL++; changed = true; }
        else if (marL < maxLevel && mN >= aN && mN >= eN && mN >= rN) { marL++; changed = true; }
        else if (raceL < maxLevel && rN >= aN && rN >= eN && rN >= mN) { raceL++; changed = true; }
        else {
            // all at maxLevel or no progress possible
            break;
        }
    }

    // after generalization, we could write out the generalized quasi-identifiers per record or modify records
    // For now, print summary
    std::cout << "Anonymization finished with levels: age=" << ageL << " edu=" << eduL << " mar=" << marL << " race=" << raceL << std::endl;
    return std::make_tuple(ageL, eduL, marL, raceL);
}

Record::Record(int a, int k_anon, education e, marital_status m, races r) {
    age = a;
    k = k_anon;
    edu = e;
    marriage = m;
    race = r;
}

// trim helpers
static inline std::string trim(const std::string &s) {
    auto start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

static inline std::string lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c){ return std::tolower(c); });
    return s;
}

static education parseEducation(const std::string &raw) {
    std::string s = lower(trim(raw));
    if (s.find("bachelor") != std::string::npos) return bachelors;
    if (s.find("some") != std::string::npos && s.find("college") != std::string::npos) return some_college;
    if (s == "11th") return eleventh;
    if (s.find("hs") != std::string::npos || s.find("high") != std::string::npos) return hs_grad;
    if (s.find("prof") != std::string::npos) return prof_school;
    if (s.find("assoc-acdm") != std::string::npos || s.find("assoc_acdm") != std::string::npos || s.find("assoc-acdm") != std::string::npos) return assoc_acdm;
    if (s.find("assoc-voc") != std::string::npos || s.find("assoc_voc") != std::string::npos) return assoc_voc;
    if (s == "9th") return ninth;
    if (s.find("7th") != std::string::npos || s.find("8th") != std::string::npos) return seventh_eighth;
    if (s == "12th" || s.find("twelv") != std::string::npos) return twelveth;
    if (s.find("master") != std::string::npos) return masters;
    if (s.find("1st") != std::string::npos || s.find("1st-4th") != std::string::npos) return first_fourth;
    if (s == "10th") return tenth;
    if (s.find("doctor") != std::string::npos) return doctorate;
    if (s.find("5th") != std::string::npos || s.find("6th") != std::string::npos) return fifth_sixth;
    if (s.find("preschool") != std::string::npos) return preschool;
    // fallback
    return null_edu;
}

static marital_status parseMarital(const std::string &raw) {
    std::string s = lower(trim(raw));
    if (s.find("married-civ-spouse") != std::string::npos) return married_civ_spouse;
    if (s.find("divorced") != std::string::npos) return divorced;
    if (s.find("never-married") != std::string::npos) return never_married;
    if (s.find("separated") != std::string::npos) return separated;
    if (s.find("widowed") != std::string::npos) return widowed;
    if (s.find("married-spouse-absent") != std::string::npos) return married_spouse_absent;
    if (s.find("married-a") != std::string::npos || s.find("married_af_spouse") != std::string::npos) return married_af_spouse;
    // fallback
    return null_marital;
}

static races parseRace(const std::string &raw) {
    std::string s = lower(trim(raw));
    if (s.find("white") != std::string::npos) return white;
    if (s.find("asian") != std::string::npos || s.find("pac") != std::string::npos) return asian_pac_islander;
    if (s.find("amer-indian") != std::string::npos || s.find("amer_indian") != std::string::npos) return amer_indian_eskimo;
    if (s.find("black") != std::string::npos) return black;
    if (s.find("other") != std::string::npos || s == "?") return other;
    // fallback
    return null_r;
}

// generate a random value of 4, 5, or 7
int assign_rand_k() {
    int k_vals[] = {4, 5, 7};
    int idx = rand() % 3;
    return k_vals[idx];
}

Dataset::Dataset() {
    std::vector<int> k_counts = {0,0,0}; // to track counts of 4, 5, 7
    std::ifstream infile(FILEPATH);
    if (!infile.is_open()) infile.open("data/adult.data");
    if (!infile.is_open()) {
        // no data file found: leave empty vector
        records.clear();
        return;
    }

    std::string line;
    while (std::getline(infile, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string field;
        std::vector<std::string> fields;
        while (std::getline(ss, field, ',')) fields.push_back(trim(field));
        if (fields.size() < 9) continue;

        float age = 0.0f;
        int k = assign_rand_k();
        if (k == 4) {
            k_counts[0]++;
        } else if (k == 5) {
            k_counts[1]++;
        } else if (k == 7) {
            k_counts[2]++;
        }
        try { age = std::stof(fields[0]); } catch(...) { age = 0.0f; }
        education edu = parseEducation(fields[3]);
        marital_status ms = parseMarital(fields[5]);
        races r = parseRace(fields[8]);

        records.emplace_back(age, k, edu, ms, r);
    }
    std::cout << "k counts: 4 -> " << k_counts[0] << ", 5 -> " << k_counts[1] << ", 7 -> " << k_counts[2] << std::endl;
}

std::string to_string(education e) {
    switch (e) {
        case bachelors: return "Bachelors";
        case some_college: return "Some-college";
        case eleventh: return "11th";
        case hs_grad: return "HS-grad";
        case prof_school: return "Prof-school";
        case assoc_acdm: return "Assoc-acdm";
        case assoc_voc: return "Assoc-voc";
        case ninth: return "9th";
        case seventh_eighth: return "7th-8th";
        case twelveth: return "12th";
        case masters: return "Masters";
        case first_fourth: return "1st-4th";
        case tenth: return "10th";
        case doctorate: return "Doctorate";
        case fifth_sixth: return "5th-6th";
        case preschool: return "Preschool";
        case null_edu: return "Unknown";
        default: return "Unknown";
    }
}

std::string to_string(marital_status m) {
    switch (m) {
        case married_civ_spouse: return "Married-civ-spouse";
        case divorced: return "Divorced";
        case never_married: return "Never-married";
        case separated: return "Separated";
        case widowed: return "Widowed";
        case married_spouse_absent: return "Married-spouse-absent";
        case married_af_spouse: return "Married-AF-spouse";
        case null_marital: return "Unknown";
        default: return "Unknown";
    }
}

std::string to_string(races r) {
    switch (r) {
        case white: return "White";
        case asian_pac_islander: return "Asian-Pac-Islander";
        case amer_indian_eskimo: return "Amer-Indian-Eskimo";
        case other: return "Other";
        case black: return "Black";
        case null_r: return "Unknown";
        default: return "Unknown";
    }
}

// --- Generalization hierarchies (programmatic maps) ---
std::map<education, std::vector<std::string>> EDU_HIER = {
    {bachelors, {"Bachelors", "College", "*"}},
    {some_college, {"Some-college", "College", "*"}},
    {eleventh, {"11th", "Highschool", "*"}},
    {hs_grad, {"HS-grad", "Highschool", "*"}},
    {prof_school, {"Prof-school", "Professional", "*"}},
    {assoc_acdm, {"Assoc-acdm", "Professional", "*"}},
    {assoc_voc, {"Assoc-voc", "Professional", "*"}},
    {ninth, {"9th", "Highschool", "*"}},
    {seventh_eighth, {"7th-8th", "Primary", "*"}},
    {twelveth, {"12th", "Highschool", "*"}},
    {masters, {"Masters", "College", "*"}},
    {first_fourth, {"1st-4th", "Primary", "*"}},
    {tenth, {"10th", "Highschool", "*"}},
    {doctorate, {"Doctorate", "College", "*"}},
    {fifth_sixth, {"5th-6th", "Primary", "*"}},
    {preschool, {"Preschool", "Primary", "*"}},
    {null_edu, {"Unknown", "Unknown", "Unknown", "*"}}
};

std::map<marital_status, std::vector<std::string>> MARITAL_HIER = {
    {married_civ_spouse, {"Married-civ-spouse", "Married", "*"}},
    {divorced, {"Divorced", "Not-married", "*"}},
    {never_married, {"Never-married", "Not-married", "*"}},
    {separated, {"Separated", "Not-married", "*"}},
    {widowed, {"Widowed", "Not-married", "*"}},
    {married_spouse_absent, {"Married-spouse-absent", "Married", "*"}},
    {married_af_spouse, {"Married-AF-spouse", "Married", "*"}},
    {null_marital, {"Unknown", "Unknown", "Unknown", "*"}}
};

std::map<races, std::vector<std::string>> RACE_HIER = {
    {white, {"White", "*"}},
    {asian_pac_islander, {"Asian-Pac-Islander", "*"}},
    {amer_indian_eskimo, {"Amer-Indian-Eskimo", "*"}},
    {other, {"Other", "*"}},
    {black, {"Black", "*"}},
    {null_r, {"Unknown", "Unknown", "Unknown", "*"}}
};

std::string generalize_age(int age, int level) {
    if (level <= 0) return std::to_string(age);
    if (level == 1) {
        int lo = (age / 5) * 5;
        int hi = lo + 4;
        return std::to_string(lo) + "-" + std::to_string(hi);
    }
    if (level == 2) {
        int lo = (age / 10) * 10;
        int hi = lo + 9;
        return std::to_string(lo) + "-" + std::to_string(hi);
    }
    if (level == 3) {
        if (age <= 24) return "Young";
        if (age <= 54) return "Adult";
        if (age <= 74) return "Senior";
        return "Elder";
    }
    return "*"; // highest level
}