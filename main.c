#include <assert.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define THREADS 2
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

uint64_t effect(uint64_t state, int a, int b, int c) {
  state = set(state, b,
              c ? get(state, a) ^ get(state, b)
                : nand(get(state, a), get(state, b)));
  return state;
}

/*int gate(int len) {
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
}*/

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
              pthread_mutex_lock(&rezL);
              char *tmp = calloc(3 * arg->len + 1, sizeof(char));
              strncpy(tmp, rez[poz], (arg->len - 1) * 3);
              tmp[arg->len * 3 - 3] = '0' + x;
              tmp[arg->len * 3 - 2] = '0' + y;
              tmp[arg->len * 3 - 1] = '0' + m;
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

  /*while (gate(j))
    fprintf(stderr, "%d\n", j++);*/
  int najd = 0;
  for (int i = 0; i < 1 << (cells * cell_s); i++) {
    if (rez[i]) {
      printf("%#x:%s\n", i, rez[i]);
      free(rez[i]);
      najd++;
    }
  }
  fprintf(stderr, "%d/%d\n", najd, 1 << (cells * cell_s));
  pthread_mutex_destroy(&rezL);
  free(rez);
  return 0;
}
