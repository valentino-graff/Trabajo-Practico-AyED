#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <cstdio>
#include <cstring>

using namespace std;

const float TASA_COMISION = 0.10f;
const int K = 5;

struct Mozo {
    int idMozo;
    char nombre[50];
    char password[20];
    float totalComision;
};

struct Producto {
    int codigo;
    char descripcion[50];
    float precio;
    int stockActual;
};

struct Comanda {
    int idMozo;
    int codigoProducto;
    int cantidad;
    float comision;
};


//--------------------------------------------------
// ENCRIPTA LA CLAVE
//--------------------------------------------------
void encriptar(char clave[], char claveEncriptada[]) {
    int i = 0;
    while (clave[i] != '\0') {
        claveEncriptada[i] = clave[i] + K;
        i++;
    }
    claveEncriptada[i] = '\0';
}


//--------------------------------------------------
// BUSCA UN MOZO POR ID (Secuencial)
//--------------------------------------------------
int buscarMozo(int idBuscado, Mozo& mozo) {
    FILE* f = fopen("mozos.dat", "rb");
    if (f == NULL) {
        return 0;
    }

    while (fread(&mozo, sizeof(Mozo), 1, f) == 1) {
        if (mozo.idMozo == idBuscado) {
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}


//--------------------------------------------------
// BUSCA UN PRODUCTO POR CODIGO (Búsqueda Binaria)
//--------------------------------------------------
int buscarProducto(int codigoBuscado, Producto& producto) {
    FILE* f = fopen("inventario.dat", "rb");
    if (f == NULL) {
        return 0;
    }

    // Calculamos la cantidad total de registros en el archivo
    fseek(f, 0, SEEK_END);
    long tamano = ftell(f);
    int cantidad = tamano / sizeof(Producto);

    int inicio = 0;
    int fin = cantidad - 1;

    while (inicio <= fin) {
        int medio = inicio + (fin - inicio) / 2;

        // Nos posicionamos en el registro medio
        fseek(f, (long)medio * sizeof(Producto), SEEK_SET);
        fread(&producto, sizeof(Producto), 1, f);

        if (producto.codigo == codigoBuscado) {
            fclose(f);
            return 1; // Encontrado
        }
        else if (producto.codigo < codigoBuscado) {
            inicio = medio + 1; // Buscar en la mitad derecha
        }
        else {
            fin = medio - 1; // Buscar en la mitad izquierda
        }
    }

    fclose(f);
    return 0; // No encontrado
}


//--------------------------------------------------
// AGREGA UNA COMANDA AL ARCHIVO DEL DIA
//--------------------------------------------------
int agregarComanda(char nombreArchivo[], Comanda nueva) {
    FILE* f = fopen(nombreArchivo, "ab");
    if (f == NULL) {
        cout << "No se pudo abrir la planilla del dia." << endl;
        return 0;
    }

    if (fwrite(&nueva, sizeof(Comanda), 1, f) != 1) {
        fclose(f);
        cout << "No se pudo guardar la venta." << endl;
        return 0;
    }

    fclose(f);
    return 1;
}


//--------------------------------------------------
// ORDENA LAS COMANDAS POR ID DE MOZO
//--------------------------------------------------
void ordenarComandas(char nombreArchivo[]) {
    FILE* f = fopen(nombreArchivo, "rb");
    if (f == NULL) {
        return;
    }

    fseek(f, 0, SEEK_END);
    long tamano = ftell(f);
    int cantidad = tamano / sizeof(Comanda);
    fseek(f, 0, SEEK_SET);

    if (cantidad <= 1) {
        fclose(f);
        return;
    }

    Comanda* comandas = new Comanda[cantidad];
    int i = 0;
    while (i < cantidad && fread(&comandas[i], sizeof(Comanda), 1, f) == 1) {
        i++;
    }
    fclose(f);

    // Ordenamiento burbuja por ID de mozo
    for (int a = 0; a < cantidad - 1; a++) {
        for (int b = 0; b < cantidad - 1 - a; b++) {
            if (comandas[b].idMozo > comandas[b + 1].idMozo) {
                Comanda aux = comandas[b];
                comandas[b] = comandas[b + 1];
                comandas[b + 1] = aux;
            }
        }
    }

    f = fopen(nombreArchivo, "wb");
    if (f != NULL) {
        for (int a = 0; a < cantidad; a++) {
            fwrite(&comandas[a], sizeof(Comanda), 1, f);
        }
        fclose(f);
    }

    delete[] comandas;
}


//--------------------------------------------------
// ACTUALIZA EL STOCK (Búsqueda Binaria para actualizar)
//--------------------------------------------------
int actualizarStock(int codigoProducto, int cantidadVendida) {
    FILE* f = fopen("inventario.dat", "rb+");
    if (f == NULL) {
        return 0;
    }

    fseek(f, 0, SEEK_END);
    long tamano = ftell(f);
    int cantidad = tamano / sizeof(Producto);

    int inicio = 0;
    int fin = cantidad - 1;
    Producto producto;

    while (inicio <= fin) {
        int medio = inicio + (fin - inicio) / 2;

        fseek(f, (long)medio * sizeof(Producto), SEEK_SET);
        fread(&producto, sizeof(Producto), 1, f);

        if (producto.codigo == codigoProducto) {
            producto.stockActual -= cantidadVendida;

            // Volvemos al inicio del registro encontrado para sobrescribir
            fseek(f, (long)medio * sizeof(Producto), SEEK_SET);

            if (fwrite(&producto, sizeof(Producto), 1, f) != 1) {
                fclose(f);
                return 0;
            }

            fclose(f);
            return 1;
        }
        else if (producto.codigo < codigoProducto) {
            inicio = medio + 1;
        }
        else {
            fin = medio - 1;
        }
    }

    fclose(f);
    return 0;
}


//--------------------------------------------------
// MAIN
//--------------------------------------------------
int main() {
    char fecha[11];
    cout << "Ingrese la fecha (DD-MM-AAAA): ";
    cin >> fecha;

    char nombreArchivo[40];
    strcpy(nombreArchivo, "comandas_");
    strcat(nombreArchivo, fecha);
    strcat(nombreArchivo, ".dat");

    int continuar = 1;

    while (continuar == 1) {
        int idMozo;
        char clave[20];
        Mozo mozo;

        cout << endl;
        cout << "Ingrese numero de mozo: ";
        cin >> idMozo;
        cout << "Ingrese clave: ";
        cin >> clave;

        if (!buscarMozo(idMozo, mozo)) {
            cout << "El numero de mozo no existe." << endl;
        }
        else {
            char claveEncriptada[20];
            encriptar(clave, claveEncriptada);

            if (strcmp(claveEncriptada, mozo.password) != 0) {
                cout << "Clave incorrecta." << endl;
            }
            else {
                cout << "Login correcto." << endl;

                int codigoProducto;
                int cantidad;
                Producto producto;

                cout << "Ingrese codigo del producto: ";
                cin >> codigoProducto;

                if (!buscarProducto(codigoProducto, producto)) {
                    cout << "El producto no existe." << endl;
                }
                else {
                    cout << "Producto: " << producto.descripcion << endl;
                    cout << "Precio: $" << producto.precio << endl;

                    cout << "Ingrese cantidad: ";
                    cin >> cantidad;

                    if (cantidad <= 0) {
                        cout << "Cantidad invalida." << endl;
                    }
                    else if (cantidad > producto.stockActual) {
                        cout << "No hay suficiente stock." << endl;
                    }
                    else {
                        float totalVenta = producto.precio * cantidad;
                        float comision = totalVenta * TASA_COMISION;

                        Comanda nueva;
                        nueva.idMozo = mozo.idMozo;
                        nueva.codigoProducto = producto.codigo;
                        nueva.cantidad = cantidad;
                        nueva.comision = comision;

                        if (agregarComanda(nombreArchivo, nueva)) {
                            if (actualizarStock(producto.codigo, cantidad)) {
                                cout << endl;
                                cout << "Venta registrada." << endl;
                                cout << "Total venta: $" << totalVenta << endl;
                                cout << "Comision: $" << comision << endl;
                            }
                            else {
                                cout << "La venta fue guardada, pero no se pudo actualizar el stock." << endl;
                            }
                        }
                    }
                }
            }
        }

        cout << endl;
        cout << "Desea cargar otra venta? (1 = SI / 0 = NO): ";
        cin >> continuar;
    }

    ordenarComandas(nombreArchivo);

    cout << endl;
    cout << "Planilla ordenada correctamente." << endl;

    return 0;
}
