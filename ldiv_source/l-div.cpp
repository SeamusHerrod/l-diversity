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

static std::string FILEPATH = "../data/adult.data";
static int MAX_AGE_LEVEL = 4; // 0..4 (4 means '*')
static int MAX_EDU_LEVEL = 2; // 0..2 (2 means '*')
static int MAX_MAR_LEVEL = 2; // 0..2 (2 means '*')
static int MAX_RACE_LEVEL = 1; // 0..1 (1 means '*')
static int NUM_ATTR = 4;
static int K1 = 4;
static int K2 = 5;
static int K3 = 7;

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

        int max_age_level = MAX_AGE_LEVEL;
        int max_edu_level = MAX_EDU_LEVEL;
        int max_mar_level = MAX_MAR_LEVEL;
        int max_race_level = MAX_RACE_LEVEL;

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
        /*
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
        */

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

    // Exhausted all combos: generalize everything to '*' in the worst case.
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


// Helper: compute suppression count required for a given generalization combo to achieve
// uniform k-anonymity (k) and t-diversity (diversity) on the occupation attribute.
static int compute_suppression_for_combo(Dataset &ds, int aL, int eL, int mL, int rL, int k, int diversity) {
    // Build groups by QI
    std::map<std::tuple<std::string,std::string,std::string,std::string>, std::vector<int>> groups;
    for (size_t i = 0; i < ds.records.size(); ++i) {
        auto key = get_qi(ds.records[i], aL, eL, mL, rL);
        groups[key].push_back(static_cast<int>(i));
    }

    int total_suppress = 0;
    // For each equivalence class, try to suppress the minimal number of records so that
    // - remaining_size >= max(k, max_k_of_remaining_members)
    // - remaining distinct sensitive values (occupations) >= diversity
    // If diversity < required distinct values present in the class, it's impossible to reach diversity by suppression
    // (since suppression cannot create new distinct values), so we must suppress the whole class.
    for (auto &p : groups) {
        auto idxs = p.second; // copy indices so we can modify
        int original_size = (int)idxs.size();

        // quick checks
        if (original_size == 0) continue;

        // count distinct occupations and per-occupation counts
        std::map<occupations,int> occ_count_full;
        int max_k_full = 1;
        for (int idx : idxs) {
            occ_count_full[ds.records[idx].occ]++;
            max_k_full = std::max(max_k_full, ds.records[idx].k);
        }
        int distinct_full = (int)occ_count_full.size();

        // If the class cannot possibly reach required diversity, we must suppress all
        if (distinct_full < diversity) {
            total_suppress += original_size;
            continue;
        }

        // Now attempt to keep as many records as possible while satisfying both k and diversity.
        // Strategy: start with the full set; if k constraint violated (i.e., size < strict_k), remove records
        // that contribute most to the strict_k (highest per-record k). When removing, prefer records from
        // occupations that have multiplicity > 1 so removing them doesn't reduce distinct count.

        // We'll maintain a multiset of candidate records sorted by (k desc, occ_count desc) for removal preference.
        // For simplicity and determinism, perform iterative removals until constraints satisfied or infeasible.

        // Current remaining set
        std::vector<int> remaining = idxs;

        auto compute_strict_k = [&](const std::vector<int> &vec) {
            int mk = 1;
            for (int id : vec) mk = std::max(mk, ds.records[id].k);
            mk = std::max(mk, k); // enforce the uniform k as a floor
            return mk;
        };

        auto compute_distinct = [&](const std::vector<int> &vec) {
            std::set<occupations> s;
            for (int id : vec) s.insert(ds.records[id].occ);
            return (int)s.size();
        };

        bool feasible = true;
        // If diversity is already satisfied and size >= strict_k, keep all
        int cur_strict_k = compute_strict_k(remaining);
        int cur_distinct = compute_distinct(remaining);
        if ((int)remaining.size() >= cur_strict_k && cur_distinct >= diversity) {
            // nothing to suppress in this group
            continue;
        }

        // Iteratively remove records until both constraints met or until we decide it's impossible
        while (true) {
            // Recompute metrics
            cur_strict_k = compute_strict_k(remaining);
            cur_distinct = compute_distinct(remaining);
            int cur_size = (int)remaining.size();

            if (cur_size >= cur_strict_k && cur_distinct >= diversity) break; // satisfied

            // If distinct < diversity here, impossible (we already checked full set distinct >= diversity,
            // but removals may have reduced distinct). To avoid that, we choose removals carefully: prefer
            // removing records from occupations with count > 1. If every occupation has count==1 and distinct<diversity,
            // impossible.
            std::map<occupations,int> occ_count;
            for (int id : remaining) occ_count[ds.records[id].occ]++;

            // Build list of candidate indices to remove, prefer those with highest k and whose occ_count>1
            int best_idx_pos = -1;
            int best_k = -1;
            bool found_multiplicity_candidate = false;
            for (size_t pos = 0; pos < remaining.size(); ++pos) {
                int id = remaining[pos];
                int rk = ds.records[id].k;
                bool mult = (occ_count[ds.records[id].occ] > 1);
                if (mult) {
                    found_multiplicity_candidate = true;
                    if (rk > best_k) { best_k = rk; best_idx_pos = (int)pos; }
                }
            }

            if (best_idx_pos == -1) {
                // No candidate that preserves distinct count — we must consider removing someone even if it reduces distinct.
                // Choose the record with highest k (to lower strict_k fastest). Tie-breaker: pick one from the largest occ bucket.
                int best_pos2 = -1;
                int best_k2 = -1;
                int best_occ_count = -1;
                for (size_t pos = 0; pos < remaining.size(); ++pos) {
                    int id = remaining[pos];
                    int rk = ds.records[id].k;
                    int oc_c = occ_count[ds.records[id].occ];
                    if (rk > best_k2 || (rk == best_k2 && oc_c > best_occ_count)) {
                        best_k2 = rk; best_pos2 = (int)pos; best_occ_count = oc_c;
                    }
                }
                if (best_pos2 == -1) { feasible = false; break; }
                // remove at best_pos2
                remaining.erase(remaining.begin() + best_pos2);
            } else {
                // remove the best multiplicity candidate
                remaining.erase(remaining.begin() + best_idx_pos);
            }

            // If we removed down to zero, impossible
            if (remaining.empty()) { feasible = false; break; }

            // If remaining size < k (uniform) it's no longer possible to keep any in group (they'd have to be suppressed)
            if ((int)remaining.size() < k) { feasible = false; break; }
        }

        if (!feasible) {
            total_suppress += original_size; // suppress whole group
        } else {
            total_suppress += (original_size - (int)remaining.size());
        }
    }

    return total_suppress;
}

    // Compute which record indices would be suppressed for a given combo using the same logic
    static std::vector<int> compute_suppressed_indices_for_combo(Dataset &ds, int aL, int eL, int mL, int rL, int k, int diversity) {
        std::map<std::tuple<std::string,std::string,std::string,std::string>, std::vector<int>> groups;
        for (size_t i = 0; i < ds.records.size(); ++i) {
            auto key = get_qi(ds.records[i], aL, eL, mL, rL);
            groups[key].push_back(static_cast<int>(i));
        }

        std::vector<int> suppressed_indices;
        for (auto &p : groups) {
            auto idxs = p.second; // copy
            int original_size = (int)idxs.size();
            if (original_size == 0) continue;

            std::map<occupations,int> occ_count_full;
            for (int idx : idxs) occ_count_full[ds.records[idx].occ]++;
            int distinct_full = (int)occ_count_full.size();

            if (distinct_full < diversity) {
                // suppress whole group
                suppressed_indices.insert(suppressed_indices.end(), idxs.begin(), idxs.end());
                continue;
            }

            std::vector<int> remaining = idxs;

            auto compute_strict_k = [&](const std::vector<int> &vec) {
                int mk = 1;
                for (int id : vec) mk = std::max(mk, ds.records[id].k);
                mk = std::max(mk, k);
                return mk;
            };

            auto compute_distinct = [&](const std::vector<int> &vec) {
                std::set<occupations> s;
                for (int id : vec) s.insert(ds.records[id].occ);
                return (int)s.size();
            };

            int cur_strict_k = compute_strict_k(remaining);
            int cur_distinct = compute_distinct(remaining);
            if ((int)remaining.size() >= cur_strict_k && cur_distinct >= diversity) continue;

            bool feasible = true;
            while (true) {
                cur_strict_k = compute_strict_k(remaining);
                cur_distinct = compute_distinct(remaining);
                int cur_size = (int)remaining.size();
                if (cur_size >= cur_strict_k && cur_distinct >= diversity) break;

                std::map<occupations,int> occ_count;
                for (int id : remaining) occ_count[ds.records[id].occ]++;

                int best_idx_pos = -1;
                int best_k = -1;
                for (size_t pos = 0; pos < remaining.size(); ++pos) {
                    int id = remaining[pos];
                    int rk = ds.records[id].k;
                    bool mult = (occ_count[ds.records[id].occ] > 1);
                    if (mult) {
                        if (rk > best_k) { best_k = rk; best_idx_pos = (int)pos; }
                    }
                }

                if (best_idx_pos == -1) {
                    int best_pos2 = -1;
                    int best_k2 = -1;
                    int best_occ_count = -1;
                    for (size_t pos = 0; pos < remaining.size(); ++pos) {
                        int id = remaining[pos];
                        int rk = ds.records[id].k;
                        int oc_c = occ_count[ds.records[id].occ];
                        if (rk > best_k2 || (rk == best_k2 && oc_c > best_occ_count)) {
                            best_k2 = rk; best_pos2 = (int)pos; best_occ_count = oc_c;
                        }
                    }
                    if (best_pos2 == -1) { feasible = false; break; }
                    int removed_id = remaining[best_pos2];
                    suppressed_indices.push_back(removed_id);
                    remaining.erase(remaining.begin() + best_pos2);
                } else {
                    int removed_id = remaining[best_idx_pos];
                    suppressed_indices.push_back(removed_id);
                    remaining.erase(remaining.begin() + best_idx_pos);
                }

                if (remaining.empty()) { feasible = false; break; }
                if ((int)remaining.size() < k) { feasible = false; break; }
            }

            if (!feasible) {
                // if infeasible, replace partially suppressed with full group suppression
                // remove any partial suppressed indices from this group and add the full group
                // For simplicity, we will remove any indices from this group's partial suppression from suppressed_indices
                for (int idx : idxs) {
                    // if idx is already in suppressed_indices, remove it
                    auto it = std::find(suppressed_indices.begin(), suppressed_indices.end(), idx);
                    if (it != suppressed_indices.end()) suppressed_indices.erase(it);
                }
                suppressed_indices.insert(suppressed_indices.end(), idxs.begin(), idxs.end());
            }
        }

        return suppressed_indices;
    }


std::tuple<int,int,int,int,int,float> achieve_k_and_l_diversity(Dataset &ds, int k, int diversity, int maxLevel) {
    // BFS over generalization combos; choose combo minimizing suppression, then minimizing distortion
    if (ds.records.empty()) return std::make_tuple(0,0,0,0,0, 0.0f);

    int max_age_level = MAX_AGE_LEVEL;
    int max_edu_level = MAX_EDU_LEVEL;
    int max_mar_level = MAX_MAR_LEVEL;
    int max_race_level = MAX_RACE_LEVEL;

    std::queue<std::tuple<int,int,int,int>> q;
    std::set<std::tuple<int,int,int,int>> tried;
    q.emplace(0,0,0,0);

    int best_suppress = std::numeric_limits<int>::max();
    float best_dist = std::numeric_limits<float>::max();
    std::tuple<int,int,int,int,int,float> best_result = std::make_tuple(max_age_level, max_edu_level, max_mar_level, max_race_level, ds.records.size(), std::numeric_limits<float>::max());

    while (!q.empty()) {
        auto cmb = q.front(); q.pop();
        if (tried.count(cmb)) continue;
        tried.insert(cmb);

        int aL = std::get<0>(cmb);
        int eL = std::get<1>(cmb);
        int mL = std::get<2>(cmb);
        int rL = std::get<3>(cmb);

        int suppress = compute_suppression_for_combo(ds, aL, eL, mL, rL, k, diversity);
        float dist = calculate_distortion(aL, eL, mL, rL);

        if (suppress < best_suppress || (suppress == best_suppress && dist < best_dist)) {
            best_suppress = suppress;
            best_dist = dist;
            best_result = std::make_tuple(aL, eL, mL, rL, suppress, dist);
        }

        // if we found a combo with zero suppression, that's best possible; return it immediately
        if (suppress == 0) {
            // compute suppressed indices (should be empty) and print for audit
            auto suppressed = compute_suppressed_indices_for_combo(ds, aL, eL, mL, rL, k, diversity);
            std::cout << "Suppressed records for chosen combo (should be none):\n";
            for (int idx : suppressed) {
                std::cout << "  idx=" << idx << " occ=" << to_string(ds.records[idx].occ) << "\n";
            }
            // write compact CSV and human-readable report for this chosen combo
            {
                // compact CSV (one-line summary)
                std::string csvpath = "k_l_diversity_result.csv";
                std::ofstream csv(csvpath);
                if (csv.is_open()) {
                    csv << "age,edu,mar,race,distortion,suppressed_count,suppressed_indices\n";
                    csv << aL << "," << eL << "," << mL << "," << rL << "," << dist << "," << suppress << ",\"";
                    for (size_t i = 0; i < suppressed.size(); ++i) {
                        if (i) csv << "|";
                        csv << suppressed[i];
                    }
                    csv << "\"\n";
                    csv.close();
                }

                // human-readable report similar to terminal output
                std::string rptpath = "k_l_diversity_report.txt";
                std::ofstream rpt(rptpath);
                if (rpt.is_open()) {
                    rpt << "Result: age=" << aL << " edu=" << eL << " mar=" << mL << " race=" << rL << " suppressed=" << suppress << " distortion=" << dist << "\n\n";

                    rpt << "Suppressed records (indices relative to dataset):\n";
                    if (suppressed.empty()) rpt << "(none)\n";
                    for (int idx : suppressed) {
                        const auto &rec = ds.records[idx];
                        rpt << idx << ": age=" << rec.age << " gen_age=" << generalize_age(rec.age, aL) << " edu=" << to_string(rec.edu)
                            << " mar=" << to_string(rec.marriage) << " race=" << to_string(rec.race) << " occ=" << to_string(rec.occ) << " k=" << rec.k << "\n";
                    }
                    rpt << "\nEquivalence classes:\n";

                    std::map<std::tuple<std::string,std::string,std::string,std::string>, std::vector<int>> groups;
                    for (size_t i = 0; i < ds.records.size(); ++i) {
                        auto key = get_qi(ds.records[i], aL, eL, mL, rL);
                        groups[key].push_back(static_cast<int>(i));
                    }
                    for (auto &p : groups) {
                        auto key = p.first;
                        auto &idxs = p.second;
                        rpt << "Class key: (" << std::get<0>(key) << ", " << std::get<1>(key) << ", " << std::get<2>(key) << ", " << std::get<3>(key) << ") size=" << idxs.size() << " occupations: ";
                        std::map<std::string,int> occ_counts;
                        for (int idx : idxs) occ_counts[to_string(ds.records[idx].occ)]++;
                        bool first = true;
                        for (auto &ocp : occ_counts) {
                            if (!first) rpt << ", ";
                            first = false;
                            rpt << ocp.first << "(" << ocp.second << ")";
                        }
                        rpt << "\n  indices: ";
                        for (size_t i = 0; i < idxs.size(); ++i) {
                            if (i) rpt << "|";
                            rpt << idxs[i];
                        }
                        rpt << "\n\n";
                    }

                    rpt.close();
                    std::cout << "Wrote k/l-diversity result to " << csvpath << " and verbose report to " << rptpath << std::endl;
                } else {
                    std::cerr << "Failed to write report to k_l_diversity_report.txt" << std::endl;
                }
            }
            return best_result;
        }

        // enqueue neighbors (within effective max)
        int effective_max_age = std::min(maxLevel, max_age_level);
        int effective_max_edu = std::min(maxLevel, max_edu_level);
        int effective_max_mar = std::min(maxLevel, max_mar_level);
        int effective_max_race = std::min(maxLevel, max_race_level);

        if (aL < effective_max_age) q.emplace(aL+1, eL, mL, rL);
        if (eL < effective_max_edu) q.emplace(aL, eL+1, mL, rL);
        if (mL < effective_max_mar) q.emplace(aL, eL, mL+1, rL);
        if (rL < effective_max_race) q.emplace(aL, eL, mL, rL+1);
    }

    // Before returning the best found combo, compute and print suppressed record indices for audit
    int ba = std::get<0>(best_result);
    int be = std::get<1>(best_result);
    int bm = std::get<2>(best_result);
    int br = std::get<3>(best_result);
    int bs = std::get<4>(best_result);
    float bd = std::get<5>(best_result);
    auto suppressed = compute_suppressed_indices_for_combo(ds, ba, be, bm, br, k, diversity);
    std::cout << "Suppressed records for best combo (suppressed_count=" << bs << ") distortion=" << bd << "):\n";
    for (int idx : suppressed) {
        std::cout << "  idx=" << idx << " occ=" << to_string(ds.records[idx].occ) << "\n";
    }
    // write verbose CSV-like result for best combo
    {
        std::string outpath = "k_l_diversity_result.csv";
        std::ofstream ofs(outpath);
        if (ofs.is_open()) {
            ofs << "age,edu,mar,race,distortion,suppressed_count,suppressed_indices\n";
            ofs << ba << "," << be << "," << bm << "," << br << "," << bd << "," << bs << ",\"";
            for (size_t i = 0; i < suppressed.size(); ++i) {
                if (i) ofs << "|";
                ofs << suppressed[i];
            }
            ofs << "\"\n\n";

            // Equivalence classes table
            ofs << "Equivalence Classes:\n";
            ofs << "ageKey,eduKey,marKey,raceKey,size,occupations,indices\n";
            std::map<std::tuple<std::string,std::string,std::string,std::string>, std::vector<int>> groups;
            for (size_t i = 0; i < ds.records.size(); ++i) {
                auto key = get_qi(ds.records[i], ba, be, bm, br);
                groups[key].push_back(static_cast<int>(i));
            }
            for (auto &p : groups) {
                auto key = p.first;
                auto &idxs = p.second;
                // write class key columns and size, then occupations as a quoted field
                ofs << std::get<0>(key) << "," << std::get<1>(key) << "," << std::get<2>(key) << "," << std::get<3>(key) << "," << idxs.size() << ",\"";
                std::map<std::string,int> occ_counts;
                for (int idx : idxs) occ_counts[to_string(ds.records[idx].occ)]++;
                bool first = true;
                for (auto &ocp : occ_counts) {
                    if (!first) ofs << "; ";
                    first = false;
                    ofs << ocp.first << "(" << ocp.second << ")";
                }
                // close quoted occupations field and write indices as the last column
                ofs << "\",";
                for (size_t ii = 0; ii < idxs.size(); ++ii) {
                    if (ii) ofs << "|";
                    ofs << idxs[ii];
                }
                ofs << "\n";
            }
            ofs << "\n";

            // Suppressed records details
            ofs << "Suppressed Records:\n";
            ofs << "idx,orig_age,gen_age,edu,mar,race,occupation,k\n";
            for (int idx : suppressed) {
                const auto &rec = ds.records[idx];
                ofs << idx << "," << rec.age << "," << generalize_age(rec.age, ba) << "," << to_string(rec.edu) << "," << to_string(rec.marriage) << "," << to_string(rec.race) << "," << to_string(rec.occ) << "," << rec.k << "\n";
            }

            ofs.close();
            std::cout << "Wrote k/l-diversity result to " << outpath << std::endl;
        } else {
            std::cerr << "Failed to write result to " << outpath << std::endl;
        }
    }
    return best_result;
}

// calculate distortion 
float calculate_distortion(int ageL, int eduL, int marL, int raceL) {
    // Using the predefined maximum levels for each attribute
    float age_dist = static_cast<float>(ageL) / MAX_AGE_LEVEL;
    float edu_dist = static_cast<float>(eduL) / MAX_EDU_LEVEL;
    float mar_dist = static_cast<float>(marL) / MAX_MAR_LEVEL;
    float race_dist = static_cast<float>(raceL) / MAX_RACE_LEVEL;
    return (static_cast<float>(age_dist + edu_dist + mar_dist + race_dist) / NUM_ATTR);
}

Record::Record(int a, int k_anon, education e, marital_status m, races r, occupations o) {
    age = a;
    k = k_anon;
    edu = e;
    marriage = m;
    race = r;
    occ = o;
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

static occupations parseOccupation(const std::string &raw) {
    std::string s = lower(trim(raw));
    if (s.find("tech-support") != std::string::npos || s.find("tech_support") != std::string::npos) return Tech_support;
    if (s.find("craft-repair") != std::string::npos || s.find("craft_repair") != std::string::npos) return Craft_repair;
    if (s.find("other-service") != std::string::npos || s.find("other_service") != std::string::npos) return Other_service;
    if (s.find("sales") != std::string::npos) return Sales;
    if (s.find("exec-managerial") != std::string::npos || s.find("exec_managerial") != std::string::npos) return Exec_managerial;
    if (s.find("prof-specialty") != std::string::npos || s.find("prof_specialty") != std::string::npos) return Prof_specialty;
    if (s.find("handlers-cleaners") != std::string::npos || s.find("handlers_cleaners") != std::string::npos) return Handlers_cleaners;
    if (s.find("machine-op-inspct") != std::string::npos || s.find("machine_op_inspct") != std::string::npos) return Machine_op_inspct;
    if (s.find("adm-clerical") != std::string::npos || s.find("adm_clerical") != std::string::npos) return Adm_clerical;
    if (s.find("farming-fishing") != std::string::npos || s.find("farming_fishing") != std::string::npos) return Farming_fishing;
    if (s.find("transport-moving") != std::string::npos || s.find("transport_moving") != std::string::npos) return Transport_moving;
    if (s.find("priv-house-serv") != std::string::npos || s.find("priv_house_serv") != std::string::npos) return Priv_house_serv;
    if (s.find("protective-serv") != std::string::npos || s.find("protective_serv") != std::string::npos) return Protective_serv;
    if (s.find("armed-forces") != std::string::npos || s.find("armed_forces") != std::string::npos) return Armed_Forces;
    // fallback
    return null_occ;
}
// generate a random value of 4, 5, or 7
int assign_rand_k(int k1, int k2, int k3) {
    int k_vals[] = {k1, k2, k3};
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
        int k = assign_rand_k(K1, K2, K3); // assign random k from given options
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
        occupations o = parseOccupation(fields[6]);

        records.emplace_back(age, k, edu, ms, r, o);
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

std::string to_string(occupations o) {
    switch (o) {
        case Tech_support: return "Tech-support";
        case Craft_repair: return "Craft-repair";
        case Other_service: return "Other-service";
        case Sales: return "Sales";
        case Exec_managerial: return "Exec-managerial";
        case Prof_specialty: return "Prof-specialty";
        case Handlers_cleaners: return "Handlers-cleaners";
        case Machine_op_inspct: return "Machine-op-inspct";
        case Adm_clerical: return "Adm-clerical";
        case Farming_fishing: return "Farming-fishing";
        case Transport_moving: return "Transport-moving";
        case Priv_house_serv: return "Priv-house-serv";
        case Protective_serv: return "Protective-serv";
        case Armed_Forces: return "Armed-Forces";
        case null_occ: return "Unknown";
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