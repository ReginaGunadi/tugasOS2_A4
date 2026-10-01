#include <stdio.h>
#include <string.h>

// Struktur data untuk proses (PCB)
struct Process {
    char pid[10];
    int arrival_time;
    int burst_time;
    int remaining_time;
};

// Struktur untuk menyimpan data balok Gantt Chart
struct GanttBlock {
    char pid[10];
    int start;
    int end;
};

// Main code
int main() {
    // // Jumlah proses dan Time Quantum untuk Round Robin
    // int n = 4;
    // int time_quantum = 2;

    // // Inisialisasi data proses [process num, arrival time, burst time, remaining time]
    // struct Process p[4] = {
    //     {"P1", 0, 8, 8},
    //     {"P2", 1, 4, 4},
    //     {"P3", 2, 2, 2},
    //     {"P4", 3, 5, 5}
    // };

    int n, time_quantum;

    // Input jumlah proses dan time quantum
    printf("Jumlah proses: ");
    scanf("%d", &n);
    printf("Time Quantum (> 0): ");
    scanf("%d", &time_quantum);

    // Inisialisasi array proses berdasarkan input user
    struct Process p[100];
    for (int i = 0; i < n; i++) {
        sprintf(p[i].pid, "P%d", i + 1);
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

    // Simulasi Round Robin
    struct GanttBlock gantt[100];
    int gantt_count = 0;

    int current_time = 0;
    int completed = 0;
    
    // Queue
    int queue[100];
    int front = 0, rear = 0;
    int in_queue[10] = {0};

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
            current_time++;
            for (int i = 0; i < n; i++) {
                if (p[i].arrival_time <= current_time && p[i].remaining_time > 0 && !in_queue[i]) {
                    queue[rear++] = i;
                    in_queue[i] = 1;
                }
            }
            continue;
        }

        // Ambil proses dari depan antrean
        int idx = queue[front++];
    
        int exec_time;
        if (p[idx].remaining_time > time_quantum) {
            exec_time = time_quantum;
        } else {
            exec_time = p[idx].remaining_time;
        }

        // Catat ke Gantt Chart
        strcpy(gantt[gantt_count].pid, p[idx].pid);
        gantt[gantt_count].start = current_time;
        
        current_time += exec_time;
        p[idx].remaining_time -= exec_time;
        
        gantt[gantt_count].end = current_time;
        gantt_count++;

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
    printf("=======================================================================\n");

    return 0;
}