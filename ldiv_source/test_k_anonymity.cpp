#include <iostream>
#include <vector>
#include <map>
#include <tuple>
#include <string>
#include <cassert>
#include "l-div.h"

// Tiny helper to build a dataset in-memory without reading the file
class TmpDataset : public Dataset {
public:
    TmpDataset() { records.clear(); }
    void add(int age, int k, education edu, marital_status ms, races r) {
        records.emplace_back(age, k, edu, ms, r);
    }
};

// compute equivalence classes given generalization levels
std::map<std::tuple<std::string,std::string,std::string,std::string>, std::vector<int>>
compute_groups(const Dataset &ds, int ageL, int eduL, int marL, int raceL) {
    std::map<std::tuple<std::string,std::string,std::string,std::string>, std::vector<int>> groups;
    for (size_t i = 0; i < ds.records.size(); ++i) {
        auto key = std::make_tuple(
            generalize_age(ds.records[i].age, ageL),
            // use public maps via get_level template - declare externs in l-div.h
            // but we don't have typed wrappers, so call the internal function via the maps
            // We'll use EDU_HIER etc and a simple accessor here
            EDU_HIER.at(ds.records[i].edu).at(std::min(eduL, (int)EDU_HIER.at(ds.records[i].edu).size()-1)),
            MARITAL_HIER.at(ds.records[i].marriage).at(std::min(marL, (int)MARITAL_HIER.at(ds.records[i].marriage).size()-1)),
            RACE_HIER.at(ds.records[i].race).at(std::min(raceL, (int)RACE_HIER.at(ds.records[i].race).size()-1))
        );
        groups[key].push_back(static_cast<int>(i));
    }
    return groups;
}

int main() {
    // Load full dataset from file and copy first 50 records for the test
    Dataset full; // uses FILEPATH / data/adult.data
    TmpDataset ds;
    size_t want = 50;
    for (size_t i = 0; i < full.records.size() && ds.records.size() < want; ++i) {
        const auto &r = full.records[i];
        ds.add(r.age, r.k, r.edu, r.marriage, r.race);
    }
    if (ds.records.empty()) {
        std::cerr << "No records loaded for test (missing data/adult.data)." << std::endl;
        return 2;
    }

    // Call anonymize with maxLevel=3 and get final levels
    auto levels = personalized_anonymize(ds, 3);
    int aL, eL, mL, rL;
    std::tie(aL, eL, mL, rL) = levels;

    auto groups = compute_groups(ds, aL, eL, mL, rL);

    // Print each equivalence class and its members with raw and generalized values
    std::cout << "Equivalence classes at levels age=" << aL << " edu=" << eL << " mar=" << mL << " race=" << rL << "\n";
    for (auto &p : groups) {
        const auto &key = p.first;
        const auto &members = p.second;
        std::cout << "Class key: (" << std::get<0>(key) << ", " << std::get<1>(key) << ", " << std::get<2>(key) << ", " << std::get<3>(key) << ")\n";
        for (int idx : members) {
            const auto &rec = ds.records[idx];
            std::cout << "  idx=" << idx << " raw(age=" << rec.age << ", edu=" << to_string(rec.edu)
                      << ", mar=" << to_string(rec.marriage) << ", race=" << to_string(rec.race)
                      << ") k=" << rec.k << " generalized=(" << std::get<0>(key) << ", "
                      << std::get<1>(key) << ", " << std::get<2>(key) << ", " << std::get<3>(key) << ")\n";
        }
        int max_k = 1;
        for (int idx : members) max_k = std::max(max_k, ds.records[idx].k);
        if ((int)members.size() < max_k) {
            std::cerr << "Test failed: group size " << members.size() << " < max_k " << max_k << "\n";
            return 2;
        }
    }

    std::cout << "k-anonymity test passed." << std::endl;
    return 0;
}
