#include <libelf.h>
#include <gelf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


static void readelf(char *path)
{
    int fd;
    Elf *elf;
    size_t shnum;
    GElf_Ehdr ehdr;

    if (!path || ((fd = open(path, O_RDONLY)) < 0)) {
        fprintf(stderr, "failed to open binary...\n");
        goto err;
    }

    elf = elf_begin(fd, ELF_C_READ, NULL);
    if (!elf) {
        fprintf(stderr, "failed to open ELF...\n");
        goto err;
    }

    if (!gelf_getehdr(elf, &ehdr)) {
        fprintf(stderr, "failed to get ELF header from binary...\n");
        goto clean;
    }

    /* if (elf_getshdrnum(elf, &shnum) < 0) { */
    /*     fprintf(stderr, "failed to open ELF...\n"); */
    /*     goto clean; */
    /* } */
    /* fprintf(stdout, "section header count: %lu\n", shnum); */
    fprintf(stdout, "section header count: %u\n", ehdr.e_shnum);
    fprintf(stdout, "section header table index of section name string table: %u\n", ehdr.e_shstrndx);

    for (int i = 0; i < ehdr.e_shnum; i++) {
        char tmp[1024];
        GElf_Shdr shdr;
        char *shrname;
        Elf_Data *data;

        Elf_Scn *scn = elf_getscn(elf, i);
        if (!scn) {
            fprintf(stderr, "failed to get section: %d from binary...\n", i);
            continue;
        }

        if (!gelf_getshdr(scn, &shdr)) {
            fprintf(stderr, "failed to get section header: %d from binary...\n", i);
            continue;
        }

        shrname = elf_strptr(elf, ehdr.e_shstrndx, shdr.sh_name);
        fprintf(stdout, "section: %d, type: %u, offset: %lu, name: %s\n", i, shdr.sh_type, shdr.sh_offset, shrname);

        data = elf_getdata(scn, NULL);
        if (data) {
            fprintf(stdout, "section data length: %lu, section data type: %d\n", data->d_size, data->d_type);
            fprintf(stdout, "section data buf: %p\n", data->d_buf);
            if (data->d_buf) {
                memcpy(tmp, data->d_buf, data->d_size < 1024 ? data->d_size : 1024);
                tmp[1023] = '\0';
                fprintf(stdout, "section data: %s\n", tmp);
            }
        }
    }

clean:
    elf_end(elf);
err:
}

int main (int argc, char **argv) {
    if (elf_version(EV_CURRENT) == EV_NONE) {
        fprintf(stderr, "unsupported libelf version...\n");
        goto out;
    }

    char *path;
    if (argc >= 2) {
        path = argv[1];
    } else {
        path = "/proc/self/exe";
    }

    fprintf(stdout, "%s\n", path);

    readelf(path);
out:
    exit(-1);
}
