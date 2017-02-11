#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define NB_REGISTERS 8
#define INITIAL_ARRAY_SIZE (1 << 16)
#define DEBUG false

typedef struct _vm {
    uint32_t *registers;                /* registers */
    uint32_t **arrays;                  /* main array containing arrays of platters */
    uint64_t *arrays_size;              /* main array size */
    uint32_t *program;                  /* byte code array */
    uint32_t *pc;                       /* program counter */
}vm_t;

enum codes{
    MOVEIF, GET, SET, ADD, MULT, DIV, NAND, STOP, ALLOC, FREE,
    PRINT, READ, LOAD, ORTHO
};


void init_vm(vm_t *vm, uint32_t *program){

    vm->registers = calloc(sizeof(uint32_t), NB_REGISTERS);
    vm->arrays_size = calloc(sizeof(uint64_t), 1);
    *(vm->arrays_size) = INITIAL_ARRAY_SIZE;
    vm->arrays = calloc(sizeof(uint32_t *), *(vm->arrays_size));        

    // stash the size in the first box
    vm->arrays[0] = program;    
    vm->pc = calloc(sizeof(int32_t), 1);   
}

void free_vm(vm_t *vm){
    free(vm->registers);
    free(vm->pc);
    for(uint64_t i = 0; i < *(vm->arrays_size); i++){
        if(vm->arrays[i] != NULL){
            // size is stashed in the first box
            free(vm->arrays[i] - 1);
        }
    }
    free(vm->arrays_size);
    free(vm->arrays);
}

void start_vm(vm_t *vm){                
    
    uint32_t *r = vm->registers;
    uint32_t **arrays = vm->arrays;    
    uint32_t *pc = vm->pc;      
    uint64_t *arrays_size = vm->arrays_size;
                
    // last index of allocated array
    uint64_t last_i = 0;

    while(true){        
        
        // get instruction
        uint32_t instr = arrays[0][*pc];
        
        // get registers
        uint8_t c = (instr >> 0) & 0b111;
        uint8_t b = (instr >> 3) & 0b111;
        uint8_t a = (instr >> 6) & 0b111;
        
        // get opcode
        uint8_t opcode = (instr >> 28) & 0b1111;            
        
        switch(opcode){

            case MOVEIF :
                if(DEBUG){
                    fprintf(stderr, "MOVEIF: r[%d]=%d r[%d]=%d r[%d]=%d\n",a, r[a], b, r[b], c, r[c]);                    
                }
                
                if(r[c]){
                    r[a] = r[b];
                }                                 
                break;

            case GET:
                if(DEBUG){
                    fprintf(stderr, "GET: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }
                r[a] = arrays[r[b]][r[c]];               
                break;
            
            case SET:
                 if(DEBUG){
                    fprintf(stderr, "SET: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }  
                arrays[r[a]][r[b]] = r[c];              
                break;

            case ADD:                
                if(DEBUG){
                    fprintf(stderr, "ADD: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }
                r[a] = (r[b] + r[c]);                                
                break;
                
            case MULT:        
                if(DEBUG){
                    fprintf(stderr, "MULT: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }        
                r[a] = (r[b] * r[c]);             
                break;
            
            case DIV: {
                if(DEBUG){
                    fprintf(stderr, "DIV: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }            
                r[a] = r[b] / r[c];
                break; 
            }
            
            case NAND:
                if(DEBUG){
                    fprintf(stderr, "NAND: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }
                r[a] = ~(r[b] & r[c]);
                break; 
                
            case STOP:    
                if(DEBUG){
                    fprintf(stderr, "STOP: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }            
                free_vm(vm);
                
                exit(EXIT_SUCCESS);
                break; 
                
            case ALLOC: {
                if(DEBUG){
                    fprintf(stderr, "ALLOC: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }
                
                uint32_t i;
                
                // go from last index in loop
                for(i = last_i + 1; i != last_i; i++){
                   
                    // restart
                    if(i == *arrays_size){
                        i = 1;
                    }
                    
                    // place found
                    if (arrays[i] == NULL){                        
                        break;
                    }                   
                }
                
                // we did a full turn, no more space left
                if(i == last_i){
                    fprintf(stderr, "%d %d\n", (int)i, (int) last_i);
                    fprintf(stderr, "REALLOC since no room left\n");                    

                    // allocate a new array of twice the size
                    vm->arrays = realloc(arrays, *(vm->arrays_size) * sizeof(uint32_t *) * 2);

                    if(vm->arrays == NULL){
                        fprintf(stderr, "realloc error");
                        exit(EXIT_FAILURE);
                    }
                    // update refferences 
                    i = *(vm->arrays_size) + 1;

                    *(vm->arrays_size) = *(vm->arrays_size) * 2;                    

                    arrays = vm->arrays;
                    arrays_size = vm->arrays_size;
                }
                
                uint32_t *ptr = calloc(sizeof(uint32_t), r[c] + 1);

                if(ptr == NULL){
                    fprintf(stderr, "calloc error");
                    exit(EXIT_FAILURE);
                }
                                        
                // stash the size in the first box
                ptr[0] = r[c];
                // return ptr + 1 so the stash is transparent for the user
                arrays[i] = ptr + 1;
                r[b] = i;
                last_i = i;                                
                break;
            }
                
            case FREE:           
                if(DEBUG){
                    fprintf(stderr, "FREE: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }
               
                if(arrays[r[c]] != NULL){
                    // since size is stashed in the first box
                    free(arrays[r[c]] - 1);
                    arrays[r[c]] = NULL;                    
                }else{
                    fprintf(stderr, "array %d is already empty\n", r[c]);
                }
                break;
    
            case PRINT:
                if(DEBUG){
                    fprintf(stderr, "PRINT: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }
                if(r[c] <= 255){                    
                    fprintf(stdout, "%c", r[c]);
                }
                else{
                    fprintf(stderr, "%c not in [0, 255]\n", r[c]);
                }
                break;
                
            case READ: {
                if(DEBUG){
                    fprintf(stderr, "READ: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }                
                uint8_t value = getc(stdin);
                if(value == EOF){
                    r[c] = 0xFF;
                }
                else{
                    r[c] = value;
                }  
                break;   
            }               
            
            case LOAD: {
                if(DEBUG){
                    fprintf(stderr, "LOAD: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }
                
                if(r[b] == 0){                    
                    // program array -> just move pc                    
                    *pc = r[c] - 1;
                    
                }else{                    
                    uint32_t *array = arrays[r[b]];
                    
                    // size stashed in the first box
                    uint32_t size = *(array - 1) + 1;                                        
                    
                    uint32_t *new_array = malloc(sizeof(uint32_t) * size);

                    if(new_array == NULL){
                        fprintf(stderr, "malloc error");
                        exit(EXIT_FAILURE);
                    }
                    
                    // copy from old to new
                    memcpy(new_array, array - 1, sizeof(uint32_t) * size);

                    arrays[0] = new_array + 1;
                    *pc = r[c] - 1;
                }                
                break; 
            }
            
            case ORTHO: {                
                a = (instr >> 25) & 0b111;
                uint32_t value = (instr & 0x1FFFFFF);
                
                if(DEBUG){
                    fprintf(stderr, "ORTHO: value=%d r[%d]=%d r[%d]=%d r[%d]=%d\n",value, a, r[a], b, r[b], c, r[c]);    
                }
                // special case                                
                r[a] = value;                
                break;
            }
        }
        
        *pc = *pc + 1;
    }

}

uint32_t *read_program(char *path){
    FILE *file = fopen(path, "r");
    
    if(file == NULL){
        fprintf(stderr, "Error opening file\n");
        exit(EXIT_FAILURE);
    }

    fseek(file, 0, SEEK_END);
    uint32_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if(size % 4 != 0){
        fprintf(stderr, "Input file is corrupt");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // stash the size in the first box
    uint32_t *program = calloc(sizeof(int32_t), size / 4 + 1);
    program[0] = size / 4;    
    fread(program + 1, sizeof(int32_t), size / 4, file);
    fclose(file);

    for(uint32_t i = 1; i < size / 4 + 1; i++){
        uint32_t big_end = program[i];
        
        // big endian to little endian 
        uint32_t litl_end = 
                ((big_end >> 24) & 0xff) |      // move byte 3 to byte 0
                ((big_end << 8) & 0xff0000) |   // move byte 1 to byte 2
                ((big_end >> 8) & 0xff00) |     // move byte 2 to byte 1
                ((big_end << 24) & 0xff000000); // byte 0 to byte 3
                
        program[i] = litl_end;        
    }    

    return program + 1;

}

int main(int argc, char **argv){
    
    if(argc != 2){
        fprintf(stderr, "Wrong number of arguments\n");
        return EXIT_FAILURE;
    }

    char* path = argv[1];    
    uint32_t *program = read_program(path);
    
    vm_t vm;        
    init_vm (&vm, program);        
    start_vm(&vm);    

    return EXIT_SUCCESS;
}
