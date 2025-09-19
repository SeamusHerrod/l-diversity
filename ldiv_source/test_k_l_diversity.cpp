#include <iostream>
#include <fstream>
#include <sstream>
#include "l-div.h"

int main() {
    Dataset full;
    if (full.records.empty()) {
        std::cerr << "No data loaded.\n";
        return 1;
    }

    Dataset subset;
    // Dataset ctor loads the file; clear it so subset contains only the first N records
    subset.records.clear();
    size_t take = std::min<size_t>(500, full.records.size());
    subset.records.reserve(take);
    for (size_t i = 0; i < take; ++i) subset.records.push_back(full.records[i]);

    std::cout << "Running achieve_k_and_l_diversity on first " << take << " records" << std::endl;
    auto res = achieve_k_and_l_diversity(subset, 6, 3, 3);
    int a = std::get<0>(res);
    int e = std::get<1>(res);
    int m = std::get<2>(res);
    int r = std::get<3>(res);
    int suppressed = std::get<4>(res);
    float distortion = std::get<5>(res);

    std::cout << "Result: age=" << a << " edu=" << e << " mar=" << m << " race=" << r << " suppressed=" << suppressed << " distortion=" << distortion << std::endl;

    // Print the global generalization levels used
    std::cout << "Generalization levels used: age=" << a << " edu=" << e << " mar=" << m << " race=" << r << std::endl;

    // Print equivalence classes under the returned levels: key and size only
    std::map<std::tuple<std::string,std::string,std::string,std::string>, std::vector<int>> groups;
    for (size_t i = 0; i < subset.records.size(); ++i) {
        auto key = get_qi(subset.records[i], a, e, m, r);
        groups[key].push_back(static_cast<int>(i));
    }
    std::cout << "Equivalence classes (key -> size -> occupations):\n";
    const int K = 6;
    const int L = 3;
    for (auto &p : groups) {
        auto key = p.first;
        auto &idxs = p.second;
        // build occupation counts for this class
        std::map<int,int> occ_counts;
        for (int idx : idxs) occ_counts[subset.records[idx].occ]++;
        std::cout << "Class key: (" << std::get<0>(key) << ", " << std::get<1>(key)
                  << ", " << std::get<2>(key) << ", " << std::get<3>(key) << ") size=" << idxs.size();
        std::cout << " occupations: ";
        bool first = true;
        for (auto &ocp : occ_counts) {
            if (!first) std::cout << ", ";
            first = false;
            std::cout << to_string(static_cast<occupations>(ocp.first)) << "(" << ocp.second << ")";
        }
        std::cout << "\n";

        // Validation: check k and l conditions
        int distinct = (int)occ_counts.size();
        if ((int)idxs.size() < K || distinct < L) {
            std::cout << "  WARNING: class fails requirements -> size=" << idxs.size() << " distinct_occs=" << distinct << "\n";
            std::cout << "  Indices in subset: ";
            for (size_t i = 0; i < idxs.size(); ++i) {
                if (i) std::cout << ", ";
                std::cout << idxs[i];
            }
            std::cout << "\n";
        }
    }

    return 0;
}
