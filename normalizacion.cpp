#include <cstdio>
#include <cstring>
using namespace std;

const int K = 5;   // para encriptar

// Se copian a la carpeta donde se ejecuta el programa
const char RUTA_INVENTARIO[] = "inventario.dat";
const char RUTA_HISTORICAS[] = "comandas_historicas.dat";

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

struct NodoMozo {
    Mozo dato;
    NodoMozo *sig;
};

struct NodoComanda {
    Comanda dato;
    NodoComanda *sig;
};

struct NodoDia {
    char fecha[11];
    NodoComanda *comandas;
    NodoDia *sig;
};

// Encriptar
void codificar(char clave[]) {
    for (int i = 0; clave[i] != '\0'; i++)
        clave[i] = clave[i] + K;
}

// Busca al mozo por nombre si no esta lo agrega al final
NodoMozo* obtenerMozo(NodoMozo *&lista, const char nombre[]) {
    NodoMozo *p = lista;
    NodoMozo *ult = NULL;

    while (p != NULL && strcmp(p->dato.nombre, nombre) != 0) {
        ult = p;
        p = p->sig;
    }
    if (p != NULL)
        return p;

    p = new NodoMozo;
    memset(&p->dato, 0, sizeof(Mozo));   
    p->dato.idMozo = (ult == NULL) ? 1 : ult->dato.idMozo + 1;
    strcpy(p->dato.nombre, nombre);
    sprintf(p->dato.password, "%d", p->dato.idMozo);   
    codificar(p->dato.password);
    p->sig = NULL;

    if (ult == NULL)
        lista = p;
    else
        ult->sig = p;
    return p;
}

// Busca la planilla del dia si no esta la crea al principio
NodoDia* obtenerDia(NodoDia *&lista, const char fecha[]) {
    NodoDia *p = lista;

    while (p != NULL && strcmp(p->fecha, fecha) != 0)
        p = p->sig;
    if (p != NULL)
        return p;

    p = new NodoDia;
    strcpy(p->fecha, fecha);
    p->comandas = NULL;
    p->sig = lista;
    lista = p;
    return p;
}

void insertarOrdenado(NodoComanda *&lista, Comanda c) {
    NodoComanda *nuevo = new NodoComanda;
    nuevo->dato = c;

    NodoComanda *ant = NULL;
    NodoComanda *p = lista;
    while (p != NULL && p->dato.idMozo <= c.idMozo) {
        ant = p;
        p = p->sig;
    }

    nuevo->sig = p;
    if (ant == NULL)
        lista = nuevo;
    else
        ant->sig = nuevo;
}

// Busqueda binaria
int buscarProducto(Producto v[], int cant, int codigo) {
    int ini = 0, fin = cant - 1;

    while (ini <= fin) {
        int medio = (ini + fin) / 2;
        if (v[medio].codigo == codigo)
            return medio;
        else if (v[medio].codigo < codigo)
            ini = medio + 1;
        else
            fin = medio - 1;
    }
    return -1;
}

bool guardarMozos(NodoMozo *lista) {
    FILE *arch = fopen("mozos.dat", "wb");
    if (arch == NULL) {
        perror("mozos.dat");
        return false;
    }
    for (NodoMozo *m = lista; m != NULL; m = m->sig)
        fwrite(&m->dato, sizeof(Mozo), 1, arch);
    fclose(arch);
    return true;
}

bool guardarDias(NodoDia *lista) {
    for (NodoDia *d = lista; d != NULL; d = d->sig) {
        char nombreArchivo[40];
        sprintf(nombreArchivo, "comandas_%s.dat", d->fecha);

        FILE *arch = fopen(nombreArchivo, "wb");
        if (arch == NULL) {
            perror(nombreArchivo);
            return false;
        }
        for (NodoComanda *c = d->comandas; c != NULL; c = c->sig)
            fwrite(&c->dato, sizeof(Comanda), 1, arch);
        fclose(arch);
    }
    return true;
}

void liberar(NodoMozo *mozos, NodoDia *dias) {
    while (mozos != NULL) {
        NodoMozo *aux = mozos;
        mozos = mozos->sig;
        delete aux;
    }
    while (dias != NULL) {
        while (dias->comandas != NULL) {
            NodoComanda *aux = dias->comandas;
            dias->comandas = aux->sig;
            delete aux;
        }
        NodoDia *aux = dias;
        dias = dias->sig;
        delete aux;
    }
}

int main() {
    ComandaHistorica venta;
    NodoMozo *mozos = NULL;
    NodoDia *dias = NULL;

    FILE *inv = fopen(RUTA_INVENTARIO, "r+b");
    if (inv == NULL) {
        perror(RUTA_INVENTARIO);
        return 1;
    }

    FILE *hist = fopen(RUTA_HISTORICAS, "rb");
    if (hist == NULL) {
        perror(RUTA_HISTORICAS);
        fclose(inv);
        return 1;
    }

    // Leer el inventario
    fseek(inv, 0, SEEK_END);
    int cantProductos = ftell(inv) / sizeof(Producto);
    rewind(inv);

    Producto *productos = new Producto[cantProductos];
    int n = 0;
    while (n < cantProductos && fread(&productos[n], sizeof(Producto), 1, inv) == 1)
        n++;

    // Procesar las ventas historicas
    while (fread(&venta, sizeof(ComandaHistorica), 1, hist) == 1) {

        NodoMozo *mozo = obtenerMozo(mozos, venta.nombreMozo);
        mozo->dato.totalComision += venta.comision;

        NodoDia *dia = obtenerDia(dias, venta.fecha);
        Comanda c = {mozo->dato.idMozo, venta.codigoProducto,
                     venta.cantidad, venta.comision};
        insertarOrdenado(dia->comandas, c);

        int pos = buscarProducto(productos, cantProductos, venta.codigoProducto);
        if (pos == -1) {
            printf("No se encontro el producto %d.\n", venta.codigoProducto);
        } else if (productos[pos].stockActual >= venta.cantidad) {
            productos[pos].stockActual -= venta.cantidad;
        } else {
            printf("Sin stock suficiente del producto %d (hay %d, se vendieron %d).\n",
                   venta.codigoProducto, productos[pos].stockActual, venta.cantidad);
            productos[pos].stockActual = 0;
        }
    }
    fclose(hist);

    // Guardar mozos y planillas si salio bien actualizar el inventario
    bool ok = guardarMozos(mozos) && guardarDias(dias);

    if (ok) {
        rewind(inv);
        fwrite(productos, sizeof(Producto), cantProductos, inv);
        printf("Normalizacion terminada correctamente.\n");
    }

    fclose(inv);
    liberar(mozos, dias);
    delete[] productos;
    return ok ? 0 : 1;
}