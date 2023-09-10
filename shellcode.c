
int main(){
    asm(
    "xor %eax, %eax;\n"
    "xor %ecx, %ecx;\n"
    "xor %edx, %edx;\n"
    "mov $0x01, %al;\n"
    "xor %ebx, %ebx;\n"
    "mov $0x02, %bl;\n"
    "int $0x80;\n"
    );
    return 1;
}