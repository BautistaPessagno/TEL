# TEL Stage 3 Requirements and Roadmap

## Alcance

Este documento planifica Stage 3 sin implementar codigo. Se basa en los
archivos de `TLA stage 3/`, el feedback de Stage I, la documentacion actual en
`doc/`, y el estado del frontend TEL existente.

Stage 3 debe transformar el frontend actual en un compilador funcional: parsear
TEL, validar semantica sobre el AST, generar artefactos C validos, y entregar la
documentacion final del proyecto.

## Fuentes revisadas

- `TLA stage 3/(2026-03-12) Proyecto Especial.pdf`
- `TLA stage 3/(2024-09-25, v0.2.0) Analisis Semantico.pdf`
- `TLA stage 3/(2024-05-16, v0.1.0) Generacion de Codigo.pdf`
- `TLA stage 3/(2024-05-16, v0.1.0) Runtime.pdf`
- `TLA stage 3/respuesta stage I.md`
- `doc/requirements.md`
- `doc/next_steps.md`
- `README.md`
- `CMakeLists.txt`
- `src/main/c/EntryPoint.c`
- `src/main/c/frontend/syntactic-analysis/AbstractSyntaxTree.h`
- `src/main/c/frontend/syntactic-analysis/BisonGrammar.y`
- `src/main/c/frontend/syntactic-analysis/BisonActions.c`
- `src/main/c/frontend/lexical-analysis/FlexPatterns.l`
- `src/main/c/frontend/lexical-analysis/FlexActions.c`
- `src/main/c/backend/code-generation/Generator.c`
- `src/main/c/backend/domain-specific/Calculator.c`
- `src/test/c/accept/`
- `src/test/c/reject/`

## Estado actual relevante

- El frontend ya reconoce y construye AST para una superficie amplia de TEL:
  declaraciones, funciones, `main`, structs, unions, enums, typedefs,
  directivas, inline C, bloques por indentacion, control flow, punteros, arrays,
  function pointers, literales, llamadas, operadores y `cnt`.
- El AST actual es la base correcta para Stage 3. No conviene redisenarlo salvo
  que una validacion o generacion concreta demuestre que falta informacion.
- `CompilerState` todavia conserva campos heredados de la calculadora y solo
  transporta la raiz del AST.
- `src/main/c/backend/code-generation/Generator.c` y
  `src/main/c/backend/domain-specific/Calculator.c` son backend heredado de la
  calculadora. No son compatibles con el AST TEL actual.
- `CMakeLists.txt` no compila los archivos de backend heredados.
- El working tree actual tiene `EntryPoint.c` modificado para reactivar el
  backend heredado. Esa modificacion debe tratarse como deuda de integracion:
  Stage 3 no debe habilitar `executeCalculator`/`executeGenerator` tal como
  estan, sino reemplazarlos por fases TEL reales.

## Requirements de Stage 3

### R1. Pipeline del compilador

- El ejecutable debe conservar el frontend Flex/Bison actual.
- La ejecucion debe quedar ordenada en fases:
  1. analisis lexico;
  2. analisis sintactico y construccion del AST;
  3. analisis semantico;
  4. generacion de codigo C;
  5. limpieza de recursos.
- Si falla lexing, parsing o semantica, el programa debe terminar con estado no
  cero y no emitir un artefacto C usable.
- Si todas las fases pasan, el compilador debe emitir codigo C deterministico y
  sintacticamente valido.

### R2. Analisis semantico

- Agregar una fase de validacion sobre el AST antes de generar C.
- Mantener una tabla de simbolos y una pila de scopes.
- Registrar simbolos globales: variables globales, funciones, `main`, structs,
  unions, enums, typedefs y constantes de enum.
- Registrar simbolos locales: parametros, variables locales e iteradores
  introducidos por `ford` si se decide que el lenguaje los declara
  implicitamente.
- Rechazar duplicados en el mismo namespace/scope cuando generen ambiguedad.
- Validar que cada identificador usado exista en un scope visible.
- Validar que los tipos nombrados existan antes de usarse o esten disponibles
  por una declaracion compatible.
- Validar llamadas a funciones: existencia, cantidad de argumentos y
  compatibilidad de tipos.
- Validar asignaciones e inicializadores: lvalue valido, compatibilidad de tipo,
  `const` no reasignable, arrays inicializados con elementos compatibles.
- Validar expresiones: aritmetica, comparaciones, logicos, bitwise, punteros,
  direcciones, dereferencias, indices de array, acceso por punto y flecha,
  `null`, function pointers y literales.
- Validar retornos: `void` no debe retornar valor, funciones no-void deben
  retornar valor compatible, y los retornos implicitos actuales deben respetar
  el tipo declarado.
- Validar control flow: `break` solo dentro de loops o switch; `cnt` solo dentro
  de loops; condiciones de `if`, `while`, `dw`, `for` y `switch` deben ser
  compatibles con el uso.
- Validar `main`: debe existir exactamente un entry point y debe traducirse a
  una firma C definida por el proyecto.
- Tratar inline C como bloque opaco: se copia a salida pero no se type-checkea.
  Esta limitacion debe documentarse.

### R3. Generacion de codigo C

- Reemplazar el generador heredado por un generador TEL -> C.
- Generar un unico archivo C por defecto, preferentemente por stdout para seguir
  el estilo CLI actual. Si se decide escribir a archivo, documentar nombre,
  ubicacion y uso.
- Emitir en orden estable: directivas, inline C global, typedefs, enums,
  structs/unions, variables globales, prototipos, funciones y `main`.
- Traducir tipos TEL a C:
  - `int`, `char`, `float`, `double`, `long`, `short`, `void` se conservan.
  - `uint` se traduce a `unsigned int`.
  - `uli` se traduce a `unsigned long int`.
  - punteros, arrays, const y function pointers deben emitirse con sintaxis C
    valida.
- Traducir declaraciones TEL `nombre:tipo` a declaraciones C `tipo nombre`.
- Traducir `main` a la firma C definida por `doc/requirements.md` o a una
  variante mas simple si el equipo la decide y documenta.
- Traducir funciones con cuerpo a definiciones y funciones sin cuerpo a
  prototipos.
- Traducir bloques indentados a bloques C con llaves.
- Traducir `ret`, `if`/`elif`/`else`, `ford`, `for`, `while`, `dw`, `switch`,
  `break`, `cnt`, declaraciones y expresiones.
- Para `ford`, preservar o reconstruir la informacion suficiente para emitir
  una inicializacion C valida del iterador.
- Emitir expresiones con parentesis conservadores para evitar errores de
  precedencia.
- Traducir `#include nombre` a la forma C acordada, por ejemplo
  `#include <nombre.h>`, y conservar `#define`.
- Copiar inline C sin modificar salvo normalizacion de indentacion necesaria.
- No generar dependencias externas no aprobadas por QRF.

### R4. Runtime

- Definir explicitamente la politica de runtime:
  - Opcion minima recomendada: TEL compila a C estandar y delega el runtime al
    compilador C y a las librerias incluidas por el usuario.
  - Opcion extendida: agregar assets runtime propios solo si hay built-ins TEL
    reales que no puedan mapearse directo a C.
- Si se agregan archivos runtime, deben vivir en una carpeta versionada,
  copiarse o incluirse de forma deterministica, y documentarse en README e
  informe final.
- Los programas de prueba que deban linkear funciones externas deben incluir
  implementaciones, inline C o un runtime documentado. Un prototipo solo no
  alcanza para pruebas ejecutables.

### R5. Testing

- Conservar los tests Stage 2 de accept/reject como baseline de frontend.
- Agregar tests de rechazo semantico para:
  - identificadores no declarados;
  - duplicados invalidos;
  - tipos inexistentes;
  - llamadas con aridad incorrecta;
  - llamadas con tipos incompatibles;
  - asignacion incompatible;
  - reasignacion de `const`;
  - return incompatible;
  - funcion no-void sin return garantizado;
  - `break` fuera de loop/switch;
  - `cnt` fuera de loop;
  - acceso a miembro inexistente;
  - indice sobre no-array;
  - dereferencia sobre no-puntero.
- Agregar tests de aceptacion semantica para programas TEL pequenos y
  especificos.
- Agregar golden tests de generacion TEL -> C para construcciones unitarias.
- Agregar tests de integracion que compilen el C generado con GCC dentro del
  container y ejecuten el binario cuando el programa tenga comportamiento
  observable.
- Agregar al menos un test de integracion mas complejo, alineado con el feedback
  de Stage I, por ejemplo Two Sum, N-Queens reducido, o un programa que combine
  arrays, punteros, funciones y control flow.

### R6. Documentacion final

- Actualizar README para Stage 3: build, test, generar C, compilar C generado,
  limitaciones y runtime.
- Mantener la especificacion Stage I en el repo.
- Preparar el informe final con las secciones minimas pedidas:
  - Tabla de Contenidos;
  - Introduccion;
  - Modelo Computacional: Dominio y Lenguaje;
  - Implementacion: Frontend, Backend, Adicionales, Dificultades Encontradas;
  - Futuras Extensiones;
  - Conclusiones;
  - Referencias;
  - Bibliografia.
- Aplicar feedback de Stage I:
  - corregir pluralizacion de nombres de equipo;
  - agregar sangria o separacion consistente de parrafos;
  - indentar listas;
  - evitar exceso o falta de separacion alrededor de titulos;
  - no usar imagenes para codigo;
  - referenciar ejemplos como listados o ejemplos, no como figuras;
  - agregar numeros de pagina;
  - usar leyendas numeradas para ejemplos.
- Documentar la metodologia de medicion de ahorro de tokens:
  - no asumir que una palabra equivale a un token;
  - indicar tokenizer/modelo usado cuando se hagan mediciones;
  - separar el costo de instrucciones y metadatos del payload TEL medido;
  - explicar el tradeoff entre compresion, minificacion y metadata necesaria;
  - comparar TEL con alternativas cercanas a C cuando haya mappings simples que
    eviten explicar un lenguaje nuevo al LLM.

## Roadmap propuesto

### Fase 0. Cierre de alcance y baseline

Objetivo: dejar claro que subset TEL debe compilar a C en Stage 3.

- La salida C por defecto sera `stdout`.
- `main` se traducira como `int main(int argc, char * argv[])`.
- TEL no tendra runtime propio por ahora: el C generado dependera del estandar
  C, los includes del usuario, inline C y el toolchain C.
- `ford i start end` introducira un iterador local implicito de tipo `int`.
- El backend heredado `Calculator`/`Generator` no es usable para TEL; las fases
  Stage 3 deben avanzar sobre `SemanticAnalyzer` y `CodeGenerator`.
- Baseline conocido antes de cambios Stage 3:
  - `docker compose run --rm compiler src/main/bash/build.sh` termino con
    estado `0`.
  - `docker compose run --rm compiler src/main/bash/test.sh` termino con
    estado `0`.
  - La suite actual contiene 32 accept fixtures y 48 reject fixtures.

Criterio de salida: decisiones cerradas y baseline Stage 2 conocido.

### Fase 1. Integracion de fases backend TEL

Objetivo: preparar la arquitectura sin implementar validaciones profundas.

- Crear modulo de analisis semantico bajo `src/main/c/backend/semantic-analysis/`.
- Crear modulo de generacion TEL -> C bajo `src/main/c/backend/code-generation/`.
- Reemplazar el uso del backend de calculadora en `EntryPoint.c` por fases TEL.
- Extender `CompilerState` para estado semantico, errores y configuracion de
  salida.
- Agregar fuentes nuevas a `CMakeLists.txt`.

Criterio de salida: el compilador sigue parseando, la fase semantica inicial
acepta programas triviales, y no se llama al backend heredado.

### Fase 2. Tabla de simbolos, scopes y tipos

Objetivo: construir la base para validar significado.

- Definir modelo de simbolos.
- Definir pila de scopes.
- Definir comparacion y compatibilidad de tipos usando el AST actual.
- Hacer una pasada de coleccion global para funciones, tipos y declaraciones
  globales.
- Hacer pasadas locales para parametros y variables.

Criterio de salida: tests de duplicados, nombres inexistentes y tipos
inexistentes pasan.

### Fase 3. Validaciones semanticas por construccion

Objetivo: rechazar programas sintacticamente validos pero semanticamente
invalidos.

- Validar declaraciones, inicializadores y asignaciones.
- Validar expresiones y operadores.
- Validar llamadas y function pointers.
- Validar returns explicitos e implicitos.
- Validar control flow.
- Validar aggregates, enums, arrays y member access.
- Definir la politica de inline C como opaca.

Criterio de salida: suite de reject semantico pasa y los accept existentes no
regresan salvo que el equipo decida endurecer semantica y actualice fixtures.

### Fase 4. Generador C unitario

Objetivo: emitir C valido para cada nodo relevante del AST.

- Implementar emisor con indentacion y buffer/salida controlada.
- Generar tipos, declaradores y function pointers.
- Generar declaraciones globales/locales.
- Generar funciones, prototipos y `main`.
- Generar statements y bloques.
- Generar expresiones con parentesis conservadores.
- Generar directivas e inline C.
- Agregar golden tests por feature.

Criterio de salida: TEL pequenos generan C esperado y compilable.

### Fase 5. Runtime y artefacto final

Objetivo: cerrar como se ejecuta lo generado.

- Documentar runtime minimo o agregar assets si se aprueba.
- Ajustar scripts para generar C, compilarlo con GCC y ejecutar integraciones.
- Separar tests de parsing/semantica de tests de generacion/ejecucion si mejora
  diagnostico.

Criterio de salida: un programa TEL completo genera C, compila y corre en el
container.

### Fase 6. Tests de integracion y regresion

Objetivo: demostrar que Stage 3 no es solo unitario.

- Mantener accept/reject sintacticos.
- Agregar accept/reject semanticos.
- Agregar golden tests de generacion.
- Agregar integraciones compiladas con GCC.
- Incluir caso complejo de referencia para mostrar valor del lenguaje.

Criterio de salida: `src/main/bash/test.sh` o scripts equivalentes cubren todo
el pipeline y devuelven estado cero.

### Fase 7. Documentacion final

Objetivo: entregar el proyecto autocontenido.

- Actualizar `doc/requirements.md` con el lenguaje final.
- Crear o actualizar documento de arquitectura backend.
- Actualizar README con comandos Stage 3.
- Preparar informe final en PDF/Notion/Confluence con la estructura exigida.
- Incorporar correcciones formales de feedback Stage I.

Criterio de salida: una persona externa puede entender el dominio, compilar,
probar y ejecutar TEL siguiendo el repo.

### Fase 8. Hardening y entrega

Objetivo: reducir sorpresas de entrega.

- Ejecutar build limpio dentro de Docker.
- Ejecutar suite completa.
- Revisar que no queden artefactos generados trackeados por error.
- Revisar que README y doc indiquen limitaciones reales.
- Entregar en rama `development` con hash completo de 40 caracteres.
- Enviar correo al QRF con integrantes, hash y link al repo.

Criterio de salida: entrega reproducible y autocontenida.

## Orden recomendado de implementacion

1. Corregir integracion del backend heredado antes de cualquier feature nueva.
2. Crear semantica minima con simbolos/scopes.
3. Agregar tests semanticos de rechazo antes de cada validacion nueva.
4. Completar type checking suficiente para las features que ya parsea TEL.
5. Implementar generacion C por nodos, empezando por declaraciones, tipos,
   funciones y expresiones simples.
6. Agregar control flow, arrays, punteros, function pointers e inline C.
7. Agregar integraciones que compilen y corran C generado.
8. Mantener la mejor superficie de lenguaje viable en el frontend, sin recortar
   TEL solo porque alguna construccion pueda quedar fuera del backend Stage 3.
   Las limitaciones reales de semantica o generacion deben documentarse.
9. Actualizar documentacion final y preparar entrega.

## Riesgos principales

- El AST de `ford` hoy pierde informacion explicita sobre si el iterador debe
  declararse. Si se quiere generar `for (int i = ...)`, conviene ajustar el AST
  o definir una convencion de generacion robusta.
- Function pointer declarations y function pointer types tienen sintaxis C
  delicada. Deben tener tests golden propios.
- Arrays y punteros requieren separar compatibilidad TEL de decaimiento C
  cuando se pasan como parametros.
- Inline C permite compilar programas que la semantica TEL no puede verificar.
  Hay que documentar esa frontera.
- Prototipos sin definicion son aceptables para generar C, pero no para linkear
  ejecutables de integracion salvo que exista runtime o implementacion inline.
- Si se agregan librerias externas, se requiere aprobacion previa del QRF y
  documentacion de version.

## Definition of done de Stage 3

- El frontend existente sigue aceptando/rechazando los fixtures esperados.
- La fase semantica rechaza programas invalidos que el parser no puede
  distinguir.
- El generador produce C deterministico y compilable para el subset TEL
  documentado.
- Hay tests unitarios, semanticos, golden e integracion ejecutable.
- README describe uso real de Stage 3.
- El informe final cumple la estructura exigida y aplica el feedback formal de
  Stage I.
- La entrega queda versionada en `development` con hash completo y repo
  autocontenido.
