/**
 * @file interrupts.hpp
 * @author Sasisekhar Govind
 * @brief template main.cpp file for Assignment 3 Part 1 of SYSC4001
 * 
 */

#include "interrupts.hpp"

void FCFS(std::vector<PCB> &ready_queue)
{
    std::sort(
        ready_queue.begin(),
        ready_queue.end(),
        [](const PCB &a, const PCB &b) {
            return a.priority > b.priority; // DESCENDING
        }
    );
}

/**
 * @brief External Priority Simulation (Non-Preemptive)
 */
std::tuple<std::string> run_simulation(std::vector<PCB> list_processes)
{
    std::vector<PCB> job_list = list_processes;
    std::vector<PCB> ready_queue;
    std::vector<PCB> wait_queue;

    unsigned int current_time = 0;
    PCB running;
    idle_CPU(running);

    std::string exec = print_exec_header();

    while (!all_process_terminated(job_list))
    {
        for (auto &p : job_list)
        {
            if (p.arrival_time == current_time && p.state == NEW)
            {
                assign_memory(p);

                p.state = READY;
                p.cpu_executed = 0;

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
                FCFS(ready_queue);

                running = ready_queue.back();
                ready_queue.pop_back();

                states old_s = running.state;
                running.state = RUNNING;

                sync_queue(job_list, running);
                exec += print_exec_status(current_time, running.PID, old_s, RUNNING);
            }
        }
        else
        {
            running.remaining_time--;
            running.cpu_executed++;

            if (running.io_freq > 0 &&
                running.cpu_executed % running.io_freq == 0 &&
                running.remaining_time > 0)
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
        std::cout << "Usage: ./EP <file>\n";
        return -1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open())
    {
        std::cerr << "Error opening file.\n";
        return -1;
    }

    std::string line;
    std::vector<PCB> processes;

    while (std::getline(file, line))
    {
        std::replace(line.begin(), line.end(), ',', ' ');
        auto fields = split_delim(line, " ");
        processes.push_back(add_process(fields));
    }

    auto [exec] = run_simulation(processes);
    write_output(exec, "output_files/execution.txt");
    return 0;
}
