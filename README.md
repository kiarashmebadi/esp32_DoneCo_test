# ESP32 FreeRTOS Task Implementation

This repository contains the implementation of a task outlined in the file **FreeRTOS on ESP32 Coding Challenge Problem.pdf**. The main code is provided in the `main.c` file, which includes the coding and execution of the task requirements.

## Details

- The code has been tested on an ESP32 development board and should also work without issues on an ESP32-S3 board.
- The implementation is based on my understanding of the task's requirements. Some misunderstandings may have resulted in implementation errors.

### Example of Ambiguity in the Task

In one section, the task states:
![image](https://github.com/user-attachments/assets/20922366-caef-4e6f-a516-5a44f1d1d375)

> "If the number is prime, then a task on Core 0 should output the string 'Prime'."

However, it does not specify whether "number" refers to `i` or `n`. Additionally, the example provided (`i = n`) led to confusion about whether `n` is a fixed number equal to 6 or if `i = n`. Therefore, I assumed that `i = n` and that the countdown should start from the input number.

If my interpretation is incorrect, the program can be modified accordingly.

---
This code was written with the assistance of AI, ChatGPT :)
best regards , Kiarash Mebadi.
