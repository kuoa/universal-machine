#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define NB_REGISTERS 8
#define MAX_ARRAY_SIZE 1 << 26
#define MOD_VALUE 4294967296

#define EXIT_STOP 42

typedef struct _vm {
    int32_t *registers;
    int32_t **arrays;
    int32_t *program;
    int32_t *pc;
}vm_t;

enum codes{
    MOVEIF, GET, SET, ADD, MULT, DIV, NAND, STOP, ALLOC, FREE,
    PRINT, READ, LOAD, ORTHO
};


void init_vm(vm_t *vm, int32_t *program){
    vm->registers = calloc(sizeof(int32_t), NB_REGISTERS);
    vm->arrays = calloc(sizeof(int32_t *), MAX_ARRAY_SIZE);    
    
    // stash the size in the first box
    vm->arrays[0] = program;
    vm->pc = calloc(sizeof(int32_t), 1);
}

void free_vm(vm_t *vm){
    free(vm->registers);
    free(vm->pc);
    for(int32_t i = 0; i < MAX_ARRAY_SIZE; i++){
        if(vm->arrays[i] != NULL){
            // stash the size in the first box
            free(vm->arrays[i] - 1);
        }
    }
    free(vm->arrays);
}

void start_vm(vm_t *vm){    
    
    bool debug = false;

    int32_t *r = vm->registers;
    int32_t **arrays = vm->arrays;
    int32_t *program = arrays[0];
    int32_t *pc = vm->pc;
    
    // last index of allocated array
    int32_t last_i = 0;

    while(true){        
        int32_t instr = program[*pc];
        
        // get registers
        uint8_t c = (instr >> 0) & 0b111;
        uint8_t b = (instr >> 3) & 0b111;
        uint8_t a = (instr >> 6) & 0b111;
        
        // get instruction
        uint8_t opcode = (instr >> 28) & 0b1111;        

        //printf("pc=%d opcd=%d a=%d b=%d c=%d\n", (int)(*pc),(int)opcode, r[a], r[b], r[c]);

        /* add debug */
        switch(opcode){

            case MOVEIF :
                if(debug){
                    fprintf(stderr, "MOVEIF: r[a]=%d r[b]=%d r[c]=%d\n", r[a], r[b], r[c]);                    
                }
                
                if(r[c]){
                    r[a] = r[b];
                }
                break;

            case GET:
                fprintf(stderr, "GET: r[a]=%d r[b]=%d r[c]=%d\n", r[a], r[b], r[c]);    
                r[a] = arrays[r[b]][r[c]];
                fprintf(stderr, "GETAF: r[a]=%d r[b]=%d r[c]=%d %d\n", r[a], r[b], r[c], arrays[r[b]][r[c]]); 
                break;
            
            case SET:
                fprintf(stderr, "SET: r[a]=%d r[b]=%d r[c]=%d\n", r[a], r[b], r[c]);    
                arrays[r[a]][r[b]] = r[c];
                fprintf(stderr, "SETAF: r[a]=%d r[b]=%d r[c]=%d\n", r[a], r[b], r[c]);  
                break;

            case ADD:                
                r[a] = (r[b] + r[c]) % MOD_VALUE;
                break;
                
            case MULT:                
                r[a] = (r[b] * r[c]) % MOD_VALUE;
                break;
            
            case DIV: {
                uint32_t rb = r[b];
                uint32_t rc = r[c];
                r[a] = rb / rc;
                break; 
            }
            
            case NAND:
                r[a] = ~(r[b] & r[c]);
                break; 
                
            case STOP:                
                free_vm(vm);
                
                exit(EXIT_STOP);
                break; 
                
            case ALLOC: {
                fprintf(stderr, "ALLOC: r[a]=%d r[b]=%d r[c]=%d\n", r[a], r[b], r[c]);                
                int32_t i;
                
                // go from last index in loop
                for(i = last_i + 1; i != last_i; i = (i + 1) % MAX_ARRAY_SIZE){
               
                    if (arrays[i] == NULL){                                                
                        // place found
                        break;
                    }
                }

                if(i == last_i){
                    fprintf(stderr, "%d %d\n", (int)i, (int) last_i);
                    fprintf(stderr, "no room for ALLOC\n");                    
                }else{
                    int32_t *ptr = calloc(sizeof(int32_t), r[c] + 1);
                        
                    // stash the size in the first box
                    ptr[0] = r[c];
                    // save ptr + 1
                    arrays[i] = ptr + 1;
                    r[b] = i;
                    last_i = i;                                    
                }
                fprintf(stderr, "ALLOCAF: r[a]=%d r[b]=%d r[c]=%d\n", r[a], r[b], r[c]);  
                
                break;
            }
                
            case FREE:           
                fprintf(stderr, "FREE: r[a]=%d r[b]=%d r[c]=%d\n", r[a], r[b], r[c]);
                if(arrays[r[c]] != NULL){
                    // since size is stashed in the first box
                    free(arrays[r[c]] - 1);
                    arrays[r[c]] = NULL;
                }else{
                    fprintf(stderr, "array %d is already empty\n", r[c]);
                }
                fprintf(stderr, "FREEA: r[a]=%d r[b]=%d r[c]=%d\n", r[a], r[b], r[c]);
                break;
    
            case PRINT:
                if(0 <= r[c] && r[c] <= 255){                    
                    fprintf(stdout, "%c", r[c]);
                }
                else{
                    fprintf(stderr, "%c not in [0, 255]\n", r[c]);
                }
                break;
                
            case READ: {
                fprintf(stdout, "READ\n");
                uint8_t value = getc(stdin);
                if(value == EOF){
                    r[c] = 0b11111111;
                }
                else{
                    r[c] = value;
                }  
                break;   
            }               
            
            case LOAD: {
                if(r[b] == 0){
                    // program array -> move pc
                    *pc = r[c];
                    
                }else{
                    
                    int32_t *array = arrays[r[b]];
                    
                    // size stashed in the first box
                    int32_t size = *(array - 1);
                    
                    int32_t *new_array = malloc(sizeof(int32_t) * size);

                    if(new_array == NULL){
                        fprintf(stderr, "malloc error");
                    }
                    
                    // copy from old to new
                    memcpy((new_array - 1), (array - 1), size);

                    arrays[0] = new_array;
                    *pc = r[c];
                }

                break; 
            }
            
            case ORTHO: {
                // special case
                a = (instr >> 25) & 0b111;
                int32_t value = (instr & 0x1FFFFFF);
                r[a] = value;
                break;
            }
        }

        
        *pc = *pc + 1;
    }

}

int32_t *read_program(char *path){
    FILE *file = fopen(path, "r");
    
    if(file == NULL){
        fprintf(stderr, "Error opening file\n");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    int64_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // stash the size in the first box
    int32_t *program = calloc(sizeof(int32_t), size / 4 + 1);
    program[0] = size / 4;

    fread(program + 1, sizeof(int32_t), size / 4, file);

    for(int32_t i = 1; i < size / 4; i++){
        int32_t big_end = program[i];
        // big endian to little endian 
        int32_t litl_end = 
                ((big_end >> 24) & 0xff) |      // move byte 3 to byte 0
                ((big_end << 8) & 0xff0000) |   // move byte 1 to byte 2
                ((big_end >> 8) & 0xff00) |     // move byte 2 to byte 1
                ((big_end << 24) & 0xff000000); // byte 0 to byte 3
                
        program[i] = litl_end;        
    }
    fclose(file);

    return (program + 1);

}

int main(int argc, char **argv){
    
    if(argc != 2){
        fprintf(stderr, "Wrong number of arguments\n");
        return EXIT_FAILURE;
    }

    char* path = argv[1];    
    int32_t *program = read_program(path);
    
    vm_t vm;
    
    init_vm (&vm, program);
    
    start_vm(&vm);
    

    return EXIT_SUCCESS;
}
