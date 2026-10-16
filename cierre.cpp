#include <cstdio>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstring>
#include <string>
#include <limits>

using namespace std;

// Debe coincidir exactamente con la estructura utilizada por ventas.cpp.
struct Comanda {
    int idMozo;
    int codigoProducto;
    int cantidad;
    float comision;
};

bool fechaValida(const string& fecha) {
    // Formato esperado: DD-MM-AAAA (10 caracteres).
    if (fecha.size() != 10 || fecha[2] != '-' || fecha[5] != '-') {
        return false;
    }

    for (int i = 0; i < 10; ++i) {
        if (i == 2 || i == 5) continue;
        if (fecha[i] < '0' || fecha[i] > '9') return false;
    }

    int dia = (fecha[0] - '0') * 10 + (fecha[1] - '0');
    int mes = (fecha[3] - '0') * 10 + (fecha[4] - '0');

    return dia >= 1 && dia <= 31 && mes >= 1 && mes <= 12;
}

int main() {
    int numeroSemana, mes, cantidadDias;

    cout << "=== CIERRE SEMANAL - BUFFET ALBERT EINSTEIN ===\n";
    cout << "Numero de semana (X para comandas_semana_sX-mm.dat): ";
    if (!(cin >> numeroSemana) || numeroSemana < 1) {
        cout << "Error: el numero de semana debe ser un entero positivo.\n";
        return 1;
    }

    cout << "Mes de la semana (1-12): ";
    if (!(cin >> mes) || mes < 1 || mes > 12) {
        cout << "Error: el mes debe estar entre 1 y 12.\n";
        return 1;
    }

    cout << "Cuantas planillas diarias queres juntar? (1-7): ";
    if (!(cin >> cantidadDias) || cantidadDias < 1 || cantidadDias > 7) {
        cout << "Error: la cantidad de planillas debe estar entre 1 y 7.\n";
        return 1;
    }

    vector<Comanda> comandasSemanales;
    vector<string> fechasIngresadas;

    for (int i = 0; i < cantidadDias; ++i) {
        string fecha;
        cout << "Fecha de la planilla " << (i + 1)
             << " (DD-MM-AAAA): ";
        cin >> fecha;

        if (!fechaValida(fecha)) {
            cout << "Error: fecha invalida. Usa el formato DD-MM-AAAA.\n";
            return 1;
        }

        // Evita procesar dos veces el mismo archivo.
        for (size_t j = 0; j < fechasIngresadas.size(); ++j) {
            if (fechasIngresadas[j] == fecha) {
                cout << "Error: esa fecha ya fue ingresada.\n";
                return 1;
            }
        }
        fechasIngresadas.push_back(fecha);

        char nombreArchivo[64];
        snprintf(nombreArchivo, sizeof(nombreArchivo),
                 "comandas_%s.dat", fecha.c_str());

        FILE* archivoDia = fopen(nombreArchivo, "rb");
        if (archivoDia == NULL) {
            cout << "Error: no se pudo abrir " << nombreArchivo
                 << ". Verifica que exista en la carpeta de trabajo.\n";
            return 1;
        }

        Comanda comanda;
        while (fread(&comanda, sizeof(Comanda), 1, archivoDia) == 1) {
            comandasSemanales.push_back(comanda);
        }

        if (ferror(archivoDia)) {
            cout << "Error al leer " << nombreArchivo << ".\n";
            fclose(archivoDia);
            return 1;
        }

        fclose(archivoDia);
        cout << "Leida la planilla " << nombreArchivo << ".\n";
    }

    // Ordena todas las ventas por identificador de mozo.
    // Las ventas del mismo mozo pueden quedar en cualquier orden entre si.
    sort(comandasSemanales.begin(), comandasSemanales.end(),
         [](const Comanda& a, const Comanda& b) {
             return a.idMozo < b.idMozo;
         });

    char nombreSemanal[64];
    snprintf(nombreSemanal, sizeof(nombreSemanal),
             "comandas_semana_s%d-%02d.dat", numeroSemana, mes);

    FILE* archivoSemanal = fopen(nombreSemanal, "wb");
    if (archivoSemanal == NULL) {
        cout << "Error: no se pudo crear " << nombreSemanal << ".\n";
        return 1;
    }

    for (size_t i = 0; i < comandasSemanales.size(); ++i) {
        if (fwrite(&comandasSemanales[i], sizeof(Comanda), 1,
                   archivoSemanal) != 1) {
            cout << "Error al escribir el archivo semanal.\n";
            fclose(archivoSemanal);
            return 1;
        }
    }

    if (fclose(archivoSemanal) != 0) {
        cout << "Error al cerrar el archivo semanal.\n";
        return 1;
    }

    cout << "\nCierre semanal terminado correctamente.\n";
    cout << "Archivo generado: " << nombreSemanal << "\n";
    cout << "Planillas procesadas: " << cantidadDias << "\n";
    cout << "Comandas guardadas: " << comandasSemanales.size() << "\n";

    return 0;
}
