 #include <stdio.h>

#define MAX_PROCESSES 10

typedef struct {
    int pid;
    int arrival;
    int burst;
    int start;
    int completion;
    int waiting;
    int turnaround;
} Process;

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

        p[i].start = 0;
        p[i].completion = 0;
        p[i].waiting = 0;
        p[i].turnaround = 0;
    }
}

void printResults(Process p[], int n, const char *title,
                  double busyTime, double idleTime) {

    int i;
    double avgWait = 0.0, avgTat = 0.0;

    /* simple power model */
    double busyPower = 2.0;   /* units per time */
    double idlePower = 0.2;
    double totalEnergy = busyTime * busyPower + idleTime * idlePower;

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

void fcfs(Process p[], int n) {
    int i, j;
    int time = 0;
    double busyTime = 0.0;
    double idleTime = 0.0;

    /* sort by arrival time (simple bubble sort) */
    for (i = 0; i < n - 1; i++) {
        for (j = 0; j < n - 1 - i; j++) {
            if (p[j].arrival > p[j + 1].arrival) {
                Process temp = p[j];
                p[j] = p[j + 1];
                p[j + 1] = temp;
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

    printResults(p, n, "FCFS Scheduling", busyTime, idleTime);
}

int main() {
    Process procs[MAX_PROCESSES];
    int n;

    inputProcesses(procs, &n);
    if (n <= 0 || n > MAX_PROCESSES) {
        return 0;
    }

    fcfs(procs, n);

    return 0;
}
