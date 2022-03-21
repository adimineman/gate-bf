#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define cells 3
int cell_s;
int cls[cells] = {0};
char **rez;

void printL(int len, int *list) {
  for (int i = 0; i < len; i++)
    printf("%d,", list[i]);
  printf("\n");
}

int fanint(int n) {
  int o = 0;
  for (int i = 0; i < cell_s; i++) {
    o |= ((i / (n + 1)) % 2) << i;
  }
  return o;
}

uint64_t filw1(int len) { return (1 << len) - 1; }

uint64_t set(uint64_t in, int ind, int val) {
  in &= ~(filw1(cell_s) << (ind * cell_s));
  in |= (val & filw1(cell_s)) << (ind * cell_s);
  return in;
}
int get(uint64_t in, int ind) { return (in >> (ind * cell_s)) & filw1(cell_s); }

uint64_t init() {
  uint64_t o = 0;
  for (int i = 0; i < cells; i++)
    o = set(o, i, fanint(i));
  return o;
}

int nand(int x, int y) { return (~(x & y)) & filw1(cell_s); }

uint64_t effect(int len, int *arg) {
  uint64_t out = init();
  for (int t = 0; t < len; t += 2) {
    int x = arg[t], y = arg[t + 1];
    out = set(out, y, nand(get(out, x), get(out, y)));
  }
  return out;
}

int gate(int len) {
  if (len) {
    int *arg = calloc(len * 2, sizeof(int));
    int be = 2;
    int i = 0;
    int new = 0;
    uint64_t citer = 0;
    clock_t start = clock();
    while (be) {
      if (citer % 100000000 == 1)
        fprintf(stderr, "\t%f\n",
                (double)((clock() - start) *
                         (pow(cells, (double)len * 2) - citer)) /
                    citer / CLOCKS_PER_SEC);
      if (be == 1)
        arg[i]++;
      else
        be = 1;
      if (arg[i] >= cells) {
        arg[i] = 0;
        i++;
        if (i >= len * 2)
          be = 0;
      } else {
        citer++;
        i = 0;
        uint64_t eff = effect(len * 2, arg);
        if (rez[eff] == 0) {
          char *tmp = calloc(len * 2, sizeof(char));
          for (int a = 0; a < len * 2; a++) {
            tmp[a] = (char)arg[a] + '0';
          }
          rez[eff] = tmp;
          // printf("%s %lx\n",tmp,eff);
          new = 1;
        } else
          continue;
      }
    }
    free(arg);
    return new;
  } else {
    uint64_t eff = init();
    if (rez[eff] == 0) {
      char *tmp = calloc(2, sizeof(char));
      tmp[0] = '-';
      return 1;
    }
    return 0;
  }
}

int main() {
  assert(cells < 5);
  cell_s = 1 << cells;
  rez = calloc(1 << (cells * cell_s), sizeof(char *));
  int j = 0;
  while (gate(j))
    fprintf(stderr, "%d\n", j++);
  for (int i = 0; i < 1 << (cells * cell_s); i++) {
    if (rez[i]) {
      printf("%#x:%s\n", i, rez[i]);
      free(rez[i]);
    }
  }
  free(rez);
  return 0;
}
