//
//  main.cpp
//  F.S.M
//
//  Created by Luciano Almeida on 21/04/22.
//

#include "FSM.h"
#include "PDA.h"
#include <iostream>

// 01, 10, 001, 110 ... 111110000, 00001111
// A finite state machine that starts in zeros and ends in ones
// or starts in ones and ends in zeros
static std::unique_ptr<Machine> make01s10sMachine() {
  Machine::state_set finalStates = {3, 4};
  Machine::state_set q = {0, 1, 2, 3, 4, 5};

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
  Machine::state_set finalStates = {1};
  Machine::state_set q = {0, 1};

  auto machine = std::make_unique<Machine>(q, /*startState=*/0, finalStates);
  machine->addTransition(0, '0', 1);
  machine->addTransition(0, '1', 0);

  machine->addTransition(1, '0', 1);
  machine->addTransition(1, '1', 0);
  
  return machine;
}

// A finite state machine that contains either ..0100.. or ..0111..
static std::unique_ptr<Machine> makeContainsEither0100or0111() {
  Machine::state_set finalStates = {4, 6};
  Machine::state_set q = {0, 1, 2, 3, 4, 5, 6};

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

// A PDA that recognizes a { 0n 1n | n >= 0 }
static std::unique_ptr<PDA::Automaton>
makeStartWithZerosAndEndOnesWithSameCount() {
  auto start = PDA::State(0);
  auto s1 = PDA::State(1);
  auto s2 = PDA::State(2);
  auto finalState = PDA::State(3);
  PDA::state_set states{start, s1, s2, finalState};
  PDA::state_set acceptingStates{finalState};
  auto e = PDA::Symbol::epsilon();

  auto automaton =
      std::make_unique<PDA::Automaton>(start, states, acceptingStates);
  automaton->addTransition(start, s1, e, e, {'$'});         // ⍷, ⍷ -> $
  automaton->addTransition(s1, s1, {'0'}, e, {'0'});        // 0, ⍷ -> 0
  automaton->addTransition(s1, s2, e, e, e);                // ⍷, ⍷ -> ⍷
  automaton->addTransition(s2, s2, {'1'}, {'0'}, e);        // 1, 0 -> ⍷
  automaton->addTransition(s2, finalState, e, {'$'}, e);    // ⍷, $ -> ⍷
  return automaton;
}

template <class StateMachine>
inline void assertAccepted(const StateMachine &M, std::string_view str) {
  auto result = M.accept(str);
  std::cout << std::boolalpha << str << ": " << result << std::endl;
  assert(result);
}

template <class StateMachine>
inline void assertNotAccepted(const StateMachine &M, std::string_view str) {
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

  // PDA
  {
    auto A = makeStartWithZerosAndEndOnesWithSameCount();

    assertNotAccepted(*A, "001");
    assertNotAccepted(*A, "00011");
    assertNotAccepted(*A, "00011");
    assertNotAccepted(*A, "10");
    assertNotAccepted(*A, "0110");
    assertNotAccepted(*A, "0111");
    assertNotAccepted(*A, "0");
    assertNotAccepted(*A, "00001110");
    assertNotAccepted(*A, "000010111");
    assertNotAccepted(*A, "000001111");

    assertAccepted(*A, "");
    assertAccepted(*A, "01");
    assertAccepted(*A, "0011");
    assertAccepted(*A, "000111");
    assertAccepted(*A, "00001111");
  }
  return 0;
}
