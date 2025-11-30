/**
 * @file interrupts_EP_RR_101214895_101324143.cpp
 * @brief template main.cpp file for Assignment 3 Part 1 of SYSC4001
 * 
 */

#include "interrupts_101214895_101324143.hpp"

const unsigned int QUANTUM = 100;

void FCFS(std::vector<PCB> &ready_queue) {
    std::sort( 
                ready_queue.begin(),
                ready_queue.end(),
                []( const PCB &first, const PCB &second ){
                    return (first.arrival_time > second.arrival_time); 
                } 
            );
}

std::tuple<std::string /* add std::string for bonus mark */ > run_simulation(std::vector<PCB> list_processes) {

    std::vector<PCB> ready_queue;   
    std::vector<PCB> wait_queue;    
    std::vector<PCB> job_list;      

    unsigned int current_time = 0;
    unsigned int quantum_counter = 0;

    PCB running;

    idle_CPU(running);

    std::string execution_status;

    execution_status = print_exec_header();
    for(auto &p : list_processes)
        job_list.push_back(p);

    while(!all_process_terminated(job_list)) {
        for(auto &process : job_list) {
            if(process.arrival_time == current_time && process.state == NEW) {

                assign_memory(process);

                process.state = READY;
                process.cpu_executed = 0;
                process.io_block = 0;

                ready_queue.push_back(process);

                sync_queue(job_list, process);
                execution_status += print_exec_status(current_time, process.PID, NEW, READY);
            }
        }

        //----------------------------------------------- MANAGE WAIT QUEUE ----------------------------------------
        for(int i = 0; i < (int)wait_queue.size(); i++) {

            PCB &p = wait_queue[i];
            p.io_block--;

            if(p.io_block == 0) {

                states old_state = WAITING;
                p.state = READY;

                ready_queue.push_back(p);
                sync_queue(job_list, p);

                execution_status += print_exec_status(current_time, p.PID, old_state, READY);

                wait_queue.erase(wait_queue.begin() + i);
                i--;
            }
        }

        //-------------------------------------------------- SCHEDULER ---------------------------------------------
        if(running.state == NOT_ASSIGNED) {

            if(!ready_queue.empty()) {

                FCFS(ready_queue);

                running = ready_queue.back();
                ready_queue.pop_back();

                states old_state = running.state;
                running.state = RUNNING;
                running.start_time = current_time;

                sync_queue(job_list, running);
                execution_status += print_exec_status(current_time, running.PID, old_state, RUNNING);

                quantum_counter = 0;
            }
        }
        else {

            running.remaining_time--;
            running.cpu_executed++;
            quantum_counter++;

            if(running.io_freq > 0 &&
               running.remaining_time > 0 &&
               (running.cpu_executed % running.io_freq == 0)) {

                states old_state = RUNNING;
                running.state = WAITING;

                running.io_block = running.io_duration;
                wait_queue.push_back(running);

                sync_queue(job_list, running);
                execution_status += print_exec_status(current_time, running.PID, old_state, WAITING);

                idle_CPU(running);
            }
            else if(running.remaining_time == 0) {

                states old_state = RUNNING;
                running.state = TERMINATED;

                free_memory(running);
                sync_queue(job_list, running);

                execution_status += print_exec_status(current_time, running.PID, old_state, TERMINATED);

                idle_CPU(running);
            }
            else if(!ready_queue.empty()) {

                FCFS(ready_queue);

                if(ready_queue.back().priority < running.priority) {

                    states old_state = RUNNING;
                    running.state = READY;

                    ready_queue.push_back(running);
                    sync_queue(job_list, running);

                    execution_status += print_exec_status(current_time, running.PID, old_state, READY);

                    idle_CPU(running);
                }
            }
            else if(quantum_counter == QUANTUM) {

                states old_state = RUNNING;
                running.state = READY;

                ready_queue.push_back(running);
                sync_queue(job_list, running);

                execution_status += print_exec_status(current_time, running.PID, old_state, READY);

                idle_CPU(running);
            }
        }

        current_time++;
    }

    execution_status += print_exec_footer();

    return std::make_tuple(execution_status);
}

int main(int argc, char** argv) {

    if(argc != 2) {
        std::cout << "ERROR!\nExpected 1 argument, received " << argc - 1 << std::endl;
        return -1;
    }

    auto file_name = argv[1];
    std::ifstream input_file;
    input_file.open(file_name);

    if (!input_file.is_open()) {
        std::cerr << "Error: Unable to open file: " << file_name << std::endl;
        return -1;
    }

    std::string line;
    std::vector<PCB> list_process;
    while(std::getline(input_file, line)) {
        std::replace(line.begin(), line.end(), ',', ' ');
        auto input_tokens = split_delim(line, " ");
        list_process.push_back(add_process(input_tokens));
    }
    input_file.close();

    auto [exec] = run_simulation(list_process);

    write_output(exec, "output_files/execution.txt");

    return 0;
}

