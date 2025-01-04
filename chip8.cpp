#include <string>
#include <SDL2/SDL.h>

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
    unsigned int display[64 * 32]; // 2048 bits stored as integers where 1 is on and 0 is off

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

    // Chip 8 Methods
    public:

        void Initialize(){
            // initialise all the registers and memory once the chip8 is created.
        };

        void LoadProgram(std::string program_path) {

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

    private:

        void InitialiseGraphics() {
            // draw empty display to the screen

            SDL_Window* window = nullptr;
            SDL_Renderer* renderer = nullptr;

            SDL_Init(SDL_INIT_VIDEO);
            SDL_CreateWindowAndRenderer(64, 32, 0, &window, &renderer);
        
            // set draw color to black to clear screen
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
        };

        void ClearGraphics() {
            // clear the display
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
        };

        void DrawGraphics() {
            // clear screen
            ClearGraphics();
            // draw the display to the screen
            for (int i = 0; i < 64 * 32; i++) {
                // if the pixel is set to 1 draw it to the screen
                if (display[i] == 1) {
                    // i mod 64 for the x coord. #
                    // i / 32 for the y coord which floors the result as per integer division.
                    SDL_RenderDrawPoint(renderer, i % 64, i / 32);
                }
            }

            SDL_RenderPresent(renderer);
            
        };

        void RunOpcode(unsigned short opcode) {
            // switch statement for all the opcodes
            switch (opcode) {
                case 0x00E0:
                    // clear the screen
                    break;
                case 0x00EE:
                    // return from subroutine
                    break;
                default:
                    // do nothing
                    break;
            }
            
        };

};

int main () {

    Chip8 myChip8;

    myChip8.Initialize();
    myChip8.LoadProgram("IBM Logo.ch8");
    myChip8.Run();
    

    return 0;
};