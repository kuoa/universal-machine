#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define NB_REGISTERS 8
#define MAX_ARRAY_SIZE 1 << 20
#define MOD_VALUE 4294967296

#define EXIT_STOP 42
#define debug false

typedef struct _vm {
    uint32_t *registers;
    uint32_t **arrays;
    uint32_t *program;
    uint32_t *pc;
}vm_t;

enum codes{
    MOVEIF, GET, SET, ADD, MULT, DIV, NAND, STOP, ALLOC, FREE,
    PRINT, READ, LOAD, ORTHO
};


void init_vm(vm_t *vm, uint32_t *program){
    
    vm->registers = calloc(sizeof(uint32_t), NB_REGISTERS);
    vm->arrays = calloc(sizeof(uint32_t *), MAX_ARRAY_SIZE);        

    // stash the size in the first box
    vm->arrays[0] = program;

    //printf("init vm size %d", (int) *(program - 1));    
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
    
    uint32_t *r = vm->registers;
    uint32_t **arrays = vm->arrays;    
    uint32_t *pc = vm->pc;       
                
    // last index of allocated array
    uint32_t last_i = 0;

    while(true){        
        
        uint32_t instr = arrays[0][*pc];
        
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
                    fprintf(stderr, "MOVEIF: r[%d]=%d r[%d]=%d r[%d]=%d\n",a, r[a], b, r[b], c, r[c]);                    
                }
                
                if(r[c]){
                    r[a] = r[b];
                }                
                 //fprintf(stderr, "r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                break;

            case GET:
                if(debug){
                    fprintf(stderr, "GET: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }
                r[a] = arrays[r[b]][r[c]];               
                break;
            
            case SET:
                 if(debug){
                    fprintf(stderr, "SET: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }  
                arrays[r[a]][r[b]] = r[c];              
                break;

            case ADD:                
                if(debug){
                    fprintf(stderr, "ADD: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }
                r[a] = (r[b] + r[c]);
                
                //fprintf(stderr, "r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                break;
                
            case MULT:        
                if(debug){
                    fprintf(stderr, "MULT: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }        
                r[a] = (r[b] * r[c]);             
                break;
            
            case DIV: {
                if(debug){
                    fprintf(stderr, "DIV: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }            
                r[a] = r[b] / r[c];
                break; 
            }
            
            case NAND:
                if(debug){
                    fprintf(stderr, "NAND: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }
                r[a] = ~(r[b] & r[c]);
                break; 
                
            case STOP:    
                if(debug){
                    fprintf(stderr, "STOP: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }            
                free_vm(vm);
                
                exit(EXIT_STOP);
                break; 
                
            case ALLOC: {
                if(debug){
                    fprintf(stderr, "ALLOC: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }
                
                uint32_t i;
                
                // go from last index in loop

                //for(i = last_i + 1; i != last_i; i = (i + 1) % MAX_ARRAY_SIZE){
                
                for(i = last_i + 1; i != last_i; i++){

                    if(i == MAX_ARRAY_SIZE){
                        i = 1;
                    }
                    if (arrays[i] == NULL){                                                
                        // place found
                        break;
                    }

                   // printf("After brak%d\n", i);
                }
                
                if(i == last_i){
                    fprintf(stderr, "%d %d\n", (int)i, (int) last_i);
                    fprintf(stderr, "no room for ALLOC\n");
                    exit(EXIT_FAILURE);
                }else{
                    uint32_t *ptr = calloc(sizeof(uint32_t), r[c] + 1);

                    if(ptr == NULL){
                        fprintf(stderr, "calloc error");
                        exit(EXIT_FAILURE);
                    }
                        
                    // stash the size in the first box
                    ptr[0] = r[c];
                    // save ptr + 1
                    arrays[i] = ptr + 1;
                    r[b] = i;
                    last_i = i;
                    //printf("%d\n", i);                  
                }                
                //fprintf(stderr, "ALLOCAF: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);  
                
                break;
            }
                
            case FREE:           
                if(debug){
                    fprintf(stderr, "FREE: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }

               // fprintf(stderr, "FREE: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);
                if(arrays[r[c]] != NULL){
                    // since size is stashed in the first box
                    free(arrays[r[c]] - 1);
                    arrays[r[c]] = NULL;                    
                }else{
                    fprintf(stderr, "array %d is already empty\n", r[c]);
                }
                //fprintf(stderr, "FREEA: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);
                break;
    
            case PRINT:
                if(debug){
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
                if(debug){
                    fprintf(stderr, "READ: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }
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
                if(debug){
                    fprintf(stderr, "LOAD: r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
                }

                //printf("\nPC = %d\n", *pc);

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

                   // fprintf(stderr, "\n\nMoving array %d of size %d to zero\n", r[b], size);

                   // printf("%d %d \n", array[0] - 1, arrays[0][0] - 1);
                   // for(uint32_t i=0; i < 100; i++){
                   //     printf("%d %d \n", array[i], arrays[0][i]);
                   // }

                }
                //printf("\nPC = %d\n", *pc);
                break; 
            }
            
            case ORTHO: {                
                a = (instr >> 25) & 0b111;
                uint32_t value = (instr & 0x1FFFFFF);
                
                if(debug){
                    fprintf(stderr, "ORTHO: value=%d r[%d]=%d r[%d]=%d r[%d]=%d\n",value, a, r[a], b, r[b], c, r[c]);    
                }
                // special case                                
                r[a] = value;
                //fprintf(stderr, "a=%d b=%d c=%d\n", a, b, c);
                //fprintf(stderr, "r[%d]=%d r[%d]=%d r[%d]=%d\n", a, r[a], b, r[b], c, r[c]);    
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
        return NULL;
    }


    fseek(file, 0, SEEK_END);
    uint32_t size = ftell(file);
    fseek(file, 0, SEEK_SET);


    // stash the size in the first box
    uint32_t *program = calloc(sizeof(int32_t), size / 4 + 1);
    program[0] = size / 4;
    printf("size %d\n", (int)program[0]);

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
