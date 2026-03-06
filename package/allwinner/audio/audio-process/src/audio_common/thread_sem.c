/**
  thread_sem.c

  Implements a simple inter-thread semaphore so not to have to deal with IPC
  creation and the like.

* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include "thread_sem.h"


/** @@ Modified code
 * fixed a bug in tsem_timed_down function.
 * added function for deinterlace case.
 **/

/** Initializes the semaphore at a given value
 *
 * @param tsem the semaphore to initialize
 * @param val the initial value of the semaphore
 *
 */
int thread_sem_init(thread_sem_t* tsem, unsigned int val) {
    int i;
    i = pthread_cond_init(&tsem->condition, NULL);
    if (i!=0) {
        return -1;
    }
    i = pthread_mutex_init(&tsem->mutex, NULL);
    if (i!=0) {
        return -1;
    }
    tsem->semval = val;
    return 0;
}

/** Destroy the semaphore
 *
 * @param tsem the semaphore to destroy
 */
void thread_sem_deinit(thread_sem_t* tsem) {
    pthread_cond_destroy(&tsem->condition);
    pthread_mutex_destroy(&tsem->mutex);
}

/** Decreases the value of the semaphore. Blocks if the semaphore
 * value is zero. If the timeout is reached the function exits with
 * error ETIMEDOUT
 *
 * @param tsem the semaphore to decrease
 * @param timevalue the value of delay for the timeout
 */
int thread_sem_timed_down(thread_sem_t* tsem, unsigned int milliSecondsDelay) {
    int err = 0;
    struct timespec final_time;
    struct timeval currentTime;
    long int microdelay;

    gettimeofday(&currentTime, NULL);
    /** convert timeval to timespec and add delay in milliseconds for the timeout */
    microdelay = ((milliSecondsDelay * 1000 + currentTime.tv_usec));
    final_time.tv_sec = currentTime.tv_sec + (microdelay / 1000000);
    final_time.tv_nsec = (microdelay % 1000000) * 1000;
    pthread_mutex_lock(&tsem->mutex);
    while (tsem->semval == 0) {
        err = pthread_cond_timedwait(&tsem->condition, &tsem->mutex, &final_time);
        if (err != 0) {
            /** @@ Modified code
             * fixed a bug in tsem_timed_down function.
             **/
            tsem->semval++;
        }
    }
    tsem->semval--;
    pthread_mutex_unlock(&tsem->mutex);
    return err;
}

/** Decreases the value of the semaphore. Blocks if the semaphore
 * value is zero.
 *
 * @param tsem the semaphore to decrease
 */
void thread_sem_down(thread_sem_t* tsem) {
    pthread_mutex_lock(&tsem->mutex);
    while (tsem->semval == 0) {
        pthread_cond_wait(&tsem->condition, &tsem->mutex);
    }
    tsem->semval--;
    pthread_mutex_unlock(&tsem->mutex);
}

/** Increases the value of the semaphore
 *
 * @param tsem the semaphore to increase
 */
void thread_sem_up(thread_sem_t* tsem) {
    pthread_mutex_lock(&tsem->mutex);
    tsem->semval++;
    pthread_cond_signal(&tsem->condition);
    pthread_mutex_unlock(&tsem->mutex);
}

/** Increases the value of the semaphore to one
 *
 * @param tsem the semaphore to increase
 */
void thread_sem_up_to_one(thread_sem_t* tsem) {
    pthread_mutex_lock(&tsem->mutex);
    if (tsem->semval == 0) {
        tsem->semval++;
        pthread_cond_signal(&tsem->condition);
    }
    pthread_mutex_unlock(&tsem->mutex);
}

/** Reset the value of the semaphore
 *
 * @param tsem the semaphore to reset
 */
void thread_sem_reset(thread_sem_t* tsem) {
    pthread_mutex_lock(&tsem->mutex);
    tsem->semval=0;
    pthread_mutex_unlock(&tsem->mutex);
}

/** Wait on the condition.
 *
 * @param tsem the semaphore to wait
 */
void thread_sem_wait(thread_sem_t* tsem) {
    pthread_mutex_lock(&tsem->mutex);
    pthread_cond_wait(&tsem->condition, &tsem->mutex);
    pthread_mutex_unlock(&tsem->mutex);
}

/** Signal the condition,if waiting
 *
 * @param tsem the semaphore to signal
 */
void thread_sem_signal(thread_sem_t* tsem) {
    pthread_mutex_lock(&tsem->mutex);
    pthread_cond_signal(&tsem->condition);
    pthread_mutex_unlock(&tsem->mutex);
}

unsigned int thread_sem_get_semval(thread_sem_t* tsem) {
    return tsem->semval;
}

