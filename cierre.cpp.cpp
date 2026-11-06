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

//--------------------------------------------------
// VALIDA UNA FECHA REAL CON FORMATO DD-MM-AAAA
//--------------------------------------------------
int fechaValida(const char fecha[]) {
    if (strlen(fecha) != 10 || fecha[2] != '-' || fecha[5] != '-') {
        return 0;
    }

    for (int i = 0; i < 10; i++) {
        if (i != 2 && i != 5 &&
            (fecha[i] < '0' || fecha[i] > '9')) {
            return 0;
        }
    }

    int dia = (fecha[0] - '0') * 10 + (fecha[1] - '0');
    int mes = (fecha[3] - '0') * 10 + (fecha[4] - '0');
    int anio = (fecha[6] - '0') * 1000 +
              (fecha[7] - '0') * 100 +
              (fecha[8] - '0') * 10 +
              (fecha[9] - '0');

    if (anio < 1 || mes < 1 || mes > 12) {
        return 0;
    }

    int diasMes[12] = { 31, 28, 31, 30, 31, 30,
                        31, 31, 30, 31, 30, 31 };

    // Se contempla si el año es bisiesto.
    if ((anio % 400 == 0) || (anio % 4 == 0 && anio % 100 != 0)) {
        diasMes[1] = 29;
    }

    return dia >= 1 && dia <= diasMes[mes - 1];
}

//--------------------------------------------------
// OBTIENE EL MES Y EL ANIO DE UNA FECHA
//--------------------------------------------------
int obtenerMes(const char fecha[]) {
    return (fecha[3] - '0') * 10 + (fecha[4] - '0');
}

int obtenerAnio(const char fecha[]) {
    return (fecha[6] - '0') * 1000 +
           (fecha[7] - '0') * 100 +
           (fecha[8] - '0') * 10 +
           (fecha[9] - '0');
}

//--------------------------------------------------
// VERIFICA QUE UNA PLANILLA ESTE ORDENADA POR IDMOZO
//--------------------------------------------------
int archivoOrdenado(const char nombreArchivo[]) {
    FILE* f = fopen(nombreArchivo, "rb");
    if (f == NULL) {
        return -1;
    }

    // Un archivo con bytes incompletos no contiene registros validos.
    fseek(f, 0, SEEK_END);
    long tamanio = ftell(f);
    if (tamanio % sizeof(Comanda) != 0) {
        fclose(f);
        return 0;
    }
    rewind(f);

    Comanda actual;
    int hayAnterior = 0;
    int idAnterior = 0;
    int ordenado = 1;

    while (fread(&actual, sizeof(Comanda), 1, f) == 1) {
        if (hayAnterior && actual.idMozo < idAnterior) {
            ordenado = 0;
            break;
        }
        idAnterior = actual.idMozo;
        hayAnterior = 1;
    }

    fclose(f);
    return ordenado;
}

//--------------------------------------------------
// APAREA DOS ARCHIVOS ORDENADOS POR IDMOZO
//--------------------------------------------------
int aparear(const char nombreA[], const char nombreB[],
            const char nombreSalida[]) {
    FILE* a = fopen(nombreA, "rb");
    if (a == NULL) {
        cout << "No se pudo abrir el archivo auxiliar." << endl;
        return 0;
    }

    FILE* b = fopen(nombreB, "rb");
    if (b == NULL) {
        fclose(a);
        cout << "No se pudo abrir la planilla diaria." << endl;
        return 0;
    }

    FILE* salida = fopen(nombreSalida, "wb");
    if (salida == NULL) {
        fclose(a);
        fclose(b);
        cout << "No se pudo crear el archivo resultado." << endl;
        return 0;
    }

    Comanda ca, cb;
    int hayA = fread(&ca, sizeof(Comanda), 1, a) == 1;
    int hayB = fread(&cb, sizeof(Comanda), 1, b) == 1;
    int correcto = 1;

    while (hayA && hayB && correcto) {
        if (ca.idMozo <= cb.idMozo) {
            correcto = fwrite(&ca, sizeof(Comanda), 1, salida) == 1;
            hayA = fread(&ca, sizeof(Comanda), 1, a) == 1;
        } else {
            correcto = fwrite(&cb, sizeof(Comanda), 1, salida) == 1;
            hayB = fread(&cb, sizeof(Comanda), 1, b) == 1;
        }
    }

    // Se copian los registros que hayan quedado en el archivo A.
    while (hayA && correcto) {
        correcto = fwrite(&ca, sizeof(Comanda), 1, salida) == 1;
        hayA = fread(&ca, sizeof(Comanda), 1, a) == 1;
    }

    // Se copian los registros que hayan quedado en el archivo B.
    while (hayB && correcto) {
        correcto = fwrite(&cb, sizeof(Comanda), 1, salida) == 1;
        hayB = fread(&cb, sizeof(Comanda), 1, b) == 1;
    }

    fclose(a);
    fclose(b);
    fclose(salida);

    if (!correcto) {
        cout << "Error al escribir el archivo resultado." << endl;
        return 0;
    }

    return 1;
}

//--------------------------------------------------
// LEE UNA FECHA SIN PERMITIR DESBORDAR EL ARREGLO
//--------------------------------------------------
void leerFecha(char fecha[]) {
    cin.width(100);
    cin >> fecha;
    cin.ignore(1000, '\n');
}

//--------------------------------------------------
// MAIN
//--------------------------------------------------
int main() {
    int semana, mes, cantidad;

    cout << "Ingrese numero de semana (1 a 5): ";
    if (!(cin >> semana) || semana < 1 || semana > 5) {
        cout << "Numero de semana invalido." << endl;
        return 1;
    }

    cout << "Ingrese mes (1 a 12): ";
    if (!(cin >> mes) || mes < 1 || mes > 12) {
        cout << "Mes invalido." << endl;
        return 1;
    }

    cout << "Ingrese cantidad de dias a considerar (1 a 7): ";
    if (!(cin >> cantidad) || cantidad < 1 || cantidad > 7) {
        cout << "La cantidad debe estar entre 1 y 7." << endl;
        return 1;
    }

    char fechas[7][11];
    int anioBase = 0;

    // Se validan todas las fechas y se impiden las fechas repetidas.
    for (int i = 0; i < cantidad; i++) {
        int fechaCorrecta = 0;

        while (!fechaCorrecta) {
            char entrada[100];

            cout << "Ingrese fecha " << i + 1 << " (DD-MM-AAAA): ";
            leerFecha(entrada);

            if (!fechaValida(entrada)) {
                cout << "Fecha inexistente o formato incorrecto." << endl;
                continue;
            }

            if (obtenerMes(entrada) != mes) {
                cout << "La fecha no pertenece al mes seleccionado." << endl;
                continue;
            }

            if (i == 0) {
                anioBase = obtenerAnio(entrada);
            } else if (obtenerAnio(entrada) != anioBase) {
                cout << "Todas las fechas deben pertenecer al mismo anio." << endl;
                continue;
            }

            int repetida = 0;
            for (int j = 0; j < i; j++) {
                if (strcmp(entrada, fechas[j]) == 0) {
                    repetida = 1;
                    break;
                }
            }

            if (repetida) {
                cout << "Esa fecha ya fue ingresada. Ingrese otra." << endl;
                continue;
            }

            strcpy(fechas[i], entrada);
            fechaCorrecta = 1;
        }
    }

    const char* auxiliar = "comandas_auxiliar.tmp";
    const char* resultado = "comandas_resultado.tmp";

    char nombreSemanal[50];
    sprintf(nombreSemanal, "comandas_semana_s%d-%02d.dat", semana, mes);

    // Se eliminan auxiliares de una ejecucion anterior, si quedaron.
    remove(auxiliar);
    remove(resultado);

    // El archivo auxiliar comienza vacio.
    FILE* inicio = fopen(auxiliar, "wb");
    if (inicio == NULL) {
        cout << "No se pudo crear el archivo auxiliar." << endl;
        return 1;
    }
    fclose(inicio);

    int planillasProcesadas = 0;
    int diasSinPlanilla = 0;

    for (int i = 0; i < cantidad; i++) {
        char nombreDiario[50];
        strcpy(nombreDiario, "comandas_");
        strcat(nombreDiario, fechas[i]);
        strcat(nombreDiario, ".dat");

        FILE* prueba = fopen(nombreDiario, "rb");

        if (prueba == NULL) {
            int confirmar;
            cout << "No existe " << nombreDiario << "." << endl;
            cout << "Confirmar que ese dia no hubo ventas? (1 = SI / 0 = NO): ";
            if (!(cin >> confirmar)) {
                remove(auxiliar);
                cout << "Entrada invalida." << endl;
                return 1;
            }

            if (confirmar != 1) {
                remove(auxiliar);
                cout << "Cierre cancelado. Vuelva a ejecutar e ingrese la fecha correcta."
                     << endl;
                return 1;
            }

            diasSinPlanilla++;
            continue;
        }
        fclose(prueba);

        int orden = archivoOrdenado(nombreDiario);
        if (orden == -1) {
            remove(auxiliar);
            cout << "No se pudo abrir " << nombreDiario << "." << endl;
            return 1;
        }
        if (!orden) {
            remove(auxiliar);
            cout << "La planilla " << nombreDiario
                 << " no esta ordenada por idMozo o tiene datos incompletos." << endl;
            cout << "No se puede realizar el apareo." << endl;
            return 1;
        }

        if (!aparear(auxiliar, nombreDiario, resultado)) {
            remove(auxiliar);
            remove(resultado);
            return 1;
        }

        if (remove(auxiliar) != 0) {
            cout << "No se pudo reemplazar el archivo auxiliar." << endl;
            remove(resultado);
            return 1;
        }

        if (rename(resultado, auxiliar) != 0) {
            cout << "No se pudo renombrar el resultado del apareo." << endl;
            remove(resultado);
            return 1;
        }

        planillasProcesadas++;
    }

    // El auxiliar contiene el acumulado semanal, incluso si quedo vacio.
    remove(nombreSemanal);
    if (rename(auxiliar, nombreSemanal) != 0) {
        cout << "No se pudo generar el archivo semanal." << endl;
        remove(auxiliar);
        return 1;
    }

    cout << endl << "Cierre semanal generado correctamente." << endl;
    cout << "Planillas procesadas: " << planillasProcesadas << endl;
    cout << "Dias confirmados sin planilla: " << diasSinPlanilla << endl;
    cout << "Archivo generado: " << nombreSemanal << endl;

    return 0;
}
