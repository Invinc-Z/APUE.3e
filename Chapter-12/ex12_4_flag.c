#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSTRINGSZ 4096

extern char **environ;

static pthread_once_t init_done = PTHREAD_ONCE_INIT;
static pthread_key_t key;
static pthread_mutex_t env_mutex = PTHREAD_MUTEX_INITIALIZER;

/* 标记：线程相关初始化是否已经完成 */
static volatile int thread_ready = 0;

/* 线程退出时释放线程私有缓冲区 */
static void free_buf(void *arg) { free(arg); }

/* 真正的线程初始化函数 */
static void thread_init(void) { pthread_key_create(&key, free_buf); }

/*
 * 启动早期使用的简单版：
 * - 不调用 pthread
 * - 不调用 malloc
 * - 不分配线程私有缓冲区
 * - 直接返回 environ 中原始字符串的 value 部分
 *
 * 注意：这不是线程安全的，只适合初始化完成前、单线程阶段使用。
 */
static char *getenv_simple(const char *name) {
  int len;
  char **envp;

  if (name == NULL)
    return NULL;

  len = (int)strlen(name);

  for (envp = environ; envp != NULL && *envp != NULL; envp++) {
    if (strncmp(name, *envp, len) == 0 && (*envp)[len] == '=') {
      return &(*envp)[len + 1];
    }
  }
  return NULL;
}

/*
 * 显式初始化接口：
 * 程序应在进入多线程阶段之前调用一次。
 */
int getenv_r_init(void) {
  int err;

  err = pthread_once(&init_done, thread_init);
  if (err != 0)
    return err;

  thread_ready = 1;
  return 0;
}

/*
 * 线程安全版 getenv_r
 *
 * 返回值：
 *   成功：返回指向“当前线程私有缓冲区”的指针
 *   失败：返回 NULL
 *
 * 行为：
 *   - 初始化前：退回简单版
 *   - 初始化后：返回每线程独立副本
 */
char *getenv_r(const char *name) {
  int len, olen;
  char *buf;
  char *value;
  char **envp;

  if (name == NULL)
    return NULL;

  /*
   * 关键点：
   * 初始化完成前，不碰 pthread_getspecific / malloc / mutex
   * 直接走简单版，避免递归初始化问题。
   */
  if (!thread_ready)
    return getenv_simple(name);

  len = (int)strlen(name);

  pthread_mutex_lock(&env_mutex);

  for (envp = environ; envp != NULL && *envp != NULL; envp++) {
    if (strncmp(name, *envp, len) == 0 && (*envp)[len] == '=') {
      value = &(*envp)[len + 1];
      olen = (int)strlen(value) + 1;
      if (olen > MAXSTRINGSZ) {
        pthread_mutex_unlock(&env_mutex);
        return NULL;
      }

      buf = pthread_getspecific(key);
      if (buf == NULL) {
        buf = malloc(MAXSTRINGSZ);
        if (buf == NULL) {
          pthread_mutex_unlock(&env_mutex);
          return NULL;
        }
        if (pthread_setspecific(key, buf) != 0) {
          free(buf);
          pthread_mutex_unlock(&env_mutex);
          return NULL;
        }
      }

      strcpy(buf, value);
      pthread_mutex_unlock(&env_mutex);
      return buf;
    }
  }

  pthread_mutex_unlock(&env_mutex);
  return NULL;
}
// ====================== test ===========================

static void *thr_fn(void *arg) {
  char *p = getenv_r("PATH");
  printf("thread: PATH=%s\n", p ? p : "(null)");
  return NULL;
}

int main(void) {
  pthread_t tid;
  char *p;

  /* 启动早期：即使不初始化，也能工作，但只是简单版 */
  p = getenv_r("HOME");
  printf("before init: HOME=%s\n", p ? p : "(null)");

  /* 在进入多线程前显式初始化 */
  if (getenv_r_init() != 0) {
    fprintf(stderr, "getenv_r_init failed\n");
    exit(1);
  }

  p = getenv_r("USER");
  printf("after init: USER=%s\n", p ? p : "(null)");

  pthread_create(&tid, NULL, thr_fn, NULL);
  pthread_join(tid, NULL);

  return 0;
}
