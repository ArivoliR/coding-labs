/*
 * Assignment 1: Marriage Pact
 * Adapted by Tinkercademy from Stanford CS106L
 * (originally by Haven Whitney, with modifications by Fabio Ibanez
 * & Jacob Roberts-Baca).
 *
 * Complete each STUDENT TODO below. Read the README carefully — the
 * requirements there (ranges, projections, sample, reserve, no raw
 * for-loops in find_matches, iterator-safe erase in run_mixer) are
 * part of the assignment, not optional polish.
 */

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <iterator>
#include <random>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

/**
 * Reads `filename` line by line and returns the applicants.
 *
 * Requirements:
 *   - Take `filename` as `const std::string&`.
 *   - Call `reserve()` before populating, with a sensible capacity.
 *     Justify your choice in short_answer.txt.
 */
std::vector<std::string> get_applicants(const std::string &filename) {
  // STUDENT TODO: Implement this function.
  // throw std::runtime_error("Not implemented: get_applicants");

  std::ifstream file(filename);
  if (!file) {
    std::cerr << "Unable to access " << filename << "\n";
    std::exit(1);
  }

  std::vector<std::string> applicants;
  applicants.reserve(1000); // 1000 lines in students.txt

  std::string student;

  while (std::getline(file, student)) {
    applicants.push_back(student);
  }
  return applicants;
}

/**
 * Returns the initials of `name`, uppercased.
 *   e.g. initials("Marceline McMillan") == "MM"
 *
 * Requirements:
 *   - Parameter must be `std::string_view` (no allocation).
 */
std::string initials(std::string_view name) {
  // STUDENT TODO: Implement this function.
  // throw std::runtime_error("Not implemented: initials");

  std::string out;
  bool space{true};

  for (char c : name) {
    if (std::isspace(c)) {
      space = true;
    } else if (space) {
      out += std::toupper(c);
      space = false;
    }
  }
  return out;
}

/**
 * Returns every applicant in `students` who shares initials with `name`.
 *
 * Requirements:
 *   - No raw `for` loops. Use std::ranges::copy_if (or views::filter
 *     piped into a vector). Use a projection where it makes the call
 *     clearer.
 *   - Take `students` as `const std::vector<std::string>&`.
 */
std::vector<std::string>
find_matches(std::string_view name, const std::vector<std::string> &students) {
  // STUDENT TODO: Implement this function.
  // throw std::runtime_error("Not implemented: find_matches");

  std::string target = initials(name);
  std::vector<std::string> matches;

  std::ranges::copy_if(
      students, std::back_inserter(matches),
      [&](const std::string &student) { return student == target; }, initials);
  return matches;
}

/**
 * Returns one randomly-chosen match, or "NO MATCHES FOUND." if empty.
 *
 * Requirements:
 *   - Use std::sample with a seeded std::mt19937.
 *   - Do NOT use pop_back() or rand() % size.
 */
std::string get_match(const std::vector<std::string> &matches) {
  // STUDENT TODO: Implement this function.
  // throw std::runtime_error("Not implemented: get_match");
  if (matches.empty()) {
    return "NO MATCHES FOUND.";
  }

  std::mt19937 seed(std::random_device{}());
  std::vector<std::string> chosen(1);

  std::sample(matches.begin(), matches.end(), chosen.begin(), 1, seed);

  return chosen[0];
}

/**
 * Runs a multi-round mixer. In each round, scan the remaining
 * applicants left-to-right; for each applicant, look for another
 * applicant with the same initials still in the pool. If found,
 * pair them, remove both from `applicants`, and record the pair.
 * Continue rounds until a full pass yields no new pairs.
 *
 * `applicants` is mutated: paired names are removed. Whatever is
 * left over at the end is unpaired.
 *
 * Requirements:
 *   - The naive "iterate and erase as you go" approach WILL invalidate
 *     your iterator. You must handle this — see the README for the
 *     three acceptable strategies — and document your choice in
 *     short_answer.txt.
 */
std::vector<std::pair<std::string, std::string>>
run_mixer(std::vector<std::string> &applicants) {
  // STUDENT TODO: Implement this function.
  // throw std::runtime_error("Not implemented: run_mixer");
}

/* #### Please don't remove this line! #### */
#include "tests/utils.hpp"
