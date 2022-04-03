#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define THREADS 16
#define cells 3

int cell_s;
char **rez;
pthread_mutex_t rezL;

typedef struct {
  uint64_t start;
  uint64_t end;
  int len;
} job_t;

void printL(int len, int *list) {
  for (int i = 0; i < len; i++)
    printf("%d,", list[i]);
  printf("\n");
}

int fanint(int n) {
  int o = 0;
  for (int i = 0; i < cell_s; i++) {
    o |= ((i / (1 << (n))) % 2) << i;
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
  for (int i = 0; i < cells; i++) {
    o = set(o, i, fanint(i));
  }
  return o;
}

int nand(int x, int y) { return (~(x & y)) & filw1(cell_s); }
int bxor(int x, int y){return (x^y)&filw1(cell_s);}

uint64_t effect(uint64_t state, int a, int b, int c) {
  state = set(state, b,
              c ? bxor(get(state, a), get(state, b))
                : nand(get(state, a), get(state, b)));
  return state;
}

void *gate(void *in) {
  job_t *arg = in;
  long done = 0;
  for (uint64_t poz = arg->start; poz < arg->end; poz++) {
    if (rez[poz] != 0 && ((int)strlen(rez[poz]) == (arg->len - 1) * 3)) {
      done = 1;
      for (int x = 0; x < cells; x++)
        for (int y = 0; y < cells; y++)
          for (int m = 0; m < 2; m++) {
            uint64_t eff = effect(poz, x, y, m);
            if (rez[eff] == 0) {
              char *tmp = calloc(3 * arg->len + 1, sizeof(char));
              strncpy(tmp, rez[poz], (arg->len - 1) * 3);
              tmp[arg->len * 3 - 3] = m?'x':'a';
              tmp[arg->len * 3 - 2] = '0' + x;
              tmp[arg->len * 3 - 1] = '0' + y;
              pthread_mutex_lock(&rezL);
              rez[eff] = tmp;
              pthread_mutex_unlock(&rezL);
            }
          }
    }
  }
  free(in);
  return (void *)done;
}

int main() {
  assert(cells < 5);
  cell_s = 1 << cells;
  rez = calloc(1L << ((uint64_t)cells * cell_s), sizeof(char *));
  assert(rez != NULL);
  int j = 1;
  int to_do = 1;

  pthread_mutex_init(&rezL, NULL);

  char *def = malloc(sizeof(char));
  strcpy(def, "");
  rez[init()] = def;
  fprintf(stderr, "%ld\n", init());

  pthread_t niti[THREADS];

  while (to_do) {
    to_do = 0;
    for (int i = 0; i < THREADS; i++) {
      job_t job = {.start = i * (1 << (cells * cell_s)) / THREADS,
                   .end = (i + 1) * (1 << (cells * cell_s)) / THREADS,
                   .len = j};
      job_t *job2 = malloc(sizeof(job_t));
      memcpy(job2, &job, sizeof(job_t));
      pthread_create(&niti[i], NULL, gate, job2);
    }
    for (int i = 0; i < THREADS; i++) {
      void *out;
      pthread_join(niti[i], &out);
      long outi = (long)out;
      to_do |= outi;
    }
    fprintf(stderr, "%d\n", j++);
  }

  int najd = 0;
  for (int i = 0; i < 1 << (cells * cell_s); i++) {
    if (rez[i]) {
      printf("%#08x:%s\n", i, rez[i]);
      free(rez[i]);
      najd++;
    }
  }
  fprintf(stderr, "%d/%d\n", najd, 1 << (cells * cell_s));
  pthread_mutex_destroy(&rezL);
  free(rez);
  return 0;
}
