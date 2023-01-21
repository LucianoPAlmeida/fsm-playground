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
