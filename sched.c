/* Final code submission - CSE316 Project */

#include <stdio.h>

#define MAX_PROCESSES 10

typedef struct {
    int pid;
    int arrival;
    int burst;
    int remaining;
    int start;
    int completion;
    int waiting;
    int turnaround;
    int started;
} Process;

/* copy original processes into a working array */
void copyProcesses(Process dest[], Process src[], int n) {
    int i;
    for (i = 0; i < n; i++) {
        dest[i] = src[i];
        dest[i].remaining = src[i].burst;
        dest[i].start = -1;
        dest[i].completion = 0;
        dest[i].waiting = 0;
        dest[i].turnaround = 0;
        dest[i].started = 0;
    }
}

/* common result printer: we pass totalEnergy directly */
void printResults(Process p[], int n, const char *title, double totalEnergy) {
    int i;
    double avgWait = 0.0, avgTat = 0.0;

    printf("\n==============================\n");
    printf("%s\n", title);
    printf("==============================\n");
    printf("PID\tAT\tBT\tST\tCT\tWT\tTAT\n");

    for (i = 0; i < n; i++) {
        p[i].turnaround = p[i].completion - p[i].arrival;
        p[i].waiting = p[i].turnaround - p[i].burst;

        avgWait += p[i].waiting;
        avgTat  += p[i].turnaround;

        printf("P%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
               p[i].pid,
               p[i].arrival,
               p[i].burst,
               p[i].start,
               p[i].completion,
               p[i].waiting,
               p[i].turnaround);
    }

    avgWait /= n;
    avgTat  /= n;

    printf("\nAverage Waiting Time    : %.2f", avgWait);
    printf("\nAverage Turnaround Time : %.2f", avgTat);
    printf("\nEstimated Energy Used   : %.2f units\n", totalEnergy);
}

/* user input */
void inputProcesses(Process p[], int *n) {
    int i;
    printf("Enter number of processes (max %d): ", MAX_PROCESSES);
    scanf("%d", n);

    if (*n <= 0 || *n > MAX_PROCESSES) {
        printf("Invalid number of processes.\n");
        return;
    }

    for (i = 0; i < *n; i++) {
        p[i].pid = i + 1;
        printf("\nProcess P%d\n", p[i].pid);
        printf("Arrival time : ");
        scanf("%d", &p[i].arrival);
        printf("Burst time   : ");
        scanf("%d", &p[i].burst);
    }
}

/* FCFS */
void fcfs(Process src[], int n) {
    Process p[MAX_PROCESSES];
    copyProcesses(p, src, n);

    int i, j;
    int time = 0;
    double busyTime = 0.0;
    double idleTime = 0.0;

    /* sort by arrival time */
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - 1 - i; j++) {
            if (p[j].arrival > p[j + 1].arrival) {
                Process t = p[j];
                p[j] = p[j + 1];
                p[j + 1] = t;
            }
        }
    }

    for (i = 0; i < n; i++) {
        if (time < p[i].arrival) {
            idleTime += (p[i].arrival - time);
            time = p[i].arrival;
        }
        p[i].start = time;
        time += p[i].burst;
        p[i].completion = time;
        busyTime += p[i].burst;
    }

    /* simple model: FCFS runs at medium power */
    double busyPower = 2.0;
    double idlePower = 0.2;
    double totalEnergy = busyTime * busyPower + idleTime * idlePower;

    printResults(p, n, "FCFS Scheduling", totalEnergy);
}

/* standard Round Robin (fixed quantum, always high power) */
void roundRobin(Process src[], int n, int quantum) {
    Process p[MAX_PROCESSES];
    copyProcesses(p, src, n);

    int time = 0;
    int completed = 0;
    double busyTime = 0.0;
    double idleTime = 0.0;

    while (completed < n) {
        int executed = 0;
        int i;

        for (i = 0; i < n; i++) {
            if (p[i].remaining > 0 && p[i].arrival <= time) {
                executed = 1;

                if (!p[i].started) {
                    p[i].start = time;
                    p[i].started = 1;
                }

                int exec = (p[i].remaining < quantum) ? p[i].remaining : quantum;
                p[i].remaining -= exec;
                time += exec;
                busyTime += exec;

                if (p[i].remaining == 0) {
                    p[i].completion = time;
                    completed++;
                }
            }
        }

        if (!executed) {
            time++;
            idleTime++;
        }
    }

    /* RR at high frequency → higher power */
    double busyPower = 3.0;
    double idlePower = 0.2;
    double totalEnergy = busyTime * busyPower + idleTime * idlePower;

    printResults(p, n, "Round Robin Scheduling", totalEnergy);
}

/* Energy-Aware Round Robin */
void energyAwareRR(Process src[], int n, int quantumHigh, int quantumLow) {
    Process p[MAX_PROCESSES];
    copyProcesses(p, src, n);

    int time = 0;
    int completed = 0;
    double idleTime = 0.0;
    double totalEnergy = 0.0;
    int index = 0;

    while (completed < n) {
        int readyCount = 0;
        int i;

        /* count ready processes */
        for (i = 0; i < n; i++) {
            if (p[i].remaining > 0 && p[i].arrival <= time) {
                readyCount++;
            }
        }

        if (readyCount == 0) {
            /* CPU idle */
            time++;
            idleTime++;
            totalEnergy += 0.2; /* idle power */
            continue;
        }

        int quantum;
        double power;

        /* high load vs low load */
        if (readyCount > 2) {     /* many processes waiting */
            quantum = quantumHigh; /* small quantum */
            power = 3.0;           /* high frequency, high power */
        } else {                  /* few processes waiting */
            quantum = quantumLow;  /* larger quantum */
            power = 1.0;          /* low frequency, low power */
        }

        int executed = 0;

        for (i = 0; i < n; i++) {
            int idx = (index + i) % n;
            if (p[idx].remaining > 0 && p[idx].arrival <= time) {
                executed = 1;

                if (!p[idx].started) {
                    p[idx].start = time;
                    p[idx].started = 1;
                }

                int exec = (p[idx].remaining < quantum) ? p[idx].remaining : quantum;
                p[idx].remaining -= exec;
                time += exec;
                totalEnergy += power * exec;

                if (p[idx].remaining == 0) {
                    p[idx].completion = time;
                    completed++;
                }

                index = (idx + 1) % n;
                break;
            }
        }

        if (!executed) {
            time++;
            idleTime++;
            totalEnergy += 0.2;
        }
    }

    printResults(p, n, "Energy-Aware Round Robin Scheduling", totalEnergy);
}

int main() {
    Process procs[MAX_PROCESSES];
    int n;
    int choice;
    int quantum, qHigh, qLow;

    inputProcesses(procs, &n);
    if (n <= 0 || n > MAX_PROCESSES) {
        return 0;
    }

    do {
        printf("\n==============================\n");
        printf("CPU Scheduling Menu\n");
        printf("==============================\n");
        printf("1. FCFS\n");
        printf("2. Round Robin\n");
        printf("3. Energy-Aware Round Robin\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            fcfs(procs, n);
        } else if (choice == 2) {
            printf("Enter time quantum for Round Robin: ");
            scanf("%d", &quantum);
            roundRobin(procs, n, quantum);
        } else if (choice == 3) {
            printf("Enter SMALL quantum (high load, e.g., 2): ");
            scanf("%d", &qHigh);
            printf("Enter LARGE quantum (low load, e.g., 4): ");
            scanf("%d", &qLow);
            energyAwareRR(procs, n, qHigh, qLow);
        } else if (choice != 0) {
            printf("Invalid choice.\n");
        }
    } while (choice != 0);

    return 0;
}
