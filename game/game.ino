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
      out() {
         static_assert(IS_PIN_DIGITAL(Pin), "not acceptable digital pin");
         pinMode(Pin, OUTPUT);
      }
      void set(bool val) {
         digitalWrite(Pin, val ? HIGH : LOW);
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

pin::card_reader<0, 1> reader_A;
pin::card_reader<2, 3> reader_B;
pin::card_reader<4, 5> reader_C;
pin::card_reader<6, 7> reader_D;

pin::out<10> lamp_1;
pin::out<11> lamp_2;
pin::out<12> lamp_3;
pin::out<13> lamp_4;
pin::out<14> lamp_5;
pin::out<15> output;

namespace finite {
   template <typename T, size_t Size = sizeof(T) * 8>
   class state {
      static_assert(Size <= sizeof(T) * 8, "");
      T bits;
   public:
      state() = default;
      state(T b): bits(b) {}
      typedef T value_type;
      
      template<size_t X>
      T get() const {
         static_assert(X < Size, "");
         return bitRead(bits, X);
      }
      
      template<size_t X>
      void set(bool value) {
         static_assert(X < Size, "");
         bitWrite(bits, X, value);
      }
      
      operator T () const {
         return bits;
      }
   };
   
   template<typename State, typename Value>
   struct state_machine_transition {
      State from, to;
      Value end_value;
      state_machine_transition(const State& f, const State& t, const Value& v)
      : from(f), to(t), end_value(v) {}
   };
   
   template<typename State, typename Value, size_t N>
   class state_machine {
      State from_states[N], to_states[N];
      Value values[N];
      int current_state_idx;
      
   public:
      template<size_t X>
      void transition(const State& from, const State& to, const Value& value){
         from_states[X] = from;
         to_states[X] = to;
         values[X] = value;
      }
      
      const Value& next_state(const State& new_state) {
         const State& current_state = from_states[current_state_idx];
         for (size_t i = current_state_idx; i < N; ++i) {
            if (from_states[i] == current_state && to_states[i] == new_state) {
               current_state_idx = i;
               break;
            }
         }
         return values[current_state_idx];
      }
   };
}

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

finite::state_machine<card_state, lamp_state, 16> machine;

void setup() {
   
#define CARDS(cards, ...) (cards, ##__VA_ARGS__,
#define PINS(pins, ...) pins, ##__VA_ARGS__)
   const size_t _ = 0;
   states
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
   
#define FROM(node) (node.state,
#define TO(node) node.state, node.value)

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
   
   Serial.begin(19200);
   while (!Serial);
   SPI.begin();
   
   reader_A.init();
   reader_B.init();
   reader_C.init();
   reader_D.init();
}

void loop() {
   
   static card_state inputs;
   
   inputs.set< 0 >(is_module_find_card_with_id(reader_A.module, String("062c565e")));
   inputs.set< 1 >(is_module_find_card_with_id(reader_B.module, String("062c565e")));
   inputs.set< 2 >(is_module_find_card_with_id(reader_C.module, String("56bbd45c")));
   inputs.set< 3 >(is_module_find_card_with_id(reader_D.module, String("5137d265")));
   
   auto outputs = machine.next_state(inputs);
   
   lamp_1.set(outputs.get< 0 >());
   lamp_2.set(outputs.get< 1 >());
   lamp_3.set(outputs.get< 2 >());
   lamp_4.set(outputs.get< 3 >());
   lamp_5.set(outputs.get< 4 >());
   output.set(outputs.get< 5 >());
}
