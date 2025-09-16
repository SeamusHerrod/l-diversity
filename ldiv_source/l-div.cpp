#include "l-div.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>

int from_l_div() {
    std::cout << "Function from_l_div called." << std::endl;
    return 0; // Placeholder return value
}

Record::Record(float a, int k_anon, education e, marital_status m, races r) {
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
        try { age = std::stof(fields[0]); } catch(...) { age = 0.0f; }
        education edu = parseEducation(fields[3]);
        marital_status ms = parseMarital(fields[5]);
        races r = parseRace(fields[8]);

        records.emplace_back(age, k, edu, ms, r);
    }
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