#include <iostream>
#include <chrono>

#include "utils/random_chooser_2.hpp"
#include "utils/random_chooser.hpp"

int main () {
    //*
    SumTree<int> random_chooser;
    /*/
    RandomChooser<int> random_chooser;
    //*/
    std::chrono::time_point<std::chrono::high_resolution_clock> last_time, current_time;

    const int nb_tests = 10000;
    const int nb_picks = 50;

    int count[nb_picks+1];


    for(int i=1; i<=nb_picks; i++) {
        random_chooser.add(i, 1);
        count[i] = 0;
    }

    last_time = std::chrono::high_resolution_clock::now();

    for(int i=0; i<nb_tests; i++) {
        for(int j=1; j<=nb_picks; j++) {
            int picked = random_chooser.pick();
            count[picked] ++;
        }
        random_chooser.reset();
    }

    current_time = std::chrono::high_resolution_clock::now();
    int time_us = std::chrono::duration_cast<std::chrono::microseconds>(current_time-last_time).count();
    std::cout << "Complete picking of : " << nb_picks << " elements in " << time_us / nb_tests << " µs average" << std::endl;

    for(int i=1; i<=nb_picks; i++) {
        //std::cout << count[i]/10000.0*55 << std::endl;
    }


    return EXIT_SUCCESS;
}