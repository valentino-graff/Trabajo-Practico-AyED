
```cpp
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
// BUSCA UN MOZO POR ID
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
// BUSCA UN PRODUCTO POR CODIGO
//--------------------------------------------------
int buscarProducto(int codigoBuscado, Producto& producto) {

    FILE* f = fopen("inventario.dat", "rb");

    if (f == NULL) {
        return 0;
    }

    while (fread(&producto, sizeof(Producto), 1, f) == 1) {

        if (producto.codigo == codigoBuscado) {

            fclose(f);

            return 1;
        }
    }

    fclose(f);

    return 0;
}


//--------------------------------------------------
// AGREGA UNA COMANDA AL ARCHIVO DEL DIA
//--------------------------------------------------
int agregarComanda(char nombreArchivo[], Comanda nueva) {

    FILE* f = fopen(nombreArchivo, "ab");

    if (f == NULL) {

        cout << "No se pudo abrir la planilla del dia."
             << endl;

        return 0;
    }

    if (fwrite(&nueva, sizeof(Comanda), 1, f) != 1) {

        fclose(f);

        cout << "No se pudo guardar la venta."
             << endl;

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


    // Nos posicionamos al final del archivo

    fseek(f, 0, SEEK_END);

    long tamano = ftell(f);


    // Calculamos la cantidad de registros

    int cantidad =
        tamano / sizeof(Comanda);


    // Volvemos al comienzo

    fseek(f, 0, SEEK_SET);


    // Si hay 0 o 1 registros,
    // no hace falta ordenar

    if (cantidad <= 1) {

        fclose(f);

        return;
    }


    // Creamos un arreglo dinámico
    // con la cantidad exacta de comandas

    Comanda* comandas =
        new Comanda[cantidad];


    // Leemos las comandas

    int i = 0;

    while (i < cantidad &&
           fread(
               &comandas[i],
               sizeof(Comanda),
               1,
               f
           ) == 1) {

        i++;
    }

    fclose(f);


    // Ordenamiento burbuja por ID de mozo

    for (int i = 0; i < cantidad - 1; i++) {

        for (int j = 0;
             j < cantidad - 1 - i;
             j++) {

            if (comandas[j].idMozo >
                comandas[j + 1].idMozo) {

                Comanda aux;

                aux = comandas[j];

                comandas[j] =
                    comandas[j + 1];

                comandas[j + 1] =
                    aux;
            }
        }
    }


    // Abrimos nuevamente para sobrescribir
    // el archivo con el orden correcto

    f = fopen(nombreArchivo, "wb");

    if (f != NULL) {

        for (int i = 0; i < cantidad; i++) {

            fwrite(
                &comandas[i],
                sizeof(Comanda),
                1,
                f
            );
        }

        fclose(f);
    }


    // Liberamos la memoria dinámica

    delete[] comandas;
}


//--------------------------------------------------
// ACTUALIZA EL STOCK
//--------------------------------------------------
int actualizarStock(
    int codigoProducto,
    int cantidadVendida
) {

    FILE* f = fopen("inventario.dat", "rb+");

    if (f == NULL) {
        return 0;
    }

    Producto producto;

    while (fread(
               &producto,
               sizeof(Producto),
               1,
               f
           ) == 1) {

        if (producto.codigo == codigoProducto) {

            producto.stockActual -= cantidadVendida;


            // Volvemos al comienzo
            // del registro leído

            fseek(
                f,
                -(long)sizeof(Producto),
                SEEK_CUR
            );


            // Sobrescribimos el registro

            if (fwrite(
                    &producto,
                    sizeof(Producto),
                    1,
                    f
                ) != 1) {

                fclose(f);

                return 0;
            }

            fclose(f);

            return 1;
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


    // Armamos el nombre del archivo del dia

    char nombreArchivo[40];

    strcpy(
        nombreArchivo,
        "comandas_"
    );

    strcat(
        nombreArchivo,
        fecha
    );

    strcat(
        nombreArchivo,
        ".dat"
    );


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


        // Buscamos al mozo

        if (!buscarMozo(
                idMozo,
                mozo
            )) {

            cout << "El numero de mozo no existe."
                 << endl;
        }

        else {

            char claveEncriptada[20];

            encriptar(
                clave,
                claveEncriptada
            );


            // Comparamos la clave ingresada
            // con la almacenada

            if (strcmp(
                    claveEncriptada,
                    mozo.password
                ) != 0) {

                cout << "Clave incorrecta."
                     << endl;
            }

            else {

                cout << "Login correcto."
                     << endl;


                int codigoProducto;

                int cantidad;

                Producto producto;


                cout << "Ingrese codigo del producto: ";
                cin >> codigoProducto;


                // Buscamos el producto

                if (!buscarProducto(
                        codigoProducto,
                        producto
                    )) {

                    cout << "El producto no existe."
                         << endl;
                }

                else {

                    cout << "Producto: "
                         << producto.descripcion
                         << endl;

                    cout << "Precio: $"
                         << producto.precio
                         << endl;


                    cout << "Ingrese cantidad: ";
                    cin >> cantidad;


                    // Validamos la cantidad

                    if (cantidad <= 0) {

                        cout << "Cantidad invalida."
                             << endl;
                    }

                    else if (
                        cantidad >
                        producto.stockActual
                    ) {

                        cout << "No hay suficiente stock."
                             << endl;
                    }

                    else {

                        float totalVenta;

                        float comision;


                        totalVenta =
                            producto.precio * cantidad;

                        comision =
                            totalVenta *
                            TASA_COMISION;


                        // Armamos la nueva comanda

                        Comanda nueva;

                        nueva.idMozo =
                            mozo.idMozo;

                        nueva.codigoProducto =
                            producto.codigo;

                        nueva.cantidad =
                            cantidad;

                        nueva.comision =
                            comision;


                        // Primero guardamos la venta

                        if (agregarComanda(
                                nombreArchivo,
                                nueva
                            )) {


                            // Si se guardo correctamente,
                            // actualizamos el stock

                            if (actualizarStock(
                                    producto.codigo,
                                    cantidad
                                )) {

                                cout << endl;

                                cout << "Venta registrada."
                                     << endl;

                                cout << "Total venta: $"
                                     << totalVenta
                                     << endl;

                                cout << "Comision: $"
                                     << comision
                                     << endl;
                            }

                            else {

                                cout << "La venta fue guardada, "
                                     << "pero no se pudo actualizar "
                                     << "el stock."
                                     << endl;
                            }
                        }
                    }
                }
            }
        }


        cout << endl;

        cout << "Desea cargar otra venta? "
             << "(1 = SI / 0 = NO): ";

        cin >> continuar;
    }


    // Ordenamos la planilla del dia

    ordenarComandas(nombreArchivo);


    cout << endl;

    cout << "Planilla ordenada correctamente."
         << endl;


    return 0;
}
```
