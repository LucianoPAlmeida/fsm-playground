//
//  PDA.h
//  F.S.M
//
//  Created by Luciano Almeida on 20/01/23.
//

#ifndef PDA_h
#define PDA_h

#include <cassert>
#include <set>
#include <stack>
#include <utility>
#include <vector>

namespace PDA {

struct state_hash;
struct symbol_hash;

class State {
protected:
  friend state_hash;

  int _id;

public:
  State(int id) : _id(id) {}
  bool operator==(const State &rhs) const { return _id == rhs._id; }
};

struct state_hash {
  std::size_t operator()(const PDA::State &k) const {
    return std::hash<int>()(k._id);
  }
};

using state_set = std::unordered_set<State, state_hash>;

class Symbol {
protected:
  friend symbol_hash;

  char _ch;

public:
  Symbol(char ch) : _ch(ch) {}
  static Symbol epsilon() { return Symbol('\0'); }

  inline bool is(char ch) const noexcept { return _ch == ch; }
  inline bool isEpsilon() const noexcept { return _ch == '\0'; }

  bool operator==(const Symbol &rhs) const { return _ch == rhs._ch; }
  bool operator!=(const Symbol &rhs) const { return _ch != rhs._ch; }
};

struct symbol_hash {
  std::size_t operator()(const PDA::Symbol &k) const {
    return std::hash<char>()(k._ch);
  }
};

using symbol_set = std::unordered_set<State, state_hash>;

/// Represents a PDA transition.
class Transition {
  Symbol _read;
  Symbol _topStack;
  Symbol _pushToStack;
  State _toState;

public:
  Transition(Symbol read, Symbol top, Symbol toPush, State toState)
      : _read(read), _topStack(top), _pushToStack(toPush), _toState(toState) {}

  bool isEpsilonTransition() const noexcept { return _read.isEpsilon(); }

  inline Symbol getInput() const noexcept { return _read; }
  inline Symbol getTop() const noexcept { return _topStack; }
  inline Symbol getPush() const noexcept { return _pushToStack; }
  inline State getToState() const noexcept { return _toState; }
};

using transition_list = std::vector<Transition>;
class StateTransitions {
  transition_list _transitions;
  transition_list _epsilonTransitions;

public:
  void add(const Transition transition) {
    if (transition.isEpsilonTransition()) {
      _epsilonTransitions.push_back(transition);
    } else {
      _transitions.push_back(transition);
    }
  }

  const transition_list &getTransitions() const noexcept {
    return _transitions;
  }

  const transition_list &getEpsilonTransitions() const noexcept {
    return _epsilonTransitions;
  }
};

class Automaton {
private:
  State _start;
  state_set _states;
  state_set _acceptingStates;

  std::unordered_map<State, StateTransitions, state_hash> _transitions;

  bool isAccepting(State s) const noexcept {
    return bool(_acceptingStates.count(s));
  }

public:
  // Implementation of Push Down Automaton (PDA)
  Automaton(State start, state_set states, state_set acceptingStates)
      : _start(start), _states(states), _acceptingStates(acceptingStates) {
    for (auto state : states) {
      _transitions.insert({state, {}});
    }
  }

  void addTransition(State fromState, State toState, Symbol input, Symbol top,
                     Symbol push) {
    _transitions[fromState].add({input, top, push, toState});
  }

  bool accept(std::string_view input) const {
    std::stack<Symbol> stack;
    size_t i = 0;
    return acceptImpl(input, stack, _start, i);
  }

private:
  static bool canTakeTransition(std::string_view input,
                                std::stack<Symbol> &stack, size_t i,
                                Transition symbolTransition) {
    const auto inputSymbol = symbolTransition.getInput();
    const auto top = symbolTransition.getTop();

    if (!inputSymbol.isEpsilon() && !inputSymbol.is(input[i])) {
      return false;
    }

    if (!top.isEpsilon()) {
      if (stack.empty())
        return false;
      if (top != stack.top())
        return false;
    }

    return true;
  }

  bool acceptImpl(std::string_view input, std::stack<Symbol> &stack,
                  State currentState, size_t i) const {
    auto it = _transitions.find(currentState);
    assert(it != _transitions.end() &&
           "All states are expected to have transitons even if empty");
    auto &[a, stateTransitions] = *it;

    auto attemptTakeTransitions = [this, &stack, &input,
                                   i](const transition_list &transitions) {
      for (const auto &symbolTransition : transitions) {
        if (!canTakeTransition(input, stack, i, symbolTransition)) {
          continue;
        }

        // Look into the top of stack and pop if not epsilon transition.
        const auto top = symbolTransition.getTop();
        if (!top.isEpsilon()) {
          assert(stack.top() == top && "Not the top of stack?");
          stack.pop();
        }

        // Push the new symbol to stack.
        const auto toPush = symbolTransition.getPush();
        if (!toPush.isEpsilon()) {
          stack.push(toPush);
        }

        // Because of undeterminism we have to attempt all possible transitions
        // for each state.
        const auto inputSymbol = symbolTransition.getInput();
        const auto toState = symbolTransition.getToState();
        auto nextI = inputSymbol.isEpsilon() ? i : i + 1;
        if (acceptImpl(input, stack, toState, nextI)) {
          return true;
        }

        // Restore the stack state if that transition is not accepted.
        if (!toPush.isEpsilon()) {
          assert(stack.top() == toPush && "Not the last pushed to stack?");
          stack.pop();
        }

        if (!top.isEpsilon()) {
          stack.push(top);
        }
      }
      return false;
    };

    if (attemptTakeTransitions(stateTransitions.getTransitions())) {
      return true;
    }

    // Attempt epsilon transitions last.
    if (attemptTakeTransitions(stateTransitions.getEpsilonTransitions())) {
      return true;
    }

    // Is only accepted if is in a final state after scanning whole input.
    return isAccepting(currentState) && i >= input.size();
  }
};
} // namespace PDA

#endif /* PDA_h */
