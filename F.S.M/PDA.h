//
//  PDA.h
//  F.S.M
//
//  Created by Luciano Almeida on 20/01/23.
//

#ifndef PDA_h
#define PDA_h

#include <vector>

namespace PDA {

class State {
  int _id;
  State(int id) : _id(id) {}
};

class Symbol {
  char _ch;

public:
  Symbol(char ch) : _ch(ch) {}
  static Symbol epsilon() { return Symbol('\0'); }

  bool isEpsilon() const noexcept { return _ch == '\0'; }
};

/// Represents a PDA transition.
class Transition {
  Symbol _read;
  Symbol _topStack;
  Symbol _pushToStack;
  State _toState;

public:
  Transition(Symbol read, Symbol top, Symbol toPush, State toState)
      : _read(read), _topStack(top), _pushToStack(toPush), _toState(toState) {}
};

class Automaton {
  // TODO: Implement
};
} // namespace PDA

#endif /* PDA_h */
