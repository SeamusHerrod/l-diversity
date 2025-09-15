#include "l-div.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

int from_l_div() {
    std::cout << "Function from_l_div called." << std::endl;
    return 0; // Placeholder return value
}

Record::Record(float a, education e, marital_status m, races r) {
    age = a;
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

Dataset::Dataset() {
    std::ifstream infile("../data/adult.data");
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
        try { age = std::stof(fields[0]); } catch(...) { age = 0.0f; }
        education edu = parseEducation(fields[3]);
        marital_status ms = parseMarital(fields[5]);
        races r = parseRace(fields[8]);

        records.emplace_back(age, edu, ms, r);
    }
}