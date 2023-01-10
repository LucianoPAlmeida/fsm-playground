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
  
  bool hasAnyTransition() const noexcept { return !_nextStates.empty(); }
};

class Machine {
  std::unordered_map<size_t, std::unique_ptr<State>> _states;
  State *_startState{nullptr};
  std::unordered_set<const State *> _deadStates;

public:
  using Transitions = std::unordered_map<char, size_t>;
  using StateSet = std::unordered_set<size_t>;
  
  Machine(const StateSet &machineStates, size_t startState, const StateSet &finalStates) {
    for (auto stateId : machineStates) {
      _states.emplace(stateId, std::make_unique<State>(stateId, bool(finalStates.count(stateId))));
    }
    _startState = _states[startState].get();
    assert(_startState != nullptr && "Invalid start state");
  }
  
  void addTransition(size_t state, char input, size_t toState) {
    assert(_states.count(state) && _states.count(toState) && "Invalid start state");

    auto *stateObj = _states[state].get();
    auto *toStateObj = _states[toState].get();
    stateObj->addNext(input, toStateObj);
    
    // If the new added transition is to a different state remove from dead stats lists.
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

// 01, 10, 001, 110 ... 111110000, 00001111
// A finite state machine that starts in zeros and ends in ones
// or starts in ones and ends in zeros
static std::unique_ptr<Machine> make01s10sMachine() {
  Machine::StateSet finalStates = {3, 4};
  Machine::StateSet q = {0, 1, 2, 3, 4, 5};

  auto machine = std::make_unique<Machine>(q, /*startState=*/0, finalStates);
  machine->addTransition(0, '0', 1);
  machine->addTransition(0, '1', 2);
  
  machine->addTransition(1, '0', 1);
  machine->addTransition(1, '1', 3);

  machine->addTransition(2, '0', 4);
  machine->addTransition(2, '1', 2);

  machine->addTransition(3, '0', 5);
  machine->addTransition(3, '1', 3);
  
  machine->addTransition(4, '0', 4);
  machine->addTransition(4, '1', 5);
  
  // Dead state
  machine->addTransition(5, '0', 5);
  machine->addTransition(5, '1', 5);

  return machine;
}

// A finite state machine starts in arbitrary 1's and 0' and ends with one or
// more zeros
static std::unique_ptr<Machine> makeEndInZerosMachine() {
  Machine::StateSet finalStates = {1};
  Machine::StateSet q = {0, 1};

  auto machine = std::make_unique<Machine>(q, /*startState=*/0, finalStates);
  machine->addTransition(0, '0', 1);
  machine->addTransition(0, '1', 0);

  machine->addTransition(1, '0', 1);
  machine->addTransition(1, '1', 0);
  
  return machine;
}

// A finite state machine that contains either ..0100.. or ..0111..
static std::unique_ptr<Machine> makeContainsEither0100or0111() {
  Machine::StateSet finalStates = {4, 6};
  Machine::StateSet q = {0, 1, 2, 3, 4, 5, 6};

  auto machine = std::make_unique<Machine>(q, /*startState=*/0, finalStates);

  machine->addTransition(0, '0', 1);
  machine->addTransition(0, '1', 0);

  machine->addTransition(1, '0', 1);
  machine->addTransition(1, '1', 2);

  machine->addTransition(2, '0', 3);
  machine->addTransition(2, '1', 5);

  machine->addTransition(3, '0', 4);
  machine->addTransition(3, '1', 2);

  machine->addTransition(4, '0', 4);
  machine->addTransition(4, '1', 4);

  machine->addTransition(5, '0', 1);
  machine->addTransition(5, '1', 6);

  machine->addTransition(6, '0', 6);
  machine->addTransition(6, '1', 6);

  return machine;
}

inline void assertAccepted(const Machine &M, std::string_view str) {
  auto result = M.accept(str);
  std::cout << std::boolalpha << str << ": " << result << std::endl;
  assert(result);
}

inline void assertNotAccepted(const Machine &M, std::string_view str) {
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

  {
    std::cout << "Either contains 0100 or 0111" << std::endl;
    auto M = makeContainsEither0100or0111();
    assertNotAccepted(*M, "01");
    assertNotAccepted(*M, "001");
    assertNotAccepted(*M, "00001");
    assertNotAccepted(*M, "1111");
    assertAccepted(*M, "0111");
    assertAccepted(*M, "0100");
    assertNotAccepted(*M, "101011");
    assertAccepted(*M, "00110100");
    assertAccepted(*M, "00110111");
    assertAccepted(*M, "0101001011");
    assertAccepted(*M, "010101010111010101");
  }
  return 0;
}
