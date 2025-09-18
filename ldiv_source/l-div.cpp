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
#include <vector>
#include <limits>
#include <queue>
#include <vector>
#include <limits>

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

std::tuple<std::string, std::string, std::string, std::string>
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

    // We'll perform a BFS over the space of level combinations, starting from (0,0,0,0).
    // This preserves a while-loop structure and ensures we attempt all combos before giving up.
        // Determine per-attribute maximum valid levels so we never explore invalid generalization levels.
        // Age: generalize_age() returns '*' for level > 3, so allow levels 0..4 (inclusive) where 4 means '*'.
        int max_age_level = 4;
        // For categorical hierarchies, allow levels 0..(depth-1) for actual levels; we'll allow an extra level equal
        // to the depth to force the last entry (which should be '*'). So valid indices are 0..depth (inclusive).
        int max_edu_level = 2;
        int max_mar_level = 2;
        int max_race_level = 1;

    std::queue<std::tuple<int,int,int,int>> q;
    std::set<std::tuple<int,int,int,int>> tried;
    q.emplace(0,0,0,0);

    int iteration = 0;
    long long best_deficit = std::numeric_limits<long long>::max();
    std::tuple<int,int,int,int> best_combo = std::make_tuple(maxLevel, maxLevel, maxLevel, maxLevel);
    int best_sum = std::numeric_limits<int>::max();

    while (!q.empty()) {
        auto cmb = q.front(); q.pop();
        if (tried.count(cmb)) continue;
        tried.insert(cmb);

        int aL = std::get<0>(cmb);
        int eL = std::get<1>(cmb);
        int mL = std::get<2>(cmb);
        int rL = std::get<3>(cmb);

        iteration++;
        std::cout << "Iteration " << iteration << ": Generalized to levels age=" << aL
                  << " edu=" << eL << " mar=" << mL << " race=" << rL << std::endl;

        // Build groups and compute total deficit for this combo
        std::map<std::tuple<std::string,std::string,std::string,std::string>, std::vector<int>> groups;
        for (size_t i = 0; i < ds.records.size(); ++i) {
            auto key = get_qi(ds.records[i], aL, eL, mL, rL);
            groups[key].push_back(static_cast<int>(i));
        }

        long long total_deficit = 0;
        for (auto &p : groups) {
            auto &idxs = p.second;
            int strict_k = 1;
            for (int idx : idxs) strict_k = std::max(strict_k, ds.records[idx].k);
            if ((int)idxs.size() < strict_k) total_deficit += (strict_k - (int)idxs.size());
        }

        if (total_deficit == 0) {
            std::cout << "Anonymization satisfied at levels: age=" << aL << " edu=" << eL << " mar=" << mL << " race=" << rL << std::endl;
            return cmb;
        } 
        //else, print the failed equivalence classes
        else {
            std::cout << "Total deficit (records needed to satisfy k-anonymity): " << total_deficit << std::endl;
            for (auto &p : groups) {
                auto &idxs = p.second;
                int strict_k = 1;
                for (int idx : idxs) strict_k = std::max(strict_k, ds.records[idx].k);
                if ((int)idxs.size() < strict_k) {
                    std::cout << "  Class key: (" << std::get<0>(p.first) << ", " << std::get<1>(p.first)
                              << ", " << std::get<2>(p.first) << ", " << std::get<3>(p.first)
                              << ") size=" << idxs.size() << " needs " << (strict_k - (int)idxs.size()) << " more to reach k=" << strict_k << std::endl;
                }
            }
        }

        // record best-effort
        int s = aL + eL + mL + rL;
        if (total_deficit < best_deficit || (total_deficit == best_deficit && s < best_sum)) {
            best_deficit = total_deficit;
            best_combo = cmb;
            best_sum = s;
        }

            // enqueue neighbors: increase any single attribute by 1 (if within per-attribute limits and the requested maxLevel)
            int effective_max_age = std::min(maxLevel, max_age_level);
            int effective_max_edu = std::min(maxLevel, max_edu_level);
            int effective_max_mar = std::min(maxLevel, max_mar_level);
            int effective_max_race = std::min(maxLevel, max_race_level);

            if (aL < effective_max_age) q.emplace(aL+1, eL, mL, rL);
            if (eL < effective_max_edu) q.emplace(aL, eL+1, mL, rL);
            if (mL < effective_max_mar) q.emplace(aL, eL, mL+1, rL);
            if (rL < effective_max_race) q.emplace(aL, eL, mL, rL+1);
    }

    // Exhausted all combos: per request, generalize everything to '*' in the worst case.
    // For categorical hierarchies we can pick a level >= the largest hierarchy depth so get_level() will return the last element (which is '*').
    // For age, generalize_age() only returns '*' for level > 3, so pick 4 to force '*'.
    std::cout << "No full anonymization possible within limits; generalizing everything to max (all '*')." << std::endl;

        // compute the full-generalization levels consistent with per-attribute depths
        int maxEduDepth = max_edu_level;
        int maxMarDepth = max_mar_level;
        int maxRaceDepth = max_race_level;

        int full_age_level = max_age_level; // level 4 maps to '*'
        int full_edu_level = maxEduDepth;
        int full_mar_level = maxMarDepth;
        int full_race_level = maxRaceDepth;

        return std::make_tuple(full_age_level, full_edu_level, full_mar_level, full_race_level);
}

// calculate distortion (average per-attribute level)


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
    //int k_vals[] = {4, 5, 7};
    int k_vals[] = {2, 3, 4};
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