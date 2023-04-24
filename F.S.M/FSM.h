//
//  FSM.h
//  F.S.M
//
//  Created by Luciano Almeida on 20/01/23.
//

#ifndef FSM_h
#define FSM_h

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class Machine {
public:
  using state_set = std::unordered_set<size_t>;
  using transitions = std::unordered_map<char, size_t>;

private:
  state_set _states;
  size_t _startState;
  state_set _deadStates;
  state_set _finalStates;

  std::unordered_map<size_t, transitions> _nextStates;

  std::pair<const size_t, bool> next(const size_t currentState,
                                     char ch) const noexcept {
    const auto stateIt = _nextStates.find(currentState);
    if (stateIt == _nextStates.end()) {
      return {0, false};
    }

    const auto &nextList = stateIt->second;
    const auto it = nextList.find(ch);
    if (it == nextList.end()) {
      return std::make_pair(static_cast<size_t>(0), false);
    }
    return std::make_pair(it->second, true);
  }

  void addNext(const size_t fromState, char c, const size_t next) noexcept {
    _nextStates[fromState].emplace(c, next);
  }

  bool isFinalState(size_t state) const noexcept {
    return _finalStates.count(state);
  }

public:
  Machine(const state_set &machineStates, size_t startState,
          const state_set &finalStates)
      : _states(machineStates), _finalStates(finalStates),
        _startState(startState) {

    assert(machineStates.count(startState) && "Invalid start state");
  }

  bool hasAnyTransition(const size_t state) const noexcept {
    const auto it = _nextStates.find(state);
    return it != _nextStates.end() && !it->second.empty();
  }

  void addTransition(size_t state, char input, size_t toState) {
    assert(_states.count(state) && _states.count(toState) &&
           "Invalid start state");

    addNext(state, input, toState);

    // If the new added transition is to a different state remove from dead
    // stats lists.
    if (_deadStates.count(state) && state != toState) {
      _deadStates.erase(state);
    } else if (state == toState && !hasAnyTransition(state)) {
      _deadStates.insert(state);
    }
  }

  bool accept(std::string_view str) const noexcept {
    auto curState = _startState;
    for (const auto ch : str) {
      auto [nextState, found] = next(curState, ch);
      if (!found) {
        return false;
      }
      curState = nextState;
      if (_deadStates.count(curState)) {
        break;
      }
    }
    return isFinalState(curState);
  }
};

#endif /* FSM_h */
