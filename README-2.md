# Emergency Hospital Management: Process Scheduling Simulation

**CS-3710 Operating Systems | Assignment 1 (Process Management)**

## 1. Group Members

- **Member 1:** Mohammad Quayim Qadri
- **Member 2:** Vaibhavi Singhania

## 2. Selected Scenario

**Emergency Hospital Management.** Patients are modelled as processes and the two doctors are modelled as two identical parallel service units, as required by the Assignment 1 specification. The scope is process management only: patient arrivals, scheduling, execution, preemption, completion, and scheduling metrics. Beds, nurses, equipment, memory allocation and concurrency control are not part of this assignment stage.

## 3. Build and Run

The program is written in C and uses only the standard library.

```bash
gcc -std=c11 -Wall -Wextra main.c -o hospital
```

It compiles with no warnings.

```bash
./hospital INPUT_FILE ALGORITHM
```

## 4. Command-Line Arguments

| Argument | Meaning |
|---|---|
| `INPUT_FILE` | Path to the input file (format in section 5) |
| `ALGORITHM` | `FCFS`, `PRIORITY`, or `PRIORITY_PREEMPTIVE` |

`NP` / `NON-PREEMPTIVE` and `PP` / `PREEMPTIVE` are also accepted as aliases for `PRIORITY` and `PRIORITY_PREEMPTIVE`.

```bash
./hospital input1.txt FCFS
./hospital input1.txt PRIORITY
./hospital input1.txt PRIORITY_PREEMPTIVE
```

Running with missing arguments prints a usage message and exits with status 1. An unknown algorithm name prints `Error` and exits with status 1. An unreadable input file prints an error and exits with status 1.

## 5. Input Format

```text
N
Q
ID,ARRIVAL,PRIORITY,SERVICE,CATEGORY
...
```

- `N`: number of patients.
- `Q`: Round Robin time quantum. It is read for format compatibility but is not used, because Round Robin is not required for the hospital scenario.
- One patient per line. A smaller priority number means a more urgent patient. `SERVICE` is the total simulated treatment time. `CATEGORY` (for example `FEVER` or `CRITICAL`) is descriptive only; scheduling uses the numeric priority.

Example (`input1.txt`):

```text
5
2
P1,0,3,10,FEVER
P2,1,2,8,FRACTURE
P3,2,1,12,CRITICAL
P4,3,3,5,MINOR
P5,4,1,7,SEVERE
```

Time is simulated, not real: a service time of 10 means 10 simulation time units.

## 6. Process Model

Each patient is a `Patient` structure with these fields:

- `id`, `arrival`, `priority`, `service`, `category`: the input record
- `remaining`: treatment time still needed
- `start_time`: first time the patient received a doctor
- `completion_time`: time treatment finished
- `started`, `completed`: status flags used by the simulation

Lifecycle: `NEW -> READY -> RUNNING -> TERMINATED`

- NEW to READY when the arrival time is reached.
- READY to RUNNING when a doctor is assigned by the scheduler.
- RUNNING to TERMINATED when the remaining service time reaches zero.
- RUNNING back to READY if the patient is preempted, with the remaining time preserved.

The simulation advances in one-unit time steps. In each step it handles completions, fills idle doctors, and (for preemptive priority) preempts as needed. It is deterministic, so the same input and algorithm always give the same result. Periods with no ready patient are handled by simply advancing time.

## 7. Scheduling Algorithms

Tie-breaking everywhere: higher priority first (where applicable), then earlier arrival, then lexicographically smaller ID.

### FCFS

Patients are served in order of arrival, with ties broken by ID. It is non-preemptive. Whenever either doctor is free, the earliest-arrived waiting patient is assigned.

### Priority (non-preemptive)

When a doctor becomes free, the waiting patient with the smallest priority number is chosen (ties as above). Once treatment starts it is never interrupted.

### Priority (preemptive)

A running patient is interrupted only when a ready patient has a **strictly** better priority than at least one running patient. Equal priorities never cause preemption. When both doctors are busy, the patient preempted is the running one with the lowest priority; if equal, the later arrival; if still equal, the lexicographically larger ID. The preempted patient returns to READY with its remaining time kept. Preemption is re-checked repeatedly within the same time step, so several high-priority patients arriving together are all handled immediately. `START` records the first time a patient ever receives a doctor.


## 9. Output Format

```text
PROCESS_ID,ARRIVAL,PRIORITY,SERVICE,START,COMPLETION,WAITING,TURNAROUND,RESPONSE
...
SUMMARY
AVERAGE_WAITING,...
AVERAGE_TURNAROUND,...
AVERAGE_RESPONSE,...
THROUGHPUT,...
UTILIZATION,...%
```

There is exactly one row per patient, sorted by patient ID, with no extra text.

## 10. Algorithm Comparison

- When it comes to priorities, preemptive priority is the best algorithm since it runs higher priorities first as soon as they arrive, which is critical especially in a hospital setting.
- **Response time:** preemptive priority gives the best average response (2.60 vs 5.00 and 5.40). The critical patients P3 and P5 start the moment they arrive (response 0) and P3 and P5 have zero waiting time.
- **Waiting and turnaround:** preemptive priority is the worst here (6.40 and 14.80). Urgent patients are served faster, but interrupted patients lose time: P1 is pushed to a turnaround of 22 and P2 has to wait for P5 to finish. Overall delay is moved onto less urgent patients.
- **FCFS vs non-preemptive priority:** FCFS is slightly better on averages for this workload. Non-preemptive priority helps P5 (waiting 6 instead of 11) but P4, the lowest priority patient, waits 14 instead of 7.
- **Throughput and utilization** are identical across algorithms on input1, because the two doctors are never idle until the end, so the last completion time and total service time are the same. Policy changes *who* is served when, not how much work there is.
- **Starvation and fairness:** under both priority algorithms, low-priority patients (P4 here) are served last. With a steady stream of urgent arrivals they could wait indefinitely; no aging is implemented. FCFS cannot starve anyone but ignores urgency, so P3 (critical) waits 7 units.
- These conclusions hold for this particular workload and should not be read as general rules.


## 11. Design Decisions and Assumptions

- Using arrays to represent patient. sorting based on scheduling algorithms.
- Treatment time is treated as a CPU burst; no real-time delays are used.
- Lower priority number means greater urgency; the category text is never used for scheduling.
- Preemption has no switching cost.
- One shared `Patient` structure and output routine are used for all algorithms.
- Patient IDs are unique and categories contain no commas, as the input constraints guarantee.
- Utilization uses the same elapsed time (last completion minus first arrival) as throughput.

## 12. Known Limitations

1. **Input validation is limited.** Missing arguments and unreadable files are handled, but malformed records, invalid values (for example negative times) and fields longer than the buffers (ID 31 characters, category 63) are not checked. Input should follow the documented format.
2. **No starvation prevention** (aging) is implemented in the priority algorithms.
