#uKernel (Micro Kernel)

## Introduction
This is a scheduler for microcontrollers. This is not in any kind an RTOS, or anything like it. 
Just create a function, create a descriptor for that function and add it to the scheduler with a period and let the scheduler do the rest.
I am trying to keep it really simple due to the memory limitations of the microcontrollers. The maximum number of task is 255 but I am sure that the memory will go out first.

## Versions
V1.0 - Initial version - 03-05-2013

## Credits
This code was written by me, but I join two schedulers that I found, the tiny-kernel-microcontroller by Sébastien Pallatin ([here](https://code.google.com/p/tiny-kernel-microcontroller/)) and the leOS by Leonardo Miliani ([here](https://github.com/leomil72/leOS)).

Thank you both for the great job, all the hard work already done by you (both).

## Author
Written by Luis Maduro

