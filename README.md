# TP Integrador — Buffet Albert Einstein

Trabajo Práctico grupal de Algoritmos y Estructuras de Datos (UTN FRBA), unidad de archivos binarios en C/C++.

**Grupo:** B

**Integrantes:**

| Nombre | Usuario de GitHub |
|---|---|
| Valentino Graff | valentino-graff |
| Aylen Cortez | AyluCortez |
| Perseo Reak| Perseo |
| Santiago Isoldi | pocho1212 |

## Quién hizo cada parte

| Programa | Responsable |
|---|---|
| `normalizacion.cpp` | Valentino Graff |
| `ventas.cpp` | Santiago Isoldi |
| `cierre.cpp` | Perseo Real |
| `resumen.cpp` | Aylen Cortez |

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

Los programas leen y escriben los `.dat` en la **carpeta donde se ejecutan**. Los archivos de `datos/` son los originales y no se modifican: se copian a la carpeta de ejecución y se trabaja sobre las copias.

```
cp datos/comandas_historicas.dat datos/inventario.dat .
```

En PowerShell:

```
copy datos\comandas_historicas.dat .
copy datos\inventario.dat .
```

`normalizacion` copia `inventario.dat` y `comandas_historicas.dat` desde `datos/` si no los encuentra en la carpeta donde se ejecuta, así que ese paso de copiar a mano es opcional para ese programa. Los originales de `datos/` nunca se modifican. El resto de los programas no copia nada: necesitan `mozos.dat` e `inventario.dat` en la carpeta de ejecución.

Compilación (desde la raíz del repo):

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

- `normalizacion` se corre **una sola vez y primero**. Si se vuelve a correr, descuenta el stock otra vez y pisa `mozos.dat` y las planillas de los días. Para repetir una prueba, hay que volver a dejar en la carpeta de ejecución una copia limpia de `inventario.dat` (la de `datos/`).
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

Las claves no se guardan tal cual en mozos.dat. A cada carácter se le suma 5.
  
### `normalizacion.cpp`

El programa carga el historial y el inventario completos en arrays dinámicos, con un tamaño igual al del archivo dividido por el tamaño del registro, por lo que no hay máximos fijos. Recorre las ventas y arma la lista de mozos: cada mozo se busca por nombre con búsqueda secuencial (no hay orden que aprovechar) y, si no está, se agrega con el id siguiente (1, 2, 3...) y su clave inicial, que es su id como texto encriptada con K = 5. A cada mozo se le va sumando la comisión de sus ventas. También arma la lista de fechas distintas, con la misma búsqueda secuencial. Para cada fecha junta sus comandas cambiando el nombre del mozo por su id, las ordena por idMozo con ordenamiento burbuja (el > estricto no mezcla las comandas de un mismo mozo) y las guarda en comandas_dd-mm-aaaa.dat, igual que mozos.dat, en la carpeta donde se ejecuta el programa (junto a los programas si se corre desde la raíz del repositorio). Para el stock, como el inventario está ordenado por código pero tiene huecos y no se puede acceder por posición, usa búsqueda binaria: descuenta lo vendido en memoria y después actualiza el mismo inventario.dat con fseek y fwrite registro por registro, sin recrearlo. Si un producto no existe lo informa, y si no alcanza el stock lo informa y lo deja en 0. Al terminar libera la memoria con delete[].

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

El archivo cierre.cpp sirve para realizar el cierre semanal de las ventas del Buffet Albert Einstein.
Primero, incluye las bibliotecas necesarias para trabajar con archivos, textos, vectores y ordenamientos. Después, define la estructura Comanda, que almacena el identificador del mozo, el código del producto, la cantidad vendida y la comisión.
Luego, la función fechaValida() verifica que las fechas tengan el formato correcto y que el día y el mes estén dentro de los rangos permitidos.
En la función principal main(), el programa solicita el número de semana, el mes y la cantidad de planillas diarias. Valida esos datos y pide las fechas de las planillas, evitando ingresar dos veces la misma fecha.
Después, abre cada archivo diario, lee sus comandas y las guarda en un vector. Una vez reunidas todas, utiliza sort() para ordenarlas por identificador de mozo.
Finalmente, crea un archivo binario semanal, guarda todas las comandas ordenadas y muestra un mensaje indicando que el cierre terminó correctamente, junto con la cantidad de planillas procesadas y comandas guardadas.

### `resumen.cpp`

(completar)

## Casos raros contemplados

| Caso | Qué hace |
|---|---|
| Mozo inexistente | `ventas` avisa y no deja seguir con esa venta |
| Clave incorrecta | `ventas` avisa y no deja cargar |
| Producto inexistente | `ventas` avisa. `normalizacion` lo informa y sigue |
| Sin stock | `ventas` rechaza la venta. `normalizacion` avisa y deja el stock en 0 |
| Cantidad menor o igual a 0 | `ventas` la rechaza |
| Planilla del día sin crear | `ventas` la crea al agregar la primera venta |
| Último mozo del cierre | (completar) |

## Supuestos
- Los mozos se identifican por nombre exacto: "Juan Perez" y "juan perez" serían dos mozos distintos.
- La comisión de las ventas históricas se toma tal cual viene del archivo.
- Los ids de mozo son 1, 2, 3... sin saltos, en orden de aparición en el historial.
