#pragma mark Arduino specific includes
#ifndef TEST
   // Core library for code-sense - IDE-based
#if defined(WIRING) // Wiring specific
#include "Wiring.h"
#elif defined(MAPLE_IDE) // Maple specific
#include "WProgram.h"
#elif defined(MPIDE) // chipKIT specific
#include "WProgram.h"
#elif defined(DIGISPARK) // Digispark specific
#include "Arduino.h"
#elif defined(ENERGIA) // LaunchPad specific
#include "Energia.h"
#elif defined(LITTLEROBOTFRIENDS) // LittleRobotFriends specific
#include "LRF.h"
#elif defined(MICRODUINO) // Microduino specific
#include "Arduino.h"
#elif defined(SPARK) || defined(PARTICLE) // Particle / Spark specific
#include "Arduino.h"
#elif defined(TEENSYDUINO) // Teensy specific
#include "Arduino.h"
#elif defined(REDBEARLAB) // RedBearLab specific
#include "Arduino.h"
#elif defined(ESP8266) // ESP8266 specific
#include "Arduino.h"
#elif defined(ARDUINO) // Arduino 1.0 and 1.5 specific
#include "Arduino.h"
#else // error
#error Platform not defined
#endif // end IDE

#include <SPI.h>
#include <MFRC522.h>
#include <Firmata.h>

namespace pin {
   
   template <size_t Pin> struct out {
      static_assert(IS_PIN_DIGITAL(Pin), "not acceptable digital pin");
      out() {
         pinMode(Pin, OUTPUT);
      }
      void set(bool val) {
         digitalWrite(Pin, val ? LOW : HIGH);
      }
   };
   
   template <size_t SS, size_t RST>
   struct card_reader {
      MFRC522 module;
      void init() {
         module.PCD_Init(SS, RST);
      }
   };
}

pin::card_reader<30, 31> reader_A;
pin::card_reader<32, 33> reader_B;
pin::card_reader<34, 35> reader_C;
pin::card_reader<36, 37> reader_D;

pin::out<2> lamp_1;
pin::out<3> lamp_2;
pin::out<4> lamp_3;
pin::out<5> lamp_4;
pin::out<6> lamp_5;
pin::out<7> output;

#endif // TEST
#pragma mark - finite state machine

namespace finite {
   
   template<typename T>
   const char * to_string(const T &t, char start){
      static char string[T::size() + 1];
      const size_t bytes = T::size();
      memset(string, '\0', bytes + 1);
      for (size_t byte = 0; byte < bytes; ++byte) {
         string[byte] = (t.get(byte)) ? start+byte : '_';
      }
      return string;
   }
   
   template <typename T, size_t Size = sizeof(T) * 8>
   class state {
      static_assert(Size <= sizeof(T) * 8, "");
      T _bits;
   public:
      state(): _bits(0) {}
      state(T b): _bits(b) {}
      typedef T value_type;
      
      template<size_t X>
      T get() const {
         static_assert(X < Size, "");
         return bitRead(_bits, X);
      }
      
      constexpr T get(size_t X) const {
         return bitRead(_bits, X);
      }
      
      template<size_t X>
      state& set(bool value) {
         static_assert(X < Size, "");
         bitWrite(_bits, X, value);
         return *this;
      }
      
      state& set(size_t X, bool value) {
         bitWrite(_bits, X, value);
         return *this;
      }
      
      constexpr T bits() const {
         return _bits;
      };
      constexpr static size_t size(){
         return Size;
      }
      constexpr bool operator == (const state& s) const {
         return _bits == s._bits;
      }
      
      const char* to_string(char start) const {
         return finite::to_string(*this, start);
      }
   };
   
   template<typename State, typename Value, size_t N>
   class state_machine {
      State from_states[N], to_states[N];
      State current_state;
      
      Value values[N];
      Value current_value;
      
      const char * names[N];
      const char* current_name;
   public:
      
      void reset() {
         current_state = from_states[0];
         current_value = values[0];
         current_name = names[0];
      }
      
      const char* state_name() const {
         return current_name;
      };
      
      state_machine(State s, Value v) : current_state(s), current_value(v), current_name("initial") {
         from_states[0] = current_state;
         to_states[0] = current_state;
         names[0] = current_name;
      }
   
      template<size_t X>
      void set_transition(const State from, const State to, const Value& value, const char* name){
         from_states[X] = from;
         to_states[X] = to;
         values[X] = value;
         names[X] = name;
      }
      
      const Value& next_state(const State state) {
         for (size_t i = 0; i < N; ++i) {
            if (from_states[i] == current_state && to_states[i]->bits() == state->bits()){
               current_state = to_states[i];
               current_value = values[i];
               current_name = names[i];
               return current_value;
            }
         }
         return current_value;
      }
   };
}

#ifndef TEST
/*
 * @brief : read MFRC522 each time function caled.
 * @return : string with TAG id
 */
bool is_module_find_card_with_id(MFRC522& module, const String& uuid) {
   byte atqa_answer[2];
   byte atqa_size = 2;
   if (MFRC522::STATUS_OK != module.PICC_WakeupA(atqa_answer, &atqa_size)) {
      return false;
   }
   
   if (!module.PICC_ReadCardSerial()) {
      return false;
   }
   
   static String content("");
   content.reserve(32);
   content = "";
   for (size_t i = 0; i < module.uid.size; ++i) {
      content.concat(String(module.uid.uidByte[i] < 0x10 ? "0" : ""));
      content.concat(String(module.uid.uidByte[i], HEX));
   }
   content.toLowerCase();
   return content.equals(uuid);
}

#endif // TEST

namespace states{
   typedef finite::state<unsigned char, 4> card;
   typedef finite::state<unsigned char, 6> lamp;
   
   /*
    * @brief : helper container for input & output bits vectors
    */
   struct in_out_pair {
      card state;
      lamp value;
      
      in_out_pair(bool A, bool B, bool C, bool D, bool L1, bool L2, bool L3, bool L4, bool L5, bool out){
         state
         .set< 0 >(A)
         .set< 1 >(B)
         .set< 2 >(C)
         .set< 3 >(D);
         
         value
         .set< 0 >(L1)
         .set< 1 >(L2)
         .set< 2 >(L3)
         .set< 3 >(L4)
         .set< 4 >(L5)
         .set< 5 >(out);
      }
   };
   
#define CARDS(cards, ...) (cards, ##__VA_ARGS__,
#define PINS_OUT(pins, ...) pins, ##__VA_ARGS__)
   
   const size_t _ = 0;
   static in_out_pair
   ____  CARDS(0,0,0,0)  PINS_OUT(_,_,_,_,_,_),
   A___  CARDS(1,0,0,0)  PINS_OUT(1,2,_,_,_,_),
   _B__  CARDS(0,1,0,0)  PINS_OUT(_,2,3,4,_,_),
   __C_  CARDS(0,0,1,0)  PINS_OUT(_,_,_,4,5,_),
   AB__  CARDS(1,1,0,0)  PINS_OUT(1,_,3,4,_,_),
   _BC_  CARDS(0,1,1,0)  PINS_OUT(_,2,3,_,5,_),
   A_C_  CARDS(1,0,1,0)  PINS_OUT(1,2,_,4,5,_),
   ABC_  CARDS(1,1,1,0)  PINS_OUT(1,_,3,_,5,_),
   A_C_2 CARDS(1,0,1,0)  PINS_OUT(1,2,3,4,5,_),
   ABC_2 CARDS(1,1,1,0)  PINS_OUT(1,2,3,4,5,_),
   ABCD  CARDS(1,1,1,1)  PINS_OUT(1,2,3,4,5,6);
}

finite::state_machine<states::card*, states::lamp, 16> machine(&states::____.state, states:: ____.value);

void setup_state_machine(){
   using namespace states;
#define FROM(node) (&node.state,
#define TO(node) &node.state, node.value, #node)
   
   machine.set_transition<1>  FROM(____)  TO(A___);
   machine.set_transition<2>  FROM(____)  TO(_B__);
   machine.set_transition<3>  FROM(____)  TO(__C_);
   
   machine.set_transition<4>  FROM(A___)  TO(AB__);
   machine.set_transition<5>  FROM(A___)  TO(A_C_);
   
   machine.set_transition<6>  FROM(_B__)  TO(AB__);
   machine.set_transition<7>  FROM(_B__)  TO(_BC_);
   
   machine.set_transition<8>  FROM(__C_)  TO(A_C_);
   machine.set_transition<9>  FROM(__C_)  TO(_BC_);
   
   machine.set_transition<10> FROM(AB__)  TO(ABC_);
   machine.set_transition<11> FROM(_BC_)  TO(ABC_);
   machine.set_transition<12> FROM(A_C_)  TO(ABC_);
   
   machine.set_transition<13> FROM(ABC_)  TO(A_C_2);
   
   machine.set_transition<14> FROM(A_C_2) TO(ABC_2);
   
   machine.set_transition<15> FROM(ABC_2) TO(ABCD);
}

#ifndef TEST

void setup() {
   setup_state_machine();
   
   Serial.begin(19200);
   while (!Serial);
   SPI.begin();
   
   reader_A.init();
   reader_B.init();
   reader_C.init();
   reader_D.init();
}

void loop() {
   static states::card inputs(0);
   
   inputs.set< 0 >(is_module_find_card_with_id(reader_A.module, String("56bbd45c")));
   inputs.set< 1 >(is_module_find_card_with_id(reader_B.module, String("b508d965")));
   inputs.set< 2 >(is_module_find_card_with_id(reader_C.module, String("062c565e")));
   inputs.set< 3 >(is_module_find_card_with_id(reader_D.module, String("1537d265")));
   
   auto outputs = machine.next_state(&inputs);
   
   lamp_1.set(outputs.get< 0 >());
   lamp_2.set(outputs.get< 1 >());
   lamp_3.set(outputs.get< 2 >());
   lamp_4.set(outputs.get< 3 >());
   lamp_5.set(outputs.get< 4 >());
   output.set(outputs.get< 5 >());
   
   Serial.println(String("in: ") + inputs.to_string('A')+" out: "+inputs.to_string('1')+" "+machine.state_name());
}

#endif // TEST
