/**
 *  @file           uKernel.c
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
 *  sure that the memory will go out first. If anyone needs more tasks let me know.
 */

#include "uKernel.h"

uint8_t _initialized;
uint32_t _counterMs;
uint8_t _numberTasks;
uKernelTaskDescriptor *pTaskSchedule;
static uKernelTaskDescriptor *pTaskFirst = NULL;

uint8_t uKernelSetTask(uKernelTaskDescriptor *pTaskDescriptor,
                       uint32_t taskInterval,
                       uKernelTaskStatus tStatus);

void uKernelInit(void)
{
    _initialized = true;
    _counterMs = 0;
    _numberTasks = 0;
    pTaskSchedule = NULL;
}

bool uKernelAddTask(uKernelTaskDescriptor *pTaskDescriptor,
                    void (*userTask)(void),
                    uint32_t taskInterval,
                    uKernelTaskStatus taskStatus)
{
    uKernelTaskDescriptor *pTaskWork = NULL;

    if ((_initialized == false) || (_numberTasks == MAX_TASKS_NUMBER)
            || (userTask == NULL))
    {
        return false;
    }

    if ((taskInterval < 1) || (taskInterval > MAX_TASK_INTERVAL))
    {
        taskInterval = 50; //50 ms by default
    }

    //check if taskStatus is valid, if not schedule
    if (taskStatus > uKernel_ONETIME_IMMEDIATESTART)
    {
        taskStatus = uKernel_SCHEDULED;
    }

    if (pTaskDescriptor != NULL)
    {
        if (pTaskFirst != NULL)
        {
            // Initialize the work pointer with the scheduler pointer (never mind is current state)
            pTaskWork = pTaskSchedule;
            // Research the last task from the circular linked list : so research the adresse of the
            // first task in testing the pTaskNext fields of each structure
            while (pTaskWork->pTaskNext != pTaskFirst)
            {
                // Set the work pointer on the next task
                pTaskWork = pTaskWork->pTaskNext;
            }
            pTaskWork->pTaskNext = pTaskDescriptor; // Insert the new task at the end of the circular linked list

            pTaskDescriptor->pTaskNext = pTaskFirst; // The next task is feedback at the first task
        }
        else
        {
            // There is no task in the scheduler, this task become the First task in the circular linked list
            pTaskFirst = pTaskDescriptor; // The pTaskFirst pointer is initialize with the first task of the scheduler
            pTaskSchedule = pTaskFirst; // Initialize the scheduler pointer at the first task
            pTaskDescriptor->pTaskNext = pTaskDescriptor; // The next task is itself because there is just one task in the circular linked list
        }
        // Common initialization for all tasks
        // no wait if the user wants the task up and running once added...
        //...otherwise we wait for the interval before to run the task
        pTaskDescriptor->plannedTask =
                _counterMs + ((taskStatus & 0x04) ? 0 : taskInterval);

        // Set the periodicity of the task
        pTaskDescriptor->userTasksInterval = taskInterval;
        // Set the task pointer on the task body
        pTaskDescriptor->taskPointer = userTask;
        //I get only the first 2 bits - I don't need the IMMEDIATESTART bit
        pTaskDescriptor->taskStatus = taskStatus & 0x03;

        _numberTasks++;

        return true;
    }
    else
    {
        // Delete all task of the scheduler
        // This case is necessary at the time of the call of the function DeleteAllTask()
        pTaskFirst = NULL;
        pTaskSchedule = NULL;

        return true;
    }
}

bool uKernelRemoveTask(uKernelTaskDescriptor *pTaskDescriptor)
{
    uKernelTaskDescriptor *pTaskCurr = NULL;

    if ((_initialized == false) || (_numberTasks == 0) ||
            pTaskDescriptor == NULL)
    {
        return false;
    }

    // Initialize the work pointer with the scheduler pointer (never mind is current state)
    pTaskCurr = pTaskFirst;

    while (pTaskCurr->pTaskNext != pTaskDescriptor)
    {
        // Set the work pointer on the next task
        pTaskCurr = pTaskCurr->pTaskNext;
    }

    if (pTaskCurr->pTaskNext == pTaskFirst)
    {
        pTaskFirst = pTaskCurr->pTaskNext->pTaskNext;
    }
    else
    {
        pTaskCurr->pTaskNext = pTaskDescriptor->pTaskNext;
    }

    _numberTasks--;

    return true;
}

bool uKernelPauseTask(uKernelTaskDescriptor *pTaskDescriptor)
{
    return (uKernelSetTask(pTaskDescriptor, NULL, uKernel_PAUSED));
}

bool uKernelResumeTask(uKernelTaskDescriptor *pTaskDescriptor, uKernelTaskStatus taskStatus)
{
    return (uKernelSetTask(pTaskDescriptor, NULL, taskStatus));
}

bool uKernelModifyTask(uKernelTaskDescriptor *pTaskDescriptor,
                       uint32_t taskInterval,
                       uKernelTaskStatus tStatus)
{
    if ((_initialized == false) || (_numberTasks == MAX_TASKS_NUMBER)
            || (pTaskDescriptor == NULL))
    {
        return false;
    }

    if (tStatus > uKernel_ONETIME_IMMEDIATESTART)
    {
        return false;
    }

    pTaskDescriptor->userTasksInterval = taskInterval;
    pTaskDescriptor->taskStatus = tStatus;

    if (tStatus == uKernel_SCHEDULED || tStatus == uKernel_ONETIME)
    {
        pTaskDescriptor->plannedTask = _counterMs + taskInterval;
    }
    else
    {
        pTaskDescriptor->plannedTask = 0;
    }

    return true;
}

uKernelTaskStatus uKernelGetTaskStatus(uKernelTaskDescriptor *pTaskDescriptor)
{
    if ((_initialized == false) || (_numberTasks == MAX_TASKS_NUMBER)
            || (pTaskDescriptor == NULL))
    {
        return uKernel_ERROR;
    }

    return pTaskDescriptor->taskStatus;
}

/**
 * Scheduling. This runs the kernel itself.
 */
void uKernelScheduler(void)
{
    while (1)
    {
        if (pTaskSchedule != NULL && _numberTasks != 0)
        {
            //the task is running
            if (pTaskSchedule->taskStatus > uKernel_PAUSED)
            {
                //this trick overrun the overflow of _counterMs
                if ((int32_t) (_counterMs - pTaskSchedule->plannedTask) >= 0)
                {
                    if (pTaskSchedule->taskStatus & uKernel_ONETIME)
                    {
                        pTaskSchedule->taskPointer(); //call the task
                        pTaskSchedule->taskStatus = uKernel_PAUSED; //pause the task
                    }
                    else
                    {
                        //let's schedule next start
                        pTaskSchedule->plannedTask =
                                _counterMs + pTaskSchedule->userTasksInterval;

                        pTaskSchedule->taskPointer(); //call the task
                    }
                }
            }
            // If a task has called the function DeleteAllTask() and if no
            // task are added, the pointer is null
            if (pTaskSchedule != NULL)
            {
                // Set the scheduler pointer on the next task
                pTaskSchedule = pTaskSchedule->pTaskNext;
            }
        }

        ClrWdt();
    }
}

void uKernelDelayMiliseconds(uint16_t delay)
{
    uint32_t newTime = _counterMs + delay;
    while (_counterMs < newTime);
}

uint8_t uKernelSetTask(uKernelTaskDescriptor *pTaskDescriptor,
                       uint32_t taskInterval,
                       uKernelTaskStatus tStatus)
{
    if ((_initialized == false) || (_numberTasks == MAX_TASKS_NUMBER)
            || (pTaskDescriptor == NULL))
    {
        return false;
    }

    pTaskDescriptor->taskStatus = tStatus;

    if (tStatus == uKernel_SCHEDULED)
    {
        if (taskInterval == NULL)
        {
            pTaskDescriptor->plannedTask =
                    _counterMs + pTaskDescriptor->userTasksInterval;
        }
        else
        {
            pTaskDescriptor->plannedTask =
                    _counterMs + taskInterval;
        }
    }

    return true;
}
