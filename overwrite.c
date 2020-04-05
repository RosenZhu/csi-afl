#include	<elf.h>
#include	<libelf.h>
#include	<gelf.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <string.h>

const int A = 0x823005;
const int B = 0x823112;

int main(int argc, char** argv) {
    int fd = open (argv[1], O_RDONLY);
    if (fd < 0) return 1;
    struct stat stat;
    if (fstat (fd, &stat) < 0) {
      close(fd);
      return 1;
    }

    char* buffer = (char*) mmap (NULL, stat.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    if (buffer == NULL) {
      close(fd);
      return 1;
    }
    elf_version(EV_CURRENT);
    Elf *elf = elf_memory(buffer, stat.st_size);

    size_t shstrndx;
    if (elf_getshdrstrndx(elf, &shstrndx) != 0) {
	fprintf(stderr, "Cannot not get section string table\n");
        return 1;
    }

    Elf_Scn *scn = NULL;
    GElf_Shdr secHead;
    char *sectionName;

    while ((scn = elf_nextscn(elf, scn)) != NULL) {
      gelf_getshdr(scn, &secHead);
      sectionName = elf_strptr(elf, shstrndx, secHead.sh_name); 
      if (strcmp(sectionName, ".dyninstInst") != 0) continue;
      int offset = A - secHead.sh_addr + secHead.sh_offset;
      buffer[offset] = 0xe9;
      *((int*)(buffer + offset + 1)) = B - A - 5;
      break;
    }

    int fd2 = creat(argv[2], S_IRWXU | S_IRWXG | S_IRWXO);
    printf("write file %d\n", fd2);
    write(fd2, buffer, stat.st_size);
    munmap(buffer, stat.st_size);
    close(fd);
    close(fd2);

}
