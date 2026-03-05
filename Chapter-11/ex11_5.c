typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
    int count;      // 当前到达的线程数
    int tripCount;  // 需要等待的线程总数
} mybarrier_t;

int pthread_barrier_wait(mybarrier_t *barrier)
{
    int ret = 0;

    pthread_mutex_lock(&barrier->mutex);

    barrier->count++;

    if (barrier->count == barrier->tripCount) {

        /* 最后一个线程到达 */
        barrier->count = 0;

        pthread_cond_broadcast(&barrier->cond);

        ret = PTHREAD_BARRIER_SERIAL_THREAD;

    } else {

        /* 其他线程等待 */
        pthread_cond_wait(&barrier->cond, &barrier->mutex);
    }

    pthread_mutex_unlock(&barrier->mutex);

    return ret;
}
