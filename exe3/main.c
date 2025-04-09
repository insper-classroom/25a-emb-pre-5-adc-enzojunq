#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>

#include "pico/stdlib.h"
#include <stdio.h>

#include "data.h"

#define WINDOW_SIZE 5  // Tamanho da janela para a média móvel

QueueHandle_t xQueueData;

// não mexer! Alimenta a fila com os dados do sinal
void data_task(void *p) {
    vTaskDelay(pdMS_TO_TICKS(400));

    int data_len = sizeof(sine_wave_four_cycles) / sizeof(sine_wave_four_cycles[0]);
    for (int i = 0; i < data_len; i++) {
        xQueueSend(xQueueData, &sine_wave_four_cycles[i], 1000000);
    }

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void process_task(void *p) {
    int data = 0;
    int window[WINDOW_SIZE] = {0};  // Armazena as últimas WINDOW_SIZE amostras
    int index = 0;                  // Índice para posição na janela circular
    int count = 0;                  // Quantidade de amostras válidas (até WINDOW_SIZE)
    int sum = 0;                    // Soma das amostras armazenadas
    int average = 0;

    while (true) {
        if (xQueueReceive(xQueueData, &data, 100)) {
            // Atualiza a soma: remove o valor mais antigo e adiciona o novo
            if (count == WINDOW_SIZE) {
                sum = sum - window[index] + data;
            } else {
                sum = sum + data;
                count++;
            }
            
            // Armazena o novo valor na janela
            window[index] = data;
            
            // Calcula a média - sempre dividindo por WINDOW_SIZE (5)
            average = sum / WINDOW_SIZE;
            
            // Imprime o valor filtrado
            printf("%d\n", average);
            
            
            // Atualiza o índice para a próxima posição na janela circular
            index = (index + 1) % WINDOW_SIZE;
           
            // deixar esse delay!
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
}

int main() {
    stdio_init_all();

    xQueueData = xQueueCreate(64, sizeof(int));

    xTaskCreate(data_task, "Data task ", 4096, NULL, 1, NULL);
    xTaskCreate(process_task, "Process task", 4096, NULL, 1, NULL);

    vTaskStartScheduler();

    while (true)
        ;
}
