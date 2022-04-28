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

public:
  Machine(const std::unordered_map<size_t, std::unordered_map<char, size_t>>
              &machineStates,
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
      for (auto &nextStates : state.second) {
        const auto &nextStateObj = _states[nextStates.second];
        stateObj->addNext(nextStates.first, nextStateObj.get());
      }
    }
  }

  bool accept(const std::string &str) const noexcept {
    const auto *curState = _startState;
    for (const auto ch : str) {
      curState = curState->next(ch);
      if (curState == nullptr)
        return false;
    }
    return curState->isFinalState();
  }
};

// 01, 10, 001, 110 ... 111110000, 00001111
// A finite state machine that starts in zeros and ends in ones
// or starts in ones and ends in zeros
static std::unique_ptr<Machine> make01s10sMachine() {
  std::unordered_map<size_t, std::unordered_map<char, size_t>> machineStates;
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

// 01, 10, 001, 110 ... 111110000, 00001111
// A finite state machine starts in arbitrary 1's and 0' and ends with one or
// more zeros
static std::unique_ptr<Machine> makeEndInZerosMachine() {
  std::unordered_map<size_t, std::unordered_map<char, size_t>> machineStates;
  machineStates.insert({0, {{'0', 1}, {'1', 0}}});
  machineStates.insert({1, {{'0', 1}, {'1', 0}}});
  std::unordered_set<size_t> finalStates = {1};

  return std::make_unique<Machine>(machineStates, finalStates,
                                   /*startState=*/0);
}

int main(int argc, const char * argv[]) {
  {
    auto M = make01s10sMachine();
    std::cout << std::boolalpha << "001: " << M->accept("001") << std::endl;
    std::cout << std::boolalpha << "100: " << M->accept("100") << std::endl;
    std::cout << std::boolalpha << "1010: " << M->accept("1010") << std::endl;
    std::cout << std::boolalpha << "0110: " << M->accept("0110") << std::endl;
    std::cout << std::boolalpha << "1: " << M->accept("1") << std::endl;
    std::cout << std::boolalpha << "0: " << M->accept("0") << std::endl;
    std::cout << std::boolalpha << "01: " << M->accept("01") << std::endl;
    std::cout << std::boolalpha << "10: " << M->accept("10") << std::endl;
    std::cout << std::boolalpha << "111110000: " << M->accept("111110000")
              << std::endl;
    std::cout << std::boolalpha << "000011111: " << M->accept("00001111")
              << std::endl;
  }

  {
    std::cout << "Ends in zeros" << std::endl;
    auto M = makeEndInZerosMachine();
    std::cout << std::boolalpha << "001: " << M->accept("001") << std::endl;
    std::cout << std::boolalpha << "100: " << M->accept("100") << std::endl;
    std::cout << std::boolalpha << "1010: " << M->accept("1010") << std::endl;
    std::cout << std::boolalpha << "0110: " << M->accept("0110") << std::endl;
    std::cout << std::boolalpha << "101: " << M->accept("101") << std::endl;
    std::cout << std::boolalpha << "1000100: " << M->accept("1000100")
              << std::endl;
    std::cout << std::boolalpha << "0000: " << M->accept("0000") << std::endl;
    std::cout << std::boolalpha << "00001: " << M->accept("00001") << std::endl;
    std::cout << std::boolalpha << "1111: " << M->accept("1111") << std::endl;
  }
  return 0;
}
