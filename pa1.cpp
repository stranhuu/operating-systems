#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <pthread.h>
#include <sstream>
#include <vector>

// Define a structure to store CPU scheduling information
struct CPUInfo {
  std::string tasks;             // Task identifiers
  std::vector<int> times;        // Task execution times
  std::vector<double> entropies; // Entropy values
};

// Function to calculate entropy for a CPU
void *calculateEntropy(void *arg_void_ptr) {
  CPUInfo *arg_ptr = (CPUInfo *)arg_void_ptr;
  std::map<char, double> freq;
  double currFreq = 0.0;
  double currH = 0.0;
  double extraFreq = 0.0;
  double H, NFreq;

  // Iterate through the scheduling information
  for (int selectedTask = 0; selectedTask < arg_ptr->tasks.length();
       selectedTask++) {
    // Check if the task frequency is already in the 'freq' map
    if (freq.count(arg_ptr->tasks[selectedTask]) == 0) {
      freq[arg_ptr->tasks[selectedTask]] =
          0.0; // Initialize frequency to 0 if not found
    }

    extraFreq = arg_ptr->times[selectedTask];
    NFreq = currFreq + extraFreq;

    if (NFreq == extraFreq) {
      H = 0.0;
    } else {
      double currentTerm = 0.0;

      if (freq[arg_ptr->tasks[selectedTask]] != 0.0) { // Log(0) is NAN
        currentTerm = freq[arg_ptr->tasks[selectedTask]] *
                      log2(freq[arg_ptr->tasks[selectedTask]]);
      }
      double newTerm = (freq[arg_ptr->tasks[selectedTask]] + extraFreq) *
                       log2(freq[arg_ptr->tasks[selectedTask]] + extraFreq);
      H = log2(NFreq) -
          ((log2(currFreq) - currH) * (currFreq)-currentTerm + newTerm) / NFreq;
    }

    // Store the calculated entropy and update the variables
    arg_ptr->entropies.push_back(H);
    currFreq = NFreq;
    currH = H;
    freq[arg_ptr->tasks[selectedTask]] += extraFreq;
  }
}

int main() {
  std::vector<std::string> input;
  std::string temp;

  // Read input from stdin and store it in a vector
  while (getline(std::cin, temp) && !temp.empty()) {
    input.push_back(temp);
  }

  int numThreads = input.size();
  CPUInfo *data = new CPUInfo[numThreads];
  pthread_t *tid = new pthread_t[numThreads];

  // Process each CPU scheduling string
  for (int i = 0; i < numThreads; i++) {
    std::stringstream ss(input[i]);
    std::string task;
    int time;

    CPUInfo cpuInfo;

    // Parse the input string to extract task identifiers and execution times
    while (ss >> task >> time) {
      cpuInfo.tasks.push_back(task[0]);
      cpuInfo.times.push_back(time);
    }

    data[i] = cpuInfo;
  }

  // Create the threads to calculate entropy for the CPUs
  for (int i = 0; i < numThreads; i++) {
    if (pthread_create(&tid[i], nullptr, calculateEntropy, &data[i])) {
      std::cerr << "Error creating thread " << i + 1 << std::endl;
      return 1;
    }
  }

  // Wait for all threads to finish then call pthread_join
  for (int i = 0; i < numThreads; i++) {
    pthread_join(tid[i], nullptr);
  }

  // Print the results for each CPU
  for (int i = 0; i < numThreads; i++) {
    std::cout << "CPU " << i + 1 << std::endl;
    std::cout << "Task scheduling information: ";
    for (int j = 0; j < data[i].tasks.size(); j++) {
      std::cout << data[i].tasks[j] << "(" << data[i].times[j] << ")";
      if (j < data[i].tasks.size() - 1) {
        std::cout << ", ";
      }
    }
    std::cout << std::endl;

    std::cout << "Entropy for CPU " << i + 1 << std::endl;
    for (int j = 0; j < data[i].entropies.size(); j++) {
      std::cout << std::fixed << std::setprecision(2) << data[i].entropies[j]
                << " ";
    }
    std::cout << std::endl;
  }

  // Clean up allocated memory
  if (tid != nullptr) {
    delete[] tid;
  }
  if (data != nullptr) {
    delete[] data;
  }

  return 0;
}

/*
Resource used:
C. A. Rincon, D. Rivas and A. M. K. Cheng, "Entropy-based scheduling performance
in real-time multiprocessor systems," 2023 57th Annual Conference on Information
Sciences and Systems (CISS), Baltimore, MD, USA, 2023, pp. 1-5,
doi: 10.1109/CISS56502.2023.10089704.
*/
