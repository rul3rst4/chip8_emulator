#include <bits/stdc++.h>
#include <cstdint>

using namespace std;

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_START_ADDRESS = 0x50;
const unsigned int DISPLAY_WIDTH = 64;
const unsigned int DISPLAY_HEIGHT = 32;

struct Vec2b {
  uint8_t x;
  uint8_t y;
}

class Rom {
 private:
  ifstream in;
  string filename;

 public:
  Rom(string filename) : filename(filename) {}
  vector<uint8_t> open_rom() {
    vector<uint8_t> data;
    in.open(filename, ios::binary | ios::in);
    while (in) {
      char c;
      in.get(c);
      if (in)
        data.push_back((uint8_t)c);
    }
    return move(data);
    // https://stackoverflow.com/questions/17473753/c11-return-value-optimization-or-move/17473869#17473869
    // https://stackoverflow.com/questions/22655059/why-is-it-ok-to-return-a-vector-from-a-function
  }
};

enum class OpCode {
  CLS,
  RET,
  SYS,
  JP_Vx_addr,
  JP_addr,
  CALL,
  SE_Vx_byte,
  SE_Vx_Vy,
  OR,
  AND,
  XOR,
  SUB,
  SHR,
  SUBN,
  SHL,
  SNE_byte,
  SNE_Vx_Vy,
  RND,
  DRW,
  SKP,
  SKNP,
  ADD_byte,
  ADD_Vx_Vy,
  ADD_I_Vx,
  LD_Vx_byte,
  LD_Vx_Vy,
  LD_I,
  LD_Vx_DT,
  LD_Vx_K,
  LD_DT_Vx,
  LD_ST_Vx,
  LD_F_Vx,
  LD_B_Vx,
  LD_I_Vx,
  LD_Vx_I,
};

enum class Registers {
  V0 = 0x00,
  V1 = 0x01,
  V2 = 0x02,
  V3 = 0x03,
  V4 = 0x04,
  V5 = 0x05,
  V6 = 0x06,
  V7 = 0x07,
  V8 = 0x08,
  V9 = 0x09,
  VA = 0x0A,
  VB = 0x0B,
  VC = 0x0C,
  VD = 0x0D,
  VE = 0x0E,
  VF = 0x0F
};

class Chip8 {
 public:
  unordered_map<Registers, uint8_t> m_registers;
  unordered_map<OpCode, function<void()>> instructions;
  vector<uint8_t> memory;
  vector<uint16_t> l_stack;  // stack
  uint16_t index_register;
  uint16_t program_counter;
  uint8_t stack_pointer;
  uint8_t delay_timer;
  uint8_t sound_timer;
  unordered_map<uint8_t, bool> input_keys;

  int display[DISPLAY_WIDTH][DISPLAY_HEIGHT];  // on is 0xFFFFFFFF and off is 0x00000000.
  uint16_t opcode;
  std::random_device rd;
  std::mt19937 mt;
  std::uniform_int_distribution<uint8_t> dist;

  const unsigned int FONTSET_SIZE = 80;
  uint8_t fontset[80] = {
      0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
      0x20, 0x60, 0x20, 0x20, 0x70,  // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
      0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
      0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
      0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
      0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
      0xF0, 0x80, 0xF0, 0x80, 0x80   // F
  };

 public:
  Chip8() : l_stack(16, 0), memory(0xFFF, 0), stack_pointer(0), program_counter(START_ADDRESS), mt(rd()), dist(0, 255U) {
    for (int i = 0; i < FONTSET_SIZE; i++) {
      memory[FONTSET_START_ADDRESS + 1] = fontset[i];
    }

    for (int i = 0; i <= 15; i++) {
      m_registers[static_cast<Registers>(i)] = 0;
    }
    for (int i = 0; i <= 15; i++) {
      input_keys[i] = false;
    }

    this->instructions[OpCode::CLS] = [&]() {
      for (int i = 0; i < DISPLAY_WIDTH; i++)
        for (int j = 0; j < DISPLAY_HEIGHT; j++)
          display[i][j] = 0;
    };
    this->instructions[OpCode::RET] = [&]() {
      stack_pointer--;
      program_counter = l_stack[stack_pointer];
    };
    this->instructions[OpCode::SYS] = [&]() {};
    this->instructions[OpCode::JP_addr] = [&]() {
      uint16_t address = opcode & 0x0FFFu;
      program_counter = address;
    };
    this->instructions[OpCode::CALL] = [&]() {
      l_stack[stack_pointer] = program_counter;
      stack_pointer++;
      program_counter = opcode & 0x0FFFu;
    };
    this->instructions[OpCode::SE_Vx_byte] = [&]() {
      uint8_t reg = (opcode & 0x0F00u) >> 8u;
      uint8_t kk = opcode & 0x00FFu;

      if (m_registers[static_cast<Registers>(reg)] == kk) {
        pg_next_instruction();
      }
    };
    this->instructions[OpCode::SNE_byte] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;
      uint8_t kk = opcode & 0x00FFu;

      if (m_registers[static_cast<Registers>(regx)] != kk) {
        pg_next_instruction();
      }
    };
    this->instructions[OpCode::SE_Vx_Vy] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;
      uint8_t regy = (opcode & 0x00F0u) >> 4u;

      if (m_registers[static_cast<Registers>(regx)] == m_registers[static_cast<Registers>(regy)]) {
        pg_next_instruction();
      }
    };
    this->instructions[OpCode::LD_Vx_byte] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;
      uint8_t kk = opcode & 0x00FFu;

      m_registers[static_cast<Registers>(regx)] = kk;
    };
    this->instructions[OpCode::ADD_byte] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;
      uint8_t kk = opcode & 0x00FFu;

      m_registers[static_cast<Registers>(regx)] += kk;
    };
    this->instructions[OpCode::LD_Vx_Vy] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;
      uint8_t regy = (opcode & 0x00F0u) >> 4u;

      m_registers[static_cast<Registers>(regx)] = m_registers[static_cast<Registers>(regy)];
    };
    this->instructions[OpCode::OR] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;
      uint8_t regy = (opcode & 0x00F0u) >> 4u;

      m_registers[static_cast<Registers>(regx)] |= m_registers[static_cast<Registers>(regy)];
    };
    this->instructions[OpCode::AND] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;
      uint8_t regy = (opcode & 0x00F0u) >> 4u;

      m_registers[static_cast<Registers>(regx)] &= m_registers[static_cast<Registers>(regy)];
    };
    this->instructions[OpCode::XOR] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;
      uint8_t regy = (opcode & 0x00F0u) >> 4u;

      m_registers[static_cast<Registers>(regx)] ^= m_registers[static_cast<Registers>(regy)];
    };
    this->instructions[OpCode::ADD_Vx_Vy] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;
      uint8_t regy = (opcode & 0x00F0u) >> 4u;

      m_registers[Registers::VF] = (uint8_t)(get_register_value(regx) + get_register_value(regy) > 0xFF);
      m_registers[static_cast<Registers>(regx)] += m_registers[static_cast<Registers>(regy)];
    };
    this->instructions[OpCode::SUB] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;
      uint8_t regy = (opcode & 0x00F0u) >> 4u;

      m_registers[Registers::VF] = (uint8_t)(get_register_value(regx) > get_register_value(regy));

      m_registers[static_cast<Registers>(regx)] -= m_registers[static_cast<Registers>(regy)];
    };
    this->instructions[OpCode::SHR] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;

      m_registers[Registers::VF] = get_register_value(regx) & 0x01;
      m_registers[get_register_enum(regx)] >>= 1;
    };
    this->instructions[OpCode::SUBN] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;
      uint8_t regy = (opcode & 0x00F0u) >> 4u;

      m_registers[Registers::VF] = (uint8_t)(get_register_value(regy) > get_register_value(regx));
      m_registers[get_register_enum(regx)] = m_registers[get_register_enum(regy)] - m_registers[get_register_enum(regx)];
    };
    this->instructions[OpCode::SHL] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;

      m_registers[Registers::VF] = (get_register_value(regx) & 0x80) >> 7u;
      m_registers[get_register_enum(regx)] <<= 1;
    };
    this->instructions[OpCode::SNE_Vx_Vy] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;
      uint8_t regy = (opcode & 0x00F0u) >> 4u;

      if (get_register_value(regx) != get_register_value(regy)) {
        pg_next_instruction();
      }
    };
    this->instructions[OpCode::LD_I] = [&]() {
      uint8_t address = opcode & 0x0FFFu;
      index_register = address;
    };
    this->instructions[OpCode::JP_Vx_addr] = [&]() {
      uint16_t address = (opcode & 0x0FFFu) + m_registers[Registers::V0];
      program_counter = address;
    };
    this->instructions[OpCode::RND] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;
      uint8_t kk = opcode & 0x00FFu;

      m_registers[get_register_enum(regx)] = dist(mt) & kk;
    };
    this->instructions[OpCode::DRW] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;
      uint8_t regy = (opcode & 0x00F0u) >> 4u;
      uint8_t n = opcode & 0x000Fu;

      Vec2b start_position = Vec2b{get_register_value(regx) % DISPLAY_WIDTH, get_register_value(regy) % DISPLAY_HEIGHT};
      m_registers[Registers::VF] = 0;
      for (int i = 0; i < n; i++) {
        uint8_t sprite_byte_1 = memory[index_register + i];

        for (int j = 0; j < 8; j++) {
          if (sprite_byte_1 & (1 << j)) {
            if (display[start_position.x + j][start_position.y + i]) {
              m_registers[Registers::VF] = 1;
            }
            display[start_position.x + j][start_position.y + i] ^= 0xFFFFFFFF;
          }
        }
      };
      this->instructions[OpCode::SKP] = [&]() {
        uint8_t regx = (opcode & 0x0F00u) >> 8u;
        uint8_t key = get_register_value(regx);

        if (input_keys[key]) {
          pg_next_instruction();
        }
      };
      this->instructions[OpCode::SKNP] = [&]() {
        uint8_t regx = (opcode & 0x0F00u) >> 8u;
        uint8_t key = get_register_value(regx);

        if (!input_keys[key]) {
          pg_next_instruction();
        }
      };
      this->instructions[OpCode::LD_Vx_DT] = [&]() {
        uint8_t regx = (opcode & 0x0F00u) >> 8u;

        m_registers[static_cast<Registers>(regx)] = delay_timer;
      };
      this->instructions[OpCode::LD_Vx_K] = [&]() {
        uint8_t regx = (opcode & 0x0F00u) >> 8u;

        int index = 0;
        for (auto& [key, value] : input_keys) {
          if (value) {
            m_registers[get_register_enum(regx)] = (uint8_t)index;
            return;
          }
          index++;
        }
        pg_previous_instruction();
      };
      this->instructions[OpCode::LD_DT_Vx] = [&]() {
        uint8_t regx = (opcode & 0x0F00u) >> 8u;

        delay_timer = m_registers[static_cast<Registers>(regx)];
      };
      this->instructions[OpCode::LD_ST_Vx] = [&]() {
        uint8_t regx = (opcode & 0x0F00u) >> 8u;

        sound_timer = m_registers[static_cast<Registers>(regx)];
      };
      this->instructions[OpCode::ADD_Vx_Vy] = [&]() {
        uint8_t regx = (opcode & 0x0F00u) >> 8u;

        index_register += m_registers[static_cast<Registers>(regx)];
      };
      this->instructions[OpCode::LD_F_Vx] = [&]() {
        uint8_t regx = (opcode & 0x0F00u) >> 8u;

        index_register = FONTSET_START_ADDRESS + (5 * get_register_value(regx));
      };
      this->instructions[OpCode::LD_B_Vx] = [&]() {
        uint8_t regx = (opcode & 0x0F00u) >> 8u;

        uint8_t reg_value = get_register_value(regx);

        memory[index_register + 2] = reg_value % 10;
        reg_value /= 10;
        memory[index_register + 1] = (reg_value % 10);
        reg_value /= 10;
        memory[index_register] = (reg_value % 10);
      };
      this->instructions[OpCode::LD_I_Vx] = [&]() {
        uint8_t regx = (opcode & 0x0F00u) >> 8u;

        for (uint8_t i = 0; i <= regx; i++) {
          memory[index_register + i] = get_register_value(i);
        }
      };
      this->instructions[OpCode::LD_Vx_I] = [&]() {
        uint8_t regx = (opcode & 0x0F00u) >> 8u;

        for (uint8_t i = 0; i <= regx; i++) {
          m_registers[get_register_enum(i)] = memory[index_register + i];
        }
      };
    }
  }
  inline Registers get_register_enum(uint8_t reg_value) { return static_cast<Registers>(reg_value); }

  inline uint8_t get_register_value(uint8_t reg_value) { return m_registers[static_cast<Registers>(reg_value)]; }

  inline void pg_next_instruction() { program_counter += 2; }
  inline void pg_previous_instruction() { program_counter -= 2; }
  void load_rom() {
    Rom rom("./res/Airplane.ch8");
    vector<uint8_t> c = rom.open_rom();

    for (int i; i < c.size(); i++) {
      memory[START_ADDRESS + i] = c[i];
    }

    cout << "Teste de final de arquivo" << endl;
  }
};

int main(int argc, char* argv[]) {
  Chip8 chip_8;

  chip_8.load_rom();
}
