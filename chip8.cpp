#include <string>
#include <SDL2/SDL.h>
#include <filesystem>
#include <vector>
#include <iostream>
#include <fstream>
#include <random>

// a class for our chip8
class Chip8 
{
    // we need somewhere to store our current opcode. 
    unsigned short currentOpcode = 0;

    /*  
        We need some memory allocated to our chip8
        the chip 8 has 4096 bytes of memory. 
        The memory  in location 0x000 to 0x1ff (0-511) is saved for the interpreter but in our case we shall use it for font storage
        This will matter when we write the function to load our program. We need to make sure to load it into the correct bit of memory 
    */
    unsigned char memory[4096] = { 0 };

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
    unsigned short programCounter = 0;
    
    /*
        The index register is a 16 bit register that is used for operations related to reading and writing memory. 
    */
    unsigned short indexRegister = 0;

    /*
        General purpose variable regsiters. Each 2 bytes long. There are 16 of them lablled V0 to VF with VF being used as a flag resister
    */

    unsigned short vRegister[16] = {0};

    unsigned char delayTimer = 0;

    unsigned char soundTimer = 0;

    /*
        We also need a stack so that we can enable subroutines and jumping
    */

    unsigned short stack[16] = { 0 };
    unsigned short stackPointer = 0;

    /*
        And finally a variable to store which keys are currently being pressed. The chip8 has 16 keys labelled 0-F
    */

    unsigned char key[16] = { 0 };

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
                std::cout << "Handling input" << std::endl;
                HandleInput(true);
                

                // run the next instruction (this needs to happen 700 times a second)
                if (step_update > 1.0f / 700.0f) {
                    std::cout << "RUnning step" << std::endl;
                    Step();
                    step_update = 0;
                    delayTimer = (delayTimer > 0) ? delayTimer - 1 : 0;
                    soundTimer = (soundTimer > 0) ? soundTimer - 1 : 0;
                }
                
                // draw the display (need to do this only 60 times a second)
                if (gpu_update > 1.0f / 60.0f) {
                    std::cout << "Drawing graphics" << std::endl;
                    DrawGraphics();                    
                    gpu_update = 0;
                }
                
                // update timers
                std::cout << "Updating timers" << std::endl;
                float delta_t = (SDL_GetTicks() - previous_time) / 1000.0f;
                
                previous_time = SDL_GetTicks(); //update previous time for next tick
                step_update += delta_t;
                gpu_update += delta_t;
                std::cout << "loop over" << std::endl;    
            }
        };
        

    private:

        void HandleInput(bool debug=false) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    exit(0);
                }
                else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                    int isPressed = (event.type == SDL_KEYDOWN) ? 1 : 0;
                    switch (event.key.keysym.sym) {
                                    case SDLK_1: key[0x1] = isPressed; break;
                                    case SDLK_2: key[0x2] = isPressed; break;
                                    case SDLK_3: key[0x3] = isPressed; break;
                                    case SDLK_4: key[0xC] = isPressed; break;
                                    case SDLK_q: key[0x4] = isPressed; break;
                                    case SDLK_w: key[0x5] = isPressed; break;
                                    case SDLK_e: key[0x6] = isPressed; break;
                                    case SDLK_r: key[0xD] = isPressed; break;
                                    case SDLK_a: key[0x7] = isPressed; break;
                                    case SDLK_s: key[0x8] = isPressed; break;
                                    case SDLK_d: key[0x9] = isPressed; break;
                                    case SDLK_f: key[0xE] = isPressed; break;
                                    case SDLK_z: key[0xA] = isPressed; break;
                                    case SDLK_x: key[0x0] = isPressed; break;
                                    case SDLK_c: key[0xB] = isPressed; break;
                                    case SDLK_v: key[0xF] = isPressed; break;
                    }
                }
                if (debug) {
                    // debug print the current key status
                    char output[17];
                    for (int i = 0; i < 16; i++) {
                        output[i] = key[i] ? '1' : '0';
                    }
                    output[16] = '\0'; // null terminate the string so we don't get random crap   
                    std::cout << output << std::endl;
                }
            }
        }

        // random byte generator function
        unsigned char RandomByte() {
            std::random_device rd;
            std::mt19937 gen(rd()); // using the mt19937 generator
             // generate a random number between 0 and 255
             // remembering that a byte is effectively any number betwwen 0 and 255
             // since 2^8 = 256
            std::uniform_int_distribution<> dis(0, 255);
            // generaqte a number and cast it to an unsigned char
            return static_cast<unsigned char>(dis(gen));
        }

        void InitialiseGraphics() {
            // draw empty display to the screen            
            // initialise graphics and events
            SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
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
                        case 0x00EE: { // RET - return from subroutine. set PC to address at the top fot he stack and then subtract 1 from stack pointer
                            programCounter = stack[stackPointer];
                            stackPointer--;
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
                case 0x2: { // Call addr call subroutine at nnn. increment stack pointer, put current PC on top. set PC to nnn
                    stackPointer++;
                    stack[stackPointer] = programCounter;
                    programCounter = third_byte;
                    break;
                }
                case 0x3: { // 3xkk - skip next instruction if Vc = kk
                    if (vRegister[nibble_2]  == second_byte) {
                        programCounter += 2;                        
                    }
                    break;
                }
                case 0x4: { // 4xkk - skip next instruction if Vx not equal to kk
                    if (vRegister[nibble_2] != second_byte) {
                        programCounter += 2;
                    }
                    break;
                }
                case 0x5: { // 5xy0 skip next instruction if vx == vy
                    if (vRegister[nibble_2] == vRegister[nibble_3]) {
                        programCounter += 2;
                    }
                    break;
                }
                case 0x6: { // Set instruction (6XNN where we set the VX register to value NN)
                    vRegister[nibble_2] = second_byte;
                    break;
                }
                case 0x7: {// 7xkk - set vx = vx + kk
                    vRegister[nibble_2] += second_byte;
                    break;
                }
                case 0x8: {// there are a few different instruction that begin 0x8 that vary depending on the fourth bit
                    switch (nibble_4) {
                        case 0x0: { // 8xy0 - sstore valu eof vy in vx
                            vRegister[nibble_2] = vRegister[nibble_3];
                            break;
                        }
                        case 0x1: { // 8xy1 perform bitwise OR on the values of Vx and Vy and then store result in Vx
                            vRegister[nibble_2] |= vRegister[nibble_3];
                            break;
                        }
                        case 0x2: { // 8xy2 perform bitwise AND much like above
                            vRegister[nibble_2] &= vRegister[nibble_3];
                            break;
                        }
                        case 0x3: { // same as above but for XOR
                            vRegister[nibble_2] ^= vRegister[nibble_3];
                            break;
                        }
                        case 0x4: { // ADD vx and vy and set Vx to answer.  Set VF to 1  if vx plus vy % 0xFF is 1
                            unsigned short sum = (vRegister[nibble_2] + vRegister[nibble_3]);
                            vRegister[0xF] = sum % 0xFF;
                            vRegister[nibble_2] = sum;
                            break;
                        }
                        case 0x5: {// SUB set Vx to Vx - Vy. if Vx > Vy set Vf to 1 
                            vRegister[0xF] = (vRegister[nibble_2] > vRegister[nibble_3]) ? 1 : 0;
                            vRegister[nibble_2] -= vRegister[nibble_3];
                            break;
                        }
                        case 0x6: { //SHR shift Vx 1 bit to the right
                            if (vRegister[nibble_2] & 0x1 == 1) {
                                vRegister[0xF] = 1;
                            } else {
                                vRegister[0xF] = 0;
                            } 
                            vRegister[nibble_2] >>= 1;
                            break;
                        }
                        case 0x7: { //  SUBN Set Vx = Vy - Vx, set VF = NOT borrow
                            vRegister[0xF] = vRegister[nibble_3] > vRegister[nibble_2] ? 1 : 0;
                            vRegister[nibble_2] = vRegister[nibble_3] - vRegister[nibble_2];
                            break;
                        }
                        case 0xE: { // Shift Vx 1 bit to the left
                            vRegister[0xF] = vRegister[nibble_2] & 0b10000000 ? 1 : 0; // done in binary because my brain is small and i couldn't think in hex :(
                            vRegister[nibble_2] <<= 1;
                            break;
                        }
                        default: {
                            // this opcode doesn't exist
                            std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;
                            break;
                        }
                    }
                }
                case 0x9: { // skip next instructuion if Vx != Vy
                    programCounter += (vRegister[nibble_2] != vRegister[nibble_3]) ? 2 : 0;
                }
                case 0xA: { // OPcode ANNN: Sets index register to value NNN
                    indexRegister = third_byte;
                    break;
                }
                case 0xB: { // PMP to pocation nnn + V0
                    programCounter = third_byte + vRegister[0];
                    break;
                }
                case 0xC: { // Cxkk set Vx to random byte AND kk
                    vRegister[nibble_2] = RandomByte() & second_byte;
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
                                                        
                        }
                    }
                    // set the carry flag
                    vRegister[0xF] = carry;
                    break;
                }
                case 0xE: {
                    // two possible instrctions here so we'll use an if statement to check which we need
                    if (second_byte == 0x9E) {
                        // skip next instruction if key with the value of Vx is pressed
                        programCounter += (key[vRegister[nibble_2]] == 1) ? 2 : 0;
                    }
                    else if (second_byte == 0xA1)
                    {
                        // skip next instruction if key with the value of Vx is not pressed
                        programCounter += (key[vRegister[nibble_2]] == 0) ? 2 : 0;
                    }
                    else {
                        // invalid opcode
                        std::cout << "Unknown opcode: " << std::hex << opcode << std::endl;                     
                    }
                    break;
                }
                case 0xF: {
                    // quite  a few codes begin 0xF
                    switch (second_byte){
                        case 0x07: {
                            vRegister[nibble_2] = delayTimer;
                            break;
                        }
                        case 0x0A: {
                            // pause until a key is pressed and then store the value of the key in Vx
                            while (true) {
                                // still need to handle input here so
                                HandleInput();
                                bool canBreak = false;
                                for (int i = 0; i < 16; i++) {
                                    if (key[i] == 1) {
                                        vRegister[nibble_2] = i;
                                        canBreak = true;
                                        break;
                                    }
                                }
                                if (canBreak) {
                                    break;
                                }
                            }
                        }
                        case 0x15: {
                            delayTimer = vRegister[nibble_2];
                            break;
                        }
                        case 0x18: {
                            soundTimer = vRegister[nibble_2];
                            break;
                        }
                        case 0x1E: {
                            indexRegister += vRegister[nibble_2];
                            break;
                        }
                        case 0x29: {
                            // memory location for font begins at 0x050 so we can
                            // set the index register to 0x050 plus 5 times the value of Vx to find the beginning
                            // of the sprite for the character held in Vx
                            indexRegister = 0x050 + vRegister[nibble_2] * 5;
                            break;
                        }
                        case 0x33: {
                            // take the decimal value of Vx and place
                            // the hundres digit in memory location in I, 
                            //tens digit at location I+1 
                            //and the ones at I+2
                            unsigned char hundreds = vRegister[nibble_2] & 0b100;
                            unsigned char tens = vRegister[nibble_2] & 0b010;
                            unsigned char ones = vRegister[nibble_2] & 0b001;
                            memory[indexRegister] = hundreds;
                            memory[indexRegister + 1] = tens;
                            memory[indexRegister + 2] = ones;
                            break;
                        }
                        case 0x55: {
                            // copy registers V0 through Vx into memory starting at memory[indexRegister]
                            for (int i; i < vRegister[nibble_2]; i++) {
                                memory[indexRegister + i] = vRegister[i];
                            }
                            break;
                        }
                        case 0x65: {
                            // read registers V0 through Vx from memory starting at location I
                            for (int i; i < vRegister[nibble_2]; i++) {
                                vRegister[i] = memory[indexRegister + i];
                            }
                            break;
                        }
                    }
                    break;
                }
            }
            
        };

        int Step() {
            // TODO: update pressed keys
            
            // fetch the opcode
            currentOpcode = memory[programCounter] << 8 | memory[programCounter + 1];
            programCounter += 2;

            // decode and execute the opcode
            RunOpcode(currentOpcode);

            return 0;
        };

};

int main () {

    Chip8 myChip8;

    myChip8.Initialize();
    myChip8.LoadProgram("programs/snek.ch8");
    myChip8.Run();    

    return 0;
};