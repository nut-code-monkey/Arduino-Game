//
//  main.cpp
//  test
//
//  Created by Maksym Lunin on 3/11/16.
//  Copyright © 2016 Maksym Lunin. All rights reserved.
//

#include <iostream>
#include "state_machine.h"

typedef finite::state<unsigned char, 4> card_state;
typedef finite::state<unsigned char, 6> lamp_state;

struct states {
   card_state state;
   lamp_state value;
   
   states(size_t A, size_t B, size_t C, size_t D, size_t L1, size_t L2, size_t L3, size_t L4, size_t L5, size_t out){
      state.set< 0 >(A != 0);
      state.set< 1 >(B != 0);
      state.set< 2 >(C != 0);
      state.set< 3 >(D != 0);
      
      value.set< 0 >(L1 != 0);
      value.set< 1 >(L2 != 0);
      value.set< 2 >(L3 != 0);
      value.set< 3 >(L4 != 0);
      value.set< 4 >(L5 != 0);
      value.set< 5 >(out != 0);
   }
};

#define CARDS(cards, ...) (cards, ##__VA_ARGS__,
#define PINS(pins, ...) pins, ##__VA_ARGS__)
const size_t _ = 0;
static states
____  CARDS(0,0,0,0)  PINS(_,_,_,_,_,_),
A___  CARDS(1,0,0,0)  PINS(1,2,_,_,_,_),
_B__  CARDS(0,1,0,0)  PINS(_,2,3,4,_,_),
__C_  CARDS(0,0,1,0)  PINS(_,_,_,4,5,_),
AB__  CARDS(1,1,0,0)  PINS(1,_,3,4,_,_),
_BC_  CARDS(0,1,1,0)  PINS(_,2,3,_,5,_),
A_C_  CARDS(1,0,1,0)  PINS(1,2,_,4,5,_),
ABC_  CARDS(1,1,1,0)  PINS(1,_,3,_,5,_),
A_C_2 CARDS(1,0,1,0)  PINS(1,2,3,4,5,_),
ABC_2 CARDS(1,1,1,0)  PINS(1,2,3,4,5,_),
ABCD  CARDS(1,1,1,1)  PINS(1,2,3,4,5,6);

finite::state_machine<card_state*, lamp_state, 16> machine(&____.state, ____.value);

void setup() {
   
#define FROM(node) ( &node.state,
#define TO(node) &node.state, node.value, #node)
   
   machine.transition<0>  FROM(____)  TO(____);

   machine.transition<1>  FROM(____)  TO(A___);
   machine.transition<2>  FROM(____)  TO(_B__);
   machine.transition<3>  FROM(____)  TO(__C_);
   
   machine.transition<4>  FROM(A___)  TO(AB__);
   machine.transition<5>  FROM(A___)  TO(A_C_);
   
   machine.transition<6>  FROM(_B__)  TO(AB__);
   machine.transition<7>  FROM(_B__)  TO(_BC_);
   
   machine.transition<8>  FROM(__C_)  TO(A_C_);
   machine.transition<9>  FROM(__C_)  TO(_BC_);
   
   machine.transition<10> FROM(AB__)  TO(ABC_);
   machine.transition<11> FROM(_BC_)  TO(ABC_);
   machine.transition<12> FROM(A_C_)  TO(ABC_);
   
   machine.transition<13> FROM(ABC_)  TO(A_C_2);
   
   machine.transition<14> FROM(A_C_2) TO(ABC_2);
   
   machine.transition<15> FROM(ABC_2) TO(ABCD);
}

void next(int bin){
   card_state inputs(bin);
   auto outputs = machine.next_state(&inputs);
   
   std::cout << "in: " << finite::to_string(inputs) << " -> " <<  finite::to_string(outputs)<< " (" << machine.state() << ")" <<  std::endl;
}

int main(int argc, const char * argv[]) {
   
   setup();
   
   next(0b0000);
   next(0b0010);
   next(0b0011);
   
   next(0b0010);
   
   next(0b0000);
   next(0b0111);
   next(0b0101);
   next(0b0111);
   next(0b1111);
   
   return 0;
}
