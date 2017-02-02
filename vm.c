#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define NB_REGISTERS 8
#define MAX_ARRAY_SIZE 1 << 26
#define MOD_VALUE 1 << 32

#define EXIT_STOP 42

struct _vm {
    int32_t *registers;
    int32_t **arrays;
    int32_t *program;
    int64_t *pc;
}vm_t;

typedef enum {
    MOVEIF, GET, SET, ADD, MULT, DIV, NAND, STOP, ALLOC, FREE,
    PRINT, READ, LOAD, ORTHO
}

int32_t *read_program(){
    return NULL;
}

void init_vm(vm_t *vm){
    vm->registers = calloc(sizeof(int32_t), NB_REGISTERS);
    vm->arrays = calloc(sizeof(*arrays), MAX_ARRAY_SIZE);
    vm->program = read_program();
    vm->pc = calloc(sizeof(int32_t), 1);
}

void run(vm_t *vm){    
    
    int32_t *r = vm->registers;
    int32_t **arrays = vm->arrays;
    int32_t *program = vm->program;    
    int32_t *pc = vm->pc;

    while(true){        
        int32_t c_instr = program[*pc];
        
        /* define macros */
        uint8_t a = 0;
        uint8_t b = 0;
        uint8_t c = 0;
        uint8_t opcode = 0;

        /* add debug */
        switch(opcode){

            case MOVEIF :
                if(r[c]){
                    r[a] = r[b];
                }
                break;

            case GET:
                r[a] = arrays[r[b]][r[c]];
                break;
            
            case SET:
                arrays[r[a]][r[b]] = r[c];
                break;

            case ADD:

                r[a] = (r[b] + r[c]) % MOD_VALUE;
                break;
                
            case MULT:
                r[a] = (r[b] * r[c]) % MOD_VALUE;
            
            case DIV:
                r[a] = r[b] / r[c];
                break; 
            
            case NAND:
                r[a] = ~(r[b] & r[c]);
                break; 
                
            case STOP:
                exit(EXIT_STOP);
                break; 
                
            case ALLOC:
                
                break; 
                
            case FREE:            
                break;
    
            case PRINT:
                if(0 <= r[c] <= 255){
                    fprintf(stdout, "%ld", r[c]);
                }
                else{
                    fprintf(stderr, "%ld not in [0, 255]\n");
                }
                break;
                
            case READ:
                    fprintf(stdout, "READ\n");
                    int32_t read_value;
                    fscanf("%ld", &read_value);
            
            case LOAD:
                break; 
            
            case ORTHO:
                break;                        
        }

        
        *pc = *pc + 1;
    }

}

int main(){


    return EXIT_SUCCESS;
}







