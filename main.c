#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

char instruction_map(char *token_in);
int find_label_address(const char *label);
unsigned int total_instr_r (int funct7,int rs2,int rs1,int funct3,int rd,int opcode);
unsigned int total_instr_i (int funct7,int rs2,int rs1,int funct3,int rd,int opcode);
unsigned int total_instr_b (int rs2,int rs1,int funct3,int rd,int opcode);
unsigned int total_instr_u (int rs1,int rd,int opcode);
unsigned int total_instr_j (int rs1,int rd,int opcode);
unsigned int total_instr_s (int rs2,int rs1,int funct3,int rd,int opcode);
// void replace_registers(const char *line, char *output); 


typedef struct 
{
    int  iopcode;
    int  ifunct3;
    int  ifunct7;
} instr_decoder;

typedef struct
{
    char *label_name;
    int label_addr;
}label_detector;


typedef struct 
{
    char *name;
    char *x_value;
} RegisterMapping;

RegisterMapping reg_map[] =
{
    {"zero", "x0"}, {"ra", "x1"}, {"sp", "x2"}, {"gp", "x3"}, {"tp", "x4"},
    {"t0", "x5"}, {"t1", "x6"}, {"t2", "x7"}, {"s0", "x8"}, {"s1", "x9"},
    {"a0", "x10"}, {"a1", "x11"}, {"a2", "x12"}, {"a3", "x13"}, {"a4", "x14"},
    {"a5", "x15"}, {"a6", "x16"}, {"a7", "x17"}, {"s2", "x18"}, {"s3", "x19"},
    {"s4", "x20"}, {"s5", "x21"}, {"s6", "x22"}, {"s7", "x23"}, {"s8", "x24"},
    {"s9", "x25"}, {"s10", "x26"}, {"s11", "x27"}, {"t3", "x28"}, {"t4", "x29"},
    {"t5", "x30"}, {"t6", "x31"}
};

label_detector branch_label[30];
instr_decoder decode_obj;

char* get_x_register(const char *reg_name) {
    for (int i = 0; i < sizeof(reg_map) / sizeof(RegisterMapping); i++) {
        if (strcmp(reg_name, reg_map[i].name) == 0) {
            return reg_map[i].x_value;
        }
    }
    return NULL; // Return NULL if not found
}


int main(int argc, char *argv[])
{
	FILE* ptr = NULL;
    FILE* BinPointer = NULL;
    FILE* HexPointer = NULL;
	char ch;
    char *reg;
    char arr[6];
    char line[500];
    char *strtol_ptr;
    long ret;
    int  rd;
    int  rs1;
    int  rs2;
    int address = 0;
    int final_address = 0;
    int B_final_address = 0;
    
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <source-file> <destination-file> <output-type>\n", argv[0]);
        fprintf(stderr, "Output type: -h for hexadecimal, -b for binary\n");
        return 1;
    }

    const char *source_file = argv[1];
    const char *destination_file = argv[2];
    const char *output_type = argv[3];

    if (strcmp(output_type, "-h") != 0 && strcmp(output_type, "-b") != 0) 
    {
        fprintf(stderr, "Invalid output type. Use -h for hexadecimal or -b for binary.\n");
        return 1;
    }

	// Opening file in reading mode
	ptr        = fopen(source_file, "r");

    if (strcmp(output_type, "-b") == 0) {
        BinPointer = fopen(destination_file, "w");
    } else if (strcmp(output_type, "-h") == 0) {
        HexPointer = fopen(destination_file, "w");
    }
    // BinPointer = fopen("Output.bin", "w");
    // HexPointer = fopen("Output.hex", "w");

	if (NULL == ptr) {
		printf("Input .asm file can't be opened \n");
	} 

/*
    //for debuging
    // Opening file in reading mode
	ptr        = fopen("test_im_only.s", "r");
    BinPointer = fopen("Output.bin", "w");
    HexPointer = fopen("Output.hex", "w");

	if (NULL == ptr) {
		printf("Input .asm file can't be opened \n");
	}

    if (NULL == BinPointer) {
		printf("Binary file can't be opened \n");
	}

    if (NULL == HexPointer) {
		printf("Hex file can't be opened \n");
	}*/

    int i = 0;
    while (fgets(line, sizeof(line), ptr)) 
    {
        line[strcspn(line, "\n")] = '\0';
        char *check_label = strchr(line, ':');
        if(check_label)
        {
            char *token = strtok(line, ":");
            // printf("Token: %s\n", token); 
            branch_label[i].label_addr = address;
            branch_label[i].label_name = malloc(strlen(token) + 1);
            if (branch_label[i].label_name == NULL) {
                perror("Failed to allocate memory");
                return -1;
            }
            strcpy(branch_label[i].label_name, token);

            i++;
        }
        else
            address = address + 4;

        printf("\n");
    }
    fclose(ptr);


    ptr        = fopen(source_file, "r");
    //ptr = fopen("test_im_only.s", "r");

    if (NULL == ptr) {
		printf("Input .a file can't be opened \n");
	}

    while (fgets(line, sizeof(line), ptr)) 
    {
        // printf("Original line: %s", line);
        // Remove the newline character from the end of the line
        line[strcspn(line, "\n")] = '\0';
        // Split the line into tokens based on commas and spaces
        char *token = strtok(line, " ,");
        
        if (token != NULL) 
        {
            char value = instruction_map(token);
            if (value == 'E') 
            {
                // printf("Error, Instruction not found\n");
            }
            else if (value == 'R')
            {
                int reg_num;
                int token_count = 0;
                while ((token = strtok(NULL, " ,")) != NULL) 
                {

                    // Extract number from the register label (e.g., x5 -> 5)
                    if (token[0] == 'x' && isdigit(token[1])) 
                    {
                        reg_num = atoi(token + 1);
                    } 
                    else 
                    {
                        char *x_reg = get_x_register(token);
                        if (x_reg != NULL) 
                        {
                            reg_num = atoi(x_reg + 1);
                        }
                    }

                    if (token_count == 0) 
                    {
                        rd = reg_num;
                    } 
                    else if (token_count == 1) 
                    {
                        rs1 = reg_num;
                    } 
                    else if (token_count == 2) 
                    {
                        rs2 = reg_num;
                    }
                    token_count++;
                }
                ret = total_instr_r(decode_obj.ifunct7,rs2,rs1,decode_obj.ifunct3,rd,decode_obj.iopcode);
                // printf("32-bit Instruction in decimal: %d\n", ret);

                // Save the 32-bit binary instruction to a .bin file
                if (strcmp(output_type, "-b") == 0) 
                {
                    fwrite(&ret,sizeof(ret),1, BinPointer);
                } 
                else if (strcmp(output_type, "-h") == 0) 
                {
                    fprintf(HexPointer,"0x%08x\n",ret);
                }
            }

            else if (value == 'I')
            {
                int reg_num;
                int token_count = 0;
                while ((token = strtok(NULL, " ,()")) != NULL) 
                {

                    // Extract number from the register label (e.g., x5 -> 5)
                    if (token[0] == 'x' && isdigit(token[1])) 
                    {
                        reg_num = atoi(token + 1);
                    } 
                    else if((isdigit(token[0])) || (token[0] == '-' && isdigit(token[1]))) // for minus signs
                    {
                        rs2 = atoi(token); //rs2 imm
                        token_count = token_count + 2;
                        continue;
                    }
                    else 
                    {
                        char *x_reg = get_x_register(token);
                        if (x_reg != NULL) 
                        {
                            reg_num = atoi(x_reg + 1);
                        }
                    }

                        // Append register binary to full_binary_str
                        if (token_count == 0) 
                        {
                            // rd
                            rd = reg_num;
                        } 
                        else if (token_count == 1) 
                        {
                            // rs1
                            rs1 = reg_num;
                            
                        } 
                        else if (token_count == 2) 
                        {
                            // rs2
                            rs2 = reg_num;
                            
                        }
                        else if (token_count == 3) 
                        {
                            // rs1 in cases of loads
                            rs1 = reg_num;
                            
                        } 
                        token_count++;
                }
                //ret = strtol(full_binary_str, &strtol_ptr, 10);
                ret = total_instr_i(decode_obj.ifunct7,rs2,rs1,decode_obj.ifunct3,rd,decode_obj.iopcode);

                // Save the 32-bit binary instruction to a .bin file
                
                
                if (strcmp(output_type, "-b") == 0) 
                {
                    fwrite(&ret,sizeof(ret),1, BinPointer);
                } 
                else if (strcmp(output_type, "-h") == 0) 
                {
                    fprintf(HexPointer,"0x%08x\n",ret);
                }
            }

            else if (value == 'S')
            {
                int reg_num;
                int token_count = 0;
                while ((token = strtok(NULL, " ,()")) != NULL) 
                {

                    // Extract number from the register label (e.g., x5 -> 5)
                    if (token[0] == 'x' && isdigit(token[1])) 
                    {
                        reg_num = atoi(token + 1);
                    }
                    else if((isdigit(token[0])) || (token[0] == '-' && isdigit(token[1]))) // for minus signs
                    {
                        rd = atoi(token);  //rd = imm
                        //token_count++;
                    }
                    else 
                    {
                        char *x_reg = get_x_register(token);
                        if (x_reg != NULL) 
                        {
                            reg_num = atoi(x_reg + 1);
                        }
                    }
                        if (token_count == 0) 
                        {
                            //rs2
                            rs2 = reg_num;
                        } 
                        else if (token_count == 2)
                        {
                            // rs1
                            rs1 = reg_num;
                            
                        }
                        
                    token_count++;
                }
                //ret = strtol(full_binary_str, &strtol_ptr, 10);
                ret = total_instr_s(rs2,rs1,decode_obj.ifunct3,rd,decode_obj.iopcode);
                // Save the 32-bit binary instruction to a .bin file
                if (strcmp(output_type, "-b") == 0) 
                {
                    fwrite(&ret,sizeof(ret),1, BinPointer);
                } 
                else if (strcmp(output_type, "-h") == 0) 
                {
                    fprintf(HexPointer,"0x%08x\n",ret);
                }
            }

            else if (value == 'B')
            {
                int reg_num;
                int token_count = 0;
                while ((token = strtok(NULL, " ,")) != NULL) 
                {

                    // Extract number from the register label (e.g., x5 -> 5)
                    if (token[0] == 'x' && isdigit(token[1])) 
                    {
                        reg_num = atoi(token + 1);
                    } 
                    else 
                    {
                        char *x_reg = get_x_register(token);
                        if (x_reg != NULL) 
                        {
                            reg_num = atoi(x_reg + 1);
                        }
                    }
                        if (token_count == 0) 
                        {
                            // rs1
                            rs1 = reg_num;
                        } 
                        else if (token_count == 1) 
                        {
                            // rs2
                            rs2 = reg_num;
                            
                        }
                        else if (token_count == 2) 
                        {
                            // label address finding
                            int label_address = find_label_address(token);
                            if (label_address != -1)
                            {
                                // printf("Label '%s' found at address %d\n", token, label_address);
                            } else 
                            {
                                printf("Label '%s' not found.\n", token);
                            }
                            rd = label_address - B_final_address;
                            
                        }
                        token_count++;
                }
                ret = total_instr_b(rs2,rs1,decode_obj.ifunct3,rd,decode_obj.iopcode);
                // Save the 32-bit binary instruction to a .bin file
                if (strcmp(output_type, "-b") == 0) 
                {
                    fwrite(&ret,sizeof(ret),1, BinPointer);
                } 
                else if (strcmp(output_type, "-h") == 0) 
                {
                    fprintf(HexPointer,"0x%08x\n",ret);
                }
            }


            else if (value == 'U')
            {
                int reg_num;
                int token_count = 0;
                while ((token = strtok(NULL, " ,")) != NULL) 
                {

                    // Extract number from the register label (e.g., x5 -> 5)
                    if (token[0] == 'x' && isdigit(token[1])) 
                    {
                        reg_num = atoi(token + 1);
                            // rd
                            rd = reg_num;
                    }
                    else if(isdigit(token[0]))
                    {
                        if(token[1] == 'x' && isdigit(token[2]))
                        {
                            long int hex_data = strtol (token,NULL,16);
                            rs1 = hex_data;
                        }
                        else
                            rs1 = atoi(token); //rs1 = imm
                    }
                    else 
                    {
                        char *x_reg = get_x_register(token);
                        if (x_reg != NULL) 
                        {
                            reg_num = atoi(x_reg + 1);
                        }
                        // rd
                        rd = reg_num;
                    }
                }
                
                ret = total_instr_u(rs1,rd,decode_obj.iopcode);
                // Save the 32-bit binary instruction to a .bin file
                if (strcmp(output_type, "-b") == 0) 
                {
                    fwrite(&ret,sizeof(ret),1, BinPointer);
                } 
                else if (strcmp(output_type, "-h") == 0) 
                {
                    fprintf(HexPointer,"0x%08x\n",ret);
                }
            }

            else if (value == 'J')
            {
                int reg_num;
                int token_count = 0;
                while ((token = strtok(NULL, " ,")) != NULL) 
                {

                    // Extract number from the register label (e.g., x5 -> 5)
                    if (token[0] == 'x' && isdigit(token[1])) 
                    {
                        reg_num = atoi(token + 1);
                    } 
                    else 
                    {
                        char *x_reg = get_x_register(token);
                        if (x_reg != NULL) 
                        {
                            reg_num = atoi(x_reg + 1);
                        }
                    }

                        if (token_count == 0) 
                        {
                            // rs1
                            rd = reg_num;
                        } 
                        else if (token_count == 1) 
                        {
                            // label address finding
                            int label_address = find_label_address(token);
                            if (label_address != -1)
                            {
                                // printf("Label '%s' found at address %d\n", token, label_address);
                            } else 
                            {
                                printf("Label '%s' not found.\n", token);
                            }
                            rs1 = label_address - final_address;
                            
                        }
                        token_count++;
                    
                }
                ret = total_instr_j(rs1,rd,decode_obj.iopcode);
                // Save the 32-bit binary instruction to a .bin file
                if (strcmp(output_type, "-b") == 0) 
                {
                    fwrite(&ret,sizeof(ret),1, BinPointer);
                } 
                else if (strcmp(output_type, "-h") == 0) 
                {
                    fprintf(HexPointer,"0x%08x\n",ret);
                }
            }
            char *check_label = strchr(line, ':');
            if(check_label)
                printf("");
            else
                final_address = final_address + 4;
        }

        B_final_address = B_final_address + 4;
        printf("\n");
        
    }

    // Closing the file
    // printf("final_address in decimal: %d\n", final_address);
    fclose(ptr);
    return 0;
}

unsigned int total_instr_j (int rs1,int rd,int opcode)
{
        int temp_rd,temp_funct7;

        temp_funct7 = (rs1 & 0xFF000) << 12;        //bit 19-12
        temp_funct7 |= ((rs1 & 0x800) << 9);         // (bit 11)

        temp_rd     = (rs1 & 0x7FE) << 20; 
        temp_rd     =  temp_rd | ((rd & 0x100000) << 11);

        rs1         = temp_rd | temp_funct7;
        rd     = rd << 7;
        opcode = opcode << 0;

        return (rs1 | rd | opcode);
}

unsigned int total_instr_u (int rs1,int rd,int opcode)
{
            rs1    = rs1 << 12; 
            rd     = rd << 7;
            opcode = opcode << 0;

            return (rs1 | rd | opcode);   

}                

unsigned int total_instr_b (int rs2,int rs1,int funct3,int rd,int opcode)
{
        int temp_rd,temp_funct7;

        temp_funct7 = (rd & 0x7E0) << 20; //bit 10-5
        temp_funct7 |= ((rd & 0x1000) << 19);           // (bit 12)

        temp_rd     = ((rd & 0x01E) << 7) | ((rd & 0x800) >> 4);

        rs2    = rs2 << 20;
        rs1    = rs1 << 15;
        funct3 = funct3 << 12;
        opcode = opcode << 0;

        return (temp_funct7 | rs2 | rs1 | funct3 | temp_rd | opcode);
}

unsigned int total_instr_s (int rs2,int rs1,int funct3,int rd,int opcode)
{
        int temp_rd,temp_funct7;

        temp_funct7 = (rd & 0xFE0) << 20; //bit 11-5

        temp_rd     = ((rd & 0x01F) << 7); //immi 4:0

        rs2    = rs2 << 20;
        rs1    = rs1 << 15;
        funct3 = funct3 << 12;
        opcode = opcode << 0;

        return (temp_funct7 | rs2 | rs1 | funct3 | temp_rd | opcode);
}

unsigned int total_instr_r (int funct7,int rs2,int rs1,int funct3,int rd,int opcode)
{
    funct7 = funct7 << 25;
    rs2    = rs2 << 20;
    rs1    = rs1 << 15;
    funct3 = funct3 << 12;
    rd     = rd << 7;
    opcode = opcode << 0;

    return (funct7 | rs2 | rs1 | funct3 | rd | opcode);

}

unsigned int total_instr_i (int funct7,int rs2,int rs1,int funct3,int rd,int opcode)
{
    if((opcode == 19) && (funct3 == 1 || funct3 == 5))
        {
            funct7 = funct7 << 25;
            rs2    = rs2 << 20;
            rs1    = rs1 << 15;
            funct3 = funct3 << 12;
            rd     = rd << 7;
            opcode = opcode << 0;
            return (funct7 | rs2 | rs1 | funct3 | rd | opcode);

        }
    else
        {
            // funct7 = funct7 << 25;
            rs2    = rs2 << 20;
            rs1    = rs1 << 15;
            funct3 = funct3 << 12;
            rd     = rd << 7;
            opcode = opcode << 0;

            return (rs2 | rs1 | funct3 | rd | opcode);
        }
    

}

int find_label_address(const char *label) 
{
    for (int j = 0; j < 30; j++) {
        if (branch_label[j].label_name != NULL && strcmp(branch_label[j].label_name, label) == 0) 
        {
            return branch_label[j].label_addr;
        }
    }
    return -1; // Return -1 if not found
}

char instruction_map(char *token_in)
{
    //R Type instructions

    if(strcmp(token_in,"add")==0)
        {
            decode_obj.iopcode = 0x33;
            decode_obj.ifunct3 = 0x0;
            decode_obj.ifunct7 = 0x00;
            return 'R'; 
        }
    else if(strcmp(token_in,"sub")==0)
        {
            decode_obj.iopcode = 0x33;
            decode_obj.ifunct3 = 0x0;
            decode_obj.ifunct7 = 0x20;
            return 'R'; 
        }
        
    else if(strcmp(token_in,"sll")==0)
        { 
            decode_obj.iopcode = 0x33;
            decode_obj.ifunct3 = 0x1;
            decode_obj.ifunct7 = 0x00;
            return 'R'; 
        }

    else if(strcmp(token_in,"slt")==0)
        { 
            decode_obj.iopcode = 0x33;
            decode_obj.ifunct3 = 0x2;
            decode_obj.ifunct7 = 0x00;
            return 'R'; 
        }

    else if(strcmp(token_in,"sltu")==0)
        { 
            decode_obj.iopcode = 0x33;
            decode_obj.ifunct3 = 0x3;
            decode_obj.ifunct7 = 0x00;
            return 'R'; 
        }

    else if(strcmp(token_in,"xor")==0)
        { 
            decode_obj.iopcode = 0x33;
            decode_obj.ifunct3 = 0x4;
            decode_obj.ifunct7 = 0x00;
            return 'R'; 
        }

    else if(strcmp(token_in,"srl")==0)
        { 
            decode_obj.iopcode = 0x33;
            decode_obj.ifunct3 = 0x5;
            decode_obj.ifunct7 = 0x00;
            return 'R'; 
        }

    else if(strcmp(token_in,"sra")==0)
        { 
            decode_obj.iopcode = 0x33;
            decode_obj.ifunct3 = 0x5;
            decode_obj.ifunct7 = 0x20;
            return 'R'; 
        }

    else if(strcmp(token_in,"or")==0)
        {
            decode_obj.iopcode = 0x33;
            decode_obj.ifunct3 = 0x6;
            decode_obj.ifunct7 = 0x00;
            return 'R'; 
        }

    else if(strcmp(token_in,"and")==0)
        { 
            decode_obj.iopcode = 0x33;
            decode_obj.ifunct3 = 0x7;
            decode_obj.ifunct7 = 0x00;
            return 'R'; 
        }
        
    //I Type instructions

    else if(strcmp(token_in,"lb")==0)
        { 
            decode_obj.iopcode = 0x03;
            decode_obj.ifunct3 = 0x0;
            //decode_obj.ifunct7 = 0x00;
            return 'I'; 
        }
        
    else if(strcmp(token_in,"lh")==0)
        {
            decode_obj.iopcode = 0x03;
            decode_obj.ifunct3 = 0x1;
            //decode_obj.ifunct7 = 0x00;
            return 'I'; 
        }
        
    else if(strcmp(token_in,"lw")==0)
        {
            decode_obj.iopcode = 0x03;
            decode_obj.ifunct3 = 0x2;
            //decode_obj.ifunct7 = 0x00;
            return 'I'; 
        }
        
    else if(strcmp(token_in,"lbu")==0)
        {
            decode_obj.iopcode = 0x03;
            decode_obj.ifunct3 = 0x4;
            //decode_obj.ifunct7 = 0x00;
            return 'I'; 
        }
        
    else if(strcmp(token_in,"lhu")==0)
        { 
            decode_obj.iopcode = 0x03;
            decode_obj.ifunct3 = 0x5;
            //decode_obj.ifunct7 = 0x00;
            return 'I'; 
        }
    else if(strcmp(token_in,"addi")==0)
        { 
            decode_obj.iopcode = 0x13;
            decode_obj.ifunct3 = 0x0;
            //decode_obj.ifunct7 = 0x00;
            return 'I'; 
        }
        
    else if(strcmp(token_in,"slli")==0)
        {
            decode_obj.iopcode = 0x13;
            decode_obj.ifunct3 = 0x1;
            decode_obj.ifunct7 = 0x00;
            return 'I'; 
        }
        
    else if(strcmp(token_in,"slti")==0)
        {  
            decode_obj.iopcode = 0x13;
            decode_obj.ifunct3 = 0x2;
            //decode_obj.ifunct7 = 0x00;
            return 'I'; 
        }
        
    else if(strcmp(token_in,"sltiu")==0)
        { 
            decode_obj.iopcode = 0x13;
            decode_obj.ifunct3 = 0x3;
            //decode_obj.ifunct7 = 0x00;
            return 'I'; 
        }
        
    else if(strcmp(token_in,"xori")==0)
        { 
            decode_obj.iopcode = 0x13;
            decode_obj.ifunct3 = 0x4;
            //decode_obj.ifunct7 = 0x00;
            return 'I'; 
        }
        
    else if(strcmp(token_in,"srli")==0)
        {  
            decode_obj.iopcode = 0x13;
            decode_obj.ifunct3 = 0x5;
            decode_obj.ifunct7 = 0x00;
            return 'I'; 
        }
        
    else if(strcmp(token_in,"srai")==0)
        { 

            decode_obj.iopcode = 0x13;
            decode_obj.ifunct3 = 0x5;
            decode_obj.ifunct7 = 0x20;
            return 'I'; 
        }
        
    else if(strcmp(token_in,"ori")==0)
        { 
            decode_obj.iopcode = 0x13;
            decode_obj.ifunct3 = 0x6;
            //decode_obj.ifunct7 = 0x00;
            return 'I'; 
        }
        
    else if(strcmp(token_in,"andi")==0)
        { 
            decode_obj.iopcode = 0x13;
            decode_obj.ifunct3 = 0x7;
            //decode_obj.ifunct7 = 0x00;
            return 'I'; 
        }
        
    else if(strcmp(token_in,"jalr")==0)
        {

            decode_obj.iopcode = 0x67;
            decode_obj.ifunct3 = 0x0;
            // decode_obj.ifunct7 = 0x00;
            return 'I'; 
        }
        
    //S Type instructions
        
    else if(strcmp(token_in,"sb")==0)
        { 
            decode_obj.iopcode = 0x23;
            decode_obj.ifunct3 = 0x0;
            return 'S'; 
        }
        
    else if(strcmp(token_in,"sh")==0)
        {
            decode_obj.iopcode = 0x23;
            decode_obj.ifunct3 = 0x1;
            return 'S'; 
        }
        
    else if(strcmp(token_in,"sw")==0)
        { 
            decode_obj.iopcode = 0x23;
            decode_obj.ifunct3 = 0x2;
            return 'S'; 
        }
        
    //B Type instructions

    else if(strcmp(token_in,"beq")==0)
        { 
            decode_obj.iopcode = 0x63;
            decode_obj.ifunct3 = 0x0;
            return 'B'; 
        }
        
    else if(strcmp(token_in,"bne")==0)
        {  
            decode_obj.iopcode = 0x63;
            decode_obj.ifunct3 = 0x1;
            return 'B'; 
        }
    else if(strcmp(token_in,"blt")==0)
        {  

            decode_obj.iopcode = 0x63;
            decode_obj.ifunct3 = 0x4;
            return 'B'; 
        }
        
    else if(strcmp(token_in,"bge")==0)
        {  

            decode_obj.iopcode = 0x63;
            decode_obj.ifunct3 = 0x5;
            return 'B';
        }
        
    else if(strcmp(token_in,"bltu")==0)
        {  

            decode_obj.iopcode = 0x63;
            decode_obj.ifunct3 = 0x6;
            return 'B';
        }
        
    else if(strcmp(token_in,"bgeu")==0)
        {  

            decode_obj.iopcode = 0x63;
            decode_obj.ifunct3 = 0x7;
            return 'B';
        }
        
    //U Type instructions
        
    else if(strcmp(token_in,"auipc")==0)
        { 
            decode_obj.iopcode = 0x17;
            return 'U'; 
        }
        
    else if(strcmp(token_in,"lui")==0)
        { 
            decode_obj.iopcode = 0x37;
            return 'U'; 
        }

    //J type
        
    else if(strcmp(token_in,"jal")==0)
        { 
            decode_obj.iopcode = 0x6F;
            return 'J'; 
        }
        
    //End    
    else 
        return 'E';

}