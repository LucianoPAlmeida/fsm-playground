//
//  regex-matcher.h
//  letcode-problems
//
//  Created by Luciano Almeida on 21/07/23.
//  Copyright © 2023 Luciano Almeida. All rights reserved.
//

#ifndef regex_matcher_h
#define regex_matcher_h

#include <optional>
#include <unordered_map>
#include <vector>

template <>
struct std::hash<std::pair<size_t, size_t>> {
  std::size_t operator()(const std::pair<size_t, size_t> &k) const {
    return std::hash<size_t>()(k.first) ^ std::hash<size_t>()(k.second);
  }
};

namespace regex {
class Token {
protected:
  char mValue;
  bool mZeroOrMore;

public:
  Token(char value, bool zeroOrMore) : mValue(value), mZeroOrMore(zeroOrMore) {}

  char getValue() const noexcept { return mValue; }
  bool isZeroOrMore() const noexcept { return mZeroOrMore; }
};

class PatternParser {
  std::string mPattern;
  size_t cur = 0;
  char getToken() const noexcept { return mPattern[cur]; }
  void consume() { ++cur; }

public:
  PatternParser(std::string pattern) : mPattern(std::move(pattern)) {}

  bool canGet() const noexcept { return cur < mPattern.size(); }

  Token next() {
    auto ch = getToken();
    consume();
    if (canGet() && getToken() == '*') {
      consume();
      return Token(ch, true);
    }
    return Token(ch, false);
  }
};

// Non-Deterministic Finite Automaton
class NFA {
  using symbol_transition_map =
      std::unordered_map<char, std::unordered_set<size_t>>;
  using transition_map = std::unordered_map<size_t, symbol_transition_map>;
  using state_attempt_set = std::unordered_set<std::pair<size_t, size_t>>;

  size_t mStartState{0};
  transition_map mTransitions;
  std::set<size_t> mFinalStates;

  void addTransition(Token token, size_t from, size_t to) {
    mTransitions[from][token.getValue()].insert(to);
  }

  void addZeroOrMoreSequenceTransitions(
      const std::vector<std::pair<size_t, Token>> &zeroOrMoreStates) {
    if (zeroOrMoreStates.size() <= 2) {
      return;
    }

    // All optinal states have a state transition that skips the
    // next one.
    for (size_t i = 0; i < zeroOrMoreStates.size() - 2; ++i) {
      for (size_t j = i + 2; j < zeroOrMoreStates.size(); ++j) {
        const auto &[stateI, _] = zeroOrMoreStates[i];
        const auto &[stateJ, tokenJ] = zeroOrMoreStates[j];
        addTransition(tokenJ, stateI, stateJ);
      }
    }
  }

public:
  NFA(PatternParser &parser) {
    auto curState = mStartState;
    std::optional<size_t> lastRequiredState;
    std::vector<std::pair<size_t, Token>> zeroOrMoreStates;

    size_t idx = 0;
    while (parser.canGet()) {
      auto token = parser.next();
      if (token.isZeroOrMore()) {
        if (curState == mStartState) {
          lastRequiredState = curState;
        }
        addTransition(token, curState, curState + 1);
        ++curState;
        addTransition(token, curState, curState);
        zeroOrMoreStates.push_back({curState, token});

        // For each required state before a sequence of
        // optional ones, add a transiton from it to all
        // in that sequence to allow for skiping each state.
        if (lastRequiredState.has_value()) {
          addTransition(token, *lastRequiredState, curState);
        }
      } else {
        addTransition(token, curState, curState + 1);
        // Add for each optional previous state a transition
        // to the next required, so each of then can be skipped.
        for (const auto &[state, _] : zeroOrMoreStates) {
          addTransition(token, state, curState + 1);
        }

        // A transition so sequence of optional states can be skipped.
        if (lastRequiredState.has_value()) {
          addTransition(token, *lastRequiredState, curState + 1);
        }

        // All optinal states have a state transition that skips the
        // next one.
        addZeroOrMoreSequenceTransitions(zeroOrMoreStates);

        zeroOrMoreStates.clear();
        ++curState;
        lastRequiredState = curState;
      }

      ++idx;
    }

    // All optinal states have a state transition that skips the
    // next one.
    addZeroOrMoreSequenceTransitions(zeroOrMoreStates);

    // Machine finishes with a sequence of one or more
    // states which are all final states.
    if (!zeroOrMoreStates.empty()) {
      for (const auto &[state, _] : zeroOrMoreStates) {
        mFinalStates.insert(state);
      }
      if (lastRequiredState.has_value()) {
        mFinalStates.insert(*lastRequiredState);
      }
    } else {
      mFinalStates.insert(curState);
    }
  }

  bool accept(std::string_view input) {
    state_attempt_set rejectedStates;
    return acceptImpl(input, mStartState, 0, rejectedStates);
  }

  void dump() {
    for (const auto &entry : mTransitions) {
      std::cout << entry.first << ": ";
      for (const auto &tentry : entry.second) {
        for (const auto state : tentry.second) {
          std::cout << "{ " << tentry.first << ", " << state << "}";
        }
      }
      std::cout << std::endl;
    }

    std::cout << "F: ";
    for (const auto fstate : mFinalStates) {
      std::cout << fstate << " ";
    }
    std::cout << std::endl;
  }

private:
  bool acceptImpl(std::string_view input, size_t curState, size_t idx,
                  state_attempt_set &rejectedStates) {
    // We read all input string.
    if (idx == input.size()) {
      return bool(mFinalStates.count(curState));
    }

    const auto inputChar = input[idx];

    auto &curStateTransitions = mTransitions[curState];

    auto attemptNext = [this, &rejectedStates, &input, &idx](size_t next) {
      // Memoization of attempted and rejected states for a given input
      // position and a next state. Is like a graph, if we did already
      // traverse that path for a given point and know that it isn't
      // accepted we don't need to go again.
      auto attempt = std::make_pair(next, idx + 1);
      if (rejectedStates.count(attempt))
        return false;

      if (acceptImpl(input, next, idx + 1, rejectedStates)) {
        return true;
      }

      rejectedStates.insert(attempt);
      return false;
    };


    // Attempt each possible transition to account for non-determinism.
    
    // Transitions for specific any symbol ".".
    for (size_t next : curStateTransitions['.']) {
      if (attemptNext(next))
        return true;
    }

    // Transitions for specific input symbol.
    for (size_t next : curStateTransitions[inputChar]) {
      if (attemptNext(next))
        return true;
    }

    return false;
  }
};

class Solution {
public:
  bool isMatch(const std::string &s, std::string p) {
    auto parser = PatternParser(std::move(p));
    auto SM = NFA(parser);
    return SM.accept(s);
  }
};
} // namespace regex

#endif /* regex_matcher_h */

