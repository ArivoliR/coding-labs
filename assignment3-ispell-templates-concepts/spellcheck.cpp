#include "spellcheck.h"

#include <algorithm>
#include <cinttypes>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ranges>
#include <set>
#include <vector>

template <typename Iterator, typename UnaryPred>
std::vector<Iterator> find_all(Iterator begin, Iterator end, UnaryPred pred);

Corpus tokenize(std::string &source) {
  Corpus corpus;
  auto spaces = find_all(source.begin(), source.end(), ::isspace);
  auto make_token = [&](auto begin, auto end) {
    return Token(source, begin, end);
  };

  std::transform(spaces.begin(), std::prev(spaces.end()),
                 std::next(spaces.begin()), std::inserter(corpus, corpus.end()),
                 make_token);
  auto empty_token = [](const Token &token) { return token.content.empty(); };
  std::erase_if(corpus, empty_token);

  return corpus;
}

std::set<Misspelling> spellcheck(const Corpus &source,
                                 const Dictionary &dictionary) {
  /* TODO: Implement this method */
  return std::set<Misspelling>();
};

/* Helper methods */

#include "utils.cpp"
