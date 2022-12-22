

// a class for our chip8
class chip8() {

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
    unsigned char display[64 * 32];  

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
            
        };

        void LoadProgram(string program_path) {

        };


}

int main () {

    chip8 myChip8;

    myChip8.Initialize();
    myChip8.LoadProgram('IBM Logo.ch8');

    // enter main loop here. calling the next 

    return 0;
};