#include <iostream>
#include <map>
#include <tuple>
#include "l-div.h"



int main(int argc, char **argv) {
    Dataset dataset; // reads entire data/adult.data (or data/adult.data fallback)
    if (dataset.records.empty()) {
        std::cerr << "No data loaded. Make sure data/adult.data exists." << std::endl;
        return 1;
    }

    // choose a maxLevel (can be tuned); keep 3 as in tests
    int maxLevel = 3;
    auto levels = personalized_anonymize(dataset, maxLevel);
    int aL = std::get<0>(levels);
    int eL = std::get<1>(levels);
    int mL = std::get<2>(levels);
    int rL = std::get<3>(levels);

    std::cout << "Chosen generalization levels: age=" << aL
              << " edu=" << eL << " mar=" << mL << " race=" << rL << std::endl;

    // Build equivalence-class summary
    std::map<std::tuple<std::string,std::string,std::string,std::string>, int> counts;
    for (size_t i = 0; i < dataset.records.size(); ++i) {
        auto key = get_qi(dataset.records[i], aL, eL, mL, rL);
        counts[key]++;
    }

    std::cout << "Equivalence classes: " << counts.size() << "\n";
    // print a few sample class sizes
    for (const auto &p : counts) {
        const auto &k = p.first;
        std::cout << "Class key: (" << std::get<0>(k) << ", " << std::get<1>(k)
                  << ", " << std::get<2>(k) << ", " << std::get<3>(k) << ") -> size=" << p.second << std::endl;
    }
    // calculate distortion:
    calculate_distortion(aL, eL, mL, rL);
    std::cout << "Distortion D = " << calculate_distortion(aL, eL, mL, rL) << std::endl;

    // Now attempt to achieve uniform k-anonymity (k=6) + 3-diversity on occupation using
    // global generalization + suppression. This will return chosen levels and suppressed count.
    auto result = achieve_k_and_l_diversity(dataset, 6, 3, maxLevel);
    int ak = std::get<0>(result);
    int ae = std::get<1>(result);
    int am = std::get<2>(result);
    int ar = std::get<3>(result);
    int suppressed = std::get<4>(result);
    float distortion = std::get<5>(result);
    std::cout << "achieve_k_and_l_diversity result: age=" << ak << " edu=" << ae << " mar=" << am << " race=" << ar << " suppressed=" << suppressed << " distortion=" << distortion << std::endl;

    // print members of each equivalence class:
    /*
    std::cout << "---------------------------------\n" << "Members of each equivalence class:\n" << "---------------------------------\n";
    for (const auto &p : counts) {
        const auto &k = p.first;
        std::cout << "Class key: (" << std::get<0>(k) << ", " << std::get<1>(k)
                  << ", " << std::get<2>(k) << ", " << std::get<3>(k) << ") -> size=" << p.second << std::endl;
        for (size_t i = 0; i < dataset.records.size(); ++i) {
            auto key = get_qi(dataset.records[i], aL, eL, mL, rL);
            if (key == k) {
                const auto &rec = dataset.records[i];
                std::cout << "  idx=" << i << " raw(age=" << rec.age << ", edu=" << to_string(rec.edu)
                          << ", mar=" << to_string(rec.marriage) << "," << "race=" << to_string(rec.race)
                            << ") k=" << rec.k << " generalized=(" << std::get<0>(key) << ", "
                          << std::get<1>(key) << ", " << std::get<2>(key) << ", " << std::get<3>(key) << ")\n";
            }
        }
    }
    */

    return 0;
}