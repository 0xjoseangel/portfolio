# Instrucciones de Compilación

## ⚠️ IMPORTANTE: Limpiar cache antes de compilar

**Debe limpiar el cache de CMake antes de cada compilación** para que se apliquen correctamente las flags de optimización y la configuración de OpenMP:

```bash
rm -rf CMakeFiles CMakeCache.txt *.cmake && cmake . && make
```

## Por qué es necesario

El proyecto usa:
- Optimizaciones agresivas: `-O3 -march=native -ffast-math`
- Paralelización: OpenMP en la función `fitness()` (operación O(N²) dominante)

Si no se limpia el cache, CMake reutiliza configuraciones previas y las optimizaciones no se aplican, resultando en ejecución ~40x más lenta (ej: S&P 500 tarda 43s en lugar de 2s).

## Compilación rápida (después de la primera)

Una vez compilado correctamente, cambios en el código fuente permiten compilar sin limpiar:
```bash
make
```

## Ejecutar

```bash
./main
```

Parámetros opcionales:
```bash
./main <max_evals> <n_seeds>
```
Ejemplo: `./main 10000 50`
