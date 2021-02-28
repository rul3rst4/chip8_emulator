#include <bits/stdc++.h>
#include <cstdint>
#include <SDL2/SDL.h>

using namespace std;

const unsigned int START_ADDRESS = 0x200;
const unsigned int FONTSET_START_ADDRESS = 0x50;
const uint8_t DISPLAY_WIDTH = 64;
const uint8_t DISPLAY_HEIGHT = 32;

class Platform {
 public:
  Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeight) {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow(title, 0, 0, windowWidth, windowHeight, SDL_WINDOW_SHOWN);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, textureWidth, textureHeight);
  }

  ~Platform() {
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
  }

  void Update(void const* buffer, int pitch) {
    SDL_UpdateTexture(texture, nullptr, buffer, pitch);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(renderer);
  }

  bool ProcessInput(unordered_map<uint8_t, bool>& keys) {
    bool quit = false;

    SDL_Event event;

    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT: {
          quit = true;
        } break;

        case SDL_KEYDOWN: {
          switch (event.key.keysym.sym) {
            case SDLK_ESCAPE: {
              quit = true;
            } break;

            case SDLK_x: {
              keys[0] = 1;
            } break;

            case SDLK_1: {
              keys[1] = 1;
            } break;

            case SDLK_2: {
              keys[2] = 1;
            } break;

            case SDLK_3: {
              keys[3] = 1;
            } break;

            case SDLK_q: {
              keys[4] = 1;
            } break;

            case SDLK_w: {
              keys[5] = 1;
            } break;

            case SDLK_e: {
              keys[6] = 1;
            } break;

            case SDLK_a: {
              keys[7] = 1;
            } break;

            case SDLK_s: {
              keys[8] = 1;
            } break;

            case SDLK_d: {
              keys[9] = 1;
            } break;

            case SDLK_z: {
              keys[0xA] = 1;
            } break;

            case SDLK_c: {
              keys[0xB] = 1;
            } break;

            case SDLK_4: {
              keys[0xC] = 1;
            } break;

            case SDLK_r: {
              keys[0xD] = 1;
            } break;

            case SDLK_f: {
              keys[0xE] = 1;
            } break;

            case SDLK_v: {
              keys[0xF] = 1;
            } break;
          }
        } break;

        case SDL_KEYUP: {
          switch (event.key.keysym.sym) {
            case SDLK_x: {
              keys[0] = 0;
            } break;

            case SDLK_1: {
              keys[1] = 0;
            } break;

            case SDLK_2: {
              keys[2] = 0;
            } break;

            case SDLK_3: {
              keys[3] = 0;
            } break;

            case SDLK_q: {
              keys[4] = 0;
            } break;

            case SDLK_w: {
              keys[5] = 0;
            } break;

            case SDLK_e: {
              keys[6] = 0;
            } break;

            case SDLK_a: {
              keys[7] = 0;
            } break;

            case SDLK_s: {
              keys[8] = 0;
            } break;

            case SDLK_d: {
              keys[9] = 0;
            } break;

            case SDLK_z: {
              keys[0xA] = 0;
            } break;

            case SDLK_c: {
              keys[0xB] = 0;
            } break;

            case SDLK_4: {
              keys[0xC] = 0;
            } break;

            case SDLK_r: {
              keys[0xD] = 0;
            } break;

            case SDLK_f: {
              keys[0xE] = 0;
            } break;

            case SDLK_v: {
              keys[0xF] = 0;
            } break;
          }
        } break;
      }
    }

    return quit;
  }

 private:
  SDL_Window* window{};
  SDL_Renderer* renderer{};
  SDL_Texture* texture{};
};

struct Vec2b {
  uint8_t x;
  uint8_t y;
};

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
    return data;
    // https://stackoverflow.com/questions/17473753/c11-return-value-optimization-or-move/17473869#17473869
    // https://stackoverflow.com/questions/22655059/why-is-it-ok-to-return-a-vector-from-a-function
  }
};

enum class OpCode {
  CLS = 0x00E0,         // 00E0
  RET = 0x00EE,         // 00EE
  SYS = 0,              // 0nnn
  JP_addr,              // 1nnn
  CALL,                 // 2nnn
  SE_Vx_byte,           // 3xkk
  SNE_byte,             // 4xkk
  SE_Vx_Vy,             // 5xy0
  LD_Vx_byte,           // 6xkk
  ADD_byte,             // 7xkk
  LD_Vx_Vy = 0x8000,    // 8xy0
  OR = 0x8001,          // 8xy1
  AND = 0x8002,         // 8xy2
  XOR = 0x8003,         // 8xy3
  ADD_Vx_Vy = 0x8004,   // 8xy4
  SUB = 0x8005,         // 8xy5
  SHR = 0x8006,         // 8xy6
  SUBN = 0x8007,        // 8xy7
  SHL = 0x800E,         // 8xyE
  SNE_Vx_Vy = 0x9000,   // 9xy0
  LD_I = 0xA000,        // Annn
  JP_Vx_addr = 0xB000,  // Bnnn
  RND = 0xC000,         // Cxkk
  DRW = 0xD000,         // Dxyn
  SKP = 0xE09E,         // Ex9E
  SKNP = 0xE0A1,        // ExA1
  LD_Vx_DT = 0xF007,    // Fx07
  LD_Vx_K = 0xF00A,     // Fx0A
  LD_DT_Vx = 0xF015,    // Fx15
  LD_ST_Vx = 0xF018,    // Fx18
  ADD_I_Vx = 0xF01E,    // Fx1E
  LD_F_Vx = 0xF029,     // Fx29
  LD_B_Vx = 0xF033,     // Fx33
  LD_I_Vx = 0xF055,     // Fx55
  LD_Vx_I = 0xF065,     // Fx65
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

  uint32_t display[DISPLAY_WIDTH * DISPLAY_HEIGHT];  // on is 0xFFFFFFFF and off is 0x00000000.
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
      memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    for (int i = 0; i <= 15; i++) {
      m_registers[static_cast<Registers>(i)] = 0;
    }
    for (int i = 0; i <= 15; i++) {
      input_keys[i] = false;
    }
    init_instructions_umap();
  }
  ~Chip8() {}
  void init_instructions_umap() {
    this->instructions[OpCode::CLS] = [&]() { memset(display, 0, sizeof(display)); };
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

      uint16_t sum = get_register_value(regx) + get_register_value(regy);

      m_registers[Registers::VF] = (sum > 255U);
      m_registers[static_cast<Registers>(regx)] = sum & 0xFFu;
    };
    this->instructions[OpCode::SUB] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;
      uint8_t regy = (opcode & 0x00F0u) >> 4u;

      m_registers[Registers::VF] = (get_register_value(regx) > get_register_value(regy));

      m_registers[static_cast<Registers>(regx)] -= m_registers[static_cast<Registers>(regy)];
    };
    this->instructions[OpCode::SHR] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;

      m_registers[Registers::VF] = get_register_value(regx) & 0x1u;
      m_registers[get_register_enum(regx)] >>= 1;
    };
    this->instructions[OpCode::SUBN] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;
      uint8_t regy = (opcode & 0x00F0u) >> 4u;

      m_registers[Registers::VF] = (get_register_value(regy) > get_register_value(regx));
      m_registers[get_register_enum(regx)] = m_registers[get_register_enum(regy)] - m_registers[get_register_enum(regx)];
    };
    this->instructions[OpCode::SHL] = [&]() {
      uint8_t regx = (opcode & 0x0F00u) >> 8u;

      m_registers[Registers::VF] = (get_register_value(regx) & 0x80u) >> 7u;
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
      uint16_t address = opcode & 0x0FFFu;
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
      uint8_t Vx = (opcode & 0x0F00u) >> 8u;
      uint8_t Vy = (opcode & 0x00F0u) >> 4u;
      uint8_t height = opcode & 0x000Fu;

      // Wrap if going beyond screen boundaries
      uint8_t xPos = m_registers[static_cast<Registers>(Vx)] % DISPLAY_WIDTH;
      uint8_t yPos = m_registers[static_cast<Registers>(Vy)] % DISPLAY_HEIGHT;

      m_registers[Registers::VF] = 0;

      for (unsigned int row = 0; row < height; ++row)
      {
        uint8_t spriteByte = memory[index_register + row];

        for (unsigned int col = 0; col < 8; ++col)
        {
          uint8_t spritePixel = spriteByte & (0x80u >> col);
          uint32_t* screenPixel = &display[(yPos + row) * DISPLAY_WIDTH + (xPos + col)];

          // Sprite pixel is on
          if (spritePixel)
          {
            // Screen pixel also on - collision
            if (*screenPixel == 0xFFFFFFFF)
            {
              m_registers[Registers::VF] = 1;
            }

            // Effectively XOR with the sprite pixel
            *screenPixel ^= 0xFFFFFFFF;
          }
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
    this->instructions[OpCode::ADD_I_Vx] = [&]() {
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

  inline Registers get_register_enum(uint8_t reg_value) { return static_cast<Registers>(reg_value); }

  inline uint8_t get_register_value(uint8_t reg_value) { return m_registers[static_cast<Registers>(reg_value)]; }

  inline void pg_next_instruction() { program_counter += 2; }
  inline void pg_previous_instruction() { program_counter -= 2; }
  void load_rom(std::string rom_name) {
    Rom rom(rom_name);
    vector<uint8_t> c = rom.open_rom();

    for (int i = 0; i < c.size(); i++) {
      memory[START_ADDRESS + i] = c[i];
    }
  }

  void decode_op_code_and_execute() {
    uint16_t final_opcode = -1;
    if (opcode == 0x00E0 || opcode == 0x00EE) {
      final_opcode = opcode;
    } else if ((opcode >> 12) >= 0 && (opcode >> 12) <= 7) {
      final_opcode = opcode >> 12;
    } else if ((opcode >> 12) >= 0x8 && (opcode >> 12) <= 0x9) {
      final_opcode = opcode & 0xF00F;
    } else if ((opcode >> 12) >= 0xA && (opcode >> 12) < 0xE) {
      final_opcode = opcode & 0xF000;
    } else if ((opcode >> 12) >= 0xE && (opcode >> 12) <= 0xF) {
      final_opcode = opcode & 0xF0FF;
    }
    if (instructions.find(static_cast<OpCode>(final_opcode)) != instructions.end()) {
      instructions[static_cast<OpCode>(final_opcode)]();
    }
  }

  void Cycle() {
    opcode = (memory[program_counter] << 8u) | memory[program_counter + 1];

    pg_next_instruction();

    decode_op_code_and_execute();

    if (delay_timer > 0) {
      delay_timer--;
    }

    if (sound_timer > 0) {
      sound_timer--;
    }
  }
};

int main(int argc, char* argv[]) {
  int videoScale = 10;
  int cycleDelay = 1;
  char const* romFilename = "./res/tetris.ch8";

  Platform platform("CHIP-8 Emulator", DISPLAY_WIDTH * videoScale, DISPLAY_HEIGHT * videoScale, DISPLAY_WIDTH, DISPLAY_HEIGHT);

  Chip8 chip8;
  chip8.load_rom(romFilename);

  int videoPitch = sizeof(chip8.display[0]) * DISPLAY_WIDTH;

  auto lastCycleTime = std::chrono::high_resolution_clock::now();
  bool quit = false;

  while (!quit) {
    quit = platform.ProcessInput(chip8.input_keys);

    auto currentTime = std::chrono::high_resolution_clock::now();
    float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count();

    if (dt > cycleDelay) {
      lastCycleTime = currentTime;

      chip8.Cycle();

      platform.Update(chip8.display, videoPitch);
    }
  }

  return 0;
}
