#include <string>
#include <SDL2/SDL.h>
#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>

// a class for our chip8
class Chip8 
{
    // we need somewhere to store our current opcode. 
    unsigned short currentOpcode;

    /*  
        We need some memory allocated to our chip8
        the chip 8 has 4096 bytes of memory. 
        The memory  in location 0x000 to 0x1ff (0-511) is saved for the interpreter but in our case we shall use it for font storage
        This will matter when we write the function to load our program. We need to make sure to load it into the correct bit of memory 
    */
    unsigned char memory[4096];

    /*
        The chip8 has a display of resolution 64 x 32 and is monochrome. 
        THe way sprites are drawn to the display involves XORing the sprite with the current display buffer so need to pick a datatype that will play nicely with that.
        Ideally I can just use a bit. perhaps there is a c++ option for that. Turns out you cannot work with single bits in c++ so smallest we can do is one byte. char will have to do. 
    */
    unsigned int display[64 * 32] = { 0 }; // 2048 bits stored as integers where 1 is on and 0 is off
    SDL_Window* window = nullptr; // window to draw to
    SDL_Renderer* renderer = nullptr; // renderer to draw with
    
    /*
        The program counter which points to the currenty instruction in memory. This is then always just a memory address. i.e. an integer between 0 and 4095 (since that's the size memory we have)
    */
    unsigned short programCounter;
    
    /*
        The index register is a 16 bit register that is used for operations related to reading and writing memory. 
    */
    unsigned short indexRegister;

    /*
        General purpose variable regsiters. Each 2 bytes long. There are 16 of them lablled V0 to VF with VF being used as a flag resister
    */

    unsigned short vRegister[16];

    unsigned char delayTimer;

    unsigned char soundTimer;

    /*
        We also need a stack so that we can enable subroutines and jumping
    */

    unsigned short stack[16];
    unsigned short stackPointer;

    /*
        And finally a variable to store which keys are currently being pressed. The chip8 has 16 keys labelled 0-F
    */

    unsigned char key[16];

    unsigned int disp_index = 0;

    // Chip 8 Methods
    public:

        void Initialize(){
            // initialise all the registers and memory once the chip8 is created.
            InitialiseGraphics();
            InitialiseFont();
            programCounter = 0x200; // start at the beginning of the program memory

        };

        void LoadProgram(std::string program_path) {
            // load the specified program into memory location 0x200
            std::filesystem::path path = program_path;
            auto length = std::filesystem::file_size(path);
            if (length == 0 || length > 4096 - 512) {
                // file is too big or too small
                return;
            }
            std::vector<std::byte> buffer(length);
            std::ifstream inputFile(program_path, std::ios::binary);
            inputFile.read(reinterpret_cast<char*>(buffer.data()), length);
            inputFile.close();

            // load buffer into memory starting at 0x200
            for (int i = 0; i < length; i++) {
                memory[i + 0x200] = static_cast<unsigned char>(buffer[i]);
            }
        };

        void Run() {
            float gpu_update = 0;
            float step_update = 0;
            float previous_time = SDL_GetTicks();
            
            // main loop
            while (true) {
                //store key press state
                //TODO...
                // run the next instruction (this needs to happen 700 times a second)
                if (step_update > 1.0f / 700.0f) {
                    Step();
                    step_update = 0;
                    delayTimer = (delayTimer > 0) ? delayTimer - 1 : 0;
                    soundTimer = (soundTimer > 0) ? soundTimer - 1 : 0;
                }
                
                // draw the display (need to do this only 60 times a second)
                if (gpu_update > 1.0f / 60.0f) {
                    DrawGraphics();
                    gpu_update = 0;
                }
                
                // update timers
                float delta_t = (SDL_GetTicks() - previous_time) / 1000.0f;
                step_update += delta_t;
                gpu_update += delta_t;
            }
        };
        

    private:

        void InitialiseGraphics() {
            // draw empty display to the screen            
            SDL_Init(SDL_INIT_VIDEO);
            SDL_CreateWindowAndRenderer(64 * 10, 32 * 10, 0, &window, &renderer);
            SDL_RenderSetScale(renderer, 10, 10);

            // set draw color to black to clear screen
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
        };

        void InitialiseFont() {
            // load the font into memory
            unsigned char font[80] = {
                0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
                0x20, 0x60, 0x20, 0x20, 0x70, // 1
                0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
                0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
                0x90, 0x90, 0xF0, 0x10, 0x10, // 4
                0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
                0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
                0xF0, 0x10, 0x20, 0x40, 0x40, // 7
                0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
                0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
                0xF0, 0x90, 0xF0, 0x90, 0x90, // A
                0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
                0xF0, 0x80, 0x80, 0x80, 0xF0, // C
                0xE0, 0x90, 0x90, 0x90, 0xE0, // D
                0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
                0xF0, 0x80, 0xF0, 0x80, 0x80  // F
            };

            // copy the font into memory (it is convention  to store the font in memory location 0x50)
            for (int i = 0; i < 80; i++) {
                memory[i + 0x050] = font[i];
            }
        }

        void ClearGraphics() {
            // clear the display
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
        };

        void DrawGraphics() {
            // clear screen
            ClearGraphics();
            // draw the display to the screen
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            for (int i = 0; i < 64 * 32; i++) {
                // if the pixel is set to 1 draw it to the screen
                if (display[i] == 1) {
                    // i mod 64 for the x coord. #
                    // i / 32 for the y coord which floors the result as per integer division.
                    SDL_RenderDrawPoint(renderer, i % 64, i / 64);
                }
            }

            SDL_RenderPresent(renderer);
            
        };

        void RunOpcode(unsigned short opcode) {
            // switch statement for all the opcodes
            // The first 4 bits of the opcode tell us what the opcode is (one hexadecimal digit since 2^4 = 16)
            // The last 12 bits tell us what to do with the opcode
            
            // grab all the various parts of the opcode up front so we don't have to do it multiple times
            unsigned short nibble_1 = (opcode & 0xF000) >> 12;        
            unsigned short nibble_2 = (opcode & 0x0F00) >> 8;
            unsigned short nibble_3 = (opcode & 0x00F0) >> 4;
            unsigned short nibble_4 = (opcode & 0x000F);
            unsigned short second_byte = (opcode & 0x00FF);
            unsigned short third_byte = (opcode & 0x0FFF);

            switch (nibble_1) {
                case 0x0: {
                    switch (opcode) {
                        case 0x00E0: {
                            // clear the display
                            for (int i = 0; i < 64 * 32; i++) {
                                display[i] = 0;
                            }
                            break;
                        }
                        default: {
                            // call RCA 1802 program at address NNN
                            // not implemented
                            break;
                        }
                    }
                    break;
                }
                case 0x1: { //jump opcode
                    programCounter = third_byte;
                    break;
                }
                case 0x6: { // Set instruction (6XNN where we set the VX register to value NN)
                    vRegister[nibble_2] = second_byte;
                    break;
                }
                case 0x7: { // Add instruction (add value NN to VX  register)
                    vRegister[nibble_2] += second_byte;
                    break;
                }
                case 0xA: { // OPcode ANNN: Sets index register to value NNN
                    indexRegister = third_byte;
                    break;
                }
                case 0xD: { // opcode DXYN draw n pixels tall sprite to the display from position x in the Vx regiter ,y from the Vy register from memory location in the index register
                    // get x and y from the registers
                    unsigned int x = vRegister[nibble_2] % 64;
                    unsigned int y = vRegister[nibble_3] % 32;
                    unsigned int height = nibble_4;
                    unsigned int carry = 0;
                    // loop through the sprite
                    for (int i=0; i < height; i++) {
                        //  check we are not off the bottom of the screen
                        if (y + i >= 32) {
                            break;
                        }
                        // get the sprite row from memory
                        unsigned char sprite_row = memory[indexRegister + i];
                        for (int j = 0; j < 8; j ++) { // for each pixel in the row
                            if (x + j >= 64) {
                                // we have reached the end of the screeen
                                break;
                            }
                            // xor the current pixel with the sprite pixel
                            unsigned int pixel = (sprite_row >> (7 - j)) & 0x1;
                            if (display[(y + i) * 64 + x + j] == 1 && pixel == 1) {
                                carry = 1;
                            };
                            display[(y + i) * 64 + x + j] ^= pixel;
                            
                            // Debug output
                            std::cout << "Drawing pixel at (" << x + j << ", " << y + i << ") with value " << pixel << std::endl;
                        }
                    }
                    // set the carry flag
                    vRegister[0xF] = carry;

                    

                }
            }
            
        };

        int Step() {
            // fetch the opcode
            currentOpcode = memory[programCounter] << 8 | memory[programCounter + 1];
            programCounter += 2;

            // decode and execute the opcode
            RunOpcode(currentOpcode);
            
            // update timers
            // draw to the screen
            // store key press state
            // loop
            return 0;
        };

};

int main () {

    Chip8 myChip8;

    myChip8.Initialize();
    myChip8.LoadProgram("programs/IBM Logo.ch8");
    myChip8.Run();
    

    return 0;
};