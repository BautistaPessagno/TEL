# Next Steps - Stage 2

## Objetivo de Stage 2

Stage 2 consiste en construir el frontend del compilador TEL.

La aplicacion debe:

1. Leer un programa TEL desde la entrada.
2. Transformarlo en un stream de tokens usando Flex.
3. Parsear esos tokens usando Bison.
4. Construir un AST que represente el programa.
5. Aceptar o rechazar los casos de prueba segun reglas lexicas y sintacticas.

No hace falta generar codigo C todavia. La generacion de codigo, validaciones semanticas completas, type-checking, includes automaticos y traduccion TEL -> C pertenecen a Stage 3.

## Estado actual

El proyecto todavia esta basado en la calculadora original:

- `src/main/c/frontend/lexical-analysis/FlexPatterns.l` reconoce enteros, operadores aritmeticos, parentesis, comentarios e imports.
- `src/main/c/frontend/syntactic-analysis/BisonGrammar.y` parsea solo `program: expression`.
- `src/main/c/frontend/syntactic-analysis/AbstractSyntaxTree.h` define un AST de calculadora.
- `src/test/c/accept` y `src/test/c/reject` tienen tests de expresiones aritmeticas.
- El backend esta comentado en `src/main/c/EntryPoint.c`, lo cual esta bien para Stage 2.

La tarea principal es reemplazar el frontend de calculadora por un frontend de TEL.

## 1. Definir el alcance real de TEL

Archivo sugerido:

- `README.md`
- opcionalmente `doc/grammar.md`

Por que:

Stage 2 pide definir la gramatica del lenguaje. Conviene dejar por escrito que subset de TEL se implementa en esta entrega.

Con que:

Implementar al menos las construcciones que van a aparecer en los tests:

- funciones con `fn`, parametros, `->` y retorno opcional;
- tipos compactos como `int`, `char`, `float`, `double`, `v`, `uint`, `uli`;
- declaraciones `x:int = 5`;
- bloques por indentacion;
- `ret`;
- condicionales `?`, `elif`, `else`;
- loops `fd`, `for`, `w`, `dw`;
- `sw`, `case`, `df`, `brk`, `cnt`;
- `str`, `en`, `td`;
- `#include` y `#define`;
- literales enteros, flotantes, chars, strings y `null`;
- expresiones con operadores de C;
- arrays, punteros basicos y llamadas a funcion.

## 2. Redisenar el AST

Archivos a cambiar:

- `src/main/c/frontend/syntactic-analysis/AbstractSyntaxTree.h`
- `src/main/c/frontend/syntactic-analysis/AbstractSyntaxTree.c`

Por que:

El AST actual solo representa:

```text
Program -> Expression -> Factor -> Constant
```

TEL necesita representar programas completos.

Con que:

Definir nodos para:

```text
Program
TopLevelDeclaration
FunctionDeclaration
StructDeclaration
EnumDeclaration
TypedefDeclaration
PreprocessorDirective
Type
Parameter
Block
Statement
Expression
Literal
Identifier
```

En `AbstractSyntaxTree.c`, agregar destructores recursivos para todos los nodos nuevos.

## 3. Actualizar la union semantica de Bison

Archivo a cambiar:

- `src/main/c/frontend/syntactic-analysis/BisonGrammar.y`

Por que:

La `%union` actual solo tiene tipos de la calculadora. Los tokens y no-terminales de TEL necesitan transportar strings, tipos, statements, bloques, funciones y expresiones.

Con que:

Agregar campos similares a:

```c
%union {
    signed int integer;
    double floating;
    char * string;
    TokenLabel token;

    Program * program;
    TopLevelDeclaration * topLevelDeclaration;
    FunctionDeclaration * functionDeclaration;
    Statement * statement;
    Expression * expression;
    Type * type;
    Block * block;
    ParameterList * parameterList;
}
```

Tambien actualizar:

- `%token`;
- `%type`;
- `%destructor`.

## 4. Rehacer el lexer

Archivos a cambiar:

- `src/main/c/frontend/lexical-analysis/FlexPatterns.l`
- `src/main/c/frontend/lexical-analysis/FlexActions.c`
- `src/main/c/frontend/lexical-analysis/FlexActions.h`

Por que:

El lexer actual solo reconoce una calculadora. TEL necesita keywords, identificadores, tipos, literales, operadores, saltos de linea e indentacion.

Con que:

Agregar tokens para:

```text
fn ret -> ? elif else fd for w dw sw df brk cnt
str en td stc
int float double char v uint uli
IDENTIFIER
INTEGER_LITERAL FLOAT_LITERAL CHAR_LITERAL STRING_LITERAL NULL_LITERAL
NEWLINE INDENT DEDENT
+ - * / % == != < > <= >= && || ! = += -= *= /=
( ) [ ] , : . #
```

Las reglas de keywords deben ir antes que la regla general de identificadores.

## 5. Implementar indentacion

Archivos a cambiar:

- `src/main/c/frontend/lexical-analysis/FlexActions.c`
- `src/main/c/frontend/lexical-analysis/FlexPatterns.l`
- opcionalmente `src/main/c/support/type/LexicalAnalyzer.h`

Por que:

TEL usa bloques por indentacion. Si Flex ignora espacios y tabs, Bison no puede saber donde empieza o termina un bloque.

Con que:

Implementar una pila de niveles de indentacion:

```text
al inicio de linea:
  contar espacios
  si el nivel sube:
    emitir INDENT
  si el nivel baja:
    emitir uno o mas DEDENT
  si el nivel no coincide con uno anterior:
    rechazar por indentacion inconsistente
```

Para simplificar, conviene rechazar mezcla de tabs y espacios.

## 6. Rehacer la gramatica

Archivo a cambiar:

- `src/main/c/frontend/syntactic-analysis/BisonGrammar.y`

Por que:

La gramatica actual arranca con una expresion aritmetica. TEL debe arrancar con un programa formado por declaraciones de alto nivel.

Con que:

Estructura base recomendada:

```bison
program:
    topLevelList
;

topLevel:
    functionDeclaration
  | structDeclaration
  | enumDeclaration
  | typedefDeclaration
  | preprocessorDirective
;

functionDeclaration:
    FN IDENTIFIER parameterList returnTypeOpt NEWLINE INDENT statementList DEDENT
;

returnTypeOpt:
    ARROW type
  | %empty
;

statement:
    variableDeclaration
  | assignment
  | returnStatement
  | ifStatement
  | whileStatement
  | forStatement
  | switchStatement
  | breakStatement
  | continueStatement
  | expressionStatement
;
```

Para expresiones, definir precedencia:

```bison
%left OR
%left AND
%left BIT_OR
%left BIT_XOR
%left BIT_AND
%left EQ NE
%left LT GT LE GE
%left SHIFT_LEFT SHIFT_RIGHT
%left ADD SUB
%left MUL DIV MOD
%right NOT BIT_NOT
```

## 7. Rehacer acciones semanticas

Archivos a cambiar:

- `src/main/c/frontend/syntactic-analysis/BisonActions.h`
- `src/main/c/frontend/syntactic-analysis/BisonActions.c`

Por que:

Las acciones actuales crean nodos de calculadora. Las acciones nuevas deben construir el AST de TEL.

Con que:

Crear funciones como:

```c
Program * ProgramSemanticAction(TopLevelDeclarationList * declarations);
FunctionDeclaration * FunctionSemanticAction(char * name, ParameterList * params, Type * returnType, Block * body);
Statement * ReturnStatementSemanticAction(Expression * expression);
Statement * IfStatementSemanticAction(Expression * condition, Block * thenBlock, ElseBranch * elseBranch);
Expression * BinaryExpressionSemanticAction(Expression * left, ExpressionOperator operator, Expression * right);
Expression * IdentifierExpressionSemanticAction(char * name);
```

La accion de `program` debe guardar la raiz en:

```c
compilerState.abstractSyntaxtTree
```

## 8. Desactivar temporalmente el backend viejo

Archivos a cambiar:

- `src/main/c/EntryPoint.c`
- `CMakeLists.txt`

Por que:

`Calculator.c` y `Generator.c` dependen del AST viejo. Cuando el AST cambie, esos modulos ya no van a compilar correctamente.

Con que:

En `EntryPoint.c`, dejar solo:

- inicializacion de frontend y AST;
- ejecucion de `executeSyntacticAnalysis()`;
- destruccion del AST;
- retorno `0` si el parseo fue correcto;
- retorno distinto de `0` si fallo.

Sacar temporalmente:

- `initializeCalculatorModule()`;
- `initializeGeneratorModule()`;
- includes de backend que ya no se usen.

En `CMakeLists.txt`, sacar temporalmente:

- `src/main/c/backend/code-generation/Generator.c`;
- `src/main/c/backend/domain-specific/Calculator.c`.

Esto es valido para Stage 2 porque el backend es parte de Stage 3.

## 9. Reemplazar los tests

Carpetas a cambiar:

- `src/test/c/accept`
- `src/test/c/reject`

Por que:

Los tests actuales son de calculadora. Stage 2 debe probar TEL.

Con que:

Accept recomendados:

```text
01-main-print
02-array-for
03-function-return
04-if-elif-else-while
05-struct
06-typedef-struct-literals
07-include-define
08-pointers-function-pointer
09-enum-switch
10-nqueens-small
```

Reject recomendados:

```text
01-bad-indentation
02-reserved-keyword-as-id
03-malformed-expression
04-invalid-return-type
05-unclosed-string
```

`division-by-zero` no deberia ser reject de Stage 2 porque no es un error lexico ni sintactico.

## 10. Actualizar README

Archivo a cambiar:

- `README.md`

Por que:

El README todavia describe el proyecto base. La entrega debe ser autocontenida.

Con que:

Agregar:

- descripcion de TEL;
- alcance de Stage 2;
- comandos de build, run y test;
- gramatica resumida;
- features implementadas;
- aclaracion de que Stage 2 construye AST pero no genera C todavia.

## Orden recomendado

1. Escribir la gramatica objetivo.
2. Disenar el AST.
3. Actualizar `%union`, `%token`, `%type` y `%destructor`.
4. Rehacer el lexer sin indentacion avanzada.
5. Parsear funciones y declaraciones simples.
6. Agregar `NEWLINE`, `INDENT` y `DEDENT`.
7. Agregar statements.
8. Agregar expresiones con precedencia.
9. Agregar structs, enums, typedefs y preprocesador.
10. Desactivar backend viejo si rompe con el nuevo AST.
11. Reemplazar tests de calculadora por tests TEL.
12. Correr `src/main/bash/build.sh` dentro de Docker.
13. Correr `src/main/bash/test.sh` dentro de Docker.

## Criterio de listo

Stage 2 esta listo cuando:

- el proyecto compila dentro del contenedor Docker;
- Flex reconoce el alfabeto TEL;
- Bison reconoce la gramatica TEL sin conflictos no justificados;
- un programa valido genera un AST y termina con exit status `0`;
- un programa invalido falla con exit status distinto de `0`;
- `src/main/bash/test.sh` pasa con los casos accept/reject de TEL;
- el repositorio documenta el alcance real de la entrega.
