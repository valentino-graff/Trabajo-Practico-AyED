#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <cstdio>
#include <cstring>

using namespace std;

struct Comanda {
    int idMozo;
    int codigoProducto;
    int cantidad;
    float comision;
};

int leerComanda(FILE* archivo, Comanda& comanda) {
    return fread(&comanda, sizeof(Comanda), 1, archivo) == 1;
}

// Aparea dos archivos previamente ordenados por idMozo.
int aparearArchivos(const char* archivoA, const char* archivoB,
                    const char* archivoSalida) {
    FILE* a = fopen(archivoA, "rb");
    if (a == NULL) {
        cout << "No se pudo abrir " << archivoA << endl;
        return 0;
    }

    FILE* b = fopen(archivoB, "rb");
    if (b == NULL) {
        cout << "No se pudo abrir " << archivoB << endl;
        fclose(a);
        return 0;
    }

    FILE* salida = fopen(archivoSalida, "wb");
    if (salida == NULL) {
        cout << "No se pudo crear " << archivoSalida << endl;
        fclose(a);
        fclose(b);
        return 0;
    }

    Comanda ca, cb;
    int hayA = leerComanda(a, ca);
    int hayB = leerComanda(b, cb);
    int correcto = 1;

    while (hayA && hayB && correcto) {
        if (ca.idMozo <= cb.idMozo) {
            correcto = fwrite(&ca, sizeof(Comanda), 1, salida) == 1;
            hayA = leerComanda(a, ca);
        } else {
            correcto = fwrite(&cb, sizeof(Comanda), 1, salida) == 1;
            hayB = leerComanda(b, cb);
        }
    }

    while (hayA && correcto) {
        correcto = fwrite(&ca, sizeof(Comanda), 1, salida) == 1;
        hayA = leerComanda(a, ca);
    }

    while (hayB && correcto) {
        correcto = fwrite(&cb, sizeof(Comanda), 1, salida) == 1;
        hayB = leerComanda(b, cb);
    }

    fclose(a);
    fclose(b);
    fclose(salida);

    if (!correcto) {
        cout << "Error al escribir " << archivoSalida << endl;
        return 0;
    }
    return 1;
}

int copiarArchivo(const char* origen, const char* destino) {
    FILE* entrada = fopen(origen, "rb");
    if (entrada == NULL) {
        cout << "No se pudo abrir " << origen << endl;
        return 0;
    }

    FILE* salida = fopen(destino, "wb");
    if (salida == NULL) {
        cout << "No se pudo crear " << destino << endl;
        fclose(entrada);
        return 0;
    }

    Comanda c;
    int correcto = 1;
    while (fread(&c, sizeof(Comanda), 1, entrada) == 1) {
        if (fwrite(&c, sizeof(Comanda), 1, salida) != 1) {
            correcto = 0;
            break;
        }
    }

    fclose(entrada);
    fclose(salida);
    return correcto;
}

int main() {
    int semana, mes;
    cout << "Ingrese numero de semana (1 a 5): ";
    cin >> semana;
    cout << "Ingrese mes (1 a 12): ";
    cin >> mes;

    if (semana < 1 || semana > 5 || mes < 1 || mes > 12) {
        cout << "Semana o mes invalido." << endl;
        return 1;
    }

    char nombreSalida[50];
    sprintf(nombreSalida, "comandas_semana_s%d-%02d.dat", semana, mes);

    char fecha[11];
    char archivoAnterior[50];
    char archivoActual[50];

    cout << "Ingrese la fecha del primer dia (DD-MM-AAAA): ";
    cin >> fecha;
    strcpy(archivoAnterior, "comandas_");
    strcat(archivoAnterior, fecha);
    strcat(archivoAnterior, ".dat");

    const char* auxiliar = "comandas_auxiliar.dat";
    const char* resultado = "comandas_resultado.dat";

    if (!copiarArchivo(archivoAnterior, auxiliar)) {
        return 1;
    }

    // Se aparean las siete planillas, una por una.
    for (int dia = 2; dia <= 7; dia++) {
        cout << "Ingrese la fecha del dia " << dia << " (DD-MM-AAAA): ";
        cin >> fecha;

        strcpy(archivoActual, "comandas_");
        strcat(archivoActual, fecha);
        strcat(archivoActual, ".dat");

        if (!aparearArchivos(auxiliar, archivoActual, resultado)) {
            remove(auxiliar);
            remove(resultado);
            return 1;
        }

        if (remove(auxiliar) != 0) {
            cout << "No se pudo eliminar el auxiliar anterior." << endl;
            remove(resultado);
            return 1;
        }

        if (rename(resultado, auxiliar) != 0) {
            cout << "No se pudo renombrar el resultado del apareo." << endl;
            return 1;
        }
    }

    // El auxiliar final ya contiene todos los registros ordenados por idMozo.
    remove(nombreSalida);
    if (rename(auxiliar, nombreSalida) != 0) {
        cout << "No se pudo crear el archivo semanal." << endl;
        return 1;
    }

    cout << "Cierre semanal generado correctamente: " << nombreSalida << endl;
    return 0;
}
