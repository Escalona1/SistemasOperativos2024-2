#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstdlib>
#include <list>
#include <string.h>
#include <algorithm>

int page_faults = 0;

struct Pagina {
    int numero_de_pagina;
    bool enMemoria;
};

struct Marco {
    int numero_de_pagina;
    bool referencia; // Bit de referencia
};

class Tabla_de_paginas {
private:
    std::unordered_map<int, Pagina> tabla;
    std::list<int> orden_de_acceso; // FIFO, LRU, Optimal
    std::vector<int>& futureReferences; // Optimal
    size_t maximo_de_marcos;
    size_t num_marcos;
    std::string algoritmo;

    // Estructuras para LRU Reloj
    std::vector<Marco> marcos; // Representa los marcos de memoria
    int puntero_reloj;         // Índice que actúa como puntero del reloj

    void FIFO() {
        int pageToReplace = orden_de_acceso.front();
        tabla.erase(pageToReplace);
        orden_de_acceso.pop_front();

        std::cout << "Página " << pageToReplace << " eliminada." << std::endl;

    }

    void Optimal() {
        int farthest = -1;
        int pageToReplace = -1;

        for (auto& entry : tabla) {
            auto it = std::find(futureReferences.begin(), futureReferences.end(), entry.first);
            if (it == futureReferences.end()) {
                pageToReplace = entry.first;
                break;
            }
            if (std::distance(futureReferences.begin(), it) > farthest) {
                farthest = std::distance(futureReferences.begin(), it);
                pageToReplace = entry.first;
            }
        }

        tabla.erase(pageToReplace);

        std::cout << "Página " << pageToReplace << " eliminada." << std::endl;
    }

    void LRU() {
        int pageToReplace = orden_de_acceso.back();
        tabla.erase(pageToReplace);
        orden_de_acceso.pop_back();

        std::cout << "Página " << pageToReplace << " eliminada." << std::endl;
    }

    void updateLRU(int pageNumber) {
        auto it = std::find(orden_de_acceso.begin(), orden_de_acceso.end(), pageNumber);
        if (it != orden_de_acceso.end()) {
            orden_de_acceso.erase(it);
        }
        orden_de_acceso.push_back(pageNumber);
    }

    void Reloj() {
        while (true) {
            // Verificar el marco apuntado por el puntero del reloj
            if (!marcos[puntero_reloj].referencia) {

                int pageToReplace = marcos[puntero_reloj].numero_de_pagina;
                tabla.erase(pageToReplace); // Eliminar de la tabla de páginas

                std::cout << "Página " << pageToReplace << " eliminada." << std::endl;

                marcos[puntero_reloj] = { -1, false }; // Limpiar el marco
                break; // Página encontrada para reemplazar
            }

            // Si el bit de referencia es 1, limpiarlo y avanzar
            marcos[puntero_reloj].referencia = false;
            puntero_reloj = (puntero_reloj + 1) % maximo_de_marcos;
        }
    }

public:
    Tabla_de_paginas(size_t maximo_de_marcos, std::string algoritmo, std::vector<int>& referencias)
        : maximo_de_marcos(maximo_de_marcos), algoritmo(algoritmo), num_marcos(0), futureReferences(referencias), puntero_reloj(0) {
        marcos.resize(maximo_de_marcos, { -1, false }); // Inicializa marcos vacíos
    }

    bool lleno() {
        return num_marcos >= maximo_de_marcos;
    }

    void accesar_pagina(int num_pagina) {
        if (tabla.find(num_pagina) != tabla.end()) {
            std::cout << "La página " << num_pagina << " ya está en memoria." << std::endl;
            if (algoritmo == "LRU Reloj simple") {
                for (auto& marco : marcos) {
                    if (marco.numero_de_pagina == num_pagina) {
                        marco.referencia = true; // Actualizar el bit de referencia
                        break;
                    }
                }
            }
            return;
        }

        if (lleno()) {
            if (algoritmo == "Optimo") {
                Optimal();
            } else if (algoritmo == "FIFO") {
                FIFO();
            } else if (algoritmo == "LRU") {
                LRU();
            } else if (algoritmo == "LRU Reloj simple") {
                Reloj();
            }
        }

        // Agregar la nueva página
        tabla[num_pagina] = { num_pagina, true };

        if (algoritmo == "FIFO") {
            orden_de_acceso.push_back(num_pagina);
        } 
        else if (algoritmo == "LRU") {
            updateLRU(num_pagina);
        } 
        else if (algoritmo == "LRU Reloj simple") {
            marcos[puntero_reloj] = { num_pagina, true };
            puntero_reloj = (puntero_reloj + 1) % maximo_de_marcos;
        }

        num_marcos++;
        page_faults++;

        std::cout << "Página " << num_pagina << " agregada." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    int frame_count = 0;        // Número de marcos de memoria
    std::string algorithm;     // Algoritmo de reemplazo
    std::string filename;      // Nombre del archivo de referencias

    // Parseo de los argumentos de línea de comandos
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-m") {
            frame_count = std::stoi(argv[++i]); // Número de marcos
        } else if (std::string(argv[i]) == "-a") {
            algorithm = argv[++i];             // Algoritmo a usar
        } else if (std::string(argv[i]) == "-f") {
            filename = argv[++i];              // Archivo con las referencias
        }
    }

    // Verificación de los parámetros
    if (frame_count <= 0 || algorithm.empty() || filename.empty()) {
        std::cerr << "Uso: " << argv[0] << " -m <num_marcos> -a <algoritmo> -f <archivo>" << std::endl;
        return 1;
    }

    // Leer referencias desde el archivo
    std::vector<int> referencias;
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Error al abrir el archivo: " << filename << std::endl;
        return 1;
    }
    int ref;
    while (file >> ref) {
        referencias.push_back(ref);
    }
    file.close();

    // Crear la tabla de páginas
    Tabla_de_paginas tabla(frame_count, algorithm, referencias);

    // Simular la memoria
    for (int num_pagina : referencias) {
        tabla.accesar_pagina(num_pagina);
    }

    // Mostrar el resultado final
    std::cout << "Número total de fallos de página: " << page_faults << std::endl;

    return 0;
}

