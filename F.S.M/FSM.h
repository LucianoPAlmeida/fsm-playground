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

class State {
  size_t _id;
  std::unordered_map<char, State *> _nextStates;
  bool _isFinal = false;

public:
  State(size_t id, bool isFinal) noexcept : _id(id), _isFinal(isFinal) {}

  void addNext(char c, State *next) noexcept { _nextStates.emplace(c, next); }

  State *next(char ch) const {
    auto it = _nextStates.find(ch);
    return it == _nextStates.end() ? nullptr : it->second;
  }

  bool isFinalState() const noexcept { return _isFinal; }

  bool hasAnyTransition() const noexcept { return !_nextStates.empty(); }
};

class Machine {
  std::unordered_map<size_t, std::unique_ptr<State>> _states;
  State *_startState{nullptr};
  std::unordered_set<const State *> _deadStates;

public:
  using Transitions = std::unordered_map<char, size_t>;
  using StateSet = std::unordered_set<size_t>;

  Machine(const StateSet &machineStates, size_t startState,
          const StateSet &finalStates) {
    for (auto stateId : machineStates) {
      _states.emplace(stateId, std::make_unique<State>(
                                   stateId, bool(finalStates.count(stateId))));
    }
    _startState = _states[startState].get();
    assert(_startState != nullptr && "Invalid start state");
  }

  void addTransition(size_t state, char input, size_t toState) {
    assert(_states.count(state) && _states.count(toState) &&
           "Invalid start state");

    auto *stateObj = _states[state].get();
    auto *toStateObj = _states[toState].get();
    stateObj->addNext(input, toStateObj);

    // If the new added transition is to a different state remove from dead
    // stats lists.
    if (_deadStates.count(stateObj) && stateObj != toStateObj) {
      _deadStates.erase(stateObj);
    } else if (stateObj == toStateObj && !stateObj->hasAnyTransition()) {
      _deadStates.insert(stateObj);
    }
  }

  bool accept(std::string_view str) const noexcept {
    auto *curState = _startState;
    for (const auto ch : str) {
      curState = curState->next(ch);
      if (curState == nullptr || _deadStates.count(curState)) {
        return false;
      }
    }
    return curState->isFinalState();
  }
};

#endif /* FSM_h */
