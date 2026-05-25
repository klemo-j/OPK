//
// Created by jakub on 25. 5. 2026.
//
/*Uloha 1. Naprogramujte viacvláknovú aplikáciu využívajúcu model producer–consumer:
• Vlákno 1 (producer):
o číta textový súbor znak po znaku
o vkladá znaky do zdieľanej fronty (std::queue<char>)
• Vlákno 2 (consumer):
o odoberá znaky z fronty
o spracovanie:
▪ čísla ignoruje
▪ malé písmená prevedie na veľké a uloží do container1 (vhodný stl
kontainer)
▪ veľké písmená uloží do container2 (vhodný stl kontainer)
• Po skončení vlákien:
o container2 utrieďte pomocou vhodného stl algoritmu
o v main() vypíšte:
1. obsah container1
2. obsah container2
Požiadavky:
Použite: std::thread a synchronizujte vlákna pomocou std::condition_variable / atomic variable
/ lockg guard
• zabezpečte korektné ukončenie consumer vlákna*/


#include <algorithm>
#include <fstream>
#include <iostream>
#include <locale>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <atomic>

std::vector<char> container1;
std::vector<char> container2;
std::mutex mtx;
std::queue<char> q;
std::atomic<bool> flag = false;



void read() {

    std::ifstream file("file.txt");
    for (char k; file.get(k);) {
        std::lock_guard<std::mutex> lock(mtx);
        q.push(k);
    };
flag = true;
}

void write() {
    while (true) {
        std::lock_guard<std::mutex> lock(mtx);
        if (!q.empty()) {
            char c = q.front();
            q.pop();
            if (std::islower(c)) {
                container1.push_back(std::toupper(c));
            }else if (std::isupper(c)){
                container2.push_back(c);
            }
        }else if (flag){
            break;
        }
    }
};

int main()
    {

    std::thread producer(read);
    std::thread consumer(write);
    producer.join();
    consumer.join();

    std::sort (container2.begin(), container2.end());

    std::cout << "container2" << '\n';
    for (char p : container2) {
        std::cout << p ;
    }
    std::cout << '\n';

    std::cout <<"container 1" << std::endl;
    for (char p : container1) {
        std::cout << p ;
    }


    return 0;
}
