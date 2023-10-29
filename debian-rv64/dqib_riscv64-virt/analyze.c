#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
  assert(argc == 2);
  int fd = open(argv[1], O_RDONLY);
  assert(fd != -1);

  struct stat s;
  assert(!fstat(fd, &s));

  char *file = mmap(NULL, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
  assert(file);

  int num_lines = 0;
  for (int i = 0; i < s.st_size; ++i) {
    num_lines += file[i] == '\n';
  }
  printf("%s contains %d lines.\n", argv[1], num_lines);

  uintptr_t *virt = calloc(num_lines, sizeof(uintptr_t));
  uintptr_t *phys = calloc(num_lines, sizeof(uintptr_t));
  uintptr_t *size = calloc(num_lines, sizeof(uintptr_t));
  for (int i = 0; i < num_lines; ++i) {
    assert(sscanf(file + (i * 51), "%lx %lx %lx\n", virt + i, phys + i,
                  size + i) == 3);
  }

  uintptr_t desired_next_virtual_page, desired_next_physical_page;
  uintptr_t first_contiguous_virtual_page, first_contiguous_physical_page,
      contiguous_pages_size;
  int num_contiguous_pages = 0;
  for (int i = 0; i < num_lines; ++i) {
    if (num_contiguous_pages == 0) {
      num_contiguous_pages = 1;
      desired_next_virtual_page = virt[i] + size[i];
      desired_next_physical_page = phys[i] + size[i];
      first_contiguous_virtual_page = virt[i];
      first_contiguous_physical_page = phys[i];
      contiguous_pages_size = size[i];
    } else if (virt[i] == desired_next_virtual_page &&
               phys[i] == desired_next_physical_page) {
      num_contiguous_pages += 1;
      desired_next_virtual_page = virt[i] + size[i];
      desired_next_physical_page = phys[i] + size[i];
      contiguous_pages_size += size[i];
    } else {
      if (num_contiguous_pages > 1) {
        printf("Contiguous virt-to-phys mappings of length %d pages. The first "
               "virtual and physical page addresses are %lx and %lx. These "
               "contiguous pages form a coalesced page of size %lx.\n",
               num_contiguous_pages, first_contiguous_virtual_page,
               first_contiguous_physical_page, contiguous_pages_size);
      }
      num_contiguous_pages = 0;
    }
  }
}
