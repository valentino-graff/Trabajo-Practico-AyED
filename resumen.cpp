#include <iostream>
#include <cstdio>
#include <cstring>
using namespace std;

// Lee: comandas_semana_sX-mm.dat (y mozos.dat para mostrar el nombre)
// Genera: reporte por pantalla

struct Mozo
{
    int idMozo;
    char nombre[50];
    char password[20];
    float totalComision;
};

struct Comanda
{
    int idMozo;
    int codigoProducto;
    int cantidad;
    float comision;
};

// Acceso directo: el mozo con id N esta en la posicion N-1 de mozos.dat
void buscarNombreMozo(int idMozo, char nombre[])
{
    strcpy(nombre, "(sin nombre)");
    FILE *archivo = fopen("mozos.dat", "rb");
    if (archivo == NULL)
    {
        return;
    }
    Mozo mozo;
    fseek(archivo, (idMozo - 1) * sizeof(Mozo), SEEK_SET);
    if (fread(&mozo, sizeof(Mozo), 1, archivo) == 1)
    {
        strcpy(nombre, mozo.nombre);
    }
    fclose(archivo);
}

void mostrarMozo(int idMozo, int productos, float comision)
{
    char nombre[50];
    buscarNombreMozo(idMozo, nombre);
    cout << idMozo << "\t" << nombre << "\t" << productos << " productos\t$" << comision << endl;
}

int main()
{
    int semana, mes;
    cout << "Numero de semana: ";
    cin >> semana;
    cout << "Mes (1 a 12): ";
    cin >> mes;

    char nombreArchivo[40];
    sprintf(nombreArchivo, "comandas_semana_s%d-%02d.dat", semana, mes);

    FILE *archivo = fopen(nombreArchivo, "rb");
    if (archivo == NULL)
    {
        cout << "No existe " << nombreArchivo << " (corre primero cierre)" << endl;
        return 1;
    }

    cout << "\n===== Resumen del cierre (" << nombreArchivo << ") =====" << endl;
    cout << "Mozo\tNombre\t\tVendio\t\tComision" << endl;

    // Corte de control por idMozo: el archivo esta ordenado por mozo,
    // asi que las ventas de cada mozo estan todas juntas.
    int totalBuffet = 0;
    Comanda comanda;
    bool hayDatos = fread(&comanda, sizeof(Comanda), 1, archivo) == 1;
    while (hayDatos)
    {
        int mozoActual = comanda.idMozo;
        int productosMozo = 0;
        float comisionMozo = 0;

        while (hayDatos && comanda.idMozo == mozoActual)
        {
            productosMozo += comanda.cantidad;
            comisionMozo += comanda.comision;
            hayDatos = fread(&comanda, sizeof(Comanda), 1, archivo) == 1;
        }

        // se termino el mozo (o el archivo, en el caso del ultimo mozo)
        mostrarMozo(mozoActual, productosMozo, comisionMozo);
        totalBuffet += productosMozo;
    }
    fclose(archivo);

    cout << "------------------------------------------------" << endl;
    cout << "Total de productos vendidos por el buffet: " << totalBuffet << endl;
    return 0;
}
