//
//  main.cpp
//  test
//
//  Created by Maksym Lunin on 3/11/16.
//  Copyright © 2016 Maksym Lunin. All rights reserved.
//

#include <iostream>

#define BOOST_TEST_MAIN 1
#define BOOST_TEST_DYN_LINK

#define BOOST_TEST_MODULE Test
#include <boost/test/unit_test.hpp>

#define TEST 1

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#include "../../../game/game.ino"

#include <cassert>

using namespace states;

states::lamp next(std::initializer_list<int> bin){
   
   states::card inputs(0);
   for (size_t i = 0; i < bin.size(); ++i) {
      inputs.set(i, *(bin.begin()+i));
   }
   auto outputs = machine.next_state(&inputs);
   std::cout << "in: " << inputs.to_string('A') << " out: " << outputs.to_string('1')<< " " << machine.state_name() << std::endl;
   return outputs;
}


BOOST_AUTO_TEST_CASE(ABC){
   setup_state_machine();
   machine.reset();

   BOOST_CHECK(next({0,0,0,0}) ==  ____ .value);
   BOOST_CHECK(next({1,0,0,0}) ==  A___ .value);
   BOOST_CHECK(next({1,1,0,0}) ==  AB__ .value);
   BOOST_CHECK(next({1,1,1,0}) ==  ABC_ .value);
   BOOST_CHECK(next({1,0,1,0}) ==  A_C_2.value);
   BOOST_CHECK(next({1,1,1,0}) ==  ABC_2.value);
   BOOST_CHECK(next({1,1,1,1}) ==  ABCD .value);
}

BOOST_AUTO_TEST_CASE(ACB){
   setup_state_machine();
   machine.reset();
   
   BOOST_CHECK(next({0,0,0,0}) ==  ____ .value);
   BOOST_CHECK(next({1,0,0,0}) ==  A___ .value);
   BOOST_CHECK(next({1,0,1,0}) ==  A_C_ .value);
   BOOST_CHECK(next({1,1,1,0}) ==  ABC_ .value);
   BOOST_CHECK(next({1,0,1,0}) ==  A_C_2.value);
   BOOST_CHECK(next({1,1,1,0}) ==  ABC_2.value);
   BOOST_CHECK(next({1,1,1,1}) ==  ABCD .value);
}

BOOST_AUTO_TEST_CASE (BAC) {
   setup_state_machine();
   machine.reset();
   
   BOOST_CHECK(next({0,0,0,0}) == ____  .value);
   BOOST_CHECK(next({0,1,0,0}) == _B__  .value);
   BOOST_CHECK(next({1,1,0,0}) == AB__  .value);
   BOOST_CHECK(next({1,1,1,0}) == ABC_  .value);
   BOOST_CHECK(next({1,0,1,0}) == A_C_2 .value);
   BOOST_CHECK(next({1,1,1,0}) == ABC_2 .value);
   BOOST_CHECK(next({1,1,1,1}) == ABCD  .value);
}

BOOST_AUTO_TEST_CASE (BCA) {
   setup_state_machine();
   machine.reset();
   
   BOOST_CHECK(next({0,0,0,0}) == ____  .value);
   BOOST_CHECK(next({0,1,0,0}) == _B__  .value);
   BOOST_CHECK(next({0,1,1,0}) == _BC_  .value);
   BOOST_CHECK(next({1,1,1,0}) == ABC_  .value);
   BOOST_CHECK(next({1,0,1,0}) == A_C_2 .value);
   BOOST_CHECK(next({1,1,1,0}) == ABC_2 .value);
   BOOST_CHECK(next({1,1,1,1}) == ABCD  .value);
}


BOOST_AUTO_TEST_CASE (CAB) {
   setup_state_machine();
   machine.reset();
   
   BOOST_CHECK(next({0,0,0,0}) == ____  .value);
   BOOST_CHECK(next({0,0,1,0}) == __C_  .value);
   BOOST_CHECK(next({1,0,1,0}) == A_C_  .value);
   BOOST_CHECK(next({1,1,1,0}) == ABC_  .value);
   BOOST_CHECK(next({1,0,1,0}) == A_C_2 .value);
   BOOST_CHECK(next({1,1,1,0}) == ABC_2 .value);
   BOOST_CHECK(next({1,1,1,1}) == ABCD  .value);
}

BOOST_AUTO_TEST_CASE (CBA) {
   setup_state_machine();
   machine.reset();
   
   BOOST_CHECK(next({0,0,0,0}) == ____  .value);
   BOOST_CHECK(next({0,0,1,0}) == __C_  .value);
   BOOST_CHECK(next({0,1,1,0}) == _BC_  .value);
   BOOST_CHECK(next({1,1,1,0}) == ABC_  .value);
   BOOST_CHECK(next({1,0,1,0}) == A_C_2 .value);
   BOOST_CHECK(next({1,1,1,0}) == ABC_2 .value);
   BOOST_CHECK(next({1,1,1,1}) == ABCD  .value);
}


BOOST_AUTO_TEST_CASE(wrong_ABC){
   setup_state_machine();
   machine.reset();
   
   BOOST_CHECK(next({0,0,0,0}) ==  ____ .value);
   BOOST_CHECK(next({1,0,0,0}) ==  A___ .value);
   BOOST_CHECK(next({0,0,0,0}) ==  A___ .value);
   BOOST_CHECK(next({1,1,0,0}) ==  AB__ .value);
   BOOST_CHECK(next({0,0,0,0}) ==  AB__ .value);
   BOOST_CHECK(next({1,1,1,0}) ==  ABC_ .value);
   BOOST_CHECK(next({0,0,0,0}) ==  ABC_ .value);
   BOOST_CHECK(next({1,0,1,0}) ==  A_C_2.value);
   BOOST_CHECK(next({0,0,0,0}) ==  A_C_2.value);
   BOOST_CHECK(next({1,1,1,0}) ==  ABC_2.value);
   BOOST_CHECK(next({0,0,0,0}) ==  ABC_2.value);
   BOOST_CHECK(next({1,1,1,1}) ==  ABCD .value);
}

