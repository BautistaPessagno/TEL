# Token Efficient Language (TEL) — Informe Final

**Diseño e Implementación de un Lenguaje · Stage III**

Autómatas, Teoría de Lenguajes y Compiladores — Ingeniería Informática — ITBA

## Equipo

| Nombres           | Apellidos | E-mail                  |
| :---------------- | :-------- | :---------------------- |
| Bautista          | Pessagno  | bpessagno@itba.edu.ar   |
| Lorenzo Alejandro | Mendez    | lmendez@itba.edu.ar     |
| Rodrigo Alejandro | Hernandez | rohernandez@itba.edu.ar |

Repositorio: <https://github.com/BautistaPessagno/TEL>

---

## Tabla de Contenidos

1. [Introducción](#1-introducción)
2. [Modelo Computacional](#2-modelo-computacional)
   - 2.1. [Dominio](#21-dominio)
   - 2.2. [Lenguaje](#22-lenguaje)
3. [Implementación](#3-implementación)
   - 3.1. [Frontend](#31-frontend)
   - 3.2. [Backend](#32-backend)
   - 3.3. [Adicionales](#33-adicionales)
   - 3.4. [Dificultades Encontradas](#34-dificultades-encontradas)
4. [Futuras Extensiones](#4-futuras-extensiones)
5. [Conclusiones](#5-conclusiones)
6. [Referencias](#6-referencias)
7. [Bibliografía](#7-bibliografía)

---

## 1. Introducción

&emsp;TEL (_Token Efficient Language_) es un lenguaje de dominio específico, de
sintaxis similar a Python, que compila a lenguaje C. Su objetivo es representar
programas escritos en un subconjunto de C con una sintaxis más compacta, de modo
de minimizar la cantidad de _tokens_ consumidos por los Modelos de Lenguaje de
Gran Escala (LLMs), sin perder información semántica y manteniendo la
legibilidad para una persona desarrolladora.

&emsp;La motivación es concreta. Cuando un desarrollador utiliza un
agente (Claude, GPT, Gemini, entre otros) para escribir, revisar o modificar código
C, la verbosidad inherente del lenguaje C genera un consumo excesivo de tokens,
tanto en la entrada (los _prompts_, la lectura de archivos y la salida de
comandos) como en la salida (las respuestas del modelo). Ese consumo impacta de
forma directa en tres dimensiones: el costo económico, ya que los proveedores
cobran por token; la latencia, dado que más tokens implican más tiempo de
respuesta; y la ventana de contexto disponible, porque cada token gastado en
sintaxis es un token menos para contenido semántico.

&emsp;El Stage III entrega un compilador funcional que recorre el pipeline
completo: lee un programa TEL desde la entrada estándar, lo lexa y parsea con
Flex y Bison, construye un Árbol de Sintaxis Abstracta (AST), valida su
semántica y, finalmente, emite código C estándar, determinista y compilable por
la salida estándar. Se trata, por lo tanto, de un compilador fuente a fuente
(_source-to-source_).

&emsp;Este documento describe el desarrollo y la concepción de las ideas detrás
del lenguaje y de su compilador.

---

## 2. Modelo Computacional

### 2.1. Dominio

&emsp;El dominio de TEL es la optimización de tokens en la interacción
humano-LLM para el desarrollo de software en C. El compilador toma como entrada
un programa en TEL y produce como artefacto de salida su equivalente en código C
estándar.

&emsp;La transformación se concibe como biyectiva en su definición conceptual:
dado cualquier programa válido en TEL, el compilador produce un único programa C
equivalente y bien formado y, recíprocamente, cualquier programa dentro del
subconjunto de C soportado tiene una representación única en TEL. Para el alcance
de este proyecto se implementa únicamente la dirección TEL → C.

&emsp;El subconjunto de C cubierto por TEL incluye:

1. tipos primitivos (`int`, `char`, `float`, `double`, `void`, `long`, `short`)
   y los tipos compuestos compactados `uint` y `uli`;
2. calificadores de tipo (`unsigned`, `long`, `short`, `const`);
3. variables y constantes;
4. arreglos y punteros, incluidos los punteros a función;
5. funciones, prototipos y punto de entrada (`main`);
6. estructuras de control (`if`/`elif`/`else`, `for`, `while`, `do-while`,
   `switch`);
7. estructuras agregadas (`struct`, `union`), enumeraciones (`enum`) y alias de
   tipo (`typedef`);
8. directivas de preprocesador básicas (`#include`, `#define`).

#### Medición del ahorro de tokens

&emsp;El valor del lenguaje se evidencia al comparar el conteo de tokens de un
programa en C contra su equivalente en TEL. Las siguientes mediciones se
obtuvieron con el tokenizador de OpenAI (`platform.openai.com/tokenizer`), sobre
el cuerpo del programa, es decir, sin contar las instrucciones previas que
enseñan TEL al modelo (ver la discusión metodológica más abajo).

&emsp;**Listado 2.1.** Conteo comparativo de tokens (C → TEL):

1. _Hola mundo_ con `main`: 27 → 8 tokens (99 → 29 caracteres).
2. Función que recorre un arreglo con un bucle `for`: 45 → 32 tokens
   (142 → 103 caracteres).
3. Definición de un `struct` con `typedef`: 26 → 21 tokens (126 → 84
   caracteres).
4. Cinco directivas `#include`: 26 → 16 tokens (101 → 80 caracteres).

&emsp;Sobre la metodología de medición conviene precisar varios puntos, en línea
con la devolución de la cátedra:

1. **Un token no equivale a una palabra.** Un LLM determina la
   _tokenización_ según cómo el modelo la haya interpretado durante su
   entrenamiento; una misma palabra puede ser un token o varios. Por eso las
   mediciones se hacen con un tokenizador real y no contando palabras.
2. **El costo de los metadatos debe separarse del payload medido.** Para que el
   LLM entienda la representación compacta, normalmente se le entregan
   instrucciones previas (qué significa cada keyword de TEL). Si esas
   instrucciones se contaran dentro de cada medición, el ahorro aparente sería
   menor. La medición correcta separa el costo de instrucciones y metadatos del
   payload TEL en sí. Esto es análogo al problema de la compresión: comprimir
   achica el payload, pero se requieren metadatos para decodificarlo, y existen
   payloads incompresibles para los cuales el resultado crece.
3. **Compresión, minificación y metadatos son un _tradeoff_.** Parte de los
   mapeos de TEL coinciden con una minificación típica de cualquier lenguaje
   (por ejemplo, reemplazar nombres largos por nombres cortos). Cuando un mapeo
   no cambia la estructura del lenguaje conocido por el modelo, no hace falta
   explicarle nada nuevo y la sección de metadatos se reduce. Una alternativa a
   considerar como punto de comparación es la minificación directa de C, que
   evita tener que enseñarle un lenguaje nuevo al LLM a cambio de un ahorro
   menor.

&emsp;Como complemento a los ejemplos puntuales del Listado 2.1, se midió el
ahorro de tokens sobre un conjunto de ocho programas más extensos —resoluciones
de problemas de estilo LeetCode, disponibles en `demo/leetcode/` y descritos en
la sección 3.3— comparando el código TEL (sin comentarios) contra el C generado
por el compilador (sin comentarios), con el mismo tokenizador (`cl100k_base`,
compatible con `platform.openai.com/tokenizer`).

&emsp;**Tabla 2.1.** Ahorro de tokens en los programas de `demo/leetcode/` (TEL
vs. C generado):

| Programa | Problema | Tokens TEL | Tokens C | Ahorro | % |
| :--- | :--- | ---: | ---: | ---: | ---: |
| `two-sum.tel` | LC 1 — Two Sum | 121 | 186 | 65 | 34.9% |
| `fizzbuzz.tel` | LC 412 — Fizz Buzz | 93 | 145 | 52 | 35.9% |
| `fibonacci.tel` | LC 509 — Fibonacci | 96 | 147 | 51 | 34.7% |
| `binary-search.tel` | LC 704 — Binary Search | 170 | 223 | 53 | 23.8% |
| `reverse-integer.tel` | LC 7 — Reverse Integer | 114 | 147 | 33 | 22.4% |
| `palindrome-number.tel` | LC 9 — Palindrome Number | 118 | 151 | 33 | 21.9% |
| `max-subarray.tel` | LC 53 — Maximum Subarray (Kadane) | 194 | 262 | 68 | 26.0% |
| `climbing-stairs.tel` | LC 70 — Climbing Stairs | 100 | 153 | 53 | 34.6% |
| **Total** |  | **1006** | **1414** | **408** | **28.9%** |

&emsp;El ahorro promedio (~29 %) es menor que el de los ejemplos puntuales del
Listado 2.1 porque estos programas mezclan declaraciones y tipos compuestos
—donde TEL ahorra más— con expresiones aritméticas que se escriben de forma casi
idéntica en ambos lenguajes. El mayor ahorro aparece en los programas dominados
por control de flujo (`fizzbuzz`, `two-sum`, `fibonacci`, `climbing-stairs`,
~35 %), y el menor en funciones cortas y aritméticas (`reverse-integer`,
`palindrome-number`, ~22 %).

### 2.2. Lenguaje

&emsp;Esta sección describe la superficie del lenguaje. La especificación
completa y normativa vive en `doc/requirements.md`; aquí se resume con ejemplos.

#### Tipos y declaraciones

&emsp;Los tipos de C se reemplazan por variantes que ocupan un único token. Las
declaraciones invierten el orden respecto de C, anteponiendo el nombre al tipo
mediante `nombre:tipo`.

&emsp;**Listado 2.2.** Mapeo de tipos y forma de las declaraciones:

```
int    -> int                 char   -> char
float  -> float               double -> double
long   -> long                short  -> short
void   -> void
uint   -> unsigned int        uli    -> unsigned long int

x:int          // int x;
y:uli = 5      // unsigned long int y = 5;
```

&emsp;La compactación de tipos compuestos es la que más ahorra: `unsigned long
int` son tres tokens en C y `uli` es uno solo en TEL.

#### Funciones y retornos

&emsp;Las funciones se declaran con `fn nombre args -> tipo_retorno`, donde
`args` es una lista de pares `nombre:tipo` separados por espacios, y la flecha
con el tipo de retorno puede omitirse cuando la función retorna `void`. El
retorno se expresa con `ret`. Si la última sentencia del cuerpo es un
identificador suelto o una expresión de comparación o lógica, se la trata como
un `ret` implícito.

&emsp;**Listado 2.3.** Declaración de funciones y retorno implícito:

```
fn foo arg1:int arg2:char -> int    // int foo(int arg1, char arg2);
fn bar x:int                        // void bar(int x);

fn max a:int b:int -> int
    a > b                           // equivale a: ret a > b
```

#### Bloques por indentación y control de flujo

&emsp;En lugar de llaves `{}`, los bloques se delimitan por indentación, al
estilo de Python. El control de flujo usa keywords cortas y, en algunos casos,
asume valores por omisión frecuentes en la programación.

&emsp;**Listado 2.4.** Estructuras de control:

```
if  -> if        elif -> else if      else -> else
ford i 0 n       // for (int i = 0; i < n; i++)
for i = 0, c, i++   // for (int i = 0; c; i++)
while -> while   dw -> do while
break -> break   cnt -> continue
```

&emsp;El `switch` admite dos variantes, distinguidas por el separador de caso.
Con `:` los casos caen al siguiente (_fall-through_) salvo `break` explícito; con
`->` cada caso termina implícitamente, sin caída.

&emsp;**Listado 2.5.** Las dos variantes de `switch`:

```
switch variable          switch variable
    1: do_one()              1 -> do_one()
    2: do_two()              2 -> do_two()
       break                 3 -> do_three()
    3: do_three()            default -> do_default()
    default: do_default()
```

#### Punteros, arreglos y punteros a función

&emsp;Los punteros usan `*` y los arreglos usan `[]`, con el nombre antepuesto al
tipo. Los literales de arreglo separan sus elementos con espacios, no con comas.
Los punteros a función usan la sintaxis `fn*`.

&emsp;**Listado 2.6.** Punteros, arreglos y punteros a función:

```
arr:int[10]            // int arr[10];
arr:int[3] = {1 2 3}   // int arr[3] = {1, 2, 3};
fn* int -> int         // puntero a función (int) -> int
fn* int int -> void    // puntero a función (int, int) -> void
fn*                    // puntero a función () -> void
```

#### Literales, comentarios e inline C

&emsp;Los literales son los de C, con una diferencia: los octales usan el prefijo
`0o` (estilo Python) en vez del `0` desnudo de C. Se admiten comentarios de línea
`//` y de bloque `/* */`. El literal `null` se traduce al puntero nulo.

&emsp;**Listado 2.7.** Literales soportados:

```
42      0xFF      0o77       3.14      1.0e-3
'a'     '\n'      "hello"    null
```

&emsp;Se permite embeber código C dentro de bloques delimitados por comillas
invertidas (`` ` ``). El inline C se copia como bloque opaco y no es
verificado por TEL.

#### Directivas y punto de entrada

&emsp;`#define` se mantiene igual. En `#include`, los headers del sistema se
escriben sin extensión ni corchetes: `#include stdio` produce `#include
<stdio.h>`. A lo sumo puede aparecer un `main` por unidad de traducción; cuando
está presente, se traduce a `int main(int argc, char * argv[])`. Una unidad de
traducción puede omitir `main` para compilarse como biblioteca u objeto.

#### Conversiones semánticas

&emsp;El lenguaje define reglas de compatibilidad de tipos que el análisis
semántico hace cumplir:

1. los tipos escalares numéricos (`char`, `short`, `int`, `uint`, `long`, `uli`,
   `float`, `double` y los `enum`) son mutuamente convertibles en asignaciones,
   inicializadores, argumentos y retornos;
2. los arreglos decaen a punteros compatibles al asignarse o pasarse como
   argumento;
3. los punteros a objeto convierten desde y hacia `void *`;
4. `null` es compatible con punteros a objeto y a función;
5. una conversión de puntero puede agregar `const` al apuntado, pero no
   quitarlo;
6. las firmas de los punteros a función deben coincidir exactamente;
7. la compatibilidad de `struct`, `union` y `enum` es nominal: se compara por el
   nombre del tipo declarado.

&emsp;La inferencia de tipos no forma parte del subset del Stage III: toda
variable debe declararse con su tipo explícito antes de usarse.

---

## 3. Implementación

&emsp;El compilador está escrito en C y construido con Flex (analizador léxico) y
Bison (analizador sintáctico). El pipeline ejecuta cinco fases ordenadas:

```
stdin -> análisis léxico -> análisis sintáctico (AST)
      -> análisis semántico -> generación de código C -> limpieza
```

&emsp;Si fallan el análisis léxico, el sintáctico o el semántico, el programa
termina con estado distinto de cero y no emite un artefacto C usable. Si todas
las fases pasan, emite C determinista y sintácticamente válido. La orquestación
vive en `EntryPoint.c`, que inicializa cada módulo (cada `initializeXModule()`
devuelve un destructor que se ejecuta en orden inverso), corre
`executeSyntacticAnalysis()` y, si tiene éxito, `executeSemanticAnalysis()` y
`executeCodeGeneration()`. El estado se transporta en `CompilerState`, una
estructura mínima que sostiene la raíz del AST.

### 3.1. Frontend

&emsp;El frontend convierte el texto de entrada en un AST.

#### Análisis léxico (Flex)

&emsp;El lexer (`FlexPatterns.l` y `FlexActions.c`) reconoce el alfabeto
completo de TEL: literales (enteros decimales, hexadecimales y octales, flotantes,
caracteres y cadenas), las keywords del lenguaje, los nueve tipos primitivos, el
conjunto de operadores aritméticos, de asignación, bit a bit, lógicos y de
comparación, los delimitadores, y las directivas `#include` y `#define`.

&emsp;El lexer utiliza varias _start conditions_ de Flex para resolver
ambigüedades:

1. **BOL** (_beginning of line_) procesa la indentación al inicio de cada línea y
   genera los tokens `INDENT` y `DEDENT`.
2. **AFTER_POSTFIX** se activa luego de un identificador o de un cierre (`)` o
   `]`) para interpretar correctamente el operador `->` y distinguirlo de la
   flecha de tipo de retorno de las funciones.
3. **INLINE_C** y **BLOCK_COMMENT** manejan, respectivamente, los bloques de
   código C embebido (delimitados por comillas invertidas) y los comentarios de
   bloque `/* */`.

&emsp;La indentación significativa se implementa con una pila de niveles
(`StackADT.c`, una pila de enteros con capacidad inicial 128 y base 0). Un
tabulador cuenta como cuatro espacios y la indentación debe ser múltiplo de
cuatro; en caso contrario el análisis falla. Cuando el nivel sube, se apila el
nuevo nivel y se emite `INDENT`; cuando baja, se desapila hasta encontrar un
nivel coincidente, emitiendo un `DEDENT` por cada salto. Al llegar al fin de
archivo se emite un terminador, se vacían los `DEDENT` pendientes hasta la base y
se emite el token de fin. El lexer también reetiqueta como `TYPEDEF_NAME` a los
identificadores que el parser ya registró como nombres de `typedef`, lo que
desambigua su uso como tipo.

#### Análisis sintáctico (Bison)

&emsp;La gramática (`BisonGrammar.y`) está libre de contexto y usa un _push
parser_ (`%define api.push-pull push`). En lugar de que el parser tire del lexer,
cada token producido por Flex empuja un paso de parseo mediante `yypush_parse()`,
integrado en el bucle de `Frontend.c`. La gramática define precedencia y
asociatividad al estilo de C, de la asignación (la más baja, asociativa a
derecha) hasta los operadores unarios y de postfijo (la más alta). Los errores
de sintaxis se informan con mensajes detallados (`%define parse.error detailed`)
y `yyerror` agrega el número de línea del token actual.

&emsp;La estructura de la gramática parte de `program`, formado por items de
nivel superior (declaraciones de variables y funciones, `main`, agregados, enums,
typedefs, directivas e inline C) y, dentro de las funciones, por sentencias
(declaraciones, `ret`, expresiones, control de flujo, `break`/`cnt`).

#### Árbol de Sintaxis Abstracta

&emsp;Las acciones semánticas (`BisonActions.c`) asignan cada nodo con `calloc` y
encadenan las listas (parámetros, sentencias, expresiones, campos) como listas
enlazadas. El AST (`AbstractSyntaxTree.h`) clasifica sus nodos mediante varias
enumeraciones: `TypeKind` (primitivos más `NAMED`, `STRUCT`, `ENUM`, `UNION`,
`POINTER`, `ARRAY`, `FUNCTION_POINTER`), `ProgramItemKind`, `StatementKind`,
`ExpressionKind` y `ExpressionOperator`. Cada acción de declaración de `typedef`
registra su nombre, lo que habilita el reetiquetado posterior en el lexer.

&emsp;Una particularidad es el retorno implícito: al cerrar el cuerpo de una
función, si la última sentencia es una expresión que consiste en un identificador
o en operadores de comparación o lógicos, se la convierte en una sentencia de
retorno. Los `DEDENT` de cierre y el manejo del cuerpo viven en estas acciones.

### 3.2. Backend

&emsp;El backend opera sobre el AST en dos fases: análisis semántico y generación
de código.

#### Análisis semántico

&emsp;El analizador (`SemanticAnalyzer.c`, apoyado en `SemanticSymbolTable.c`)
recorre el AST con un contexto, `SemanticAnalysisContext`, que sostiene la tabla
de símbolos, el tipo de retorno de la función en curso, la profundidad de bucles
y de `switch` (para validar `break` y `cnt`) y una bandera de error acumulado. La
validación se realiza en dos pasadas:

1. **Colección global.** Registra todos los símbolos de nivel superior
   (variables globales, funciones y sus prototipos, agregados, enums, typedefs y
   constantes de enum) sin validar todavía los cuerpos. Las redeclaraciones de
   funciones deben tener firmas compatibles y los duplicados se rechazan.
2. **Validación.** Recorre los cuerpos de las funciones y las sentencias,
   validando tipos, expresiones, llamadas, retornos y control de flujo.

&emsp;La tabla de símbolos implementa _scopes_ léxicos encadenados, con dos
espacios de nombres separados: el ordinario (variables, funciones, typedefs,
constantes de enum) y el de _tags_ (`struct`, `union`, `enum`). Al entrar a un
bloque se apila un scope y al salir se desapila; la búsqueda recorre la cadena
desde el scope actual hacia la raíz, y la detección de duplicados consulta solo
el scope actual.

&emsp;Las validaciones cubren, entre otras:

1. tipos nombrados existentes; prohibición de variables, parámetros y campos
   `void`; prohibición de funciones que retornen arreglos; cotas de arreglo
   enteras y mayores que cero; arreglos sin tamaño solo como parámetros;
2. distinción entre _lvalue_ y _rvalue_, y rechazo de la asignación a _lvalues_
   de solo lectura (`const`);
3. compatibilidad de asignaciones e inicializadores, con conversiones aritméticas
   habituales, decaimiento de arreglo a puntero, conversiones desde y hacia
   `void *`, `null`, agregado de `const` y coincidencia exacta de firmas de
   puntero a función; compatibilidad nominal de agregados;
4. expresiones: aritméticas, comparaciones, lógicas, bit a bit, dirección (`&`),
   dereferencia (`*`), indexación, acceso por punto y por flecha, literales y
   llamadas;
5. llamadas a función: existencia, aridad y compatibilidad de los argumentos;
6. retornos: `void` no debe retornar valor, las funciones no-`void` deben
   retornar un valor compatible, y se exige que **todos** los caminos de una
   función no-`void` garanticen el retorno (cadenas `if`/`else` con todas las
   ramas cubiertas, `switch` con `default`, bucles siempre verdaderos sin
   `break`);
7. control de flujo: `break` solo dentro de un bucle o `switch`, `cnt` solo
   dentro de un bucle, y condiciones compatibles con su uso.

&emsp;El inline C se trata como un bloque opaco: se copia a la salida pero no se
verifica su tipo. Esta limitación es deliberada y se documenta como frontera del
análisis.

#### Generación de código C

&emsp;El generador (`CodeGenerator.c`) emite C determinista por la salida
estándar con un contexto que sostiene el flujo de salida, el nivel de indentación
(cuatro espacios por nivel) y una bandera de error de E/S. La emisión se realiza
en un orden estable de fases que respeta las reglas de declaración previa de C:

1. directivas de preprocesador;
2. inline C global;
3. enums;
4. agregados (`struct`/`union`);
5. typedefs;
6. prototipos de funciones;
7. variables globales;
8. definiciones de funciones;
9. `main`.

&emsp;Este orden garantiza que los tipos se definan antes de usarse en los
prototipos, que las funciones puedan llamarse entre sí mediante sus prototipos y
que los inicializadores globales puedan referenciar funciones.

&emsp;La traducción de tipos asigna cada `TypeKind` a su forma en C (`uint` →
`unsigned int`, `uli` → `unsigned long int`, etc.). Los declaradores complejos
—punteros, arreglos y punteros a función— se construyen de adentro hacia afuera
(`_buildDeclarator`), siguiendo la sintaxis "inside-out" de C, agregando
paréntesis donde la precedencia lo requiere (por ejemplo `(*nombre)(params)` para
un puntero a función, o `(*nombre)[N]` para un puntero a arreglo). Otros detalles
de traducción: los literales octales `0o…` se reescriben a la forma `0…` de C,
`null` se emite como `((void *)0)`, los literales de arreglo se separan con comas
y `main` se emite con la firma fija `int main(int argc, char *argv[])`.

&emsp;La estrategia de paréntesis es deliberadamente conservadora: toda operación
binaria y unaria (salvo el postfijo) se emite entre paréntesis, y el receptor de
una indexación o de un acceso a miembro también se parentiza. El resultado es C
válido y sin ambigüedad de precedencia, a costa de un poco más de texto.

### 3.3. Adicionales

&emsp;Más allá del compilador, el proyecto incluye:

1. **Skill `writing-tel`.** El repositorio publica una _Agent Skill_ que enseña a
   un agente (como Claude) a escribir TEL correcto. Se instala con
   `npx skills add https://github.com/BautistaPessagno/TEL --skill writing-tel`.
2. **Entorno reproducible.** Todo el desarrollo y las pruebas corren dentro de un
   servicio de Docker Compose, de modo que los artefactos de Linux generados
   coincidan con el entorno de corrección.
3. **Integración continua.** El repositorio expone un _badge_ de pipeline que
   corre el build y la suite de tests sobre la rama de desarrollo.
4. **Detección de errores de memoria.** El build usa `-fsanitize=address`, de
   modo que las fugas y los errores de memoria hacen fallar los tests dentro del
   contenedor.
5. **Política de runtime.** TEL no provee un runtime propio. El C generado
   depende del estándar C, de los `#include` del usuario, del inline C y del
   _toolchain_ C. Esta decisión mantiene el artefacto simple y portable; la
   contracara es que un programa que dependa de funciones externas debe proveer
   su implementación (o un inline C) para poder enlazarse en una prueba de
   integración ejecutable.

#### Pruebas

&emsp;Los tests son programas TEL ubicados en `src/test/c/accept/` (deben
terminar con estado `0`) y `src/test/c/reject/` (deben terminar con estado
distinto de cero). La suite actual contiene **52 casos de aceptación** y **114
casos de rechazo**, ejecutados por `src/main/bash/test.sh`.

&emsp;La cobertura abarca tres niveles. En el nivel **léxico/sintáctico**, los
rechazos incluyen indentación inconsistente o de ancho inválido, _dedent_ a un
nivel inexistente, declaraciones al estilo C, `elif` huérfano, cabeceras
malformadas de `for` y `switch`, inline C sin cerrar y comentarios de bloque sin
cerrar. En el nivel **semántico**, los rechazos verifican redefinición de
prototipos, llamadas adelantadas, asignaciones de tipo incompatible, falta de
retorno garantizado, `switch` sin `default`, cadenas demasiado largas para un
arreglo de `char`, retorno de un valor `void`, `break`/`cnt` fuera de contexto,
indexación de un no-arreglo, dereferencia de un no-puntero, campo `void` en un
agregado y punteros a objeto incompatibles. Entre los **casos de aceptación** se
incluyen pruebas unitarias por construcción y casos de integración que combinan
varias características, como un programa de punteros y arreglos, un programa mixto
y la resolución del problema _Two Sum_.

#### Programas de demostración

&emsp;El directorio `demo/leetcode/` agrega ocho programas que resuelven
problemas clásicos de estilo LeetCode: _Two Sum_, _Fizz Buzz_, _Fibonacci_,
_Binary Search_, _Reverse Integer_, _Palindrome Number_, _Maximum Subarray_
(algoritmo de Kadane) y _Climbing Stairs_. Cada programa se valida de extremo a
extremo dentro del contenedor: se traduce de TEL a C con el compilador, el C
resultante se compila con `gcc -Wall` y el binario se ejecuta, comparando su
salida —impresa con `printf`, tratado como función de biblioteca externa— contra
el resultado esperado del problema. Estos programas, además de servir como
ejemplos de uso del lenguaje, son la base de la medición de tokens de la
Tabla 2.1.

&emsp;Esta validación expuso una limitación del generador de código: un literal
negativo dentro de un literal de arreglo separado por espacios (por ejemplo
`{-2 1 -3 4}`) se interpreta como una resta entre elementos consecutivos
(`1 - 3`), produciendo un arreglo con menos elementos de los esperados.
`max-subarray.tel` evita el problema inicializando el arreglo elemento por
elemento; los demás literales de arreglo del proyecto solo usan valores
positivos y no se ven afectados. Esta limitación queda documentada como trabajo
futuro de la generación de código.

### 3.4. Dificultades Encontradas

&emsp;Las principales dificultades durante el desarrollo fueron:

1. **Indentación significativa en Flex.** Flex no maneja bloques por indentación
   de forma nativa. Hubo que implementar una pila de niveles, generar `INDENT` y
   `DEDENT` de forma explícita y vaciar los `DEDENT` pendientes al fin de
   archivo para que Bison supiera dónde empieza y termina cada bloque.
2. **Ambigüedad del operador `->`.** La flecha aparece tanto en el operador de
   acceso por puntero como en el tipo de retorno de las funciones. Se resolvió
   con la _start condition_ AFTER_POSTFIX, que solo interpreta `->` como acceso
   cuando viene después de un identificador o de un cierre.
3. **Sintaxis de declaradores en C.** Los punteros, los arreglos y, sobre todo,
   los punteros a función tienen una sintaxis "inside-out" delicada. Generar
   declaradores válidos exigió construirlos de adentro hacia afuera y parentizar
   con cuidado.
4. **El iterador implícito de `ford`.** El AST debía conservar la información
   suficiente para emitir una inicialización C válida del iterador. Se decidió
   que `ford i inicio fin` introduce un iterador local implícito de tipo `int`,
   registrado en el nodo con una bandera (`declaresIterator`).
5. **Decaimiento de arreglos a punteros.** Hubo que separar la compatibilidad de
   tipos de TEL del decaimiento de arreglos a punteros que ocurre cuando se pasan
   como parámetros.
6. **Garantía de retorno por todos los caminos.** Determinar si una función
   no-`void` retorna en todos sus caminos requirió analizar cadenas `if`/`else`,
   `switch` con `default` y bucles siempre verdaderos.
7. **Migración del backend heredado.** El proyecto base incluía un backend de
   calculadora atado al AST viejo. Hubo que reemplazarlo por las fases de
   análisis semántico y generación de TEL sin romper el frontend.

---

## 4. Futuras Extensiones

&emsp;A partir del estado actual, las extensiones naturales son:

1. **Dirección C → TEL.** Completar la transformación biyectiva implementando el
   sentido inverso, de modo de poder comprimir código C existente a TEL.
2. **Inferencia de tipos.** Permitir declaraciones como `x = 5` sin tipo
   explícito, lo que reduciría aún más los tokens.
3. **Ubicaciones de fuente en el AST.** Hoy los nodos del AST no llevan
   posición, por lo que los errores semánticos se reportan sin número de línea.
   Agregar la ubicación mejoraría los diagnósticos.
4. **Verificación parcial del inline C.** Hoy el inline C es opaco; podría
   verificarse al menos parcialmente.
5. **Medición empírica automatizada del ahorro de tokens.** Integrar un
   tokenizador al pipeline de pruebas para medir el ahorro de forma sistemática y
   por modelo.
6. **Ampliar el subconjunto de C.** Incorporar más construcciones a medida que se
   demuestren valiosas para los flujos humano-LLM.

---

## 5. Conclusiones

&emsp;El Stage III cumple su objetivo: el compilador recorre el pipeline completo
desde el texto TEL hasta el código C, con análisis léxico, sintáctico, semántico
y generación de código funcionando de extremo a extremo. El generador produce C
determinista y compilable para el subconjunto documentado, y el análisis
semántico rechaza los programas que el parser por sí solo no puede distinguir.

&emsp;Las mediciones muestran que TEL reduce de forma consistente la cantidad de
tokens respecto del C equivalente, con el mayor ahorro en los tipos compuestos y
en las directivas, lo que valida la premisa del dominio. La suite de 52 casos de
aceptación y 114 de rechazo, junto con casos de integración como _Two Sum_,
respalda la robustez del compilador y deja en evidencia el comportamiento del
lenguaje frente a programas reales.

&emsp;El proyecto queda autocontenido y reproducible: una persona externa puede
clonar el repositorio, construir y probar el compilador dentro de Docker, generar
C a partir de TEL y compilarlo con el _toolchain_ C estándar.

---

## 6. Referencias

&emsp;Material citado explícitamente en este documento:

1. Cátedra de Autómatas, Teoría de Lenguajes y Compiladores. _Proyecto Especial
   — Diseño e Implementación de un Lenguaje_ (2026-03-12). ITBA.
2. Cátedra ATLC. _Análisis Léxico_ (v1.0.0, 2025-09-03).
3. Cátedra ATLC. _Análisis Sintáctico_ (v0.1.0, 2024-05-08).
4. Cátedra ATLC. _Análisis Semántico_ (v0.2.0, 2024-09-25).
5. Cátedra ATLC. _Generación de Código_ (v0.1.0, 2024-05-16).
6. Cátedra ATLC. _Runtime_ (v0.1.0, 2024-05-16).
7. OpenAI. _Tokenizer_. <https://platform.openai.com/tokenizer>.
8. Repositorio del proyecto TEL. <https://github.com/BautistaPessagno/TEL>.
9. Especificación TEL — Stage I (`doc/TEL - Stage 1.pdf`) y requerimientos
   (`doc/requirements.md`).

---

## 7. Bibliografía

&emsp;Material consultado que no se cita de forma directa en el texto:

1. Aho, A., Lam, M., Sethi, R. y Ullman, J. _Compilers: Principles, Techniques,
   and Tools_ (2.ª edición), 2006.
2. _Flex — The Fast Lexical Analyzer Generator_, documentación oficial del
   proyecto GNU Flex.
3. _Bison — GNU Parser Generator_, manual oficial del proyecto GNU Bison.
4. Fowler, M. _Phoenix Server_ (2021-07-10) y Morris, K. _Immutable Server_
   (2013-06-13), sobre entornos reproducibles.
