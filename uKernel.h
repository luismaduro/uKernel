/**
 *  @file           uKernel.h
 *  @author         Luis Maduro
 *  @version        1.0
 *  @date           03/05/2013
 *  @copyright		GNU General Public License
 *
 *  @brief This is a scheduler for micrcontrollers. 
 *  This is not any kind of RTOS, or anything like it. 
 *  Just create a function, create a descriptor for that function and added to 
 *  the scheduler with a period and let the scheduler do the rest. There is no 
 *	priority and I am tring to keep it really simple due to the memory 
 *  limitations of micrcontrollers. The maximum number of task is 255 but I am
 *  sure that the memory will go out first. If anyone needs more tasks let me
 *  know.
 */

#ifndef UKERNEL_H
#define	UKERNEL_H

#ifdef	__cplusplus
extern "C"
{
#endif

#include <xc.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define MAX_TASKS_NUMBER            255

/**Set your max interval here (max 2^32-1) - default 3600000 (1 hour)*/
#define MAX_TASK_INTERVAL           3600000UL

typedef enum
{
    /**For a task that doesn't have to start immediately.*/
    uKernel_PAUSED = 0x00, //0b00000000
    /**For a normal task that has to start after its scheduling*/
    uKernel_SCHEDULED = 0x01, //0b00000001
    /**For a task that has to run only once.*/
    uKernel_ONETIME = 0x02, //0b00000010
    /**For a task that has to be executed once it has been added.*/
    uKernel_IMMEDIATESTART = 0x05, //0b00000101
    /**For the task to be executed one time as soon as it is added.*/
    uKernel_ONETIME_IMMEDIATESTART = 0x07, //0b00000111
    /**Error, task not found.*/
    uKernel_ERROR = 0xFF //0b11111111
} uKernelTaskStatus;

/**Function pointer on the task body.*/
typedef void (*TaskBody)(void);

typedef struct _uKernelTaskDescriptor
{
    //    /**Pointer to the previous task in the list.*/
    //    struct _uKernelTaskDescriptor *pTaskPrevious;
    /**Used to store the pointers to user's tasks*/
    TaskBody taskPointer;
    /**Used to store the interval between each task's run*/
    uint32_t userTasksInterval;
    /**Used to store the next time a task will have to be executed*/
    uint32_t plannedTask;
    /**Used to store the status of the tasks*/
    uKernelTaskStatus taskStatus;
    /**Pointer to the next task in the list.*/
    struct _uKernelTaskDescriptor *pTaskNext;
} uKernelTaskDescriptor;

extern uint32_t _counterMs;

/**
 * This funtion as to be called before doing anything with the tasker. It
 * initiates the tasker subsystems. If this funtion is not called before doing
 * anything with the tasker all funtions will return false.
 */
void uKernelInit(void);
/**
 * Add a task into the circular linked list for the scheduler.
 * @param pTaskDescriptor   Descriptor of the task.
 * @param userTask          Function pointer on the task body
 * @param taskInterval Scheduled interval in milliseconds at which you want your
 *                     routine to be executed.
 * @param taskStatus Status of the task to be added to the scheduler, status can
 *                   be: PAUSED, for a task that doesn't have to start immediately;
 *                   SCHEDULED, for a normal task that has to start after its
 *                   scheduling; ONETIME, for a task that has to run only once;
 *                   IMMEDIATESTART, for a task that has to be executed once it
 *                   has been added to the scheduler; ONETIME_IMMEDIATESTART, for
 *                   a task that as to be executed now and it will only be
 *                   executed one time.
 * @return True or False
 * @see @uKernelTaskStatus
 */
bool uKernelAddTask(uKernelTaskDescriptor *pTaskDescriptor,
                    void (*userTask)(void),
                    uint32_t taskInterval,
                    uKernelTaskStatus taskStatus);
/**
 * This funtion is used to remove the task from the scheduler.
 * @param pTaskDescriptor Descriptor of the task to be removed.
 * @return Return true if all went well, false otherwise.
 */
bool uKernelRemoveTask(uKernelTaskDescriptor *userTaskDescriptor);
/**
 * This funtion is used to pause the task on the scheduler.
 * @param pTaskDescriptor Descriptor of the task to be paused.
 * @return Return true if all went well, false otherwise.
 */
bool uKernelPauseTask(uKernelTaskDescriptor *pTaskDescriptor);
/**
 * This funtion is used to restart a funtion that has been paused.
 * @param pTaskDescriptor Descriptor of the task to be resumed.
 * @return Return true if all went well, false otherwise.
 */
bool uKernelResumeTask(uKernelTaskDescriptor *pTaskDescriptor,
                       uKernelTaskStatus taskStatus);
/**
 * Add a task into the circular linked list for the scheduler.
 * @param pTaskDescriptor   Descriptor of the task.
 * @param taskInterval Scheduled interval in milliseconds at which you want your
 *                     routine to be executed.
 * @param tStatus Status of the task to be added to the scheduler, status can
 *                   be: PAUSED, for a task that doesn't have to start immediately;
 *                   SCHEDULED, for a normal task that has to start after its
 *                   scheduling; ONETIME, for a task that has to run only once;
 *                   IMMEDIATESTART, for a task that has to be executed once it
 *                   has been added to the scheduler; ONETIME_IMMEDIATESTART, for
 *                   a task that as to be executed now and it will only be
 *                   executed one time.
 * @return True or False
 * @see @uKernelTaskStatus
 */
bool uKernelModifyTask(uKernelTaskDescriptor *pTaskDescriptor,
                       uint32_t taskInterval,
                       uKernelTaskStatus tStatus);
/**
 * Funtion to check if a task is running.
 * @param userTask Task to check the status.
 * @return The status of the tasks.
 * @retval PAUSED Task is paused/not running.
 * @retval SCHEDULED Task is running.
 * @retval ONETIME Task is scheduled to run in a near future.
 * @retval ERROR There was an error (task not found)
 */
uKernelTaskStatus uKernelGetTaskStatus(uKernelTaskDescriptor *pTaskDescriptor);
/**
 * Scheduling. This runs the kernel itself.
 */
void uKernelScheduler(void);
/**
 * Just a simple delay in miliseconds. Not related to the Tasker system.
 */
void uKernelDelayMiliseconds(uint16_t delay);

#ifdef	__cplusplus
}
#endif

#endif	/* UKERNEL_H */

