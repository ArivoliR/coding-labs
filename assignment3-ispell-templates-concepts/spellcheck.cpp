#include "spellcheck.h"

#include <algorithm>
#include <cinttypes>
#include <iostream>
#include <iterator>
#include <numeric>
#include <ranges>
#include <set>
#include <vector>

template <std::input_iterator Iterator,
          std::indirect_unary_predicate<Iterator> UnaryPred>
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
  namespace rv = std::ranges::views;

  auto is_misspelled = [&](const Token &token) {
    return !dictionary.contains(token.content);
  };

  auto make_misspelling = [&](const Token &token) {
    auto is_one_edit_away = [&](const std::string &word) {
      return levenshtein(token.content, word) == 1;
    };

    auto suggestion_view = dictionary | rv::filter(is_one_edit_away);
    std::set<std::string> suggestions(suggestion_view.begin(),
                                      suggestion_view.end());

    return Misspelling{token, suggestions};
  };

  auto has_suggestions = [](const Misspelling &m) {
    return !m.suggestions.empty();
  };

  auto view = source | rv::filter(is_misspelled) |
              rv::transform(make_misspelling) | rv::filter(has_suggestions);

  return std::set<Misspelling>(view.begin(), view.end());
};

/* Helper methods */

#include "utils.cpp"
