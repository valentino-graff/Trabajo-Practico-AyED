Trabajo Práctico grupal de Algoritmos y Estructuras de Datos (UTN FRBA), unidad de archivos binarios en C/C++.

**Grupo: B**

**Integrantes:**

| Nombre             | Usuario de GitHub |

| Valentino Graff    | valentino_graff   |
| (completar)        | (completar)       |
| (completar)        | (completar)       |
| (completar)        | (completar)       |

## Quién hizo cada parte

| Programa | Responsable |
|---|---|
| `normalizacion.cpp` | Valentino Graff |
| `ventas.cpp` | (completar) |
| `cierre.cpp` | (completar) |
| `resumen.cpp` | (completar) |

## Estructura del repo

```
.
├── enunciado.pdf
├── datos/                 # archivos provistos por la cátedra (no se tocan)
│   ├── comandas_historicas.dat
│   ├── inventario.dat
│   └── dump_datos_de_prueba.txt
├── .gitignore
├── normalizacion.cpp
├── ventas.cpp
├── cierre.cpp
└── resumen.cpp
```

Cada programa tiene su propio `main`. No hay `main.cpp`.

## Cómo compilar y correr

Los programas leen y escriben los `.dat` en la **carpeta donde se ejecutan**. Los archivos de `datos/` son los originales y no se modifican: se copian a la raíz y se trabaja sobre las copias.

```
cp datos/comandas_historicas.dat datos/inventario.dat .
```

En PowerShell:

```
copy datos\comandas_historicas.dat .
copy datos\inventario.dat .
```

Compilación (siempre desde la raíz del repo):

```
g++ -O2 -o normalizacion normalizacion.cpp
g++ -O2 -o ventas        ventas.cpp
g++ -O2 -o cierre        cierre.cpp
g++ -O2 -o resumen       resumen.cpp
```

### Orden de ejecución

Cada programa deja los archivos que usa el siguiente.

| # | Programa | Lee | Genera |
|---|---|---|---|
| 1 | `./normalizacion` | `comandas_historicas.dat`, `inventario.dat` | `mozos.dat`, `comandas_dd-mm-aaaa.dat` (uno por día), `inventario.dat` actualizado |
| 2 | `./ventas` | `mozos.dat`, `inventario.dat` | agrega a `comandas_dd-mm-aaaa.dat`, actualiza `inventario.dat` |
| 3 | `./cierre` | `comandas_dd-mm-aaaa.dat` | `comandas_semana_sX-mm.dat` |
| 4 | `./resumen` | `comandas_semana_sX-mm.dat` | reporte por pantalla |

**Importante:**

- `normalizacion` se corre **una sola vez y primero**. Si se corre de nuevo, descuenta el stock otra vez y pisa `mozos.dat` y las planillas de los días. Para repetir una prueba, volver a copiar `inventario.dat` desde `datos/` y volver a correr.
- Los cuatro programas se ejecutan **desde la misma carpeta**, para que se encuentren los archivos entre sí.

### Datos de prueba para `ventas`

`normalizacion` asigna los ids por orden de aparición en el historial. La clave inicial de cada mozo es su número como texto: el mozo 1 entra con la clave `1`, el 2 con `2`, y así.

## Decisiones técnicas

### Estructuras (iguales en todos los programas)

```cpp
struct Mozo     { int idMozo; char nombre[50]; char password[20]; float totalComision; };
struct Comanda  { int idMozo; int codigoProducto; int cantidad; float comision; };
struct Producto { int codigo; char descripcion[50]; float precio; int stockActual; };
```

Todos los registros que van a disco son de tamaño fijo (`char[N]`, nunca `string`). Con las structs de la ficha técnica, `sizeof(ComandaHistorica) = 76` y `sizeof(Producto) = 64`.

### Claves: corrimiento con `K = 5`

Las claves no se guardan tal cual en `mozos.dat`. A cada carácter se le suma `K = 5`. Ejemplo: la clave `"1"` (código 49) se guarda como `'6'` (código 54).

- **Verificación del login:** al texto que escribe el mozo se le aplica la misma cuenta y se compara con lo guardado. Nunca hace falta desencriptar.
- **Por qué se puede deshacer:** sumar `K` se revierte restando `K`, y cada carácter distinto da un resultado distinto, así que no se pierde información. Por eso no sirve "dividir por dos": 16 y 17 darían lo mismo.
- **Límite:** es un corrimiento simple, suficiente para que la clave no se lea a simple vista, pero no es seguridad real.
- `K` y la cuenta tienen que ser idénticos en `normalizacion.cpp` y en `ventas.cpp`.

### `normalizacion.cpp`

- Mozos: Lista enlazada. Cada mozo se busca por nombre (búsqueda secuencial, porque no hay orden). Si no está, se agrega al final con el id siguiente. Cada venta suma su comisión al TotalComision del mozo.
- Planillas por día: Lista enlazada de días, y dentro de cada día una lista de comandas. Los días se buscan en forma secuencial porque aparecen sin orden.
- Orden por mozo: cada comanda se inserta ya ordenada por idMozo. Con <= queda detrás de las anteriores del mismo mozo, así que se conserva el orden de carga.
- Inventario: Se usa búsqueda binaria porque está ordenado por código. Se carga en un array del tamaño exacto (tamaño del archivo / sizeof(Producto)).
- Stock: se descuenta en memoria y se actualiza el mismo inventario.dat, sin recrearlo.

### `ventas.cpp`

Solicita la fecha y crea el nombre del archivo donde se guardarán las comandas del día.
Verifica el ingreso del mozo, buscando su número de identificación y comprobando su contraseña encriptada.
Busca el producto en el inventario y muestra su descripción y precio.
Valida la cantidad solicitada, comprobando que sea mayor que cero y que haya suficiente stock.
Calcula el total de la venta y la comisión del mozo, que corresponde al 10 % del importe vendido.
Registra la venta en un archivo del día y descuenta del inventario las unidades vendidas.
Ordena las comandas por número de mozo cuando se termina de cargar las ventas.
Además, utiliza archivos binarios (.dat) para almacenar la información de los mozos, los productos y las ventas.

### `cierre.cpp`

COMPLETAR

### `resumen.cpp`

COMPLETAR

## Supuestos

- Los mozos se identifican por nombre exacto: "Juan Perez" y "juan perez" serían dos mozos distintos.
- La comisión de las ventas históricas se toma tal cual viene del archivo.
- Los ids de mozo son 1, 2, 3... sin saltos, en orden de aparición en el historial.
