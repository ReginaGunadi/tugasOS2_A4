#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Struktur data untuk proses (PCB)
struct Process {
    char pid[30];
    int arrival_time;
    int burst_time;
    int remaining_time;
};

// Struktur untuk menyimpan data balok Gantt Chart
struct GanttBlock {
    char pid[30];
    int start;
    int end;
};

struct PreemptionLog {
    int time;
    char pid[30];
    int remaining_time;
};

struct QueueNode {
    struct QueueNode *next;
    int value;
};

struct Queue {
    struct QueueNode *front;
    struct QueueNode *rear;
};

void queue_enqueue(struct Queue* q, int value) {
    struct QueueNode* node = (struct QueueNode *) malloc(sizeof(struct QueueNode));
    if (!node) return;

    node->value = value;
    node->next = NULL;

    if (q->rear) {
        q->rear->next = node;
    } else {
        q->front = node;
    }

    q->rear = node;
}

int queue_dequeue(struct Queue *q) {
    if (q->front == NULL) return -1;

    struct QueueNode* node = q->front;
    int value = node->value;

    q->front = node->next;
    if (q->front == NULL) {
        q->rear = NULL;
    }

    free(node);
    return value;
}

void queue_free(struct Queue *q) {
    struct QueueNode* node = q->front;

    while (node) {
        struct QueueNode* next = node->next;
        free(node);
        node = next;
    }

    q->front = NULL;
    q->rear = NULL;
}

// Main code
int main() {
    int n, time_quantum;

    // Input jumlah proses dan time quantum
    printf("Jumlah proses: ");
    scanf("%d", &n);
    printf("Time Quantum (> 0): ");
    scanf("%d", &time_quantum);

    // Alokasi dinamis untuk array proses berdasarkan input n
    struct Process *p = (struct Process *)malloc(n * sizeof(struct Process));
    if (p == NULL) {
        printf("Alokasi memori gagal!\n");
        return 1;
    }

    for (int i = 0; i < n; i++) {
        snprintf(p[i].pid, sizeof(p[i].pid), "P%d", i + 1);
        printf("%s - masukkan Arrival Time dan Burst Time: ", p[i].pid);
        scanf("%d %d", &p[i].arrival_time, &p[i].burst_time);
        p[i].remaining_time = p[i].burst_time; // Set remaining time awal sama dengan burst time
    }
    printf("\n");

    // Bagian 1: Informasi Input Process
    printf("=======================================================================\n");
    printf("PROCESS INPUT\n");
    printf("=======================================================================\n");
    printf("Time Quantum : %d\n", time_quantum);
    printf("%-10s %-15s %-12s\n", "PID", "Arrival Time", "Burst Time");
    printf("-----------------------------------------------------------------------\n");
    
    for (int i = 0; i < n; i++) {
        printf("%-10s %-15d %-12d\n", p[i].pid, p[i].arrival_time, p[i].burst_time);
    }
    printf("=======================================================================\n\n");

    // Simulasi Round Robin dengan Gantt Chart dan Preemption Logs yang dinamis (bisa resize pakai realloc)
    int gantt_capacity = 100;
    int gantt_count = 0;
    struct GanttBlock *gantt = (struct GanttBlock *)malloc(gantt_capacity * sizeof(struct GanttBlock));

    int preemption_capacity = 100;
    int preemption_count = 0;
    struct PreemptionLog *preemption_logs = (struct PreemptionLog *)malloc(preemption_capacity * sizeof(struct PreemptionLog));

    int current_time = 0;
    int completed = 0;
    
    // Alokasi dinamis untuk Queue dan In-Queue tracker
    int *queue = (int *)malloc(n * sizeof(int));
    int front = 0, rear = 0;
    int *in_queue = (int *)calloc(n, sizeof(int)); // calloc otomatis set nilai awal ke 0

    // Bagian 6: Menghitung context switch
    int context_switch_counter = 0; 
    int last_process_idx = -1; 

    for (int i = 0; i < n; i++) {
        if (p[i].arrival_time <= current_time) {
            queue[rear++] = i;
            in_queue[i] = 1;
        }
    }

    if (rear == 0) {
        current_time = p[0].arrival_time;
        queue[rear++] = 0;
        in_queue[0] = 1;
    }

    // Looping utama scheduling sampai semua proses selesai
    while (completed < n) {
        if (front == rear) {
            int next_arrival = -1;
            for (int i = 0; i < n; i++) {
                if (p[i].remaining_time > 0 && p[i].arrival_time > current_time) {
                    if (next_arrival == -1 || p[i].arrival_time < next_arrival) {
                        next_arrival = p[i].arrival_time;
                    }
                }
            }

            if (next_arrival != -1) {
                // Resize gantt array jika kapasitas penuh
                if (gantt_count >= gantt_capacity) {
                    gantt_capacity *= 2;
                    gantt = (struct GanttBlock *)realloc(gantt, gantt_capacity * sizeof(struct GanttBlock));
                }

                strcpy(gantt[gantt_count].pid, "Idle");
                gantt[gantt_count].start = current_time;
                gantt[gantt_count].end = next_arrival;
                gantt_count++;

                current_time = next_arrival;

                // Masukkan proses yang datang pada waktu tersebut ke queue
                for (int i = 0; i < n; i++) {
                    if (p[i].arrival_time <= current_time && p[i].remaining_time > 0 && !in_queue[i]) {
                        queue[rear++] = i;
                        in_queue[i] = 1;
                    }
                }
            } else {
                current_time++;
            }
            continue;
        }

        // Ambil proses dari depan antrean
        int idx = queue[front++];

        // Bagian 6: Perbarui jumlah context switch bila process idx != process idx sebelumnya
        if (last_process_idx != -1 && idx != last_process_idx){
            context_switch_counter++; 
        }
        last_process_idx = idx;

        int exec_time;
        if (p[idx].remaining_time > time_quantum) {
            exec_time = time_quantum;
        } else {
            exec_time = p[idx].remaining_time;
        }

        // Resize gantt array jika kapasitas penuh
        if (gantt_count >= gantt_capacity) {
            gantt_capacity *= 2;
            gantt = (struct GanttBlock *)realloc(gantt, gantt_capacity * sizeof(struct GanttBlock));
        }

        // Catat ke Gantt Chart
        strcpy(gantt[gantt_count].pid, p[idx].pid);
        gantt[gantt_count].start = current_time;
        
        current_time += exec_time;
        p[idx].remaining_time -= exec_time;
        
        gantt[gantt_count].end = current_time;
        gantt_count++;

        // Cek apakah quantum habis tapi proses belum selesai (preemption)
        if (exec_time == time_quantum && p[idx].remaining_time > 0) {
            if (preemption_count >= preemption_capacity) {
                preemption_capacity *= 2;
                preemption_logs = (struct PreemptionLog *)realloc(preemption_logs, preemption_capacity * sizeof(struct PreemptionLog));
            }
            preemption_logs[preemption_count].time = current_time;
            strcpy(preemption_logs[preemption_count].pid, p[idx].pid);
            preemption_logs[preemption_count].remaining_time = p[idx].remaining_time;
            preemption_count++;
        }

        // Cek proses lain yang baru datang
        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= current_time && p[i].remaining_time > 0 && !in_queue[i]) {
                queue[rear++] = i;
                in_queue[i] = 1;
            }
        }

        if (p[idx].remaining_time > 0) {
            queue[rear++] = idx;
        } else {
            completed++;
        }
    }

    // Bagian 2: Gantt Chart / Timeline
    printf("=======================================================================\n");
    printf("CPU EXECUTION TIMELINE (GANTT CHART)\n");
    printf("=======================================================================\n");
    
    // Print kotak proses
    printf("|");
    for (int i = 0; i < gantt_count; i++) {
        printf(" %s |", gantt[i].pid);
    }
    printf("\n");
    
    // Print angka timeline di bawahnya
    printf("%d", gantt[0].start);
    for (int i = 0; i < gantt_count; i++) {
        int space_len = strlen(gantt[i].pid) + 3;
        printf("%*d", space_len, gantt[i].end);
    }
    printf("\n");

    // Bagian Quantum and Preemption Information
    printf("=======================================================================\n");
    printf("QUANTUM AND PREEMPTION INFORMATION\n");
    printf("=======================================================================\n");
    for (int i = 0; i < preemption_count; i++) {
        printf("t=%d : quantum %s habis (sisa BT=%d) -> READY\n",
        preemption_logs[i].time,
        preemption_logs[i].pid,
        preemption_logs[i].remaining_time);
    }
    printf("Total Preemption : %d\n", preemption_count);

    // Bagian 3: Scheduling Table
    // CT dan first start dicari dari array gantt[]
    printf("\n=======================================================================\n");
    printf("SCHEDULING TABLE\n");
    printf("=======================================================================\n");
    printf("%-8s %-6s %-6s %-6s %-6s %-6s %-6s\n", "PID","AT","BT","CT","TAT","WT","RT");
    printf("-----------------------------------------------------------------------\n");

    double sum_wt = 0, sum_tat = 0, sum_rt = 0;
    for (int i = 0; i < n; i++) {
        int first_start = -1, ct = 0;
        for (int g = 0; g < gantt_count; g++) {
            if (strcmp(gantt[g].pid, p[i].pid) == 0) {
                if (first_start == -1) first_start = gantt[g].start; // blok pertama
                ct = gantt[g].end;                                   // blok terakhir (urut waktu)
            }
        }
        int tat = ct - p[i].arrival_time;        // TAT = CT - AT
        int wt  = tat - p[i].burst_time;         // WT  = TAT - BT
        int rt  = first_start - p[i].arrival_time; // RT = start pertama - AT
        sum_wt += wt; sum_tat += tat; sum_rt += rt;
        printf("%-8s %-6d %-6d %-6d %-6d %-6d %-6d\n", p[i].pid,
            p[i].arrival_time, p[i].burst_time, ct, tat, wt, rt);
    }
    printf("=======================================================================\n\n");

    // Bagian 4: Nilai rata-rata
    printf("=======================================================================\n");
    printf("SCHEDULING PERFORMANCE\n");
    printf("=======================================================================\n");
    printf("Average Waiting Time    : %.2f\n", sum_wt / n);
    printf("Average Turnaround Time : %.2f\n", sum_tat / n);
    printf("Average Response Time   : %.2f\n", sum_rt / n);
    printf("=======================================================================\n");

    // Bagian 5: Utilisasi CPU dan Throughput
    // Rumus: CPU Utilization = (CPU Busy Time/Total Simulation Time) * 100%
    //        Throughput = Jumlah Process Selesai/Total Simulation Time
    double total_simulation_time = current_time;   // waktu selesai proses 
    double total_busy_time = 0;                    // burst time (waktu kerja CPU)

    for (int i = 0; i < n; i++){
        total_busy_time += p[i].burst_time;
    }

    double cpu_util = (total_busy_time/total_simulation_time) * 100;
    double throughput = n/total_simulation_time; 

    printf("CPU UTILIZATION AND THROUGHPUT\n");
    printf("=======================================================================\n");
    printf("CPU Utilization      : %.2f%%\n", cpu_util);
    printf("Throughput           : %.2f process/time unit\n", throughput);

    // Bagian 6: Context Switch Information
    printf("=======================================================================\n");
    printf("CONTEXT SWITCH INFORMATION\n");
    printf("=======================================================================\n");
    printf("Total Context Switch : %d\n", context_switch_counter);

    // Bagian 7: Process State Transitions
    printf("=======================================================================\n");
    printf("PROCESS STATE TRANSITIONS\n");
    printf("=======================================================================\n");

    for (int i = 0; i < n; i++) {
        printf("P%d: NEW", i+1);  // Start dari process
        int end = p[i].arrival_time;  // end disimpan untuk dicetak READY ataupun TERMINATED
                                      // READY yang pertama adalah saat proses itu datang pertama kali

        for (int g = 0; g < gantt_count; g++) {
            if (strcmp(gantt[g].pid, p[i].pid) == 0) {
                printf(" -> READY(t=%d)", end);  // cetak end yang ternyata bukan akhir eksekusi
                printf(" -> RUNNING(t=%d)", gantt[g].start);

                end = gantt[g].end;  // ganti end menjadi akhiran gantt
                                     // saat loop selesai (gantt chart habis)
                                     // end akan dicetak oleh TERMINATED
            }
        }

        printf(" -> TERMINATED(t=%d)\n", end);
    }

    // Bebaskan memori yang sudah di-malloc untuk menghindari memory leaks
    free(p);
    free(gantt);
    free(preemption_logs);
    free(queue);
    free(in_queue);

    return 0;
}
