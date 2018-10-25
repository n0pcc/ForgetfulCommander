/*
 * clang -s -m32 -nostdlib -nodefaultlibs -fPIC -Wl,-shared entry.c -oentry
 */

__attribute__((naked)) void entry() {
    __asm__ volatile(
      ".intel_syntax noprefix\n"
      "push eax\n"
      "pushad\n"
      "pushfd\n"

      "sub esp, 0x5000\n"

      // get the pathname of the binary with readlink
      "mov eax, 85\n"
      // /proc/self/exe
      "push 0x6578\n"
      "push 0x652f666c\n"
      "push 0x65732f63\n"
      "push 0x6f72702f\n"
      // pathname of the symlink
      "mov ebx, esp\n"
      // buf
      "lea ecx, [esp + 0x4010]\n"
      // bufsiz
      "mov edx, 0xfff\n"
      "int 0x80\n"

      // cleanup arguments from stack
      "add esp, 0x10\n"

      "cmp eax, 0\n"
      "jle fail\n"

      // save the length of the pathname
      "mov edi, eax\n"

      // write newline at the end of the pathname
      "lea ecx, [esp + 0x4000 + eax]\n"
      "mov byte ptr [ecx], 0xa\n"

      // inc saved length, to also compare the newline
      "inc edi\n"

      // open /proc/self/maps to get the base address of the binary
      "mov eax, 5\n"
      "push 0x737061\n"
      "push 0x6d2f666c\n"
      "push 0x65732f63\n"
      "push 0x6f72702f\n"
      "mov ebx, esp\n"
      "xor ecx, ecx\n"
      "int 0x80\n"

      // cleanup arguments from stack
      "add esp, 0x10\n"

      "cmp eax, 0\n"
      "jl fail\n"

      // save the fd
      "mov esi, eax\n"

      // loop to get the program base
      // read from /proc/self/maps
      "read:"
      "mov eax, 3\n"
      "mov ebx, esi\n"
      "mov ecx, esp\n"
      "mov edx, 0x4000\n"
      "int 0x80\n"

      "cmp eax, 0\n"
      "jle close\n"

      // set maps iterator to 0
      "xor ecx, ecx\n"
      // set pathname iterator to 0
      "xor edx, edx\n"
      // ebp will be set to 0, if we got a match
      "mov ebp, 1\n"

      "find_pathname:\n"
      "cmp ecx, eax\n"
      "jge find_pathname_finished\n"

      // char from the maps buffer
      "movzx ebx, byte ptr [esp + ecx]\n"
      // compare with char in the pathname buffer
      // always inc maps iterator
      "inc ecx\n"
      "cmp byte ptr [esp + edx + 0x4000], bl\n"
      "jnz find_pathname_no_match\n"
      // inc pathname iterator
      "inc edx\n"
      // have we found the full pathname?
      "cmp edx, edi\n"
      "jnz find_pathname\n"
      // jepp
      "xor ebp, ebp\n"
      "jmp find_pathname_finished\n"

      "find_pathname_no_match:\n"
      // reset pathname iterator
      "xor edx, edx\n"
      "jmp find_pathname\n"

      "find_pathname_finished:\n"

      // did we have a match?
      "test ebp, ebp\n"
      "jnz read\n"
      // jepp
      // search the beginning of the maps line
      "find_beginning_of_line:"
      "xor ebx, ebx\n"
      "dec ecx\n"
      // is the maps iterator non null?
      "cmp ecx, 0\n"
      "setg bl\n"
      // is the char before the maps iterator not a '\n'
      "cmp byte ptr [esp + ecx - 1], 0xa\n"
      "setnz bh\n"
      "test bl, bh\n"
      "jnz find_beginning_of_line\n"

      // convert the 8 hex digits to 4 bytes => the base address
      "xor edx, edx\n"
      "mov ebp, ecx\n"
      "add ecx, 8\n"
      "convert_base:"
      "cmp ebp, ecx\n"
      "jge close\n"
      "rol edx, 4\n"

      // check if it's a number
      "xor ebx, ebx\n"
      "cmp byte ptr [esp + ecx + ebp], 0x30\n"
      "setge bl\n"
      "cmp byte ptr [esp + ecx + ebp], 0x39\n"
      "setle bh\n"
      "sub byte ptr [esp + ecx + ebp], 0x30\n"
      "xor dl, byte ptr [esp + ecx + ebp]\n"
      "inc ebp\n"
      "test bl, bh\n"
      "jnz convert_base\n"
      // wasn't a number, revert stuff
      "dec ebp\n"
      "xor dl, byte ptr [esp + ecx + ebp]\n"
      "add byte ptr [esp + ecx + ebp], 0x30\n"
      // check if it's a lowercase hex letter
      "xor ebx, ebx\n"
      "cmp byte ptr [esp + ecx + ebp], 0x61\n"
      "setge bl\n"
      "cmp byte ptr [esp + ecx + ebp], 0x7a\n"
      "setle bh\n"
      "sub byte ptr [esp + ecx + ebp], 0x57\n"
      "xor dl, byte ptr [esp + ecx + ebp]\n"
      "inc ebp\n"
      "test bl, bh\n"
      "jnz convert_base\n"
      // wasn't a lowercase hex letter, revert stuff
      "dec ebp\n"
      "xor dl, byte ptr [esp + ecx + ebp]\n"
      "add byte ptr [esp + ecx + ebp], 0x61\n"
      // check if it's an uppercase hex letter
      "xor ebx, ebx\n"
      "cmp byte ptr [esp + ecx + ebp], 0x41\n"
      "setge bl\n"
      "cmp byte ptr [esp + ecx + ebp], 0x5a\n"
      "setle bh\n"
      "sub byte ptr [esp + ecx + ebp], 0x37\n"
      "xor dl, byte ptr [esp + ecx + ebp]\n"
      "inc ebp\n"
      "test bl, bh\n"
      "jnz convert_base\n"
      // wasn't an uppercase hex letter, failed to convert
      "xor edx, edx\n"
      "jmp close\n"

      "jmp read\n"

      "close:\n"
      // close the fd
      "mov eax, 6\n"
      "mov ebx, esi\n"
      "int 0x80\n"

      // if the base hasn't been found quit
      "test edx, edx\n"
      "jz fail\n"

      // set the oep as return
      "add edx, 0x2050\n"
      "mov dword ptr [esp + 0x5024], edx\n"
      "sub edx, 0x50\n"

      "mov ebp, eax\n"
      "inc eax\n"
      "inc eax\n"

      "dec_outer_loop:\n"
      "mov esi, 0x466c7578\n"
      "xor ebx, ebx\n"
      "cmp eax, ebp\n"
      "jz dec_outer_loop_finished\n"

      "mov ecx, 0x400\n"

      "dec_inner_loop:\n"
      "xor dword ptr [edx], ebx\n"
      "xor dword ptr [edx], esi\n"
      "ror esi, 8\n"
      "mov ebx, dword ptr [edx]\n"
      "add edx, 4\n"
      "loop dec_inner_loop\n"

      "dec eax\n"
      "jmp dec_outer_loop\n"

      "dec_outer_loop_finished:\n"

      "add esp, 0x5000\n"
      "popfd\n"
      "popad\n"
      "ret\n"

      "fail:\n"
      // exit the program gracefully
      "mov eax, 1\n"
      "xor ebx, ebx\n"
      "int 0x80");
}
