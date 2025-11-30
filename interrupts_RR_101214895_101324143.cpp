/**
 * @file interrupts.hpp
 * @author Sasisekhar Govind
 * @brief template main.cpp file for Assignment 3 Part 1 of SYSC4001
 * 
 */

#include "interrupts.hpp"

const unsigned int QUANTUM = 100;

/**
 * @brief Round Robin Simulation (fixed and stable)
 */
std::tuple<std::string> run_simulation(std::vector<PCB> list_processes)
{
    std::vector<PCB> job_list;
    std::vector<PCB> ready_queue;
    std::vector<PCB> wait_queue;

    for (auto &p : list_processes)
        job_list.push_back(p);

    PCB running;
    idle_CPU(running);

    unsigned int current_time = 0;
    unsigned int quantum_counter = 0;

    std::string exec = print_exec_header();

    while (!all_process_terminated(job_list))
    {
        for (auto &p : job_list)
        {
            if (p.arrival_time == current_time && p.state == NEW)
            {
                assign_memory(p);
                p.state = READY;

                ready_queue.push_back(p);
                sync_queue(job_list, p);

                exec += print_exec_status(current_time, p.PID, NEW, READY);
            }
        }
        for (int i = 0; i < (int)wait_queue.size(); i++)
        {
            PCB &p = wait_queue[i];
            p.io_block--;

            if (p.io_block == 0)
            {
                states old_s = WAITING;
                p.state = READY;

                ready_queue.push_back(p);
                sync_queue(job_list, p);

                exec += print_exec_status(current_time, p.PID, old_s, READY);

                wait_queue.erase(wait_queue.begin() + i);
                i--;
            }
        }
        if (running.state == NOT_ASSIGNED)
        {
            if (!ready_queue.empty())
            {
                running = ready_queue.front();
                ready_queue.erase(ready_queue.begin());

                states old_s = running.state;
                running.state = RUNNING;
                running.start_time = current_time;

                quantum_counter = 0;

                sync_queue(job_list, running);
                exec += print_exec_status(current_time, running.PID, old_s, RUNNING);
            }
        }
        else
        {
            running.remaining_time--;
            quantum_counter++;

            if (running.io_freq > 0 &&
                running.remaining_time > 0 &&
                ((running.processing_time - running.remaining_time) % running.io_freq == 0))
            {
                states old_s = RUNNING;
                running.state = WAITING;

                running.io_block = running.io_duration;
                wait_queue.push_back(running);

                sync_queue(job_list, running);
                exec += print_exec_status(current_time, running.PID, old_s, WAITING);

                idle_CPU(running);
            }
            else if (running.remaining_time == 0)
            {
                states old_s = RUNNING;
                running.state = TERMINATED;

                sync_queue(job_list, running);
                free_memory(running);

                exec += print_exec_status(current_time, running.PID, old_s, TERMINATED);

                idle_CPU(running);
            }
            else if (quantum_counter == QUANTUM)
            {
                states old_s = RUNNING;
                running.state = READY;

                ready_queue.push_back(running);
                sync_queue(job_list, running);

                exec += print_exec_status(current_time, running.PID, old_s, READY);

                idle_CPU(running);
            }
            else
            {
                sync_queue(job_list, running);
            }
        }

        current_time++;
    }

    exec += print_exec_footer();
    return {exec};
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: ./RR <input_file>\n";
        return -1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open())
    {
        std::cerr << "Error: Cannot open file.\n";
        return -1;
    }

    std::string line;
    std::vector<PCB> processes;

    while (std::getline(file, line))
    {
        std::replace(line.begin(), line.end(), ',', ' ');
        auto t = split_delim(line, " ");
        processes.push_back(add_process(t));
    }

    auto [exec] = run_simulation(processes);
    write_output(exec, "output_files/execution.txt");

    return 0;
}
