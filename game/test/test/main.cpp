//
//  main.cpp
//  test
//
//  Created by Maksym Lunin on 3/11/16.
//  Copyright © 2016 Maksym Lunin. All rights reserved.
//

#include <iostream>

#define TEST 1

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#include "../../../game/game.ino"

#include <cassert>

states::lamp next(std::initializer_list<int> bin){
   
   states::card inputs(0);
   for (size_t i = 0; i < bin.size(); ++i) {
      inputs.set(i, *(bin.begin()+i));
   }
   auto outputs = machine.next_state(&inputs);
   std::cout << "in: " << finite::to_string(inputs, 'A') << " out: " << finite::to_string(outputs, '1')<< " " << machine.state() << std::endl;
   return outputs;
}

int main(int argc, const char * argv[]) {
   
   setup_state_machine();
   
   assert(next({0,0,0,0}) == states:: ____  .value);
   assert(next({0,1,0,0}) == states:: _B__  .value);
   assert(next({1,1,0,0}) == states:: AB__  .value);
   assert(next({0,1,0,0}) == states:: AB__  .value);
   assert(next({0,0,0,0}) == states:: AB__  .value);
   assert(next({1,1,1,0}) == states:: ABC_  .value);
   assert(next({1,0,1,0}) == states:: A_C_2 .value);
   assert(next({1,1,1,0}) == states:: ABC_2 .value);
   assert(next({1,1,1,1}) == states:: ABCD  .value);
   
   return 0;
}
