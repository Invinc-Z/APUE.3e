#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

extern char **environ;

static pthread_once_t env_once = PTHREAD_ONCE_INIT;
static pthread_mutex_t env_mutex;

/*
 * 如果当前 environ 数组是我们自己分配的，则记录下来，
 * 这样下次扩容后可以安全释放旧数组。
 */
static char **managed_environ = NULL;

static void env_init(void) { pthread_mutex_init(&env_mutex, NULL); }

/* 返回 NAME 的长度；如果没有 '='，则返回整个字符串长度 */
static int name_len(const char *str) {
  const char *eq;

  if (str == NULL || *str == '\0')
    return -1;

  eq = strchr(str, '=');
  if (eq == str) /* "=value" 非法 */
    return -1;

  if (eq == NULL)
    return (int)strlen(str);

  return (int)(eq - str);
}

/*
 * putenv_r:
 *   str = "NAME=value"  -> 设置或替换
 *   str = "NAME"        -> 删除该环境变量
 *
 * 返回:
 *   0       成功
 *   EINVAL  参数不合法
 *   ENOMEM  内存不足
 */
int putenv_r(const char *str) {
  int i, envc, nlen;
  const char *eq;
  char *newstr = NULL;

  if (str == NULL || *str == '\0')
    return EINVAL;

  nlen = name_len(str);
  if (nlen <= 0)
    return EINVAL;

  eq = strchr(str, '=');

  if (pthread_once(&env_once, env_init) != 0)
    return EINVAL;

  if (pthread_mutex_lock(&env_mutex) != 0)
    return EINVAL;

  /* 统计当前环境变量个数 */
  for (envc = 0; environ != NULL && environ[envc] != NULL; envc++)
    ;

  /* 查找是否已有同名变量 */
  for (i = 0; i < envc; i++) {
    if (strncmp(environ[i], str, nlen) == 0 && environ[i][nlen] == '=') {
      break;
    }
  }

  /*
   * 情况 1：删除
   * 输入格式 "NAME"
   */
  if (eq == NULL) {
    if (i < envc) {
      /*
       * 只有我们自己管理的字符串才应该 free。
       * 本实现约定：凡是通过 putenv_r 插入/替换进去的，
       * 都是我们自己复制的。
       */
      free(environ[i]);

      for (; i < envc; i++)
        environ[i] = environ[i + 1];
    }

    pthread_mutex_unlock(&env_mutex);
    return 0;
  }

  /*
   * 情况 2/3：设置或替换
   * 统一复制字符串，避免直接引用调用者传入的缓冲区。
   */
  newstr = malloc(strlen(str) + 1);
  if (newstr == NULL) {
    pthread_mutex_unlock(&env_mutex);
    return ENOMEM;
  }
  strcpy(newstr, str);

  /*
   * 情况 2：已存在，替换
   */
  if (i < envc) {
    free(environ[i]);
    environ[i] = newstr;

    pthread_mutex_unlock(&env_mutex);
    return 0;
  }

  /*
   * 情况 3：不存在，追加
   * 需要 envc + 2 个槽位：
   *   [0..envc-1] 旧项
   *   [envc]      新项
   *   [envc+1]    NULL
   */
  {
    char **oldenv = environ;
    char **newenv;

    newenv = malloc((size_t)(envc + 2) * sizeof(char *));
    if (newenv == NULL) {
      free(newstr);
      pthread_mutex_unlock(&env_mutex);
      return ENOMEM;
    }

    for (i = 0; i < envc; i++)
      newenv[i] = oldenv[i];

    newenv[envc] = newstr;
    newenv[envc + 1] = NULL;

    environ = newenv;

    /*
     * 只有 oldenv 是我们上次自己分配的数组时，才能释放。
     * 进程初始的 environ 可能不是 malloc 来的，不能乱 free。
     */
    if (oldenv == managed_environ)
      free(oldenv);

    managed_environ = newenv;
  }

  pthread_mutex_unlock(&env_mutex);
  return 0;
}
