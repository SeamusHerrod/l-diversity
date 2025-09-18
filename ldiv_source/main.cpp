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
    int printed = 0;
    for (const auto &p : counts) {
        if (printed++ >= 10) break;
        const auto &k = p.first;
        std::cout << "Class key: (" << std::get<0>(k) << ", " << std::get<1>(k)
                  << ", " << std::get<2>(k) << ", " << std::get<3>(k) << ") -> size=" << p.second << std::endl;
    }

    return 0;
}