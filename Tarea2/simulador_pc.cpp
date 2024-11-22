#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <vector>
#include <stdlib.h>
#include <chrono>
#include <fstream>

std::ofstream logFile;


int Productor(){
    int producto = rand() % 100;
    //logFile << "Producto: " << producto << std::endl;
    return producto;
}

void Consumidor(float producto){
    float consumo = sqrt(producto);
    //logFile << "Consumo: " << consumo << std::endl;
}

class Monitor {
private:
    int contador;
    int* buffer;
    int in = 0, out = 0;
    int N;
    int t;
    int countConsumido = 0;
    int aProducir;
    std::mutex mutex;
    std::condition_variable cv;

    void resize(int newSize){
        int* newBuffer = new int[newSize];

        for (int i = 0; i < contador; i++){
            newBuffer[i] = buffer[(out+i) % N];
        }

        delete[] buffer;
        buffer = newBuffer;

        in = contador;
        out = 0;
        N = newSize;
    }

public:
    Monitor(int N, int t, int aProducir): N(N), t(t), aProducir(aProducir), buffer(new int[N]), contador(0) {}
    ~Monitor() { delete[] buffer; }

    void Producir(int producto) {
        std::unique_lock<std::mutex> lock(mutex);

        if (contador == N){
            resize(N*2);
            logFile << "Resize: " << N << std::endl;
        }

        cv.wait(lock, [this] { return contador < N; });

        buffer[in] = producto;
        in = (in + 1) % N;
        contador++;

        cv.notify_one();  // Notifica a un consumidor que hay un producto disponible
    }

    int Consumir() {

        std::unique_lock<std::mutex> lock(mutex);
        auto timeout = std::chrono::seconds(t);

        if (!cv.wait_for(lock, timeout, [this] { return contador > 0; })) return -1;

        int consumir = buffer[out];
        out = (out + 1) % N;
        contador--;


        if (contador <= N / 4 && N > 2){
            resize(N / 2);
            logFile << "Resize: " << N << std::endl;
        }

        countConsumido++;
        logFile << "Progreso: "<< countConsumido * 100.0 / aProducir << "%" <<std::endl;
        cv.notify_one();  // Notifica a un productor que hay espacio disponible

        return consumir;
    }
};

void Thread_Productor(Monitor& monitor, int numProduccion) {

    for (int i = 0; i < numProduccion; i++) {
        int producto = Productor();
        monitor.Producir(producto);
    }

    logFile << "Productor muerto de cansancio" << std::endl;
}

void Thread_Consumidor(Monitor& monitor) {

    while (true) {

        auto start_time = std::chrono::steady_clock::now();
        int consumo = monitor.Consumir();

        if (consumo == -1){
            logFile << "Consumidor muerto de hambre" << std::endl;
            break;
        }

        Consumidor(consumo);
    }
}

int main(int argc, char* argv[]) {

    srand(time(NULL));

    logFile.open("log.txt", std::ios::out | std::ios::trunc);

    if (!logFile.is_open()) {
        std::cerr << "Error al abrir el archivo de log." << std::endl;
        return 1;
    }

    int p = std::atoi(argv[2]); //Numero de productores
    int c = std::atoi(argv[4]); //Numedo de consumidores
    int s = std::atoi(argv[6]); //Tamaño de la cola
    int t = std::atoi(argv[8]); //Tiempo de espera cosumidores

    int numProduccion = 20; // Cantidad elementos a producir por porductor
    int aProducir = numProduccion * p; //Cantidad de elementos a producir

    if (p <= 0 || c <= 0 || s <= 0) {
        std::cerr << "Los parámetros deben ser mayores que 0.\n";
        return 1;
    }

    Monitor monitor(s, t, aProducir);

    std::vector<std::thread> productores;
    for (int i = 0; i < p; i++) {
        productores.emplace_back(Thread_Productor, std::ref(monitor), numProduccion);
    }

    std::vector<std::thread> consumidores;
    for (int i = 0; i < c; i++) {
        consumidores.emplace_back(Thread_Consumidor, std::ref(monitor));
    }

    for (auto& productor : productores) {
        productor.join();
    }

    for (auto& consumidor : consumidores) {
        consumidor.join();
    }

    return 0;
}
