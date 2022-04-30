//
//  main.cpp
//  F.S.M
//
//  Created by Luciano Almeida on 21/04/22.
//

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
};

class Machine {
  std::unordered_map<size_t, std::unique_ptr<State>> _states;
  State *_startState{nullptr};
  std::unordered_set<const State *> _deadStates;

public:
  using Transitions = std::unordered_map<char, size_t>;

  Machine(const std::unordered_map<size_t, Transitions> &machineStates,
          const std::unordered_set<size_t> &finalStates, size_t startState) {
    for (auto &entry : machineStates) {
      const auto stateId = entry.first;
      _states.emplace(stateId, std::make_unique<State>(
                                   stateId, bool(finalStates.count(stateId))));
    }
    _startState = _states[startState].get();
    assert(_startState != nullptr && "Invalid start state");

    for (const auto &state : machineStates) {
      auto &stateObj = _states[state.first];
      bool isDeadState = true;
      for (auto &nextStates : state.second) {
        const auto &nextStateObj = _states[nextStates.second];
        stateObj->addNext(nextStates.first, nextStateObj.get());
        isDeadState &= stateObj == nextStateObj;
      }
      if (isDeadState) {
        _deadStates.insert(stateObj.get());
      }
    }
  }

  bool accept(const std::string &str) const noexcept {
    auto *curState = _startState;
    for (const auto ch : str) {
      curState = curState->next(ch);
      if (curState == nullptr || _deadStates.count(curState))
        return false;
    }
    return curState->isFinalState();
  }
};

// 01, 10, 001, 110 ... 111110000, 00001111
// A finite state machine that starts in zeros and ends in ones
// or starts in ones and ends in zeros
static std::unique_ptr<Machine> make01s10sMachine() {
  std::unordered_map<size_t, Machine::Transitions> machineStates;
  machineStates.insert({0, {{'0', 1}, {'1', 2}}});
  machineStates.insert({1, {{'0', 1}, {'1', 3}}});
  machineStates.insert({2, {{'1', 2}, {'0', 4}}});
  machineStates.insert({3, {{'1', 3}, {'0', 5}}});
  machineStates.insert({4, {{'0', 4}, {'1', 5}}});
  machineStates.insert({5, {{'1', 5}, {'0', 5}}}); // Dead State
  std::unordered_set<size_t> finalStates = {3, 4};

  return std::make_unique<Machine>(machineStates, finalStates,
                                   /*startState=*/0);
}

// A finite state machine starts in arbitrary 1's and 0' and ends with one or
// more zeros
static std::unique_ptr<Machine> makeEndInZerosMachine() {
  std::unordered_map<size_t, Machine::Transitions> machineStates;
  machineStates.insert({0, {{'0', 1}, {'1', 0}}});
  machineStates.insert({1, {{'0', 1}, {'1', 0}}});
  std::unordered_set<size_t> finalStates = {1};

  return std::make_unique<Machine>(machineStates, finalStates,
                                   /*startState=*/0);
}

inline void assertAccepted(const Machine &M, const std::string &str) {
  auto result = M.accept(str);
  std::cout << std::boolalpha << str << ": " << result << std::endl;
  assert(result);
}

inline void assertNotAccepted(const Machine &M, const std::string &str) {
  auto result = M.accept(str);
  std::cout << std::boolalpha << str << ": " << result << std::endl;
  assert(!result);
}

int main(int argc, const char * argv[]) {
  {
    auto M = make01s10sMachine();
    assertAccepted(*M, "001");
    assertAccepted(*M, "100");
    assertAccepted(*M, "001");
    assertNotAccepted(*M, "1010");
    assertNotAccepted(*M, "0110");
    assertNotAccepted(*M, "1");
    assertNotAccepted(*M, "0");
    assertAccepted(*M, "01");
    assertAccepted(*M, "10");
    assertAccepted(*M, "111110000");
    assertAccepted(*M, "000011111");
  }

  {
    std::cout << "Ends in zeros" << std::endl;
    auto M = makeEndInZerosMachine();
    assertNotAccepted(*M, "001");
    assertAccepted(*M, "100");
    assertAccepted(*M, "1010");
    assertAccepted(*M, "0110");
    assertNotAccepted(*M, "101");
    assertAccepted(*M, "1000100");
    assertAccepted(*M, "0000");
    assertNotAccepted(*M, "00001");
    assertNotAccepted(*M, "1111");
  }

  return 0;
}
