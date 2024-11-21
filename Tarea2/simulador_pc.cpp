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

int Productor(){
    int producto = rand() % 100;
    std::cout << "Producto: " << producto << std::endl;
    return producto;
}

void Consumidor(float producto){
    float consumo = sqrt(producto);
    std::cout << "Consumo: " << consumo << std::endl;
}

class Monitor {
private:
    int contador;
    int* buffer;
    int in = 0, out = 0;
    int N;
    int t;
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
    Monitor(int N, int t): N(N), t(t), buffer(new int[N]), contador(0) {}
    ~Monitor() { delete[] buffer; }

    void Producir(int producto) {
        std::unique_lock<std::mutex> lock(mutex);

        if (contador == N){
            resize(N*2);
            std::cout << "Re-size: " << N << std::endl;
        }

        cv.wait(lock, [this] { return contador < N; });

        buffer[in] = producto;
        in = (in + 1) % N;
        contador++;

        cv.notify_one();  // Notifica a un consumidor que hay un producto disponible
    }

    int Consumir() {
        std::cout << N << std::endl;
        std::unique_lock<std::mutex> lock(mutex);
        auto timeout = std::chrono::seconds(t);

        if (!cv.wait_for(lock, timeout, [this] { return contador > 0; })) return -1;
          // Espera mientras el buffer esté vacío

        int consumir = buffer[out];
        out = (out + 1) % N;
        contador--;


        if (contador <= N / 4 && N > 2){
            resize(N / 2);
            std::cout << "Re-size: " << N << std::endl;
        }

        cv.notify_one();  // Notifica a un productor que hay espacio disponible

        return consumir;
    }
};

void Thread_Productor(Monitor& monitor) {

    for (int i = 0; i < 50 ; i++) {
        int producto = Productor();
        monitor.Producir(producto);
    }

    std::cout << "Productor muerto de cansancio" << std::endl;
}

void Thread_Consumidor(Monitor& monitor) {

    while (true) {

        auto start_time = std::chrono::steady_clock::now();
        int consumo = monitor.Consumir();

        if (consumo == -1){
            std::cout << "Consumidor muerto de hambre" << std::endl;
            break;
        }

        Consumidor(consumo);
    }
}

int main(int argc, char* argv[]) {
    srand(time(NULL));

    // Simulando que p, c, s, y t son obtenidos como parámetros:
    int p = std::atoi(argv[2]);
    int c = std::atoi(argv[4]);
    int s = std::atoi(argv[6]);
    int t = std::atoi(argv[8]);

    if (p <= 0 || c <= 0 || s <= 0) {
        std::cerr << "Los parámetros deben ser mayores que 0.\n";
        return 1;
    }

    Monitor monitor(s, t);

    std::vector<std::thread> productores;
    for (int i = 0; i < p; i++) {
        productores.emplace_back(Thread_Productor, std::ref(monitor));
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
