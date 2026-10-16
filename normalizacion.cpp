#include <iostream>
#include <cstdio>
#include <cstring>
using namespace std;

//corrimiento
const int K = 5;

struct ComandaHistorica {
    char fecha[11];
    char nombreMozo[50];
    int codigoProducto;
    int cantidad;
    float comision;
};

struct Producto {
    int codigo;
    char descripcion[50];
    float precio;
    int stockActual;
};

struct Mozo {
    int idMozo;
    char nombre[50];
    char password[20];
    float totalComision;
};

struct Comanda {
    int idMozo;
    int codigoProducto;
    int cantidad;
    float comision;
};

struct Fecha {
    char texto[11];  
};

// Encriptar sumando K 
void codificar(char clave[]) {
    for (int i = 0; clave[i] != '\0'; i++)
        clave[i] = clave[i] + K;
}

// Si el archivo no esta en esta carpeta, lo copia desde datos/ o ../datos/
bool prepararArchivo(const char nombre[]) {
    FILE *f = fopen(nombre, "rb");
    if (f != NULL) {
        fclose(f);
        return true;
    }

    const char *carpetas[] = {"datos/", "../datos/"};
    for (int i = 0; i < 2; i++) {
        char origen[80];
        strcpy(origen, carpetas[i]);
        strcat(origen, nombre);

        FILE *in = fopen(origen, "rb");
        if (in != NULL) {
            FILE *out = fopen(nombre, "wb");
            if (out == NULL) {
                fclose(in);
                return false;
            }
            char c;
            while (fread(&c, 1, 1, in) == 1)
                fwrite(&c, 1, 1, out);
            fclose(in);
            fclose(out);
            cout << "Copiado " << origen << endl;
            return true;
        }
    }

    cout << "No se encontro " << nombre << endl;
    return false;
}

// Devuelve la posicion del mozo con ese nombre o -1 si no esta
int buscarMozo(Mozo mozos[], int cant, const char nombre[]) {
    for (int i = 0; i < cant; i++) {
        if (strcmp(mozos[i].nombre, nombre) == 0)
            return i;
    }
    return -1;
}

// Devuelve la posicion de la fecha o -1 si no esta
int buscarFecha(Fecha fechas[], int cant, const char fecha[]) {
    for (int i = 0; i < cant; i++) {
        if (strcmp(fechas[i].texto, fecha) == 0)
            return i;
    }
    return -1;
}

// Busqueda binaria por codigo
int buscarProducto(Producto productos[], int cant, int codigo) {
    int inicio = 0;
    int fin = cant - 1;

    while (inicio <= fin) {
        int medio = (inicio + fin) / 2;

        if (productos[medio].codigo == codigo)
            return medio;
        else if (productos[medio].codigo < codigo)
            inicio = medio + 1;
        else
            fin = medio - 1;
    }
    return -1;
}

// Ordena las comandas por idMozo
void ordenarComandas(Comanda comandas[], int cant) {
    for (int i = 0; i < cant - 1; i++) {
        for (int j = 0; j < cant - 1 - i; j++) {
            if (comandas[j].idMozo > comandas[j + 1].idMozo) {
                Comanda aux = comandas[j];
                comandas[j] = comandas[j + 1];
                comandas[j + 1] = aux;
            }
        }
    }
}

int main() {
    if (!prepararArchivo("inventario.dat") ||
        !prepararArchivo("comandas_historicas.dat"))
        return 1;

    FILE *arch;

    //Leer las ventas historicas
    arch = fopen("comandas_historicas.dat", "rb");
    if (arch == NULL) {
        cout << "No se pudo abrir comandas_historicas.dat" << endl;
        return 1;
    }

    fseek(arch, 0, SEEK_END);
    int cantVentas = ftell(arch) / sizeof(ComandaHistorica);
    rewind(arch);

    ComandaHistorica *ventas = new ComandaHistorica[cantVentas];
    int i = 0;
    while (i < cantVentas && fread(&ventas[i], sizeof(ComandaHistorica), 1, arch) == 1)
        i++;
    fclose(arch);

    //Leer el inventario
    arch = fopen("inventario.dat", "rb");
    if (arch == NULL) {
        cout << "No se pudo abrir inventario.dat" << endl;
        return 1;
    }

    fseek(arch, 0, SEEK_END);
    int cantProductos = ftell(arch) / sizeof(Producto);
    rewind(arch);

    Producto *productos = new Producto[cantProductos];
    i = 0;
    while (i < cantProductos && fread(&productos[i], sizeof(Producto), 1, arch) == 1)
        i++;
    fclose(arch);

    // 3. Armar la lista de mozos y de fechas, y descontar el stock
    Mozo *mozos = new Mozo[cantVentas];
    Fecha *fechas = new Fecha[cantVentas];
    int cantMozos = 0;
    int cantFechas = 0;

    for (i = 0; i < cantVentas; i++) {

        // Mozo: si no esta, se agrega con el id siguiente
        int posMozo = buscarMozo(mozos, cantMozos, ventas[i].nombreMozo);
        if (posMozo == -1) {
            posMozo = cantMozos;
            memset(&mozos[posMozo], 0, sizeof(Mozo));
            mozos[posMozo].idMozo = cantMozos + 1;
            strcpy(mozos[posMozo].nombre, ventas[i].nombreMozo);
            sprintf(mozos[posMozo].password, "%d", mozos[posMozo].idMozo);
            codificar(mozos[posMozo].password);
            cantMozos++;
        }
        mozos[posMozo].totalComision += ventas[i].comision;

        // Fecha: si no esta, se agrega
        if (buscarFecha(fechas, cantFechas, ventas[i].fecha) == -1) {
            strcpy(fechas[cantFechas].texto, ventas[i].fecha);
            cantFechas++;
        }

        // Stock
        int posProd = buscarProducto(productos, cantProductos, ventas[i].codigoProducto);
        if (posProd == -1) {
            cout << "No se encontro el producto " << ventas[i].codigoProducto << endl;
        } else if (productos[posProd].stockActual >= ventas[i].cantidad) {
            productos[posProd].stockActual -= ventas[i].cantidad;
        } else {
            cout << "Sin stock suficiente del producto " << ventas[i].codigoProducto << endl;
            productos[posProd].stockActual = 0;
        }
    }

    // 4. Guardar los mozos
    arch = fopen("mozos.dat", "wb");
    if (arch == NULL) {
        cout << "No se pudo crear mozos.dat" << endl;
        return 1;
    }
    for (i = 0; i < cantMozos; i++)
        fwrite(&mozos[i], sizeof(Mozo), 1, arch);
    fclose(arch);

    //Guardar la planilla de cada dia
    Comanda *comandas = new Comanda[cantVentas];

    for (i = 0; i < cantFechas; i++) {

        //Juntar las comandas de esta fecha
        int cantComandas = 0;
        for (int j = 0; j < cantVentas; j++) {
            if (strcmp(ventas[j].fecha, fechas[i].texto) == 0) {
                int posMozo = buscarMozo(mozos, cantMozos, ventas[j].nombreMozo);

                comandas[cantComandas].idMozo = mozos[posMozo].idMozo;
                comandas[cantComandas].codigoProducto = ventas[j].codigoProducto;
                comandas[cantComandas].cantidad = ventas[j].cantidad;
                comandas[cantComandas].comision = ventas[j].comision;
                cantComandas++;
            }
        }

        ordenarComandas(comandas, cantComandas);

        char nombreArchivo[40];
        sprintf(nombreArchivo, "comandas_%s.dat", fechas[i].texto);

        arch = fopen(nombreArchivo, "wb");
        if (arch == NULL) {
            cout << "No se pudo crear " << nombreArchivo << endl;
            return 1;
        }
        for (int j = 0; j < cantComandas; j++)
            fwrite(&comandas[j], sizeof(Comanda), 1, arch);
        fclose(arch);
    }

    //Actualizar el inventario
    arch = fopen("inventario.dat", "r+b");
    if (arch == NULL) {
        cout << "No se pudo actualizar inventario.dat" << endl;
        return 1;
    }
    for (i = 0; i < cantProductos; i++) {
        fseek(arch, i * sizeof(Producto), SEEK_SET);
        fwrite(&productos[i], sizeof(Producto), 1, arch);
    }
    fclose(arch);

    delete[] ventas;
    delete[] productos;
    delete[] mozos;
    delete[] fechas;
    delete[] comandas;

    cout << "Normalizacion terminada correctamente." << endl;
    return 0;
}
