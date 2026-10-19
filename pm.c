#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char id[32];

    int arrival;
    int priority;
    int service;
    int remaining;

    char category[64];

    int start_time;
    int completion_time;

    int started;
    int completed;
} Patient;

int compare_arrival(const void *a, const void *b){
    Patient *p1 = (Patient *)a;
    Patient *p2 = (Patient *)b;

    //first sorting by arrival time
    if(p1->arrival != p2->arrival){
        return p1->arrival - p2->arrival;
    }

    //then sorting by process ID lexicographically
    return strcmp(p1->id, p2->id);
}

int fcfs(Patient* p, int n){
    int n_done = 0;
    int time = 0;

    //implementing the two doctors
    int doc[2] = {-1,-1};
    int idle = -1;
    int i;
    int i0;
    int i1;

    while(n_done<n){

        //printf("%d\n",n_done);

        if(doc[0] == -1){
            idle = 0;
        }
        else if (doc[1] == -1){
            idle = 1;
        }

        //if doctor is idle, make it busy
        if(idle+1){
            for(i = 0; i<n; i++){

                if( (p[i].arrival<=time) && (p[i].started == -1) ){

                    doc[idle] = i;
                    p[i].started = 1;
                    p[i].start_time = time;


                    //check if still idle doctor remaining
                    if(doc[0] == -1){
                        idle = 0;
                    }
                    else if (doc[1] == -1){
                        idle = 1;
                    }
                    else{
                        idle = -1;
                        break;
                    }

                }
            }
        }


        //decrease remaining time of currently processes. if process completed, make doctor idle.
        if(doc[0] != -1){
            i0 = doc[0];
            p[i0].remaining--;
            

            if(p[i0].remaining == 0){
                p[i0].completed = 1;
                p[i0].completion_time = time+1;
                n_done++;
                doc[0] = -1;
            }
        }
        if(doc[1] != -1){
            i1 = doc[1];
            p[i1].remaining--;

            if(p[i1].remaining == 0){
                p[i1].completed = 1;
                p[i1].completion_time = time+1;
                n_done++;
                doc[1] = -1;
            }
        }

        time++;
    }

    /*
    printf("fcfs:\n");
    for(i = 0; i < n; i++){
        printf("%s | Arrival: %d | Service: %d | Start_time: %d | Comp_time: %d | Completed: %d\n",
               p[i].id, p[i].arrival, p[i].service, p[i].start_time, p[i].completion_time, p[i].completed);
    }
    */

    return 1;
}









////////////////////////////////////////////////////////////////////////////////////
//PS
///////////////////////////////////////////////////////////////////////////////////

int priority_nonpreemptive(Patient* p, int n) {
    int n_done = 0;
    int time = 0;

    int doc[2] = {-1, -1};


    int i, j, k, d;

    //ready queue
    int ready[n];
    int ready_count = 0;

    //which processes have already been added to the ready queue
    int added_to_ready[n];
    memset(added_to_ready, 0, n * sizeof(int));

    while(n_done < n){



        //moving newly arrived patients into ready
        for (i = 0; i < n; i++){
            if ( (p[i].arrival <= time) && (added_to_ready[i] == 0) && (p[i].started == -1) ){
                ready[ready_count++] = i;
                added_to_ready[i] = 1;
            }
        }




        //sorting the ready queue based on PS rules
        for (i = 0; i < ready_count - 1; i++) {

            for (j = i + 1; j < ready_count; j++) {
                int p1 = ready[i];
                int p2 = ready[j];

                int should_swap = 0;

                //Higher priority
                if (p[p1].priority > p[p2].priority) {
                    should_swap = 1;
                } 

                //Earlier arrival time
                else if (p[p1].priority == p[p2].priority) {
                    if (p[p1].arrival > p[p2].arrival) {
                        should_swap = 1;
                    } 

                    //Lexicographically smaller id
                    else if (p[p1].arrival == p[p2].arrival) {
                        if (strcmp(p[p1].id, p[p2].id) > 0) {
                            should_swap = 1;
                        }
                    }
                }

                if (should_swap) {
                    int temp = ready[i];
                    ready[i] = ready[j];
                    ready[j] = temp;
                }
            }
        }




        //assigning idle doctors to patients
        for(d = 0; d < 2; d++){
            if ((doc[d] == -1) && (ready_count > 0)) {
                int next_id = ready[0];

                doc[d] = next_id;
                p[next_id].started = 1;
                p[next_id].start_time = time;

                //removing patient from ready queue. Shifting elements left
                for (int k = 0; k<ready_count - 1; k++) {
                    ready[k] = ready[k + 1];
                }
                ready_count--;
            }
        }




        //work on current patients
        for(d = 0; d < 2; d++) {
            
            if (doc[d] != -1) {
                int idx = doc[d];
                p[idx].remaining = p[idx].remaining - 1;

                if (p[idx].remaining == 0) {
                    p[idx].completed = 1;
                    p[idx].completion_time = time + 1;
                    n_done++;
                    doc[d] = -1;
                }
            }
        }


        time++;
    }

    /*
    printf("non-pre PS:\n");
    for(i = 0; i < n; i++){
        printf("%s | Arrival: %d | Service: %d | Priority: %d | Start_time: %d | Comp_time: %d\n",
               p[i].id, p[i].arrival, p[i].service, p[i].priority, p[i].start_time, p[i].completion_time);
    }
    */

    return 0;
}











///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//preemptive PS
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////



//compare two processes. returns negative if p1 has higher preference than p2
int compare_process_preference(Patient p1, Patient p2) {
    if(p1.priority != p2.priority){
        return p1.priority - p2.priority;
    }
    if(p1.arrival != p2.arrival){
        return p1.arrival - p2.arrival;
    }
    return strcmp(p1.id, p2.id);
}

//find the best ready patient waiting to be treated
int find_best_ready_patient(Patient* p, int n, int doc0, int doc1, int time){
    int best_idx = -1;
    int i;

    for(i = 0; i<n; i++){
        if( (p[i].arrival <= time) && (p[i].completed == -1) && (i != doc0) && (i != doc1) ){
            if(best_idx == -1){
                best_idx = i;
            }
            else{
                //checking if process i has higher preference than the current best one
                if(compare_process_preference(p[i], p[best_idx]) < 0){
                    best_idx = i;
                }
            }
        }
    }
    return best_idx;
}

//choosing which running process to preempt
int get_process_to_preempt(Patient* p, int doc0, int doc1) {

    if (doc0 == -1 && doc1 == -1){
        return -1;
    }
    if (doc0 != -1 && doc1 == -1){
        return doc0;
    }
    if (doc0 == -1 && doc1 != -1){
        return doc1;
    }

    //both the doctors are busy
    Patient p0= p[doc0];
    Patient p1 = p[doc1];



    if (p0.priority > p1.priority) {
        return doc0;
    }
    else if (p1.priority > p0.priority) {
        return doc1;
    }

    if (p0.arrival > p1.arrival) {
        return doc0;
    }
    else if (p1.arrival > p0.arrival) {
        return doc1;
    }

    if (strcmp(p0.id, p1.id) > 0) {
        return doc0;
    }
    else {
        return doc1;
    }

}


int priority_preemptive(Patient* p, int n) {
    int n_done = 0;
    int time = 0;
    int doc0= -1;
    int doc1= -1;

    while(n_done < n){

        //working on currently running patients
        if (doc0 != -1){
            p[doc0].remaining--;

            if(p[doc0].remaining == 0){
                p[doc0].completed = 1;
                p[doc0].completion_time = time;
                n_done++;
                doc0 = -1;
            }

        }

        if (doc1 != -1) {

            p[doc1].remaining--;

            if (p[doc1].remaining == 0) {
                p[doc1].completed = 1;
                p[doc1].completion_time = time;
                n_done++;
                doc1 = -1;
            }

        }

        if (n_done >= n){
            break;
        }

        //make idle doctors busy if any patient ready
        while(1){

            int best_ready = find_best_ready_patient(p, n, doc0, doc1, time);

            if (best_ready == -1){
                break;
            }


            if (doc0 == -1) {
                doc0 = best_ready;
                if (p[best_ready].started == -1){
                    p[best_ready].started = 1;
                    p[best_ready].start_time = time;
                }

            }
            else if (doc1 == -1){
                doc1 = best_ready;
                if (p[best_ready].started == -1){
                    p[best_ready].started = 1;
                    p[best_ready].start_time = time;
                }
            }
            else {
                break; // Both doctors are full
            }
        }

        //handling preemption
        while(1){
            int best_ready = find_best_ready_patient(p, n, doc0, doc1, time);

            if((best_ready == -1) || (doc0 == -1 && doc1 == -1)){
                break;
            }

            int to_preempt = get_process_to_preempt(p, doc0, doc1);
            
            if (to_preempt != -1) {
                if (p[best_ready].priority < p[to_preempt].priority) {
                    if (to_preempt == doc0){
                        doc0 = best_ready;
                    }
                    else{
                        doc1 = best_ready;
                    }

                    if(p[best_ready].started == -1){
                        p[best_ready].started = 1;
                        p[best_ready].start_time = time;
                    }

                }
                else {
                    break;
                }
            } else {
                break;
            }
        }

        time++;
    }

    /*

    printf("Preemptive PS\n");
    for (int i = 0; i < n; i++) {
        printf("%s | Arrival: %d | Service: %d | Priority: %d | Start: %d | Completion: %d\n",
               p[i].id, p[i].arrival, p[i].service, p[i].priority, p[i].start_time, p[i].completion_time);
    }
    */

    return 0;
}








//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//results
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int compare_by_id(const void *a, const void *b) {
    Patient *p1 = (Patient *)a;
    Patient *p2 = (Patient *)b;
    return strcmp(p1->id, p2->id);
}
void print_standardized_output(Patient* p, int n) {

    qsort(p, n, sizeof(Patient), compare_by_id);

    double total_wt = 0;
    double total_tat = 0;
    double total_rt = 0;
    int total_service_time = 0;
    int makespan = 0;

    printf("PROCESS_ID,ARRIVAL,PRIORITY,SERVICE,START,COMPLETION,WAITING,TURNAROUND,RESPONSE\n");

    for (int i = 0; i<n; i++) {
        int start = p[i].start_time;
        int completion= p[i].completion_time;
        int tat = completion - p[i].arrival;
        int wt = tat - p[i].service;
        int rt = start - p[i].arrival;

        total_tat += tat;
        total_wt += wt;
        total_rt += rt;
        total_service_time += p[i].service;

        if(completion > makespan){
            makespan = completion;
        }

        printf("%s,%d,%d,%d,%d,%d,%d,%d,%d\n", p[i].id, p[i].arrival, p[i].priority, p[i].service, start, completion, wt, tat, rt);
    }


    double avg_wt = total_wt / n;
    double avg_tat = total_tat / n;
    double avg_rt = total_rt / n;
    

    int first_arrival = p[0].arrival;

    for (int i = 1; i < n; i++) {
        if (p[i].arrival < first_arrival) {
            first_arrival = p[i].arrival;
        }
    }

    int elapsed = makespan - first_arrival;

    double throughput = (elapsed > 0) ? (double)n / elapsed : 0.0;

    double utilization = (elapsed > 0) ? ((double)total_service_time / (elapsed * 2.0)) * 100.0 : 0.0;


    printf("SUMMARY\n");
    printf("AVERAGE_WAITING,%.2f\n", avg_wt);
    printf("AVERAGE_TURNAROUND,%.2f\n", avg_tat);
    printf("AVERAGE_RESPONSE,%.2f\n", avg_rt);
    printf("THROUGHPUT,%.4f\n", throughput);
    printf("UTILIZATION,%.2f%%\n", utilization);
}






//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//main
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]){
    
    char *input_file = argv[1];
    char *algorithm = argv[2];

    // parsing input

    FILE *fp = fopen(input_file, "r");

    if (fp == NULL){
        printf("Error in opening file");
        return 1;
    }

    int n = 0;
    int q = 0;
    int i=0;

    fscanf(fp, "%d", &n);
    fscanf(fp, "%d", &q);

    Patient p[n];

    for(i = 0; i < n; i++){
            if(fscanf(fp, " %31[^,],%d,%d,%d,%63[^\r\n]", p[i].id, &p[i].arrival, &p[i].priority, &p[i].service, p[i].category) != 5){
                fprintf(stderr, "Invalid  row %d\n", i+1);
                fflush(stderr);
                fclose(fp);
                return 1;
            }

            p[i].remaining = p[i].service;
            p[i].start_time = -1;
            p[i].completion_time = -1;
            p[i].started = -1;
            p[i].completed = -1;
    }

    fclose(fp);

    qsort(p, n, sizeof(Patient), compare_arrival);
    /*

    printf("%d processes:\n", n);
    for(i = 0; i < n; i++){
        printf("%s | Arrival: %d | Priority: %d | Service: %d | Category: %s\n",
               p[i].id, p[i].arrival, p[i].priority, p[i].service, p[i].category);
    }
    */



    if(strcmp(algorithm, "FCFS") == 0){
        fcfs(p, n);
    } 
    else if (strcmp(algorithm, "PRIORITY") == 0 || strcmp(algorithm, "NON-PREEMPTIVE") == 0 || strcmp(algorithm, "NP") == 0) {
        priority_nonpreemptive(p, n);
    }
    else if (strcmp(algorithm, "PRIORITY_PREEMPTIVE") == 0 || strcmp(algorithm, "PREEMPTIVE") == 0 || strcmp(algorithm, "PP") == 0) {
        priority_preemptive(p, n);
    }
    else{
        printf("Error\n");
        return 1;
    }


    print_standardized_output(p, n);

    

    return 0;
}

