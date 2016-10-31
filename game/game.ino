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

#include <digitalWriteFast.h>

#include <Firmata.h>

namespace pin {
   
   template <size_t Pin> struct out {
      out() {
         static_assert(IS_PIN_DIGITAL(Pin), "not acceptable digital pin");
         pinModeFast(Pin, OUTPUT);
      }
      void set(bool val) {
         digitalWriteFast(Pin, val ? HIGH : LOW);
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
   
   template<typename In, typename Out, size_t N>
   class state_machine {
      In in_states[N];
      Out out_states[N];
      size_t current_state;
      
   public:
      template<size_t X>
      void set_state(const typename In::value_type& in, const typename Out::value_type& out){
         in_states[X] = In(in);
         out_states[X] = Out(out);
      }
      
      const Out& next_state(const In& new_state) {
         if (current_state < N - 1) {
            if (in_states[current_state + 1] == new_state) {
               return out_states[++current_state];
            }
         }
         return out_states[current_state];
      }
   };
}

bool is_module_find_card_with_id(MFRC522& module, const String& uuid) {
   if (!module.PICC_IsNewCardPresent()) {
      return false;
   }
   
   if (!module.PICC_ReadCardSerial()) {
      return false;
   }
   
   static String content("");
   content = "";
   for (size_t i = 0; i < module.uid.size; ++i) {
      content.concat(String(module.uid.uidByte[i] < 0x10 ? "0" : ""));
      content.concat(String(module.uid.uidByte[i], HEX));
   }
   content.toLowerCase();
   return content.equals(uuid);
}

typedef finite::state<unsigned char, 4> input_type;
typedef finite::state<unsigned char, 6> output_type;

finite::state_machine<input_type, output_type, 7> machine;

void setup() {
   /*
    0)   ____          ()
    1)   a___          (1 2)
    2)   ab__          (1   3 4)
    3)   abc_          (1   3   5)
    4)   a_c_          (1   3   5)
    5)   abc_          (1 2 3 4 5)
    6)   abcd          (          out)
    */
   machine.set_state<0>(0b0000, 0b000000);
   machine.set_state<1>(0b0001, 0b000011);
   machine.set_state<2>(0b0011, 0b001101);
   machine.set_state<3>(0b0111, 0b010101);
   machine.set_state<4>(0b0101, 0b010101);
   machine.set_state<5>(0b0111, 0b011111);
   machine.set_state<6>(0b1111, 0b000001);
   
   Serial.begin(9600);
   while (!Serial);
   SPI.begin();
   
   reader_A.init();
   reader_B.init();
   reader_C.init();
   reader_D.init();
}

void loop() {
   
   static input_type inputs;
   
   inputs.set<0>(is_module_find_card_with_id(reader_A.module, String("062c565e")));
   inputs.set<1>(is_module_find_card_with_id(reader_B.module, String("062c565e")));
   inputs.set<2>(is_module_find_card_with_id(reader_C.module, String("56bbd45c")));
   inputs.set<3>(is_module_find_card_with_id(reader_D.module, String("5137d265")));
   
   auto outputs = machine.next_state(inputs);
   
   lamp_1.set(outputs.get<0>());
   lamp_2.set(outputs.get<1>());
   lamp_3.set(outputs.get<2>());
   lamp_4.set(outputs.get<3>());
   lamp_5.set(outputs.get<4>());
   
   output.set(outputs.get<5>());
}
