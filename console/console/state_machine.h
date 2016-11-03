
   //
//  state_machine.h
//  game
//
//  Created by Maksym Lunin on 3/11/16.
//  Copyright © 2016 Maksym Lunin. All rights reserved.
//

#ifndef state_machine_h
#define state_machine_h

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

namespace finite {
   template <typename T, size_t Size = sizeof(T) * 8>
   class state {
      static_assert(Size <= sizeof(T) * 8, "");
      T _bits;
   public:
      
      state(): _bits(0) {};
      state(T b): _bits(b) {}
      typedef T value_type;
      
      template<size_t X>
      T get() const {
         static_assert(X < Size, "");
         return bitRead(_bits, X);
      }
      
      T get(size_t X) const {
         return bitRead(_bits, X);
      }
      
      template<size_t X>
      void set(bool value) {
         static_assert(X < Size, "");
         bitWrite(_bits, X, value);
      }
      
      T bits() const {
         return _bits;
      };
      
      constexpr static size_t size(){
         return Size;
      }
   };
   
   template<typename T>
   const char * to_string(const T &t){
      static char string[T::size() + 1];
      const size_t bytes = T::size();
      memset(string, '\0', bytes + 1);
      for (size_t byte = 0; byte < bytes; ++byte) {
         string[byte] = (t.get(byte)) ? '1' : '0';
      }
      return string;
   }
   
   template<typename State, typename Value>
   struct state_machine_transition {
      State from, to;
      Value end_value;
      state_machine_transition(const State f, const State t, const Value& v)
      : from(f), to(t), end_value(v) {}
   };
   
   template<typename State, typename Value, size_t N>
   class state_machine {
      State from_states[N], to_states[N];
      Value values[N];
      const char * names[N];
      State current_state;
      Value current_value;
      const char* current_name;
   public:
      const char* state() const {
         return current_name;
      };
      
      state_machine(State s, Value v) : current_state(s), current_value(v), current_name("initial") {}
      
      template<size_t X>
      void transition(const State from, const State to, const Value& value, const char* name){
         from_states[X] = from;
         to_states[X] = to;
         values[X] = value;
         names[X] = name;
      }
      
      const Value& next_state(const State new_state) {
         
         for (size_t i = 0; i < N; ++i) {
            if (from_states[i] == current_state && to_states[i]->bits() == new_state->bits()){
               current_state = to_states[i];
               current_value = values[i];
               current_name = names[i];
               break;
            }
         }
         
         return current_value;
      }
   };
}

#endif /* state_machine_h */
