/**
  thread_sem.h

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


#ifndef __THREAD_SEMAPHORE_H__
#define __THREAD_SEMAPHORE_H__


/** @@ Modified code
 * fixed a bug in tsem_timed_down function.
 * added function for deinterlace case.
 **/

/** The structure contains the semaphore value, mutex and green light flag
 */
typedef struct thread_sem_t{
  pthread_cond_t condition;
  pthread_mutex_t mutex;
  unsigned int semval;
}thread_sem_t;

/** Initializes the semaphore at a given value
 *
 * @param tsem the semaphore to initialize
 *
 * @param val the initial value of the semaphore
 */
int thread_sem_init(thread_sem_t* tsem, unsigned int val);

/** Destroy the semaphore
 *
 * @param tsem the semaphore to destroy
 */
void thread_sem_deinit(thread_sem_t* tsem);

/** Decreases the value of the semaphore. Blocks if the semaphore
 * value is zero.
 *
 * @param tsem the semaphore to decrease
 */
void thread_sem_down(thread_sem_t* tsem);

/** Decreases the value of the semaphore. Blocks if the semaphore
 * value is zero. If the timeout is reached the function exits with
 * error ETIMEDOUT
 *
 * @param tsem the semaphore to decrease
 * @param timevalue the value of delay for the timeout
 */
int thread_sem_timed_down(thread_sem_t* tsem, unsigned int milliSecondsDelay);

/** Increases the value of the semaphore
 *
 * @param tsem the semaphore to increase
 */
void thread_sem_up(thread_sem_t* tsem);

/** Increases the value of the semaphore to one
 *
 * @param tsem the semaphore to increase
 */
void thread_sem_up_to_one(thread_sem_t* tsem);

/** Reset the value of the semaphore
 *
 * @param tsem the semaphore to reset
 */
void thread_sem_reset(thread_sem_t* tsem);

/** Wait on the condition.
 *
 * @param tsem the semaphore to wait
 */
void thread_sem_wait(thread_sem_t* tsem);

/** Signal the condition,if waiting
 *
 * @param tsem the semaphore to signal
 */
void thread_sem_signal(thread_sem_t* tsem);

unsigned int thread_sem_get_semval(thread_sem_t* tsem);

#endif
