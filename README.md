# Process Scheduling Simulator

## 1. Introduction  
This program simulates **process scheduling** using the **FCFS**, **Round Robin (RR)**, **SJF**, and **SRTN** algorithms.  
It reads process information and scheduling parameters from an input file and outputs the **Gantt chart**, **turnaround time**, and **waiting time** for each process.

## 2. Compilation and Execution  
### 2.1. Compile the program  
The program is written in **C++** and can be compiled using **g++**:  
```bash
g++ src/*.cpp -o main.exe
```
Alternatively, you can use **Visual Studio**, **Clang**, or **MinGW**.

### 2.2. Run the program  
After compilation, run the program with the following command:  
```bash
main.exe <INPUT_FILE> <OUTPUT_FILE>
```
Example:
```bash
main.exe input.txt output.txt
```

## 3. Input File Format  
### 3.1. Structure of `input.txt`  
- **Line 1**: An integer (1-4) indicating the scheduling algorithm:  
  - `1` - FCFS (First Come First Serve)  
  - `2` - RR (Round Robin)  
  - `3` - SJF (Shortest Job First)  
  - `4` - SRTN (Shortest Remaining Time Next)  

- **Line 2**: If the algorithm is **Round Robin**, this line contains the `quantum` value (a positive integer). Otherwise, it contains the number of processes `N` (maximum 4).  

- **Lines 3 and beyond**: Each line describes a process in the format:  
  ```
  <Arrival Time> [<CPU Burst Time> <Resource Usage Time>(<ResourceID>)]
  ```
  - **Arrival Time**: The time the process arrives in the ready queue.  
  - **CPU Burst Time**: The time required for execution on the CPU.  
  - **Resource Usage Time (ResourceID)**: The time spent using a resource (R1 or R2).  

Example `input.txt`:
```
3
3
0 8 2(R1) 3
2 3 4(R2) 2
5 1 6(R1) 4
```

## 4. Output File Format  
### 4.1. Structure of `output.txt`  
- **Line 1**: CPU Gantt chart, showing process IDs in execution order. The `_` character represents idle time.  
- **Line 2 (and line 3 if applicable)**: Gantt chart for resources R1 and R2.  
- **Line 4**: Turnaround time for each process (space-separated).  
- **Line 5**: Waiting time for each process (**only counted while waiting for CPU**).  

Example `output.txt`:
```
1 1 1 1 1 1 1 1 3 2 2 2 1 1 1 _ 2 2 3 3 3 3
_ _ _ _ _ _ _ _ 1 1 3 3 3 3 3 3 _ _ _ _ _ _
_ _ _ _ _ _ _ _ _ _ _ _ 2 2 2 2 _ _ _ _ _ _
15 16 17
2 7 5
```

## 5. Supported Scheduling Algorithms  
### 5.1. **FCFS (First Come First Serve)**  
- Processes execute in arrival order.  
- No preemption; a process runs until completion.  

### 5.2. **Round Robin (RR)**  
- Each process receives a time slice (`quantum`).  
- If it doesn’t finish within `quantum`, it moves to the back of the queue.  

### 5.3. **SJF (Shortest Job First)**  
- The process with the **shortest CPU burst** is executed first.  
- No preemption; a process runs until completion.  

### 5.4. **SRTN (Shortest Remaining Time Next)**  
- A preemptive version of SJF.  
- If a new process arrives with a **shorter remaining time**, it **preempts** the currently running process.  

## 6. Important Notes  
- Each process **can use CPU and resources up to 2 times**.  
- The system has **only 2 resources (R1 and R2)**, both scheduled using FCFS.  
- **Waiting time is counted only for the CPU queue**.  

