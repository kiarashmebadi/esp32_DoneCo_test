// This code is written by Kiarash Mebadi for the testing task of the company Done.
// Created with the assistance of ChatGPT :)
// Version: 0.3
// Utilizes FreeRTOS on ESP32 with proper synchronization and timer handling.

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <esp_system.h>

// Queue to store received numbers
QueueHandle_t numberQueue;

// Current number being processed
int currentNumber = -1;

// Flag to indicate if a number is being processed
bool isProcessing = false;

// Synchronization flag for Bar and Foo
bool isFooTurn = true;

// Synchronization flag for PrimeTask with Bar and Foo
bool PrimeStartFlag = false;

// Timer trigger flag
bool timerTriggered = false;

// Timer handle
TimerHandle_t oneSecondTimer;

// Semaphore for synchronization
SemaphoreHandle_t syncSemaphore;

// Function to check if a number is prime
bool isPrime(int num) {
    if (num <= 1) return false;
    for (int i = 2; i * i <= num; ++i) {
        if (num % i == 0) return false;
    }
    return true;
}

// Timer callback to toggle between Foo and Bar
void oneSecondTimerCallback(TimerHandle_t xTimer) {
    if (currentNumber >= 0) {
        // Toggle between Foo and Bar
        timerTriggered = true;
    } else {
        // Stop processing and disable the timer
        isProcessing = false;
        timerTriggered = false;
        xTimerStop(oneSecondTimer, 0);
    }
}

// Task for Prime Checking (Core 0)
void primeTask(void *param) {
    while (true) {
        if (PrimeStartFlag) {
            xSemaphoreTake(syncSemaphore, portMAX_DELAY); // Lock
            if (isPrime(currentNumber+1)) {
                Serial.printf(" Prime\n");
            }
            else
            {
              Serial.printf("\n");
            }
            PrimeStartFlag = false;
            xSemaphoreGive(syncSemaphore); // Unlock
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // Short delay to avoid high CPU usage
    }
}


// Task for Foo (Core 0)
void fooTask(void *param) {
    while (true) {
        if (isProcessing && timerTriggered && isFooTurn && currentNumber >= 0) {
            xSemaphoreTake(syncSemaphore, portMAX_DELAY); // Lock
            Serial.printf("Foo %d", currentNumber);
            timerTriggered = false;
            currentNumber--; // Decrement the number
            isFooTurn = false;
            PrimeStartFlag = true;
            xSemaphoreGive(syncSemaphore); // Unlock
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // Short delay to yield CPU
    }
}

// Task for Bar (Core 1)
void barTask(void *param) {
    while (true) {
        if (isProcessing && timerTriggered && !isFooTurn && currentNumber >= 0) {
            xSemaphoreTake(syncSemaphore, portMAX_DELAY); // Lock
            Serial.printf("Bar %d", currentNumber);
            isFooTurn = true;
            timerTriggered = false;
            currentNumber--; // Decrement the number
            PrimeStartFlag = true;
            xSemaphoreGive(syncSemaphore); // Unlock
        }
        vTaskDelay(10 / portTICK_PERIOD_MS); // Short delay to yield CPU
    }
}

// Task for receiving numbers through serial input
void serialTask(void *param) {
    while (true) {
        if (Serial.available()) {
            String input = Serial.readStringUntil('\n');

            // Validate input
            if (input.toInt() == 0 && input != "0") {
                Serial.println("Invalid input. Please enter a valid positive number.");
                continue;
            }
            int newNumber = input.toInt();
            if (newNumber == 0) {
                Serial.println("Restarting ESP32...");
                esp_restart();
            } else if (uxQueueSpacesAvailable(numberQueue) > 0) {
                xQueueSend(numberQueue, &newNumber, portMAX_DELAY);
                Serial.printf("Received %d\n", newNumber);
            } else {
                Serial.println("Buffer is full");
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

// Task for processing numbers from the queue
void processTask(void *param) {
    while (true) {
        if (!isProcessing) {
            int nextNumber;
            if (xQueueReceive(numberQueue, &nextNumber, portMAX_DELAY)) {
                isProcessing = true;
                currentNumber = nextNumber;
                isFooTurn = false; // Start with bar
                if (currentNumber % 2 == 0)
                {
                    isFooTurn = true; // Start with Foo
                }
                xTimerStart(oneSecondTimer, 0); // Start the timer
            }
        }
        vTaskDelay(50 / portTICK_PERIOD_MS);
    }
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    Serial.println("Start... Version:0.3");

    // Create the synchronization semaphore
    syncSemaphore = xSemaphoreCreateMutex();

    // Create a queue to store up to 8 numbers
    numberQueue = xQueueCreate(8, sizeof(int));

    // Create the FreeRTOS timer (1-second period)
    oneSecondTimer = xTimerCreate(
        "OneSecondTimer",                  // Timer name
        pdMS_TO_TICKS(1000),           // Timer period (1 second)
        pdTRUE,                        // Auto-reload timer
        (void *)0,                     // Timer ID
        oneSecondTimerCallback            // Callback function
    );

    // Create tasks for Foo and Bar
    xTaskCreatePinnedToCore(fooTask, "FooTask", 2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(barTask, "BarTask", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(primeTask, "PrimeTask", 2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(serialTask, "SerialTask", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(processTask, "ProcessTask", 2048, NULL, 1, NULL, 0);
}

void loop() {
    // FreeRTOS handles all tasks; no code needed here
}
