#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_INVESTMENTS 1000
#define BUFFER_SIZE 256

double calculate_sharpe(int read_fd, int write_fd) {
    double arr[3];
    read(read_fd, arr, sizeof(arr));
    double sharpe = (arr[0] - arr[2]) / arr[1];
    write(write_fd, &sharpe, sizeof(sharpe));
    return sharpe;
}

int main() {
    char line[BUFFER_SIZE];
    double sharpes[MAX_INVESTMENTS]; 
    int investment_count = 0;

    while (fgets(line, sizeof(line), stdin)) {
        if (strncmp(line, "finish", 6) == 0) {
            break;
        }

        int pipe1[2], pipe2[2];
        if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
            return 0;
        }

        pid_t pid = fork();
        if (pid < 0) {
            return 0;
        }

        if (pid == 0) {
            close(pipe1[1]);
            close(pipe2[0]);

            double sharpe = calculate_sharpe(pipe1[0], pipe2[1]);

            close(pipe1[0]);
            close(pipe2[1]);
            exit(0);
        } else {
            close(pipe1[0]);
            close(pipe2[1]);

            double Ri, Rf, Ai;
            if (sscanf(line, "%lf %lf %lf", &Ri, &Ai, &Rf) == 3) {
                double arr[3] = {Ri, Ai, Rf};
                write(pipe1[1], arr, sizeof(arr));
                close(pipe1[1]);
            }

            wait(NULL);

            double sharpeFromChild;
            read(pipe2[0], &sharpeFromChild, sizeof(sharpeFromChild));
            close(pipe2[0]);

            sharpes[investment_count++] = sharpeFromChild;
        }
    }

    int best_investment = 0;
    double max_sharpe = sharpes[0];
    for (int i = 0; i < investment_count; i++) {
        printf("%.2f\n", sharpes[i]);
        if (sharpes[i] > max_sharpe) {
            max_sharpe = sharpes[i];
            best_investment = i + 1;
        }
    }
    printf("Selected Investment: %d\n", best_investment);
    return 0;
}
