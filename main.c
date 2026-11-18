#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* =========================================================================
   DECLARACIONES ADELANTADAS
   ========================================================================= */
struct Sala;
struct NodoCliente;
struct Cliente;
struct Titulo;
struct NodoTitulo;
struct NodoListaEspera;
struct Pase;
struct NodoPase;
struct Tienda;

struct Titulo *buscarTituloEnSalas(struct Tienda *ibcomics, char *nombreBuscar);
struct Titulo *buscarTituloPorNombre(struct NodoTitulo *head, char *nombreBuscado);
int calcularTiempoEstimadoEsperaDias(struct Titulo *tituloEncontrado);
int contarCantidadClientesEnListaEspera(struct NodoListaEspera *head);
int eliminarClienteDeListaEspera(struct NodoListaEspera **head, int idBuscado);
void desvincularPasesDeSala(struct NodoCliente *raiz, struct Sala *sala);
void liberarCliente(struct Cliente *cliente);
int vaciarListaEspera(struct NodoListaEspera **head);
/* =========================================================================
   ESTRUCTURAS GLOBALES DEL PROYECTO
   ========================================================================= */
struct Pase {
    int idPase;
    int tipoPase; /* 1:General / 2:Infantil / 3:Familiar / 4:Prioritario  */
    int valor;
    int estado; /* 1:Activo / 2:Utilizado / 3:Anulado / 4:Vencido */
    char *horarioUso; /*Maniana, Tarde, Noche*/
    int arregloFecha[5]; /*[0]: Dia / [1]: Mes / [2]: Anio / [3]: Hora / [4]: Minuto*/
    struct Sala *datosSala;
};

struct NodoPase {
    struct Pase *datosPase;
    struct NodoPase *sig;
};

struct Cliente {
    struct NodoPase *headPase;
    struct Sala *datosSala;
    char rutCliente[13];
    char *nombreCliente;
    int edad;
    int idCliente;
    char *correo;
};

struct NodoCliente {
    struct Cliente *datosCliente;
    struct NodoCliente *der, *izq;
};

struct NodoListaEspera {
    struct Cliente *datosCliente;
    struct NodoListaEspera *sig;
};

struct Titulo {
    struct NodoListaEspera *headListaEspera;
    int idTitulo;
    char *nombreTitulo;
    int tipoTitulo; /* 1:SerieRegular / 2:MiniSerie / 3:TomoRecopilatorio / 4:NovelaGrafica / 5:Comic / 6:Fanzine */
    int clasificacionEdad;
    int precio;
    int cantidadEjemplares;
    int estado; /* 1:Disponible / 2:Agotado / 3:Preventa / 4:Descatalogado */
    int estadoListaEspera; /* 1:Activo / 2:Suspendido */
    int ejemplaresPorDespacho;
    int diasDemoraDespacho;
};

struct NodoTitulo {
    struct Titulo *datosTitulo;
    struct NodoTitulo *ant, *sig;
};

struct Sala {
    char *nombreSala;
    int idSala;
    int tematica; /* 1:Superheroe / 2:Manga y Anime / 3:Novela Grafica / 4:Comic Independiente / 5:Infantil y Humor Grafico / 6:Coleccionista */
    char *horario;  /*Maniana, Tarde, Noche*/
    int arregloFecha[5];  /*[0]: Dia / [1]: Mes / [2]: Anio / [3]: Hora / [4]: Minuto*/
    int aforoMax;
    int aforoActual; /*personas dentro de la sala*/
    int cantTrabajadores;
    struct NodoTitulo *headNodoTitulo;
};

struct Tienda {
    char *nombreTienda;
    int cantidadSalas;
    struct Sala **salas;
    struct NodoCliente *raiz;
    int diaAbierto;   /*1: Abierto / 2: Cerrado*/
    int arregloFechaAct[3];  /*[0]: Dia / [1]: Mes / [2]: Anio */
    int totalClientes;  /*dentro de la tienda en el dia*/
};
/* =========================================================================
   UTILIDADES
   ========================================================================= */
/* inputEntero contra puntos, guiones y letras usando fgets */
int inputEntero(void) {
    char buffer[100];
    int numero;
    int i;
    int esValido;

    while (1) {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            printf("ERROR: Ingrese un numero valido: ");
            continue;
        }
        buffer[strcspn(buffer, "\n")] = '\0';

        if (buffer[0] == '\0') {
            printf("ERROR: Ingrese un numero valido: ");
            continue;
        }
        esValido = 1;
        for (i = 0; buffer[i] != '\0'; i++) {
            if (i == 0 && buffer[i] == '-') {
                continue;
            }
            if (buffer[i] < '0' || buffer[i] > '9') {
                esValido = 0;
                break;
            }
        }
        if (!esValido) {
            printf("ERROR: Ingrese un numero valido (sin puntos, letras ni simbolos): ");
            continue;
        }
        numero = atoi(buffer);
        return numero;
    }
}

/* Reserva memoria y devuelve una copia del texto */
char *copiarTexto(const char *textoOriginal) {
    char *copia;
    if (textoOriginal == NULL) {
        return NULL;
    }
    copia = (char *) malloc(strlen(textoOriginal) + 1);
    if (copia != NULL) {
        strcpy(copia, textoOriginal);
    }
    return copia;
}
/*Valida que una cadena solo contenga letras sin nigun caracter distinto*/
int validarNombreSoloLetrasYEspacios(const char *cadenaNombre) {
    int indice;
    char caracterActual;

    if (cadenaNombre == NULL || cadenaNombre[0] == '\0') {
        return 0;
    }
    for (indice = 0; cadenaNombre[indice] != '\0'; indice++) {
        caracterActual = cadenaNombre[indice];
        if (!((caracterActual >= 'A' && caracterActual <= 'Z') ||
              (caracterActual >= 'a' && caracterActual <= 'z') ||
              caracterActual == ' ')) {
            return 0;
        }
    }
    return 1;
}

/* =========================================================================
   MODULO DE CLIENTES Y ARBOL 
   ========================================================================= */
/*Busca a un cliente en el arbol binario utilizando el id*/
struct Cliente *buscarClientePorID(struct NodoCliente *raiz, int idBuscado) {
    if (raiz == NULL) {
        return NULL;
    }
    if (raiz->datosCliente->idCliente == idBuscado) {
        return raiz->datosCliente;
    }
    if (raiz->datosCliente->idCliente < idBuscado) {
        return buscarClientePorID(raiz->der, idBuscado);
    } else {
        return buscarClientePorID(raiz->izq, idBuscado);
    }
}
/*Busca a un cliente en el arbol recorruendo de forma recursiva por el rut*/
struct Cliente *buscarClientePorRut(struct NodoCliente *raiz, char *rut) {
    struct Cliente *clienteEncontrado;
    if (raiz == NULL) {
        return NULL;
    }
    if (strcmp(raiz->datosCliente->rutCliente, rut) == 0) {
        return raiz->datosCliente;
    }
    clienteEncontrado = buscarClientePorRut(raiz->izq, rut);
    if (clienteEncontrado != NULL) {
        return clienteEncontrado;
    }
    return buscarClientePorRut(raiz->der, rut);
}
/*Detecta si la entrada es un rut o si es un id Numerico*/
struct Cliente *buscarClientePorRutOIdArbol(struct NodoCliente *raiz, char *rutId) {
    int i;
    if (raiz == NULL || rutId == NULL || rutId[0] == '\0') {
        return NULL;
    }
    for (i = 0; rutId[i] != '\0'; i++) {
        if (rutId[i] == '-' || rutId[i] == '.') {
            return buscarClientePorRut(raiz, rutId);
        }
    }
    return buscarClientePorID(raiz, atoi(rutId));
}
/*Verfica y pide a un cliente  ap partir de la tienda*/
struct Cliente *verificarPedirCliente(struct Tienda *ibcomics, char *texto) {
    if (ibcomics == NULL) {
        return NULL;
    }
    return buscarClientePorRutOIdArbol(ibcomics->raiz, texto);
}
/*Genera un ID aleatorio unico que no este repetido en el arbol de clientes*/
int generarIdUnico(struct NodoCliente *raiz) {
    int nuevoId;
    int esUnico;
    nuevoId = 0;
    esUnico = 0;

    while (esUnico != 1) {
        nuevoId = (rand() % 10000) + (rand() % 10000) + 1;
        if (buscarClientePorID(raiz, nuevoId) == NULL) {
            esUnico = 1;
        }
    }
    return nuevoId;
}
/*Resreva la memoria e inicializa una estrucura de clientes y sus pases*/
struct Cliente *crearEstructuraCliente(int id, const char *nombre, const char *rut, int edad, const char *correo) {
    struct Cliente *nuevoCliente;

    nuevoCliente = (struct Cliente *)malloc(sizeof(struct Cliente));
    if (nuevoCliente == NULL) {
        return NULL;
    }

    nuevoCliente->idCliente = id;
    nuevoCliente->edad = edad;
    strcpy(nuevoCliente->rutCliente, rut);

    nuevoCliente->nombreCliente = copiarTexto(nombre);
    nuevoCliente->correo = copiarTexto(correo);

    if (nuevoCliente->nombreCliente == NULL || nuevoCliente->correo == NULL) {
        free(nuevoCliente->nombreCliente);
        free(nuevoCliente->correo);
        free(nuevoCliente);
        return NULL;
    }

    nuevoCliente->datosSala = NULL;
    nuevoCliente->headPase = (struct NodoPase *)malloc(sizeof(struct NodoPase));
    if (nuevoCliente->headPase == NULL) {
        free(nuevoCliente->nombreCliente);
        free(nuevoCliente->correo);
        free(nuevoCliente);
        return NULL;
    }
    nuevoCliente->headPase->datosPase = NULL;
    nuevoCliente->headPase->sig = NULL;

    return nuevoCliente;
}
/* Encuentra el nodo padre adecuado para insertar un nuevo cliente en el ABB*/
struct NodoCliente *encontrarPadreParaCliente(struct NodoCliente *raiz, int idNuevo) {
    struct NodoCliente *actual;

    if (raiz != NULL) {
        actual = raiz;
        while (actual != NULL) {
            if (actual->datosCliente->idCliente < idNuevo) {
                if (actual->der == NULL) {
                    return actual;
                }
                actual = actual->der;
            } else {
                if (actual->izq == NULL) {
                    return actual;
                }
                actual = actual->izq;
            }
        }
    }
    return NULL;
}
/*Inserta un nodo cliente dentro del árbol binario de búsqueda*/
int insertarClienteEnArbol(struct NodoCliente **raiz, struct Cliente *nuevoCliente) {
    struct NodoCliente *nuevoNodo;
    struct NodoCliente *padre;

    if (nuevoCliente == NULL) {
        return 0;
    }

    nuevoNodo = (struct NodoCliente *)malloc(sizeof(struct NodoCliente));
    if (nuevoNodo == NULL) {
        return 0;
    }
    nuevoNodo->datosCliente = nuevoCliente;
    nuevoNodo->izq = NULL;
    nuevoNodo->der = NULL;

    padre = encontrarPadreParaCliente(*raiz, nuevoCliente->idCliente);
    if (padre == NULL) {
        *raiz = nuevoNodo;
    } else if (padre->datosCliente->idCliente < nuevoCliente->idCliente) {
        padre->der = nuevoNodo;
    } else {
        padre->izq = nuevoNodo;
    }
    return 1;
}
/* Libera de forma segura toda la memoria asociada a un cliente y sus pases*/
void liberarCliente(struct Cliente *cliente) {
    struct NodoPase *actual;
    struct NodoPase *siguiente;

    if (cliente == NULL) {
        return;
    }

    actual = cliente->headPase ? cliente->headPase->sig : NULL;
    while (actual != NULL) {
        siguiente = actual->sig;
        if (actual->datosPase != NULL) {
            /* Liberar memoria dinámica interna del pase si fue asignada */
            if (actual->datosPase->horarioUso != NULL) {
                free(actual->datosPase->horarioUso);
            }
            free(actual->datosPase);
        }
        free(actual);
        actual = siguiente;
    }
    
    if (cliente->headPase) {
        free(cliente->headPase);
    }
    if (cliente->nombreCliente) {
        free(cliente->nombreCliente);
    }
    if (cliente->correo) {
        free(cliente->correo);
    }
    free(cliente);
}
/* Remueve al cliente de las listas de espera de todas las salas de la tienda.*/
int removerClienteDeListasDeEspera(struct Tienda *tienda, int idCliente) {
    int i;
    int totalRemovidos;
    struct NodoTitulo *tituloActual;

    totalRemovidos = 0;
    if (tienda == NULL || tienda->salas == NULL) {
        return 0;
    }

    for (i = 0; i < tienda->cantidadSalas; i++) {
        if (tienda->salas[i] != NULL && tienda->salas[i]->headNodoTitulo != NULL) {
            tituloActual = tienda->salas[i]->headNodoTitulo->sig;
            while (tituloActual != NULL) {
                if (tituloActual->datosTitulo != NULL &&
                    eliminarClienteDeListaEspera(&(tituloActual->datosTitulo->headListaEspera), idCliente) == 1) {
                    totalRemovidos++;
                }
                tituloActual = tituloActual->sig;
            }
        }
    }
    return totalRemovidos;
}
/*Cuenta cuántos pases válidos posee un cliente registrado.*/
int contarPasesCliente(struct Cliente *cliente) {
    int totalPases;
    struct NodoPase *actual;

    totalPases = 0;
    if (cliente == NULL || cliente->headPase == NULL) {
        return 0;
    }

    actual = cliente->headPase->sig;
    while (actual != NULL) {
        if (actual->datosPase != NULL) {
            totalPases++;
        }
        actual = actual->sig;
    }
    return totalPases;
}
/* Procesa la eliminación física de un nodo cliente dentro del ABB y sus dependencias.*/
int procesarEliminacionClienteArbol(struct Tienda *tienda, int idEliminar, int *pasesBorrados, int *listasBorradas) {
    struct NodoCliente **raiz;
    struct NodoCliente *aEliminar;
    struct NodoCliente *padre;
    struct NodoCliente *hijo;
    struct NodoCliente *sucesor;
    struct NodoCliente *padreSucesor;

    if (tienda == NULL || tienda->raiz == NULL) {
        return 0;
    }

    raiz = &(tienda->raiz);
    padre = NULL;
    aEliminar = *raiz;

    while (aEliminar != NULL && aEliminar->datosCliente->idCliente != idEliminar) {
        padre = aEliminar;
        if (idEliminar < aEliminar->datosCliente->idCliente) {
            aEliminar = aEliminar->izq;
        } else {
            aEliminar = aEliminar->der;
        }
    }
    if (aEliminar == NULL) {
        return 0;
    }

    *listasBorradas = removerClienteDeListasDeEspera(tienda, idEliminar);
    *pasesBorrados = contarPasesCliente(aEliminar->datosCliente);

    if (aEliminar->izq != NULL && aEliminar->der != NULL) {
        padreSucesor = aEliminar;
        sucesor = aEliminar->der;
        while (sucesor->izq != NULL) {
            padreSucesor = sucesor;
            sucesor = sucesor->izq;
        }
        liberarCliente(aEliminar->datosCliente);
        aEliminar->datosCliente = sucesor->datosCliente;
        if (padreSucesor == aEliminar) {
            padreSucesor->der = sucesor->der;
        } else {
            padreSucesor->izq = sucesor->der;
        }
        free(sucesor);
    } else {
        if (aEliminar->izq != NULL) {
            hijo = aEliminar->izq;
        } else {
            hijo = aEliminar->der;
        }

        if (padre == NULL) {
            *raiz = hijo;
        } else if (padre->izq == aEliminar) {
            padre->izq = hijo;
        } else {
            padre->der = hijo;
        }
        liberarCliente(aEliminar->datosCliente);
        free(aEliminar);
    }
    return 1;
}
/* Muestra por consola los datos detallados de un cliente específico.*/
void mostrarInformacionCliente(struct Cliente *cliente) {
    if (cliente == NULL) return;
    printf(" ============ DATOS DEL CLIENTE ============\n");
    printf(" ID     : %d \n", cliente->idCliente);
    printf(" Nombre : %s \n", cliente->nombreCliente);
    printf(" RUT    : %s \n", cliente->rutCliente);
    printf(" Edad   : %d anos \n", cliente->edad);
    printf(" Correo : %s \n", cliente->correo);
    printf(" ===========================================\n\n");
}

void listarClientes(struct NodoCliente *raiz) {
    if (raiz == NULL) {
        return;
    }
    listarClientes(raiz->izq);
    printf("   [%d] %-25s | RUT: %-12s | Edad: %3d anos \n",
           raiz->datosCliente->idCliente,
           raiz->datosCliente->nombreCliente,
           raiz->datosCliente->rutCliente,
           raiz->datosCliente->edad);
    listarClientes(raiz->der);
}

/* MENU MODIFICAR CLIENTE con validaciones respectovas */
void menuModificarCliente(struct Cliente *clienteModificar) {
    int opcionSeleccionada;
    char bufferTemporal[50];

    if (clienteModificar == NULL) {
        printf("\nError: El cliente a modificar no existe.\n");
        return;
    }

    do {
        printf("\n ============ MENU MODIFICAR CLIENTE ============\n\n");
        printf("     Opcion [1]: Modificar nombre \n");
        printf("     Opcion [2]: Modificar edad \n");
        printf("     Opcion [3]: Modificar correo \n\n");
        printf("     Opcion [0]: Salir al menu anterior \n\n");
        printf("   Por favor, ingrese una opcion: ");
        opcionSeleccionada = inputEntero();
        printf("\n");

        switch (opcionSeleccionada) {
            case 1:
                printf("Nombre actual: %s \n", clienteModificar->nombreCliente);
                printf("Ingrese el nuevo nombre (solo letras sin tildes y espacios): ");
                fgets(bufferTemporal, sizeof(bufferTemporal), stdin);
                bufferTemporal[strcspn(bufferTemporal, "\n")] = '\0';

                while (!validarNombreSoloLetrasYEspacios(bufferTemporal)) {
                    printf("Error: Use solo letras (sin tildes) y espacios. Ingrese nuevamente: ");
                    fgets(bufferTemporal, sizeof(bufferTemporal), stdin);
                    bufferTemporal[strcspn(bufferTemporal, "\n")] = '\0';
                }

                if (clienteModificar->nombreCliente != NULL) {
                    free(clienteModificar->nombreCliente);
                }
                clienteModificar->nombreCliente = copiarTexto(bufferTemporal);
                printf("¡Nombre modificado con exito! Nuevo nombre: %s \n\n", clienteModificar->nombreCliente);
                break;

            case 2:
                printf("Edad actual: %d \n", clienteModificar->edad);
                printf("Ingrese la nueva edad (1-120): ");
                clienteModificar->edad = inputEntero();
                while (clienteModificar->edad < 1 || clienteModificar->edad > 120) {
                    printf("Error: Ingrese una edad valida (1-120): ");
                    clienteModificar->edad = inputEntero();
                }
                printf("¡Edad modificada con exito! Nueva edad: %d \n\n", clienteModificar->edad);
                break;

            case 3:
                printf("Correo actual: %s \n", clienteModificar->correo);
                printf("Ingrese el nuevo correo: ");
                fgets(clienteModificar->correo, 50, stdin);
                clienteModificar->correo[strcspn(clienteModificar->correo, "\n")] = '\0';
                printf("¡Correo modificado con exito! Nuevo correo: %s \n\n", clienteModificar->correo);
                break;

            case 0:
                printf("Saliendo del submenu.\n");
                break;
            default:
                printf("\n ¡ERROR! Opcion no valida. Intente nuevamente. \n");
                break;
        }
    } while (opcionSeleccionada != 0);
}
/*Interfaz para registrar un nuevo cliente validando entradas y unicidad de RUT*/
void registrarCliente(struct NodoCliente **raiz) {
    int idGenerado;
    int edadCliente;
    char nombreCliente[50];
    char rutCliente[13];
    char correoCliente[50];
    struct Cliente *nuevoCliente;

    printf(" ============ MENU REGISTRAR CLIENTE ============\n\n");

    printf("Ingrese el nombre completo (solo letras sin tildes y espacios): ");
    fgets(nombreCliente, sizeof(nombreCliente), stdin);
    nombreCliente[strcspn(nombreCliente, "\n")] = '\0';
    while (!validarNombreSoloLetrasYEspacios(nombreCliente)) {
        printf("Error: Use solo letras (sin tildes) y espacios. Ingrese nuevamente: ");
        fgets(nombreCliente, sizeof(nombreCliente), stdin);
        nombreCliente[strcspn(nombreCliente, "\n")] = '\0';
    }

    printf("Ingrese el RUT del cliente (Ej: 12.345.678-9): ");
    fgets(rutCliente, sizeof(rutCliente), stdin);
    rutCliente[strcspn(rutCliente, "\n")] = '\0';

    if (buscarClientePorRut(*raiz, rutCliente) != NULL) {
        printf("Error: Ya existe un cliente registrado con ese RUT.\n");
        return;
    }

    printf("Ingrese la edad del cliente (1-120): ");
    edadCliente = inputEntero();
    while (edadCliente < 1 || edadCliente > 120) {
        printf("Error: Ingrese una edad valida (1-120): ");
        edadCliente = inputEntero();
    }

    printf("Ingrese el correo del cliente: ");
    fgets(correoCliente, sizeof(correoCliente), stdin);
    correoCliente[strcspn(correoCliente, "\n")] = '\0';

    idGenerado = generarIdUnico(*raiz);
    nuevoCliente = crearEstructuraCliente(idGenerado, nombreCliente, rutCliente, edadCliente, correoCliente);

    if (insertarClienteEnArbol(raiz, nuevoCliente)) {
        printf("\n¡Se ha anadido con exito al cliente!\n");
        printf("Nombre: %s\n", nuevoCliente->nombreCliente);
        printf("RUT: %s\n", nuevoCliente->rutCliente);
        printf("ID asignado automaticamente: %d\n\n", nuevoCliente->idCliente);
    } else {
        printf("Error al registrar el cliente en el arbol.\n");
        liberarCliente(nuevoCliente);
    }
}
/* Interfaz de consola para eliminar un cliente por ID o RUT*/
void eliminarClienteInterfaz(struct Tienda *tienda) {
    int pasesBorrados;
    int listasBorradas;
    char entradaBusqueda[50];
    struct Cliente *clienteAOperar;

    pasesBorrados = 0;
    listasBorradas = 0;

    printf(" ============ MENU ELIMINAR CLIENTE ============\n\n");
    if (tienda == NULL || tienda->raiz == NULL) {
        printf("Error: No existe ningun cliente registrado en el sistema.\n");
        return;
    }
    printf("Ingrese el ID o RUT del cliente a eliminar: ");
    fgets(entradaBusqueda, sizeof(entradaBusqueda), stdin);
    entradaBusqueda[strcspn(entradaBusqueda, "\n")] = '\0';

    clienteAOperar = verificarPedirCliente(tienda, entradaBusqueda);
    if (clienteAOperar == NULL) {
        printf("Error: No se pudo encontrar al cliente ingresado.\n\n");
        return;
    }
    if (procesarEliminacionClienteArbol(tienda, clienteAOperar->idCliente, &pasesBorrados, &listasBorradas)) {
        printf("¡Cliente eliminado del sistema con exito!\n");
        printf("Pases eliminados: %d | Listas de espera de las que fue removido: %d\n", pasesBorrados, listasBorradas);
    } else {
        printf("Error al intentar eliminar al cliente.\n");
    }
}
/*Menú principal de gestión de clientes*/
void mostrarMenuClientes(struct Tienda *tienda) {
    int opcionMenu;
    int opcionValida;
    char entradaBusqueda[50];
    struct Cliente *clienteActual;
    struct NodoCliente **raizClientes;

    clienteActual = NULL;
    raizClientes = &(tienda->raiz);

    do {
        opcionValida = 1;
        printf("\n  =========================================================\n");
        printf("  ||         B I E N V E N I D O   A L   M E N U         ||\n");
        printf("  ||                     C L I E N T E S                 ||\n");
        printf("  =========================================================\n\n");
        printf("      Opcion [1]: Registrar nuevo cliente en el sistema \n");
        printf("      Opcion [2]: Modificar datos de un cliente \n");
        printf("      Opcion [3]: Eliminar cliente del sistema \n");
        printf("      Opcion [4]: Buscar cliente por ID o RUT \n");
        printf("      Opcion [5]: Listar todos los clientes \n\n");
        printf("      Opcion [0]: Salir al menu principal \n\n");
        printf("   Por favor, ingrese una opcion: ");
        opcionMenu = inputEntero();
        printf("\n");

        switch (opcionMenu) {
            case 1:
                registrarCliente(raizClientes);
                break;
            case 2:
                printf(" ============ MENU MODIFICAR CLIENTE ============\n");
                printf("Ingrese el ID o RUT del cliente: ");
                fgets(entradaBusqueda, sizeof(entradaBusqueda), stdin);
                entradaBusqueda[strcspn(entradaBusqueda, "\n")] = '\0';

                clienteActual = verificarPedirCliente(tienda, entradaBusqueda);
                if (clienteActual != NULL) {
                    printf("¡Cliente encontrado con exito!\n");
                    menuModificarCliente(clienteActual);
                } else {
                    printf("Error: No se pudo encontrar al cliente ingresado.\n\n");
                }
                break;
            case 3:
                eliminarClienteInterfaz(tienda);
                break;
            case 4:
                printf(" ============ MENU BUSCAR CLIENTE (ID o RUT) ============\n");
                printf("Ingrese el ID o RUT del cliente: ");
                fgets(entradaBusqueda, sizeof(entradaBusqueda), stdin);
                entradaBusqueda[strcspn(entradaBusqueda, "\n")] = '\0';

                clienteActual = verificarPedirCliente(tienda, entradaBusqueda);
                if (clienteActual != NULL) {
                    mostrarInformacionCliente(clienteActual);
                } else {
                    printf("Error: No se encontro ningun cliente con los datos ingresados.\n\n");
                }
                break;
            case 5:
                printf(" ============ MENU LISTAR CLIENTES ============\n\n");
                if (*raizClientes == NULL) {
                    printf("Error: No hay clientes registrados en el sistema.\n\n");
                } else {
                    listarClientes(*raizClientes);
                }
                break;
            case 0:
                printf("Saliendo del menu de clientes.\n");
                break;
            default:
                opcionValida = 0;
        }
        if (opcionValida == 0) {
            printf("\n   ¡ERROR! Opcion no valida. Intente nuevamente. \n");
        }
    } while (opcionMenu != 0);
}
/* =========================================================================
   MODULO DE SALAS Y TITULOS 
   ========================================================================= */
/* Esta funcion crea el arreglo de punteros a salas vacio */
   struct Sala **crearArregloSalas(int maxSalas) {
    struct Sala **arregloSalas;
    int i;
    if (maxSalas <= 0) {
        return NULL;
    }
    arregloSalas = (struct Sala **) malloc(sizeof(struct Sala *) * maxSalas);
    if (arregloSalas == NULL) {
        return NULL;
    }
    for (i = 0; i < maxSalas; i++) {
        arregloSalas[i] = NULL;
    }
    return arregloSalas;
}

/* Esta funcion busca una sala en la tienda segun su ID */

struct Sala *buscarSalaPorId(struct Tienda *tienda, int idSala) {
    int i;
    if (tienda == NULL) {
        return NULL;
    }
    for (i = 0; i < tienda->cantidadSalas; i++) {
        if (tienda->salas[i]->idSala == idSala) {
            return tienda->salas[i];
        }
    }
    return NULL;
}
/* Esta funcion busca una sala en la tienda segun su Tematica */
/*recibe el numero de la tematica y la busca en el arreglo salas*/
struct Sala *buscarSalaPorTematica(struct Tienda *tienda, int tematica) {
    int i;
    if (tienda == NULL) {
        return NULL;
    }
    for (i = 0; i < tienda->cantidadSalas; i++) {
        if (tienda->salas[i]->tematica == tematica) {
            return tienda->salas[i];
        }
    }
    return NULL;
}
/* Esta funcion busca una sala en la tienda segun su nombre */
struct Sala *buscarSalaPorNombre(struct Tienda *tienda, char *nombreSala) {
    int i;
    if (tienda == NULL || nombreSala == NULL) {
        return NULL;
    }
    for (i = 0; i < tienda->cantidadSalas; i++) {
        if (strcmp(tienda->salas[i]->nombreSala, nombreSala) == 0) {
            return tienda->salas[i];
        }
    }
    return NULL;
}
/* Esta funcion agrega una sala al arreglo de la tienda */
struct Sala *agregarSala(struct Tienda *tienda, struct Sala *sala, int tamSalas) {
    if (tienda == NULL || sala == NULL || tienda->salas == NULL) {
        return NULL;
    }
    if (tienda->cantidadSalas >= tamSalas) {
        return NULL;
    }
    if (buscarSalaPorId(tienda, sala->idSala) != NULL || buscarSalaPorTematica(tienda, sala->tematica) != NULL) {
        return NULL;
    }
    tienda->salas[tienda->cantidadSalas] = sala;
    tienda->cantidadSalas++;
    return sala;
}
/* Esta funcion crea un nodo de la lista doblemente enlazada de titulos con el titulo recibido */
struct NodoTitulo *crearNodoTitulo(struct Titulo *titulo) {
    struct NodoTitulo *nuevoNodo = (struct NodoTitulo *) malloc(sizeof(struct NodoTitulo));
    if (nuevoNodo == NULL) {
        return NULL;
    }
    nuevoNodo->datosTitulo = titulo;
    nuevoNodo->ant = NULL;
    nuevoNodo->sig = NULL;
    return nuevoNodo;
}
/* Esta funcion crea una sala  y su nodo fantasma de titulos */
struct Sala *crearSala(int idSala, char *nombreSala, int tematica, char *horario, int aforoMax, int cantTrabajadores) {
    struct Sala *nuevaSala;
    if (nombreSala == NULL || horario == NULL || tematica < 1 || tematica > 6) {
        return NULL;
    }
    nuevaSala = (struct Sala *) malloc(sizeof(struct Sala));
    if (nuevaSala == NULL) {
        return NULL;
    }
    nuevaSala->nombreSala = (char *) malloc(strlen(nombreSala) + 1);
    nuevaSala->horario = (char *) malloc(strlen(horario) + 1);
    nuevaSala->headNodoTitulo = crearNodoTitulo(NULL);
    
    if(nuevaSala->nombreSala == NULL || nuevaSala->horario == NULL || nuevaSala->headNodoTitulo == NULL){
        free(nuevaSala->nombreSala);
        free(nuevaSala->horario);
        free(nuevaSala->headNodoTitulo);
        free(nuevaSala);
        return NULL;
    }
    strcpy(nuevaSala->nombreSala, nombreSala);
    strcpy(nuevaSala->horario, horario);

    nuevaSala->idSala = idSala;
    nuevaSala->tematica = tematica;
    nuevaSala->aforoMax = aforoMax;
    nuevaSala->aforoActual = 0;
    nuevaSala->cantTrabajadores = cantTrabajadores;
    return nuevaSala;
}
/*cambia el estado de una lista de espera*/
void actualizarEstadoListaEspera(struct Titulo *titulo) {
    if (titulo == NULL) {
        return;
    }
    if (titulo->estado == 2 || titulo->estado == 3) { /*Si titulo esta agotado o en preventa, el estado de la lista de espera será Activo*/
        titulo->estadoListaEspera = 1;
    } else {
        titulo->estadoListaEspera = 2; /*Si titulo está dispo, en preventa o descatalogado, el estado de lista de espera será Suspendido*/
    }
}

/* Esta funcion crea un titulo  */
struct Titulo *crearTitulo(int idTitulo, char *nombreTitulo, int tipoTitulo, int clasificacionEdad, int precio, int cantidadEjemplares, int estado, int ejemplaresPorDespacho, int diasDemoraDespacho) {
    struct Titulo *nuevoTitulo;
    if (nombreTitulo == NULL || tipoTitulo < 1 || tipoTitulo > 6 || estado < 1 || estado > 4 || ejemplaresPorDespacho < 0 || diasDemoraDespacho < 0) {
        return NULL;
    }
    nuevoTitulo = (struct Titulo *) malloc(sizeof(struct Titulo));
    if (nuevoTitulo == NULL) return NULL;
    nuevoTitulo->nombreTitulo = (char *) malloc(strlen(nombreTitulo) + 1);
    if (nuevoTitulo->nombreTitulo == NULL){
        free(nuevoTitulo);
        return NULL;
    }
    strcpy(nuevoTitulo->nombreTitulo, nombreTitulo);
    nuevoTitulo->headListaEspera = NULL;
    nuevoTitulo->idTitulo = idTitulo;
    nuevoTitulo->tipoTitulo = tipoTitulo;
    nuevoTitulo->clasificacionEdad = clasificacionEdad;
    nuevoTitulo->precio = precio;
    nuevoTitulo->cantidadEjemplares = cantidadEjemplares;
    nuevoTitulo->estado = estado;
    nuevoTitulo->ejemplaresPorDespacho = ejemplaresPorDespacho;
    nuevoTitulo->diasDemoraDespacho = diasDemoraDespacho;
    actualizarEstadoListaEspera(nuevoTitulo);  /*ahora no se pide ingresar un valor, se hace el calculo automatico*/
    return nuevoTitulo;
}
/* Esta funcion enlaza un nodo al final de la lista de titulos */
struct NodoTitulo *enlazarNodoTitulo(struct NodoTitulo *headNodoTitulo, struct NodoTitulo *nuevoNodo) {
    struct NodoTitulo *rec;
    if (headNodoTitulo == NULL || nuevoNodo == NULL) {
        return NULL;
    }
    rec = headNodoTitulo;
    while (rec->sig != NULL) {
        rec = rec->sig;
    }
    rec->sig = nuevoNodo;
    nuevoNodo->ant = rec;
    return nuevoNodo;
}
/* Esta funcion busca un titulo por nombre dentro de la lista de titulos de una sala */
struct Titulo *buscarTituloPorNombre(struct NodoTitulo *headNodoTitulo, char *nombreBuscar) {
    struct NodoTitulo *rec;
    if (headNodoTitulo == NULL || nombreBuscar == NULL) {
        return NULL;
    }
    rec = headNodoTitulo->sig;
    while (rec != NULL) {
        if (rec->datosTitulo != NULL && strcmp(rec->datosTitulo->nombreTitulo, nombreBuscar) == 0) {
            return rec->datosTitulo;
        }
        rec = rec->sig;
    }
    return NULL;
}
/* Esta funcion busca un titulo por nombre por todas las salas de la tienda */
struct Titulo *buscarTituloEnSalas(struct Tienda *ibcomics, char *nombreBuscar) {
    int i;
    struct Titulo *tituloEncontrado;
    if (ibcomics == NULL || nombreBuscar == NULL) {
        return NULL;
    }
    for (i = 0; i < ibcomics->cantidadSalas; i++) {
        if (ibcomics->salas[i] != NULL) {
            tituloEncontrado = buscarTituloPorNombre(ibcomics->salas[i]->headNodoTitulo, nombreBuscar);
            if (tituloEncontrado != NULL) {
                return tituloEncontrado;
            }
        }
    }
    return NULL;
}
/* Esta funcion busca un titulo por ID dentro de la lista de titulos de una sala */
struct Titulo *buscarTituloPorId(struct NodoTitulo *headNodoTitulo, int idTitulo) {
    struct NodoTitulo *rec;
    if (headNodoTitulo == NULL) {
        return NULL;
    }
    rec = headNodoTitulo->sig;
    while (rec != NULL) {
        if (rec->datosTitulo != NULL && rec->datosTitulo->idTitulo == idTitulo) {
            return rec->datosTitulo;
        }
        rec = rec->sig;
    }
    return NULL;
}
/* Esta funcion busca un titulo por ID recorriendo todas las salas */
struct Titulo *buscarTituloEnSalasPorId(struct Tienda *ibcomics, int idTitulo) {
    int i;
    struct Titulo *tituloEncontrado;
    if (ibcomics == NULL) {
        return NULL;
    }
    for (i = 0; i < ibcomics->cantidadSalas; i++) {
        if (ibcomics->salas[i] != NULL) {
            tituloEncontrado = buscarTituloPorId(ibcomics->salas[i]->headNodoTitulo, idTitulo);
            if (tituloEncontrado != NULL) {
                return tituloEncontrado;
            }
        }
    }
    return NULL;
}
/* Esta funcion agrega un titulo a una sala si su ID y verifica que su nombre no estan registrados en ninguna sala */
struct Titulo *agregarTitulo(struct Tienda *tienda, struct Sala *sala, struct Titulo *titulo) {
    struct NodoTitulo *nuevoNodo;
    if (tienda == NULL || sala == NULL || titulo == NULL) {
        return NULL;
    }
    if (buscarTituloEnSalasPorId(tienda, titulo->idTitulo) != NULL || buscarTituloEnSalas(tienda, titulo->nombreTitulo) != NULL) {
        return NULL;
    }
    nuevoNodo = crearNodoTitulo(titulo);
    if (nuevoNodo == NULL) {
        return NULL;
    }
    enlazarNodoTitulo(sala->headNodoTitulo, nuevoNodo);
    return titulo;
}
/* Esta funcion muestra los datos de un titulo en pantalla */
void mostrarDatosTitulo(struct Titulo *titulo) {
    if (titulo == NULL) {
        return;
    }
    printf("\n Titulo encontrado con exito!\n");
    printf(" ============ DATOS DEL TITULO ============\n");
    printf("\n");
    printf("ID: %d \n", titulo->idTitulo);
    printf("Nombre: %s \n", titulo->nombreTitulo);
    if (titulo->tipoTitulo == 1) {
        printf("Tipo: Serie Regular \n");
    } else if (titulo->tipoTitulo == 2) {
        printf("Tipo: Mini Serie \n");
    } else if (titulo->tipoTitulo == 3) {
        printf("Tipo: Tomo Recopilatorio \n");
    } else if (titulo->tipoTitulo == 4) {
        printf("Tipo: Novela Grafica \n");
    } else if (titulo->tipoTitulo == 5) {
        printf("Tipo: Comic \n");
    } else {
        printf("Tipo: Fanzine \n");
    }
    printf("Clasificacion de edad: %d\n", titulo->clasificacionEdad);
    printf("Precio: $%d \n", titulo->precio);
    printf("Ejemplares: %d \n", titulo->cantidadEjemplares);
    if (titulo->estado == 1) {
        printf("Estado: Disponible \n");
    } else if (titulo->estado == 2) {
        printf("Estado: Agotado \n");
    } else if (titulo->estado == 3) {
        printf("Estado: Preventa \n");
    } else {
        printf("Estado: Descatalogado \n");
    }
    if (titulo->estadoListaEspera == 1) {
        printf("Lista de espera: Activa \n");
    } else {
        printf("Lista de espera: Suspendida \n");
    }
    printf("Despacho: %d ejemplares cada %d dias \n\n", titulo->ejemplaresPorDespacho, titulo->diasDemoraDespacho);
}
/* Esta funcion muestra todos los titulos de una sala */
void listarTitulos(struct Sala *sala) {
    struct NodoTitulo *rec;
    if (sala == NULL || sala->headNodoTitulo == NULL) {
        return;
    }
    rec = sala->headNodoTitulo->sig;
    if (rec == NULL) {
        printf("Error: La sala no tiene titulos.\n");
        return;
    }
    while (rec != NULL) {
        if (rec->datosTitulo != NULL) {
            printf("   [%d] %s | $%d | Ejemplares: %d \n", rec->datosTitulo->idTitulo,
                   rec->datosTitulo->nombreTitulo, rec->datosTitulo->precio, rec->datosTitulo->cantidadEjemplares);
        }
        rec = rec->sig;
    }
}
/* Esta funcion muestra por pantalla los datos de una sala junto con sus titulos */
void mostrarDatosSala(struct Sala *sala) {
    if (sala == NULL) {
        return;
    }
    printf("\n Sala encontrada con exito!\n");
    printf(" ============ DATOS DE LA SALA ============\n");
    printf("\n");
    printf("ID: %d \n", sala->idSala);
    printf("Nombre: %s \n", sala->nombreSala);
    if (sala->tematica == 1) {
        printf("Tematica: Superheroe \n");
    } else if (sala->tematica == 2) {
        printf("Tematica: Manga y Anime \n");
    } else if (sala->tematica == 3) {
        printf("Tematica: Novela Grafica \n");
    } else if (sala->tematica == 4) {
        printf("Tematica: Comic Independiente \n");
    } else if (sala->tematica == 5) {
        printf("Tematica: Infantil y Humor Grafico \n");
    } else {
        printf("Tematica: Coleccionista \n");
    }
    printf("Horario: %s \n", sala->horario);
    printf("Aforo maximo: %d \n", sala->aforoMax);
    printf("Trabajadores: %d \n", sala->cantTrabajadores);
    printf("Titulos de la sala:\n");
    listarTitulos(sala);
    printf("\n");
}
/* Esta funcion muestra un resumen de todas las salas de la tienda */
void listarSalas(struct Tienda *tienda) {
    int i;
    if (tienda == NULL || tienda->cantidadSalas == 0) {
        printf("Error: No hay salas registradas.\n");
        return;
    }
    for (i = 0; i < tienda->cantidadSalas; i++) {
        printf("   [%d] %s | Horario: %s | Aforo: %d \n", tienda->salas[i]->idSala,
               tienda->salas[i]->nombreSala, tienda->salas[i]->horario, tienda->salas[i]->aforoMax);
    }
}
/*pide el id del titulo hasta que exista*/
struct Titulo *verificarPedirTitulo(struct Tienda *ibcomics, int idTitulo) {
    struct Titulo *tituloEncontrado = NULL;

    if (ibcomics == NULL || idTitulo <= 0) {
        return NULL;
    }
    tituloEncontrado = buscarTituloEnSalasPorId(ibcomics, idTitulo);  /*busca si el titulo está en alguna sala*/
    return tituloEncontrado;
}

/* Esta funcion muestra el menu para modificar los datos de un titulo */
void menuModificarTitulo(struct Tienda *tienda, struct Titulo *tituloModificar) {
    int opcionModificar;
    char nuevoNombre[100];
    struct Titulo *tituloExistente;

    if (tienda == NULL || tituloModificar == NULL) {
        printf("\nError: No existe el titulo que se quiere modificar.\n");
        return;
    }
    do {
        printf("\n");
        printf(" ============ MENU MODIFICAR TITULO ============\n");
        printf("\n");
        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        printf("\n");
        printf("      Opcion [1]: Modificar nombre \n");
        printf("      Opcion [2]: Modificar tipo \n");
        printf("      Opcion [3]: Modificar clasificacion de edad \n");
        printf("      Opcion [4]: Modificar precio \n");
        printf("      Opcion [5]: Modificar cantidad de ejemplares \n");
        printf("      Opcion [6]: Modificar estado \n");
        printf("      Opcion [7]: Modificar ejemplares por despacho \n");
        printf("      Opcion [8]: Modificar dias de demora del despacho \n");
        printf("\n");
        printf("      Opcion [0]: Salir de Menu \n");
        printf("\n");
        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        opcionModificar = inputEntero();
        printf("\n");
        switch (opcionModificar) {
            case 1:
                printf("Ha seleccionado la opcion 1: Modificar nombre \n");
                printf("Nombre registrado: %s \n", tituloModificar->nombreTitulo);
                printf("Ingrese el nuevo nombre = ");
                fgets(nuevoNombre, 100, stdin);
                nuevoNombre[strcspn(nuevoNombre, "\n")] = '\0';
                tituloExistente = buscarTituloEnSalas(tienda, nuevoNombre);
                if (strlen(nuevoNombre) == 0) {
                    printf("Error: El nombre no puede estar vacio.\n\n");
                } else if (tituloExistente != NULL && tituloExistente != tituloModificar) {
                    printf("Error: Ya existe un titulo con ese nombre.\n\n");
                } else {
                    tituloModificar->nombreTitulo = (char *) realloc(tituloModificar->nombreTitulo, strlen(nuevoNombre) + 1);
                    strcpy(tituloModificar->nombreTitulo, nuevoNombre);
                    printf("Nombre modificado con exito! Nuevo nombre: %s \n\n", tituloModificar->nombreTitulo);
                }
                break;
            case 2:
                printf("Ha seleccionado la opcion 2: Modificar tipo \n");
                printf("Tipo registrado: %d \n", tituloModificar->tipoTitulo);
                printf(" === [1] Serie Regular / [2] Mini Serie / [3] Tomo Recopilatorio ===\n");
                printf(" === [4] Novela Grafica / [5] Comic / [6] Fanzine ===\n");
                printf("Ingrese el nuevo tipo (1-6) = ");
                tituloModificar->tipoTitulo = inputEntero();
                while (tituloModificar->tipoTitulo < 1 || tituloModificar->tipoTitulo > 6) {
                    printf("Error: Ingrese un tipo valido (1-6) = ");
                    tituloModificar->tipoTitulo = inputEntero();
                }
                printf("Tipo modificado con exito! Nuevo tipo: %d \n\n", tituloModificar->tipoTitulo);
                break;
            case 3:
                printf("Ha seleccionado la opcion 3: Modificar clasificacion de edad \n");
                printf("Clasificacion registrada: %d \n", tituloModificar->clasificacionEdad);
                printf("Ingrese la nueva clasificacion = ");
                tituloModificar->clasificacionEdad = inputEntero();
                while (tituloModificar->clasificacionEdad < 0){
                    printf("Error: La clasificacion no puede ser negativa. Ingrese la nueva clasificacion = ");
                    tituloModificar->clasificacionEdad = inputEntero();
                }
                printf("Clasificacion modificada con exito! Nueva clasificacion: %d \n\n", tituloModificar->clasificacionEdad);
                break;
            case 4:
                printf("Ha seleccionado la opcion 4: Modificar precio \n");
                printf("Precio registrado: %d \n", tituloModificar->precio);
                printf("Ingrese el nuevo precio = ");
                tituloModificar->precio = inputEntero();
                while(tituloModificar->precio < 0){
                    printf("Error: El precio no puede ser negativo. Ingrese el nuevo precio = ");
                    tituloModificar->precio = inputEntero();
                }
                printf("Precio modificado con exito! Nuevo precio: %d \n\n", tituloModificar->precio);
                break;;
            case 5:
                printf("Ha seleccionado la opcion 5: Modificar cantidad de ejemplares \n");
                printf("Ejemplares registrados: %d \n", tituloModificar->cantidadEjemplares);
                printf("Ingrese la nueva cantidad = ");
                tituloModificar->cantidadEjemplares = inputEntero();
                while(tituloModificar->cantidadEjemplares < 0){
                    printf("Error: La cantidad de ejemplares no puede ser negativa. Ingrese la nueva cantidad = ");
                    tituloModificar->cantidadEjemplares = inputEntero();
                }
                printf("Cantidad modificada con exito! Nueva cantidad de ejemplares: %d \n\n", tituloModificar->cantidadEjemplares);
                break;
            case 6:
                printf("Ha seleccionado la opcion 6: Modificar estado \n");
                printf("Estado registrado: %d \n", tituloModificar->estado);
                printf(" === [1] Disponible / [2] Agotado / [3] Preventa / [4] Descatalogado ===\n");
                printf("Ingrese el nuevo estado (1-4) = ");
                tituloModificar->estado = inputEntero();
                while (tituloModificar->estado < 1 || tituloModificar->estado > 4) {
                    printf("Error: Ingrese un estado valido (1-4) = ");
                    tituloModificar->estado = inputEntero();
                }
                actualizarEstadoListaEspera(tituloModificar);
                printf("Estado modificado con exito! Nuevo estado: %d \n\n", tituloModificar->estado);
                break;
            case 7:
                printf("Ha seleccionado la opcion 7: Modificar ejemplares por despacho \n");
                printf("Ejemplares por despacho: %d \n", tituloModificar->ejemplaresPorDespacho);
                printf("Ingrese el nuevo valor = ");
                tituloModificar->ejemplaresPorDespacho = inputEntero();
                printf("Valor modificado con exito! Nuevo valor de ejemplares por despacho: %d \n\n", tituloModificar->ejemplaresPorDespacho);
                break;
            case 8:
                printf("Ha seleccionado la opcion 8: Modificar dias de demora del despacho \n");
                printf("Dias demora despacho: %d \n", tituloModificar->diasDemoraDespacho);
                printf("Ingrese el nuevo valor = ");
                tituloModificar->diasDemoraDespacho = inputEntero();
                printf("Valor modificado con exito! Nuevo valor de dias de demora: %d \n\n", tituloModificar->diasDemoraDespacho);
                break;

            case 0:
                printf("Saliendo del menu...\n");
                break;
            default:
                printf("\n  ERROR! Opcion no valida. Intente nuevamente :) \n");
                break;
        }
    } while (opcionModificar != 0);
}

/* Esta funcion muestra el menu para modificar los datos de una sala */
void menuModificarSala(struct Sala *salaModificar) {
    int opcionModificar;
    char nuevoTexto[100];

    if (salaModificar == NULL) {
        printf("\nError: No existe la sala que se quiere modificar.\n");
        return;
    }

    do {
        printf("\n");
        printf(" ============ MENU MODIFICAR SALA ============\n");
        printf("\n");
        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        printf("\n");
        printf("      Opcion [1]: Modificar nombre \n");
        printf("      Opcion [2]: Modificar horario \n");
        printf("      Opcion [3]: Modificar aforo maximo \n");
        printf("      Opcion [4]: Modificar cantidad de trabajadores \n");
        printf("\n");
        printf("      Opcion [0]: Salir de Menu \n");
        printf("\n");
        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        opcionModificar = inputEntero();
        printf("\n");
        switch (opcionModificar) {
            case 1:
                printf("Ha seleccionado la opcion 1: Modificar nombre \n");
                printf("Nombre registrado: %s \n", salaModificar->nombreSala);
                printf("Ingrese el nuevo nombre = ");
                fgets(nuevoTexto, 100, stdin);
                nuevoTexto[strcspn(nuevoTexto, "\n")] = '\0';
                if (strlen(nuevoTexto) > 0) {
                    salaModificar->nombreSala = (char *) realloc(salaModificar->nombreSala, strlen(nuevoTexto) + 1);
                    strcpy(salaModificar->nombreSala, nuevoTexto);
                    printf("Nombre modificado con exito! Nuevo nombre: %s \n\n", salaModificar->nombreSala);
                }
                break;
            case 2:
                printf("Ha seleccionado la opcion 2: Modificar horario \n");
                printf("Horario registrado: %s \n", salaModificar->horario);
                printf("Ingrese el nuevo horario = ");
                fgets(nuevoTexto, 100, stdin);
                nuevoTexto[strcspn(nuevoTexto, "\n")] = '\0';
                if (strlen(nuevoTexto) > 0) {
                    salaModificar->horario = (char *) realloc(salaModificar->horario, strlen(nuevoTexto) + 1);
                    strcpy(salaModificar->horario, nuevoTexto);
                    printf("Horario modificado con exito! Nuevo horario: %s \n\n", salaModificar->horario);
                }
                break;
            case 3:
                printf("Ha seleccionado la opcion 3: Modificar aforo maximo \n");
                printf("Aforo maximo registrado: %d \n", salaModificar->aforoMax);
                printf("Ingrese el nuevo aforo maximo = ");
                salaModificar->aforoMax = inputEntero();
                while(salaModificar->aforoMax <= 0){
                    printf("Error: Aforo debe ser mayor a 0. Ingrese el nuevo aforo maximo = ");
                    salaModificar->aforoMax = inputEntero();
                }
                printf("Aforo modificado con exito! Nuevo aforo maximo: %d \n\n", salaModificar->aforoMax);
                break;
            case 4:
                printf("Ha seleccionado la opcion 4: Modificar cantidad de trabajadores \n");
                printf("Trabajadores registrados: %d \n", salaModificar->cantTrabajadores);
                printf("Ingrese la nueva cantidad de trabajadores = ");
                salaModificar->cantTrabajadores = inputEntero();
                while(salaModificar->cantTrabajadores <= 0){
                    printf("Error: Trabajadores deben ser mayor a 0. Ingrese la nueva cantidad de trabajadores = ");
                    salaModificar->cantTrabajadores = inputEntero();
                }
                printf("Cantidad modificada con exito! Nueva cantidad de trabajadores: %d \n\n", salaModificar->cantTrabajadores);
                break;
            case 0:
                printf("Saliendo del menu...\n");
                break;
            default:
                printf("\n  ERROR! Opcion no valida. Intente nuevamente :) \n");
                break;
        }
    } while (opcionModificar != 0);
}

/* Esta funcion elimina una sala del arreglo y libera la memoria de la sala */
int eliminarSala(struct Tienda *tienda, struct Sala *salaEliminar) {
    int i;
    struct NodoTitulo *rec, *siguiente;

    if (tienda == NULL || tienda->salas == NULL || salaEliminar == NULL) {
        return 0;
    }

    i = 0;
    while (i < tienda->cantidadSalas && tienda->salas[i] != salaEliminar) {
        i++;
    }
    if (i == tienda->cantidadSalas) {
        return 0;
    }

    if (tienda->raiz != NULL) {
        desvincularPasesDeSala(tienda->raiz, salaEliminar);
        tienda->totalClientes -= salaEliminar->aforoActual;
    }

    while (i < tienda->cantidadSalas - 1) {
        tienda->salas[i] = tienda->salas[i + 1];
        i++;
    }
    tienda->salas[tienda->cantidadSalas - 1] = NULL;
    tienda->cantidadSalas--;

    rec = salaEliminar->headNodoTitulo;
    while (rec != NULL) {
        siguiente = rec->sig;
        if (rec->datosTitulo != NULL) {
            vaciarListaEspera(&(rec->datosTitulo->headListaEspera));
            free(rec->datosTitulo->nombreTitulo);
            free(rec->datosTitulo);
        }
        free(rec);
        rec = siguiente;
    }

    free(salaEliminar->nombreSala);
    free(salaEliminar->horario);
    free(salaEliminar);
    return 1;
}
/* Esta funcion cuenta la cantidad de personas en una sala */
int contarPersonasEnSala( struct Sala *sala) {
    return sala->aforoActual;

}
/* Esta funcion cuenta la cantidad de titulos en una sala */
int contarTitulosEnSala(struct Sala *sala) {
    int cantidad = 0;
    struct NodoTitulo *rec;
    if (sala == NULL || sala->headNodoTitulo == NULL) {
        return 0;
    }
    rec = sala->headNodoTitulo->sig;
    while (rec != NULL) {
        cantidad++;
        rec = rec->sig;
    }
    return cantidad;
}

/* Esta funcion ordena las salas por cantidad de personas de mayor a menor */
struct Tienda *ordenarSalasPorPersonas(struct Tienda *tienda) {
    int i, j;
    struct Sala *aux;
    if (tienda == NULL || tienda->salas == NULL) {
        return NULL;
    }
    for (i = 0; i < tienda->cantidadSalas - 1; i++) {
        for (j = 0; j < tienda->cantidadSalas - 1 - i; j++) {
            if (contarPersonasEnSala(tienda->salas[j]) < contarPersonasEnSala(tienda->salas[j + 1])) {
                aux = tienda->salas[j];
                tienda->salas[j] = tienda->salas[j + 1];
                tienda->salas[j + 1] = aux;
            }
        }
    }
    return tienda;
}
/* Esta funcion ordena las salas por cantidad de titulos de mayor a menor */
struct Tienda *ordenarSalasPorTitulos(struct Tienda *tienda) {
    int i, j;
    struct Sala *aux;
    if (tienda == NULL || tienda->salas == NULL) {
        return NULL;
    }
    for (i = 0; i < tienda->cantidadSalas - 1; i++) {
        for (j = 0; j < tienda->cantidadSalas - 1 - i; j++) {
            if (contarTitulosEnSala(tienda->salas[j]) < contarTitulosEnSala(tienda->salas[j + 1])) {
                aux = tienda->salas[j];
                tienda->salas[j] = tienda->salas[j + 1];
                tienda->salas[j + 1] = aux;
            }
        }
    }
    return tienda;
}

struct Sala *crearYAgregarSala(struct Tienda *tienda, int tamSalas) {
    int idSala, tematica, aforoMax, cantTrabajadores;
    char nombreSala[100], horario[100];
    struct Sala *nuevaSala;

    if (tienda == NULL || tienda->salas == NULL) {
        return NULL;
    }
    if (tienda->cantidadSalas >= tamSalas) {
        printf("Error: No se pueden registrar mas salas (maximo %d).\n\n", tamSalas);
        return NULL;
    }

    printf(" ============ MENU REGISTRAR SALA ============\n");
    printf("\n");

    printf("Ingrese el id de la sala = ");
    idSala = inputEntero();
    while (idSala <= 0 || buscarSalaPorId(tienda, idSala) != NULL) {
        printf("Error: El ID debe ser positivo y no estar registrado. Ingrese otro ID = ");
        idSala = inputEntero();
    }

    /* supuesto: el nombre no se repite para que la busqueda por nombre sea unica */
    printf("Ingrese el nombre de la sala = ");
    fgets(nombreSala, 100, stdin);
    nombreSala[strcspn(nombreSala, "\n")] = '\0';
    while (strlen(nombreSala) == 0 || buscarSalaPorNombre(tienda, nombreSala) != NULL) {
        printf("Error: El nombre no puede estar vacio ni repetido. Ingrese otro nombre = ");
        fgets(nombreSala, 100, stdin);
        nombreSala[strcspn(nombreSala, "\n")] = '\0';
    }

    printf(" Opciones disponibles : \n");
    printf(" === [1] SuperHeroe / [2] Manga y Anime / [3] Novela Grafica  ===\n");
    printf(" === [4] Comic Independiente / [5] Infantil y Humor Grafico / [6] Coleccionista ===\n");
    printf("Ingrese la tematica (1-6) = ");
    tematica = inputEntero();
    while (tematica < 1 || tematica > 6 || buscarSalaPorTematica(tienda, tematica) != NULL) {
        printf("Error: Ingrese una tematica valida (1-6) que no tenga sala = ");
        tematica = inputEntero();
    }

    printf("Ingrese el horario = ");
    fgets(horario, 100, stdin);
    horario[strcspn(horario, "\n")] = '\0';
    while (strlen(horario) == 0) {
        printf("Error: El horario no puede estar vacio. Ingrese el horario = ");
        fgets(horario, 100, stdin);
        horario[strcspn(horario, "\n")] = '\0';
    }

    printf("Ingrese el aforo maximo = ");
    aforoMax = inputEntero();
    while (aforoMax <= 0) {
        printf("Error: El aforo debe ser mayor a 0. Ingrese el aforo maximo = ");
        aforoMax = inputEntero();
    }

    printf("Ingrese la cantidad de trabajadores = ");
    cantTrabajadores = inputEntero();
    while (cantTrabajadores < 0) {
        printf("Error: La cantidad no puede ser negativa. Ingrese la cantidad de trabajadores = ");
        cantTrabajadores = inputEntero();
    }

    nuevaSala = crearSala(idSala, nombreSala, tematica, horario, aforoMax, cantTrabajadores);
    if (nuevaSala == NULL) {
        return NULL;
    }
    return agregarSala(tienda, nuevaSala, tamSalas);
}

void mostrarMenuSalas(struct Tienda *tienda, int tamSalas) {
    int opcionMenu, opcionValida, idOperar;
    char nombreOperar[100];
    struct Sala *salaOperar = NULL;

    do {
        opcionValida = 1;
        printf("\n");
        printf("  =========================================================\n");
        printf("  ||                                                     ||\n");
        printf("  ||         B I E N V E N I D O   A L   M E N U         ||\n");
        printf("  ||                                                     ||\n");
        printf("  ||                    S A L A S                        ||\n");
        printf("  ||                                                     ||\n");
        printf("  =========================================================\n");
        printf("\n");

        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        printf("\n");
        printf("      Opcion [1]: Registrar nueva Sala en el sistema \n");
        printf("      Opcion [2]: Modificar datos de una Sala \n");
        printf("      Opcion [3]: Eliminar Sala del sistema \n");
        printf("      Opcion [4]: Buscar Sala por ID \n");
        printf("      Opcion [5]: Buscar Sala por nombre \n");
        printf("      Opcion [6]: Listar todas las Salas \n");
        printf("      Opcion [7]: Ordenar Salas por cantidad de personas \n");
        printf("      Opcion [8]: Ordenar Salas por cantidad de titulos \n");
        printf("\n");
        printf("      Opcion [0]: Salir de Menu \n");
        printf("\n");

        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        opcionMenu = inputEntero();
        printf("\n");
        switch (opcionMenu) {
            case 1:
                printf("Ha seleccionado la opcion 1: Registrar nueva sala en el sistema \n");
                salaOperar = crearYAgregarSala(tienda, tamSalas);
                if (salaOperar != NULL) {
                    printf("Sala registrada con exito! \n\n");
                }
                break;
            case 2:
                printf("Ha seleccionado la opcion 2: Modificar datos de una sala \n");
                printf("Ingrese el id de la sala = ");
                idOperar = inputEntero();
                salaOperar = buscarSalaPorId(tienda, idOperar);
                if (salaOperar != NULL) {
                    menuModificarSala(salaOperar);
                } else {
                    printf("Error: No se pudo encontrar la sala con el ID %d.\n\n", idOperar);
                }
                break;
            case 3:
                printf("Ha seleccionado la opcion 3: Eliminar sala del sistema \n");
                printf("Ingrese el id de la sala = ");
                idOperar = inputEntero();
                salaOperar = buscarSalaPorId(tienda, idOperar);
                if (salaOperar != NULL && eliminarSala(tienda, salaOperar) == 1) {
                    printf("Sala eliminada con exito! \n\n");
                } else {
                    printf("Error: No se pudo encontrar la sala con el ID %d.\n\n", idOperar);
                }
                break;
            case 4:
                printf("Ha seleccionado la opcion 4: Buscar sala por ID \n");
                printf("Ingrese el id de la sala = ");
                idOperar = inputEntero();
                salaOperar = buscarSalaPorId(tienda, idOperar);
                if (salaOperar != NULL) {
                    mostrarDatosSala(salaOperar);
                } else {
                    printf("Error: No se pudo encontrar la sala con el ID %d.\n\n", idOperar);
                }
                break;
            case 5:
                printf("Ha seleccionado la opcion 5: Buscar sala por nombre \n");
                printf("Ingrese el nombre de la sala = ");
                fgets(nombreOperar, 100, stdin);
                nombreOperar[strcspn(nombreOperar, "\n")] = '\0';
                salaOperar = buscarSalaPorNombre(tienda, nombreOperar);
                if (salaOperar != NULL) {
                    mostrarDatosSala(salaOperar);
                } else {
                    printf("Error: No se encontro ninguna sala con el nombre ingresado.\n\n");
                }
                break;
            case 6:
                printf("Ha seleccionado la opcion 6: Listar todas las salas \n");
                printf(" ============ MENU LISTAR SALAS ============\n");
                printf("\n");
                listarSalas(tienda);
                break;
            case 7:
                printf("Ha seleccionado la opcion 7: Ordenar salas por cantidad de personas \n");
                ordenarSalasPorPersonas(tienda);
                printf("\n =====> Salas ordenadas por cantidad de personas (de mayor a menor) <=====\n");
                listarSalas(tienda);
                break;
            case 8:
                printf("Ha seleccionado la opcion 8: Ordenar salas por cantidad de titulos \n");
                ordenarSalasPorTitulos(tienda);
                printf("\n =====> Salas ordenadas por cantidad de titulos (de mayor a menor) <=====\n");
                listarSalas(tienda);
                break;
            case 0:
                printf("Saliendo del menu...\n");
                break;
            default:
                opcionValida = 0;
        }
        if (opcionValida == 0) {
            printf("\n  ERROR! Opcion no valida. Intente nuevamente :) \n");
        }
    } while (opcionMenu != 0);
}

int eliminarTitulo(struct Tienda *tienda, struct Titulo *tituloEliminar) {
    int i;
    struct NodoTitulo *rec;
    struct NodoListaEspera *recEspera, *siguienteEspera;

    if (tienda == NULL || tienda->salas == NULL || tituloEliminar == NULL) {
        return 0;
    }

    for (i = 0; i < tienda->cantidadSalas; i++) {
        if (tienda->salas[i] != NULL && tienda->salas[i]->headNodoTitulo != NULL) {
            rec = tienda->salas[i]->headNodoTitulo->sig;
            while (rec != NULL && rec->datosTitulo != tituloEliminar) {
                rec = rec->sig;
            }
            if (rec != NULL) {
                rec->ant->sig = rec->sig;
                if (rec->sig != NULL) {
                    rec->sig->ant = rec->ant;
                }
                /* supuesto: lista de espera circular con nodo fantasma, igual que en eliminarSala */
                vaciarListaEspera(&(tituloEliminar->headListaEspera));
                free(tituloEliminar->nombreTitulo);
                free(tituloEliminar);
                free(rec);
                return 1;
            }
        }
    }
    return 0;
}

struct Titulo *crearYAgregarTitulo(struct Tienda *tienda, struct Sala *sala) {
    int idTitulo, tipoTitulo, clasificacionEdad, precio, cantidadEjemplares;
    int estado, ejemplaresPorDespacho, diasDemoraDespacho;
    char nombreTitulo[100];
    struct Titulo *nuevoTitulo;

    if (tienda == NULL || sala == NULL) {
        return NULL;
    }

    printf(" ============ MENU REGISTRAR TITULO ============\n");
    printf("\n");

    printf("Ingrese el id del titulo = ");
    idTitulo = inputEntero();
    while (idTitulo <= 0 || buscarTituloEnSalasPorId(tienda, idTitulo) != NULL) {
        printf("Error: El ID debe ser positivo y no estar registrado. Ingrese otro ID = ");
        idTitulo = inputEntero();
    }

    printf("Ingrese el nombre del titulo = ");
    fgets(nombreTitulo, 100, stdin);
    nombreTitulo[strcspn(nombreTitulo, "\n")] = '\0';
    while (strlen(nombreTitulo) == 0 || buscarTituloEnSalas(tienda, nombreTitulo) != NULL) {
        printf("Error: El nombre no puede estar vacio ni repetido. Ingrese otro nombre = ");
        fgets(nombreTitulo, 100, stdin);
        nombreTitulo[strcspn(nombreTitulo, "\n")] = '\0';
    }

    printf(" Opciones disponibles : \n");
    printf(" === [1] Serie Regular / [2] Mini Serie / [3] Tomo Recopilatorio ===\n");
    printf(" === [4] Novela Grafica / [5] Comic / [6] Fanzine ===\n");
    printf("Ingrese el tipo (1-6) = ");
    tipoTitulo = inputEntero();
    while (tipoTitulo < 1 || tipoTitulo > 6) {
        printf("Error: Ingrese un tipo valido (1-6) = ");
        tipoTitulo = inputEntero();
    }

    printf("Ingrese la clasificacion de edad = ");
    clasificacionEdad = inputEntero();
    while (clasificacionEdad < 0) {
        printf("Error: La clasificacion no puede ser negativa. Ingrese la clasificacion de edad = ");
        clasificacionEdad = inputEntero();
    }

    printf("Ingrese el precio = ");
    precio = inputEntero();
    while (precio <= 0) {
        printf("Error: El precio debe ser mayor a 0. Ingrese el precio = ");
        precio = inputEntero();
    }

    printf("Ingrese la cantidad de ejemplares = ");
    cantidadEjemplares = inputEntero();
    while (cantidadEjemplares < 0) {
        printf("Error: La cantidad no puede ser negativa. Ingrese la cantidad de ejemplares = ");
        cantidadEjemplares = inputEntero();
    }

    printf(" Opciones disponibles : \n");
    printf(" === [1] Disponible / [2] Agotado / [3] Preventa / [4] Descatalogado ===\n");
    printf("Ingrese el estado (1-4) = ");
    estado = inputEntero();
    while (estado < 1 || estado > 4) {
        printf("Error: Ingrese un estado valido (1-4) = ");
        estado = inputEntero();
    }

    printf("Ingrese los ejemplares por despacho = ");
    ejemplaresPorDespacho = inputEntero();
    while (ejemplaresPorDespacho < 0) {
        printf("Error: El valor no puede ser negativo. Ingrese los ejemplares por despacho = ");
        ejemplaresPorDespacho = inputEntero();
    }

    printf("Ingrese los dias de demora del despacho = ");
    diasDemoraDespacho = inputEntero();
    while (diasDemoraDespacho < 0) {
        printf("Error: El valor no puede ser negativo. Ingrese los dias de demora del despacho = ");
        diasDemoraDespacho = inputEntero();
    }

    nuevoTitulo = crearTitulo(idTitulo, nombreTitulo, tipoTitulo, clasificacionEdad, precio, cantidadEjemplares, estado, ejemplaresPorDespacho, diasDemoraDespacho);
    if (nuevoTitulo == NULL) {
        return NULL;
    }
    return agregarTitulo(tienda, sala, nuevoTitulo);
}

void mostrarMenuTitulos(struct Tienda *tienda) {
    int opcionMenu, opcionValida, idOperar;
    char nombreOperar[100];
    struct Sala *salaOperar = NULL;
    struct Titulo *tituloOperar = NULL;

    do {
        opcionValida = 1;
        printf("\n");
        printf("  =========================================================\n");
        printf("  ||                                                     ||\n");
        printf("  ||         B I E N V E N I D O   A L   M E N U         ||\n");
        printf("  ||                                                     ||\n");
        printf("  ||                  T I T U L O S                      ||\n");
        printf("  ||                                                     ||\n");
        printf("  =========================================================\n");
        printf("\n");

        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        printf("\n");
        printf("      Opcion [1]: Registrar nuevo Titulo en una sala \n");
        printf("      Opcion [2]: Modificar datos de un Titulo \n");
        printf("      Opcion [3]: Eliminar Titulo del sistema \n");
        printf("      Opcion [4]: Buscar Titulo por ID \n");
        printf("      Opcion [5]: Buscar Titulo por nombre \n");
        printf("      Opcion [6]: Listar los Titulos de una sala \n");
        printf("\n");
        printf("      Opcion [0]: Salir de Menu \n");
        printf("\n");

        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        opcionMenu = inputEntero();
        printf("\n");
        switch (opcionMenu) {
            case 1:
                printf("Ha seleccionado la opcion 1: Registrar nuevo titulo en una sala \n");
                printf("Ingrese el id de la sala = ");
                idOperar = inputEntero();
                salaOperar = buscarSalaPorId(tienda, idOperar);
                if (salaOperar != NULL) {
                    tituloOperar = crearYAgregarTitulo(tienda, salaOperar);
                    if (tituloOperar != NULL) {
                        printf("Titulo registrado con exito! \n\n");
                    }
                } else {
                    printf("Error: No se pudo encontrar la sala con el ID %d.\n\n", idOperar);
                }
                break;
            case 2:
                printf("Ha seleccionado la opcion 2: Modificar datos de un titulo \n");
                printf("Ingrese el id del titulo = ");
                idOperar = inputEntero();
                tituloOperar = buscarTituloEnSalasPorId(tienda, idOperar);
                if (tituloOperar != NULL) {
                    menuModificarTitulo(tienda, tituloOperar);
                } else {
                    printf("Error: No se pudo encontrar el titulo con el ID %d.\n\n", idOperar);
                }
                break;
            case 3:
                printf("Ha seleccionado la opcion 3: Eliminar titulo del sistema \n");
                printf("Ingrese el id del titulo = ");
                idOperar = inputEntero();
                tituloOperar = buscarTituloEnSalasPorId(tienda, idOperar);
                if (tituloOperar != NULL && eliminarTitulo(tienda, tituloOperar) == 1) {
                    printf("Titulo eliminado con exito! \n\n");
                } else {
                    printf("Error: No se pudo encontrar el titulo con el ID %d.\n\n", idOperar);
                }
                break;
            case 4:
                printf("Ha seleccionado la opcion 4: Buscar titulo por ID \n");
                printf("Ingrese el id del titulo = ");
                idOperar = inputEntero();
                tituloOperar = buscarTituloEnSalasPorId(tienda, idOperar);
                if (tituloOperar != NULL) {
                    mostrarDatosTitulo(tituloOperar);
                } else {
                    printf("Error: No se pudo encontrar el titulo con el ID %d.\n\n", idOperar);
                }
                break;
            case 5:
                printf("Ha seleccionado la opcion 5: Buscar titulo por nombre \n");
                printf("Ingrese el nombre del titulo = ");
                fgets(nombreOperar, 100, stdin);
                nombreOperar[strcspn(nombreOperar, "\n")] = '\0';
                tituloOperar = buscarTituloEnSalas(tienda, nombreOperar);
                if (tituloOperar != NULL) {
                    mostrarDatosTitulo(tituloOperar);
                } else {
                    printf("Error: No se encontro ningun titulo con el nombre ingresado.\n\n");
                }
                break;
            case 6:
                printf("Ha seleccionado la opcion 6: Listar los titulos de una sala \n");
                printf("Ingrese el id de la sala = ");
                idOperar = inputEntero();
                salaOperar = buscarSalaPorId(tienda, idOperar);
                if (salaOperar != NULL) {
                    printf(" ============ MENU LISTAR TITULOS ============\n");
                    printf("\n");
                    listarTitulos(salaOperar);
                } else {
                    printf("Error: No se pudo encontrar la sala con el ID %d.\n\n", idOperar);
                }
                break;
            case 0:
                printf("Saliendo del menu...\n");
                break;
            default:
                opcionValida = 0;
        }
        if (opcionValida == 0) {
            printf("\n  ERROR! Opcion no valida. Intente nuevamente :) \n");
        }
    } while (opcionMenu != 0);
}

/* =========================================================================
   MODULO DE LISTA DE ESPERA (Angela)
   ========================================================================= */
/*--------------------------AGREGAR-----------------------------*/
/*solo crea el nodo de forma aislada en la ram*/

struct NodoListaEspera *crearNodoListasEspera (struct Cliente *clienteUnirse) {
    struct NodoListaEspera *nuevoNodo;

    if (clienteUnirse == NULL) {
        return NULL;
    }

    /*Crear memoria al nodo nuevo*/
    nuevoNodo = malloc(sizeof(struct NodoListaEspera));
    nuevoNodo-> datosCliente = clienteUnirse; /*guarda el puntero del cliente dentro de datoscliente*/

    nuevoNodo->sig = NULL;/*al espacio sig del struct lo apunto a null*/
    return nuevoNodo;
}

struct NodoListaEspera *enlazarNodoListaEspera(struct NodoListaEspera **head, struct NodoListaEspera *nuevoNodo) {
    struct NodoListaEspera *rec;
    if (head == NULL || nuevoNodo == NULL) {
        return NULL;
    }

    /*Si hay una lista vacia*/
    if (*head == NULL) {
        /*Si esta vacia el head se convierte en el primero y ultimo*/
        *head = nuevoNodo;
        nuevoNodo->sig = nuevoNodo;  /*como seria el primer nodo se apunta a si mismo*/

        return nuevoNodo;
    }

    /*recorrer hasta el ultimo nodo*/
    rec = *head;
    while (rec ->sig != *head) {
        rec = rec->sig;
    }

    rec->sig = nuevoNodo;
    nuevoNodo->sig = *head;
    return nuevoNodo;
}

/*-----------------------VERIFICAR----------------------------------------*/

struct Cliente *buscarClienteEnListaEsperaPorRut(struct NodoListaEspera *head, char *rut){
    struct NodoListaEspera *rec;

    if (head == NULL || rut == NULL) {
        return NULL;
    }

    rec = head;
    /*recorro la lista circular*/
    do {
        if (strcmp(rec->datosCliente->rutCliente, rut) == 0) {
            return rec->datosCliente;
        }
        rec = rec -> sig;
    }while (rec != head);

    return NULL;
}

struct Cliente *buscarClienteEnListaEsperaPorId(struct NodoListaEspera *head, int idBuscado) {
    struct NodoListaEspera *rec;

    if (head == NULL || idBuscado <= 0) {
        return NULL;
    }
    rec = head;
    do {
        if (rec->datosCliente->idCliente == idBuscado) {
            return rec->datosCliente;
        }
        rec = rec->sig;
    }while (rec != head);

    return NULL;
}

/*esta funcion llama a dos funciones, para que me busque al cliente en la lista de espera independientemente si se ingresa su rut o id*/
struct Cliente *buscarClientePorRutOIdListaEspera(struct NodoListaEspera *head, char *rutId) {
    struct Cliente *clienteEncontrado = NULL;
    int esRut  = 0 ; /*0: es id , 1: es rut */
    int i;

    if (head == NULL) {
        return NULL;
    }

    /*Si el texto tiene guion se asume que es rut*/
    /*se recorre todo el char ingresado*/
    for (i = 0; rutId[i] != '\0' ; i++) {
        if (rutId[i] == '-' || rutId[i] == '.') {
            esRut = 1;
            break;  /*Con leer uno ya se sabe que es rut*/
        }
    }
    /*se hacen dos funciones extra para despues poder llamarlas */
    if (esRut == 1) {
        clienteEncontrado = buscarClienteEnListaEsperaPorRut(head, rutId); /*busco por rut*/
    }
    else {
        clienteEncontrado = buscarClienteEnListaEsperaPorId(head, atoi(rutId)); /*busco por id convierto char a int*/
    }
    return clienteEncontrado;
}

/*Busca al cliente dentro de la lista de espera para que no esté repetido*/
struct Cliente *verificarPedirClienteListaEspera(struct Titulo *titulo, char *texto) {
    struct Cliente *clienteEncontrado;
    if (titulo == NULL) {
        return NULL;
    }
    clienteEncontrado = buscarClientePorRutOIdListaEspera(titulo->headListaEspera, texto);
    return clienteEncontrado;
}


/*------------------------------MOSTRAR--------------------------------------*/
int contarCantidadClientesEnListaEspera(struct NodoListaEspera *head) {
    int contador = 0;
    struct NodoListaEspera *rec;

    if (head != NULL) {
        rec = head;
        /*recorre toda la lista circular*/
        do {
            contador ++;
            rec = rec->sig;
        }while (rec != head);
    }
    return contador;
}


struct Titulo *obtenerTituloMayorListaEspera(struct NodoTitulo *head) {
    struct NodoTitulo *rec;
    struct Titulo *tituloMayor = NULL;
    int mayor = -1;
    int cantListaEspera = 0;

    if (head == NULL || head->sig == NULL) {
        return NULL;
    }
    rec = head->sig;

    /*Mi lista de titulos es una lista con nodo fantasma*/
    while (rec != NULL) {

        if (rec->datosTitulo != NULL) {
            cantListaEspera = contarCantidadClientesEnListaEspera(rec->datosTitulo->headListaEspera);
            if (cantListaEspera > 0 && (tituloMayor == NULL || cantListaEspera > mayor)) { /*tomara siempre al primer elemento como el mayor para poder comparar*/
                mayor = cantListaEspera;
                tituloMayor = rec->datosTitulo;
            }
        }
        rec = rec->sig;
    }
    return tituloMayor;
}

int calcularTiempoEstimadoEsperaDias(struct Titulo *tituloEncontrado) {
    int cantidadListaEspera = 0;
    int despachos = 0;
    int resultado = 0;

    /*int para ignorar decimales*/

    if (tituloEncontrado == NULL) {
        return -1;
    }

    cantidadListaEspera = contarCantidadClientesEnListaEspera(tituloEncontrado->headListaEspera);

    /*para no tener division por 0*/
    if (tituloEncontrado->ejemplaresPorDespacho > 0) {
        despachos = (cantidadListaEspera + tituloEncontrado->ejemplaresPorDespacho - 1) / tituloEncontrado->ejemplaresPorDespacho;

        resultado = despachos * tituloEncontrado->diasDemoraDespacho;
        return resultado;
    }

    /*se hizo un calculo para hacer una estimacion de la cantidad de dias de listas de espera*/
    return -1;
}

/*elimina los nodos*/
int vaciarListaEspera(struct NodoListaEspera **head) {
    struct NodoListaEspera *inicio;
    struct NodoListaEspera *rec;
    struct NodoListaEspera *siguiente;

    if (head == NULL || *head == NULL) {
        return 0;
    }
    inicio = *head;  /*para saber cuando hizo la vuelta completa*/
    rec = inicio->sig; /*empieza en el segundo nodo*/
    while (rec != inicio) {
        siguiente = rec->sig; /*guarda el siguiente antes de borrar el actual*/
        free(rec);
        rec = siguiente;
    }
    free(inicio);  /*se destruye el ultimo nodo que quedaba (head)*/
    *head = NULL;
    return 1;
}


/*-----------------------ELIMINAR----------------------------*/
/*retorna 1 si la eliminacion ocurrio con exito y 0 si hubo un fallo*/
int eliminarClienteDeListaEspera(struct NodoListaEspera **head, int idBuscado) {
    struct NodoListaEspera *rec;
    struct NodoListaEspera *nodoQuitado;

    if (head == NULL || *head == NULL || idBuscado <= 0) {
        return 0;
    }

    /*el cliente esta en el head */
    if ((*head)->datosCliente->idCliente == idBuscado) {  /*si el id buscado es igual al de head*/
        nodoQuitado = *head;
        if (nodoQuitado->sig == nodoQuitado) {
            *head = NULL;                       /* era el unico nodo (se apunta a si mismo) */
        } else {
            rec = nodoQuitado;
            do {   /* si no es el unico nodo buscar el ultimo */
                rec = rec->sig;
            }while (rec->sig != nodoQuitado);

            rec->sig = nodoQuitado->sig;   /*el head se quita y se reemplaza con el nodo siguiente*/
            *head = nodoQuitado->sig;       /*el ultimo nodo deja de apuntar hacia el head y apunta hacia al siguiente de head*/
        }
        free(nodoQuitado);
        return 1;
    }

    /* el cliente esta mas adelante */
    rec = *head;
    do {
        if (rec->sig->datosCliente->idCliente == idBuscado) {  /*me recorre hasta encontrar al nodo que tiene el id buscado*/
            nodoQuitado = rec->sig;  /*se guarda para no perderlo*/
            rec->sig = nodoQuitado->sig; /*el que esta antes del que se debe eliminar ahora apunta hacia el que esta despues del eliminado*/
            free(nodoQuitado);
            return 1;
        }
        rec = rec->sig;
    }while (rec->sig != *head);

    return 0;
}
/*-----------listar--------------------*/

void listarClientesListaEspera(struct NodoListaEspera *head) {
    struct NodoListaEspera *rec;
    int lugarCliente = 1;

    if (head == NULL) {
        printf("La lista se encuentra vacia\n");
        return;
    }

    rec = head;

    do {
        printf("\n[%d] | ID: %d | RUT: %s | NOMBRE: %s | EDAD: %d \n", lugarCliente, rec->datosCliente->idCliente, rec->datosCliente->rutCliente, rec->datosCliente->nombreCliente, rec->datosCliente->edad);
        lugarCliente++;
        rec = rec -> sig;
    }while (rec != head);
}


/*AGREGAR CLIENTE A LA LISTA DE ESPERA*/
void agregarClienteListaEsperaMenu(struct Tienda *ibcomics) {
    char idRutCliente[15];
    int idTituloBuscado;
    struct Cliente *clienteEnTienda, *clienteEnFila;
    struct Titulo *tituloEncontrado;
    struct NodoListaEspera *nuevoNodo;

    if (ibcomics == NULL) {
        return;
    }
    printf(" ============ MENU AGREGAR CLIENTE LISTA DE ESPERA ============\n\n");
    printf("Ingrese el ID del titulo que desea buscar: \n");
    idTituloBuscado = inputEntero();

    tituloEncontrado = verificarPedirTitulo(ibcomics, idTituloBuscado);
    if (tituloEncontrado == NULL) {
        printf("El titulo no se encuentra en la tienda\n");
        return;
    }
    printf("Titulo encontrado: %s\n\n", tituloEncontrado->nombreTitulo);

    printf("Ingrese ID del cliente o RUT (Ej: 12.345.678-9) =\n");
    fgets(idRutCliente, sizeof(idRutCliente), stdin);
    idRutCliente[strcspn(idRutCliente, "\n")] = '\0';

    clienteEnTienda = verificarPedirCliente(ibcomics,idRutCliente);
    if (clienteEnTienda == NULL) {
        printf("Error: El cliente no se encuentra registrado en la tienda\n");
        return;
    }

    clienteEnFila = verificarPedirClienteListaEspera(tituloEncontrado, idRutCliente);
    if (clienteEnFila != NULL) {
        printf("Error: El cliente ya se encuentra dentro de la lista de espera\n");
        return;
    }
    if (tituloEncontrado->estadoListaEspera != 1) {  /*1: Activo*/
        printf("Error: La lista de espera se encuentra suspendida, (solo se activa si el titulo esta agotado o en preventa)\n");
        return;
    }
    nuevoNodo = crearNodoListasEspera(clienteEnTienda);
    if (nuevoNodo == NULL) {
        printf("Error al agregar cliente");
        return;
    }
    enlazarNodoListaEspera(&(tituloEncontrado->headListaEspera), nuevoNodo);
    printf("\nCliente agregado a la lista de espera \n");
    mostrarInformacionCliente(clienteEnTienda);
}

void quitarClienteListaEsperaMenu(struct Tienda *ibcomics) {
    int idTituloBuscado;
    char idRutCliente[15];
    struct Titulo *tituloEncontrado = NULL;
    struct Cliente *clienteEncontrado = NULL;

    if (ibcomics == NULL) {
        return;
    }
    printf(" ============ MENU QUITAR CLIENTE LISTA DE ESPERA ============\n");
    printf("Ingrese el ID del titulo que desea buscar: \n");
    idTituloBuscado = inputEntero();

    tituloEncontrado = verificarPedirTitulo(ibcomics, idTituloBuscado);
    if (tituloEncontrado == NULL) {
        printf("El titulo no se encuentra en la tienda\n");
        return;
    }
    printf("Titulo encontrado: %s\n\n", tituloEncontrado->nombreTitulo);
    printf("Ingrese ID del cliente o RUT (Ej: 12.345.678-9) =\n");
    fgets(idRutCliente, sizeof(idRutCliente), stdin);
    idRutCliente[strcspn(idRutCliente, "\n")] = '\0';

    /*no se verifica si el cliente está en tienda pq se asume que si está en una lista de espera ya está en la tienda*/

    clienteEncontrado = verificarPedirClienteListaEspera(tituloEncontrado, idRutCliente);
    if (clienteEncontrado == NULL) {
        printf("Error: No se encontro al cliente en la lista de espera.\n");
        return;
    }
    if (eliminarClienteDeListaEspera(&(tituloEncontrado->headListaEspera), clienteEncontrado->idCliente) == 1) {
        printf("Cliente eliminado con exito!\n");
    } else {
        printf("Error: El cliente no pudo ser eliminado.\n");
    }
    mostrarInformacionCliente(clienteEncontrado);
}

void MenuCalcularTiempoEstimadoEsperaDias(struct Tienda *ibcomics) {
    struct Titulo *tituloEncontrado = NULL;
    int idTituloBuscado;
    int cantidadDias;

    if (ibcomics == NULL) {
        return;
    }

    printf(" ============ MENU CALCULAR TIEMPO ESTIMADO DE ESPERA POR TITULO ============\n\n");
    printf("Ingrese el ID del titulo que desea buscar = \n");

    idTituloBuscado = inputEntero();

    tituloEncontrado = verificarPedirTitulo(ibcomics, idTituloBuscado);
    if (tituloEncontrado == NULL) {
        printf("El titulo no se encuentra en la tienda\n");
        return;
    }

    printf("Titulo encontrado con exito!\n");
    mostrarDatosTitulo(tituloEncontrado);

    cantidadDias = calcularTiempoEstimadoEsperaDias(tituloEncontrado);

    if (cantidadDias == -1) {
        printf("Error: No se pudo calcular el tiempo de espera estimado (sin despachos programados).\n");
    }
    else if (cantidadDias == 1) {
        printf(" =====> El tiempo estimado es de %d dia <=====\n", cantidadDias);
    }
    else {
        printf(" =====> El tiempo estimado es de %d dias <=====\n", cantidadDias);
    }
}

void MenuContarCantidadClientesEnListaEspera(struct Tienda *ibcomics) {
    struct Titulo *tituloEncontrado = NULL;
    int idTituloBuscado;
    int cantidadClientes;
    if (ibcomics == NULL) {
        return;
    }
    printf(" ============ MENU CONTAR CANTIDAD DE CLIENTES EN LISTA DE ESPERA ============\n");
    printf("Ingrese el ID del titulo que desea buscar = \n");

    idTituloBuscado = inputEntero();

    tituloEncontrado = verificarPedirTitulo(ibcomics, idTituloBuscado);
    if (tituloEncontrado == NULL) {
        printf("El titulo no se encuentra en la tienda\n");
        return;
    }

    printf("Titulo encontrado con exito!\n");
    mostrarDatosTitulo(tituloEncontrado);

    cantidadClientes = contarCantidadClientesEnListaEspera(tituloEncontrado->headListaEspera);

    if (cantidadClientes == 1) {
        printf("\n Cantidad de personas en la lista de espera de %s es de : %d persona\n", tituloEncontrado->nombreTitulo, cantidadClientes);
    }
    else {
        printf("\n Cantidad de personas en la lista de espera de %s es de : %d personas\n", tituloEncontrado->nombreTitulo, cantidadClientes);
    }
}

void mostrarTituloMayorListaEsperaEnSalaMenu(struct Tienda *ibcomics) {
    struct Titulo *tituloEncontrado = NULL;
    struct Sala *salaEncontrada = NULL;
    int opcionSeleccionada = 0;

    if (ibcomics == NULL) {
        return;
    }

    printf(" ============ MENU MOSTRAR TITULO CON MAYOR LISTA DE ESPERA POR SALA ============\n");
    printf("\n");

    do {
        printf(" Opciones disponibles : \n");
        printf(" === [1] SuperHeroe / [2] Manga y Anime / [3] Novela Grafica  ===\n");
        printf(" === [4] Comic Independiente / [5] Infantil y Humor Grafico / [6] Coleccionista ===\n");

        printf("\n Ingrese la sala que desea buscar: \n");
        opcionSeleccionada = inputEntero();

        if (opcionSeleccionada < 1 || opcionSeleccionada > 6) {
            printf("Error: Opcion fuera de rango (1-6). Intente nuevamente.\n\n");
        }
        /*sala encontrada sigue como null y vuelve a repetir la pregunta*/
        else {
            salaEncontrada = buscarSalaPorTematica(ibcomics, opcionSeleccionada);

            if (salaEncontrada == NULL) {
                printf("Error: No hay ninguna sala registrada con esa tematica. Intente nuevamente.\n\n");
            }
        }
    }while (salaEncontrada == NULL);

    printf("Sala encontrada con exito!\n");
    printf("Nombre de la sala: %s\n", salaEncontrada->nombreSala);

    tituloEncontrado = obtenerTituloMayorListaEspera(salaEncontrada->headNodoTitulo);  /*solo busca el titulo, no se necesita validar si existe*/

    if (tituloEncontrado != NULL) {
        printf("\n =====> El titulo con mayor lista de espera es: '%s' <=====\n", tituloEncontrado->nombreTitulo);
    } else {
        printf("\n No hay titulos con lista de espera en esta sala.\n");
    }
}

void menuListarClientesListaEspera(struct Tienda *ibcomics) {
    struct Titulo *tituloEncontrado;
    int idTituloBuscado;

    if (ibcomics == NULL) {
        return;
    }

    printf(" ============ MENU LISTAR LISTA DE ESPERA ============\n");
    printf("\n");
    printf("Ingrese el ID del titulo que desea buscar = \n");

    idTituloBuscado = inputEntero();

    tituloEncontrado = verificarPedirTitulo(ibcomics, idTituloBuscado);
    if (tituloEncontrado == NULL) {
        printf("El titulo no se encuentra en la tienda\n");
        return;
    }

    printf("El titulo ha sido encontrado! \n");
    printf("Nombre: %s\n", tituloEncontrado->nombreTitulo);
    if (tituloEncontrado->estadoListaEspera == 1) {
        printf("Estado de la lista de espera: Activa \n");
    }
    else {
        printf("Estado de la lista de espera: Suspendida \n");
    }
    printf("\n");
    listarClientesListaEspera(tituloEncontrado->headListaEspera);
}

void vaciarListaEsperaMenu(struct Tienda *ibcomics) {
    struct Titulo *tituloEncontrado = NULL;
    int idTituloBuscado;
    int resultado;

    if (ibcomics == NULL) {
        return;
    }

    printf(" ============ MENU VACIAR LISTA DE ESPERA ============\n");
    printf("\n");
    printf("Ingrese el ID del titulo que desea buscar = \n");

    idTituloBuscado = inputEntero();

    tituloEncontrado = verificarPedirTitulo(ibcomics, idTituloBuscado);
    if (tituloEncontrado == NULL) {
        printf("El titulo no se encuentra en la tienda\n");
        return;
    }

    printf("El titulo ha sido encontrado! \n");
    printf("Nombre: %s", tituloEncontrado->nombreTitulo);

    resultado = contarCantidadClientesEnListaEspera(tituloEncontrado->headListaEspera);
    if (resultado == 0) {
        printf("La lista de espera de este titulo ya se encuentra vacia.\n");
        return;
    }
    vaciarListaEspera(&(tituloEncontrado->headListaEspera));
    printf("Se vacio la lista de espera de %s (%d cliente(s) removidos).\n", tituloEncontrado->nombreTitulo, resultado);
}

/*------------------MENU LISTA ESPERA---------------*/

void imprimirMenuListaEspera(struct Tienda *ibcomics) {
    int opcionSeleccionada;

    do {
        printf("\n");
        printf("  =========================================================\n");
        printf("  ||                                                     ||\n");
        printf("  ||         B I E N V E N I D O   A L   M E N U         ||\n");
        printf("  ||                                                     ||\n");
        printf("  ||             L I S T A   D E   E S P E R A           ||\n");
        printf("  ||                                                     ||\n");
        printf("  =========================================================\n");
        printf("\n");

        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        printf("\n");
        printf("      Opcion [1]: Agregar Cliente a Lista de espera \n");
        printf("      Opcion [2]: Quitar Cliente de Lista de espera \n");
        printf("      Opcion [3]: Mostrar tiempo estimado de Lista de espera \n");
        printf("      Opcion [4]: Mostrar cantidad de Clientes en una Lista de espera \n");
        printf("      Opcion [5]: Mostrar Titulo con mayor Lista de espera por sala \n");
        printf("      Opcion [6]: Listar clientes en una lista de espera \n");
        printf("\n");
        printf("      Opcion [7]: Vaciar Lista de espera \n");
        printf("      Opcion [8]: Salir de Menu \n");
        printf("\n");

        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        opcionSeleccionada = inputEntero();
        printf("\n");

        switch (opcionSeleccionada) {
            case 1:
                printf("Ha seleccionado la opcion 1: Agregar cliente a Lista de espera \n");
                agregarClienteListaEsperaMenu(ibcomics);
                break;

            case 2:
                printf("Ha seleccionado la opcion 2: Quitar cliente de Lista de espera \n");
                quitarClienteListaEsperaMenu(ibcomics);
                break;

            case 3:
                printf("Ha seleccionado la opcion 3: Mostrar tiempo estimado de Lista de espera \n");
                MenuCalcularTiempoEstimadoEsperaDias(ibcomics);
                break;

            case 4:
                printf("Ha seleccionado la opcion 4: Mostrar cantidad de clientes en Lista de espera \n");
                MenuContarCantidadClientesEnListaEspera(ibcomics);
                break;

            case 5:
                printf("Ha seleccionado la opcion 5: Mostrar titulo con Mayor lista de espera por sala \n");
                mostrarTituloMayorListaEsperaEnSalaMenu(ibcomics);
                break;

            case 6:
                printf("Ha seleccionado la opcion 6: Listar clientes en una lista de espera \n");
                menuListarClientesListaEspera(ibcomics);
                break;

            case 7:
                printf("Ha seleccionado la opcion 7: Vaciar Lista de espera \n");
                vaciarListaEsperaMenu(ibcomics);
                break;

            case 8:
                printf("Saliendo del menu...\n");
                break;

            default :
                printf("\n  ERROR! Opcion no valida. Intente nuevamente :) \n");
                break;
        }

    } while (opcionSeleccionada != 8);
}
/* =========================================================================
   MODULO DE PASES (Nico)
   ========================================================================= */


struct NodoPase *crearNodoPase(void) {
    struct NodoPase *nodoPase = (struct NodoPase *) malloc(sizeof(struct NodoPase));
    if (nodoPase == NULL) {
        return NULL;
    }
    nodoPase->datosPase = NULL;
    nodoPase->sig = NULL;
    return nodoPase;
}

struct Pase *crearPase(void) {
    struct Pase *pase = (struct Pase *) malloc(sizeof(struct Pase));
    if (pase == NULL) {
        return NULL;
    }
    pase->idPase = 0;
    pase->tipoPase = 0;
    pase->valor = 0;
    pase->estado = 0;
    pase->horarioUso = NULL;
    pase->datosSala = NULL;
    pase->arregloFecha[0] = 0;
    pase->arregloFecha[1] = 0;
    pase->arregloFecha[2] = 0;
    pase->arregloFecha[3] = 0;
    pase->arregloFecha[4] = 0;
    pase->horarioUso = NULL;
    return pase;
}

struct Pase *buscarPasePorId(struct NodoPase *pases, int idBuscado) {
    struct NodoPase *actual;
    if (pases == NULL) {
        return NULL;
    }
    actual = pases->sig;
    while (actual != NULL) {
        if (actual->datosPase != NULL && actual->datosPase->idPase == idBuscado) {
            return actual->datosPase;
        }
        actual = actual->sig;
    }
    return NULL;
}


int buscarPaseEnCLientes(struct NodoCliente *abb, int idBuscado) {
    if (abb == NULL) {
        return 0;
    }
    if (abb->datosCliente != NULL && abb->datosCliente->headPase != NULL) {
        if (abb->datosCliente->headPase->sig != NULL) {
            if (buscarPasePorId(abb->datosCliente->headPase, idBuscado) != NULL) {
                return 1;
            }
        }
    }
    if (buscarPaseEnCLientes(abb->izq, idBuscado)) return 1;
    if (buscarPaseEnCLientes(abb->der, idBuscado)) return 1;

    return 0;
}

int crearIdPase(struct NodoCliente *abb) {
    int idNuevo = 0;
    int esUnico = 0;

    while (esUnico != 1) {
        idNuevo = ((rand() % 1000) * 100) + (rand() % 100) + 1;

        if (buscarPaseEnCLientes(abb, idNuevo) == 0) {
            esUnico = 1;
        }
    }
    return idNuevo;
}

int agregarPaseACliente(struct Cliente *cliente, struct Pase *pase) {
    struct NodoPase *nuevoNodo, *actual;

    nuevoNodo = crearNodoPase();
    if (nuevoNodo == NULL) {
        return 0;
    }
    nuevoNodo->datosPase = pase;

    actual = cliente->headPase;
    while (actual->sig != NULL) {
        actual = actual->sig;
    }
    actual->sig = nuevoNodo;
    return 1;
}

/* Crea un pase completo con ID nuevo. Retorna NULL si falla la memoria */
struct Pase *crearPaseCompleto(struct Tienda *tienda, int tipoPase, int valorPase, int fechaUsoPase[], struct Sala *sala) {
    struct Pase *nuevo = crearPase();
    if (nuevo == NULL) {
        return NULL;
    }
    nuevo->idPase = crearIdPase(tienda->raiz);
    nuevo->valor = valorPase;
    nuevo->estado = 1;
    nuevo->tipoPase = tipoPase;
    nuevo->arregloFecha[0] = fechaUsoPase[0]; /*Dia*/
    nuevo->arregloFecha[1] = fechaUsoPase[1]; /*Mes*/
    nuevo->arregloFecha[2] = fechaUsoPase[2]; /*Anio*/
    nuevo->arregloFecha[3] = fechaUsoPase[3]; /*Hora*/
    nuevo->arregloFecha[4] = fechaUsoPase[4]; /*Minuto*/
    if (nuevo->arregloFecha[3] >= 8 && nuevo->arregloFecha[3] < 12) {
        nuevo->horarioUso = copiarTexto("Maniana");
    }else if (nuevo->arregloFecha[3] >= 12 && nuevo->arregloFecha[3] < 18) {
        nuevo->horarioUso = copiarTexto("Tarde");
    }else {
        nuevo->horarioUso = copiarTexto("Noche");
    }
    if (nuevo->horarioUso == NULL) {
        free(nuevo);
        return NULL;
    }
    nuevo->datosSala = sala;
    return nuevo;
}
/*resolver que el puntero datoSala no queden apuntando a una sala ya eliminada*/
void desvincularPasesDeSala(struct NodoCliente *raiz, struct Sala *sala) {
    struct NodoPase *rec;
       if (raiz == NULL){
        return;
    }   
    if (raiz->datosCliente != NULL && raiz->datosCliente->headPase != NULL) {
        rec = raiz->datosCliente->headPase->sig;
        while (rec != NULL) {
            if (rec->datosPase != NULL && rec->datosPase->datosSala == sala) {
                rec->datosPase->datosSala = NULL;
            }
            rec = rec->sig;
        }
    }
    desvincularPasesDeSala(raiz->izq, sala);
    desvincularPasesDeSala(raiz->der, sala);
}

void crearFechaPase(int fechaUsoPase[], struct Tienda *tienda) {
    int dia, mes, anio, hora, minuto, maxDia;  /*maxDia es el maximo de dia que tendra un mes*/
    do {
        printf("Ingrese el anio de uso: ");
        anio = inputEntero();
        printf("\n");
    }while (anio != tienda->arregloFechaAct[2] && !(tienda->arregloFechaAct[1] == 12 && anio == tienda->arregloFechaAct[2] + 1));

    do {
        printf("Ingrese el mes de uso(Solo puede ingresar hasta un mes despues de la fecha actual): ");
        mes = inputEntero();
        printf("\n");
    }while (mes < 1 || mes > 12 || (anio != tienda->arregloFechaAct[2] && mes != 1) || (anio == tienda->arregloFechaAct[2] && mes != tienda->arregloFechaAct[1] && mes != tienda->arregloFechaAct[1]+ 1));

    if (mes == 2) {
        if (anio % 400 == 0 || (anio % 4 == 0 && anio % 100 != 0) ) {
            maxDia = 29;
        }else {
            maxDia = 28;
        }
    }else if (mes == 4 || mes == 6 || mes == 9 || mes == 11) {
        maxDia = 30;
    }else {
        maxDia = 31;
    }

    do {
        printf("Ingrese el dia de Uso: ");
        dia = inputEntero();
        printf("\n");
    }while (dia < 1 || dia > maxDia || (mes == tienda->arregloFechaAct[1] && dia < tienda->arregloFechaAct[0]));

    do {
        printf("Ingrese la hora de uso (8hrs - 21hrs): ");
        hora = inputEntero();
        printf("\n");
    }while (hora < 8 || hora > 21);

    do {
        printf("Ingrese minuto de uso(00min-59min): ");
        minuto = inputEntero();
        printf("\n");
    }while (minuto < 00 || minuto > 59);

    fechaUsoPase[0] = dia;
    fechaUsoPase[1] = mes;
    fechaUsoPase[2] = anio;
    fechaUsoPase[3] = hora;
    fechaUsoPase[4] = minuto;

    printf("\nPase creado con fecha: %2d/%2d/%2d, a las %2d:%2dhrs\n", fechaUsoPase[0], fechaUsoPase[1],fechaUsoPase[2], fechaUsoPase[3], fechaUsoPase[4]);
}

void comprobarPase(int *tipoPase, int precios[],int *valorPase,struct Cliente *clienteComprador,int *esValidoPase) {
    *esValidoPase = 1;
    switch (*tipoPase) {
        case 1:
            *valorPase = precios[0];
            break;
        case 2:
            *valorPase = precios[1];
            if (clienteComprador->edad > 10 || clienteComprador->edad < 1) {
                *esValidoPase = 0;
                return;
            }
            *esValidoPase = 1;
            break;
        case 3:
            *valorPase = precios[2];

            break;
        case 4:
            *valorPase = precios[3];

            break;
        default:
            break;
    }

}

void seleccionarTipoPase(int *tipoPase, int precios[],int *valorPase,struct Cliente *clienteComprador,int *esValidoPase){
    do {
        printf("\n");
        printf("   Seleccione el tipo de pase que desea comprar : \n");
        printf("\n");
        printf("      Opcion [1]: Pase general \n");
        printf("                  Valor del pase: %d\n", precios[0]);
        printf("      Opcion [2]: Pase infantil (Maximo 10 anios) \n");
        printf("                  Valor del pase: %d\n", precios[1]);
        printf("      Opcion [3]: Pase familiar (4 integrantes) \n");
        printf("                  Valor del pase: %d\n", precios[2]);
        printf("      Opcion [4]: Pase prioritario \n");
        printf("                  Valor del pase: %d\n", precios[3]);
        printf("\n");
        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        *tipoPase = inputEntero();
    } while (*tipoPase < 1 || *tipoPase > 4);

    comprobarPase(tipoPase,precios,valorPase,clienteComprador,esValidoPase);

}

void seleccionarSala(struct Tienda* tienda,int *salaSeleccionada) {
    int i;
    do {
        printf("\n");
        printf("   Ingrese la sala que desea visitar : \n");
        printf("\n");
        for (i = 0 ; i < tienda->cantidadSalas; i++) {
            printf("      Opcion [%d]: %s \n", i + 1, tienda->salas[i]->nombreSala);
        }
        printf("\n");
        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        *salaSeleccionada = inputEntero();
    } while (*salaSeleccionada < 1 || *salaSeleccionada > tienda->cantidadSalas);
}

struct Pase *crearCopiaPaseFamiliar(struct Pase *paseOriginal) {    //* Esta funcion hace una copia del pase creado para cada miembro familiar*//
    struct Pase *paseCopia;

    if (paseOriginal == NULL) {
        return NULL;
    }

    paseCopia = crearPase();

    if (paseCopia == NULL) {
        return NULL;
    }

    *paseCopia = *paseOriginal; //*Se usan punteros para no apuntar la misma direccion de memoria*//
    paseCopia->horarioUso = NULL;
    if (paseOriginal->horarioUso != NULL) {
        paseCopia->horarioUso = copiarTexto(paseOriginal->horarioUso);
        if (paseCopia->horarioUso == NULL) {
            free(paseCopia);
            return NULL;
        }
    }
    return paseCopia;
}

int crearPaseIndividual(struct Pase *nuevo, struct Cliente *clienteComprador) {
    if (nuevo == NULL) {
        return 0;
    }
    if (agregarPaseACliente(clienteComprador, nuevo) == 0) {
        free(nuevo->horarioUso);
        free(nuevo);
        return 0;
    }
    return 1;
}

int agregarCLienteFamiliar(struct Tienda *tienda, int idClienteFamiliar[4]) {
    int i, j, idCliente, esIdValido;

    do {
        esIdValido = 1;

        printf("\n");
        printf("Para salir ingrese 0\n");

        for (i = 0; i < 4; i++) {
            printf("Ingrese el id del cliente integrante %d = ", i + 1);
            idCliente = inputEntero();

            if (idCliente == 0) {
                return 0;
            }

            if (buscarClientePorID(tienda->raiz, idCliente) == NULL) {
                printf("Error: No se pudo encontrar al cliente con el ID %d.\n", idCliente);
                printf("Por favor, registrelo primero desde el menu de clientes.\n");
                esIdValido = 0;
                break;
            }
            for (j = 0; j < i; j++) {
                if (idClienteFamiliar[j] == idCliente) {
                    printf("Error: El cliente ya fue ingresado, registre un nuevo cliente.\n");
                    esIdValido = 0;
                    break;
                }
            }
            if (!esIdValido) {
                break;
            }
            idClienteFamiliar[i] = idCliente;
        }
    } while (esIdValido == 0);

    return 1;
}

int crearPasesFamiliares(struct Tienda *tienda, struct Pase *nuevo,int idClienteFamiliar[4]) {
    struct Cliente *cliente;
    struct Pase *pases[4];
    struct NodoPase *nodos[4];
    struct NodoPase *nodoActual;
    int i,j;

    for (i = 0; i < 4; i++) {
        pases[i] = NULL;
        nodos[i] = NULL;
    }
    pases[0] = nuevo;

    for (i = 1; i < 4; i++) {
        pases[i] = crearCopiaPaseFamiliar(nuevo);
        if (pases[i] == NULL) {
            break;
        }
    }
    if (i == 4) {
        for (i = 0; i < 4; i++) {
            nodos[i] = crearNodoPase();
            if (nodos[i] == NULL) {
                break;
            }
            nodos[i]->datosPase = pases[i];
        }
    }

    if (i==4) {
        for (i = 0; i < 4; i++) {
            cliente = buscarClientePorID(tienda->raiz, idClienteFamiliar[i]);
            if (cliente == NULL|| cliente->headPase == NULL) {
                break;
            }
        }
    }
    if (i != 4) {
        for (j = 0; j < 4; j++) {
            free(nodos[j]);
            if (pases[j] != NULL) {
                free(pases[j]->horarioUso);
                free(pases[j]);
            }
        }
        return 0;
    }
    for (i = 0; i < 4; i++) {
        cliente = buscarClientePorID(tienda->raiz, idClienteFamiliar[i]);
        nodoActual = cliente->headPase;
        while (nodoActual->sig != NULL) {
            nodoActual = nodoActual->sig;
        }
        nodoActual->sig = nodos[i];
    }
    return 1;
}

void comprarPases(struct Tienda *tienda, int precios[]) {
    int tipoPase, valorPase = 0, idClienteCompra, salaSeleccionada, esValidoPase;
    int fechaUsoPase[5];
    int idClienteFamiliar[4];
    struct Cliente *clienteComprador;
    struct Sala *salaElegida;
    struct Pase *nuevo;

    printf(" ============ MENU COMPRAR PASES ============\n");
    printf("\n");

    printf("Ingrese el id del cliente = ");
    idClienteCompra = inputEntero();

    clienteComprador = buscarClientePorID(tienda->raiz, idClienteCompra);

    if (clienteComprador == NULL) {
        printf("Error: No se pudo encontrar al cliente con el ID %d.\n", idClienteCompra);
        printf("Por favor, registrelo primero desde el Menu de Clientes.\n");
        return;
    }

    if (tienda->cantidadSalas == 0) {
        printf("Error: No hay salas registradas. Registre una sala primero.\n");
        return;
    }

    seleccionarTipoPase(&tipoPase,precios,&valorPase,clienteComprador,&esValidoPase);
    if (esValidoPase == 0) {
        printf("Error: El cliente no cumple con las condiciones de edad para este pase.\n");
        return;
    }

    seleccionarSala(tienda,&salaSeleccionada);
    salaElegida = tienda->salas[salaSeleccionada - 1];

    if (salaElegida == NULL) {
        printf("Error: No se encontro la sala seleccionada.\n");
        return;
    }
    printf("Sala seleccionada con exito!\n");

    crearFechaPase(fechaUsoPase,tienda);
    printf("Valor del pase: %d\n", valorPase);

    nuevo = crearPaseCompleto(tienda, tipoPase, valorPase, fechaUsoPase, salaElegida);
    if (nuevo == NULL) {
        printf("Error al crear el pase, volviendo al menu\n");
        return;
    }

    if (tipoPase != 3) {
        if (crearPaseIndividual(nuevo, clienteComprador) == 0) {
            printf("Error: No se pudo crear el pase.\n");
            return;
        }
    } else {
        if (agregarCLienteFamiliar(tienda,idClienteFamiliar)==0) {
            free(nuevo->horarioUso);
            free(nuevo);
            printf("Error al elegir clientes\n");
            return;
        }

        if (crearPasesFamiliares(tienda,nuevo,idClienteFamiliar)== 0) {
            printf("Error al crear pases familiares\n");
            return;
        }
    }
    printf("\n");
    printf("Pase creado con exito!\n");
    printf("ID del pase: %d\n", nuevo->idPase);
}
void verDatosPases(struct NodoCliente *clientes) {
    int idDelPase, idDelCliente;
    struct Cliente *cliente;
    struct Pase *mostrado;

    printf(" ============ MENU VER DATOS DE PASE ============\n");
    printf("\n");
    printf("Ingrese el id del cliente = ");
    idDelCliente = inputEntero();
    cliente = buscarClientePorID(clientes, idDelCliente);
    if (cliente == NULL) {
        printf("Error: No se pudo encontrar al cliente con el ID %d.\n", idDelCliente);
        printf("Por favor, vuelva al menu principal y registre primero al cliente desde el Menu de Clientes (Opcion 1).\n");
        return;
    }
    if (cliente->headPase->sig == NULL) {
        printf("Error: El cliente no tiene pases.\n");
        return;
    }

    printf("Ingrese el id del pase = ");
    idDelPase = inputEntero();
    mostrado = buscarPasePorId(cliente->headPase, idDelPase);
    if (mostrado == NULL) {
        printf("Error: No se pudo encontrar el pase con el ID %d.\n", idDelPase);
        return;
    }

    printf("Pase encontrado con exito!\n");
    printf("Id del pase: %d\n", mostrado->idPase);
    printf("Valor del pase: %d\n", mostrado->valor);
    if (mostrado->estado == 1) {
        printf("Estado: Activo");
    }else if (mostrado->estado == 2){
        printf("Estado: Utilizado");
    }else if (mostrado->estado == 3) {
        printf("Estado: Anulado");
    }else {
        printf("Estado: Vencido");
    }
    if (mostrado->tipoPase == 1) {
        printf("Tipo: General");
    }else if (mostrado->tipoPase == 2) {
        printf("Tipo: Infantil");
    }else if (mostrado->tipoPase == 3) {
        printf("Tipo: Familiar");
    }else {
        printf("Tipo: Prioritario");
    }
    if (mostrado->datosSala != NULL) {
    printf("Sala: %s\n", mostrado->datosSala->nombreSala);
    }
    else {
    printf("Sala: (sin sala asignada)\n");
    }

}
char *nombreEstadoPase(int estado) {
    if (estado == 1) {
        return "Activo";
    } else if (estado == 2) {
        return "Utilizado";
    } else if (estado == 3) {
        return "Anulado";
    } else if (estado == 4) {
        return "Vencido";
    }
    return "Desconocido";
}

void listarPasesDeCliente(struct Cliente *cliente) {
    struct NodoPase *rec;
    rec = cliente->headPase->sig;
    while (rec!=NULL) {
        if (rec->datosPase != NULL) {
            printf("ID = [%d]  |  Valor = %d  |  Horario = %s  |  Estado = %s\n", rec->datosPase->idPase, rec->datosPase->valor, rec->datosPase->horarioUso, nombreEstadoPase(rec->datosPase->estado));
        }
        rec = rec->sig;
    }
}

void anularPases(struct NodoCliente *abb) {
    int idCliente, id_Pase;
    struct Cliente *cliente;
    struct NodoPase *nodoPase;

    printf(" ============ MENU ANULAR PASE ============\n");
    printf("\n");
    printf("Ingrese el id del cliente = ");
    idCliente = inputEntero();
    cliente = buscarClientePorID(abb, idCliente);
    if (cliente == NULL || cliente->headPase->sig == NULL) {
        printf("Error: El cliente no tiene pases o no fue encontrado.\n");
        return;
    }

    printf("Ingrese el id del pase a anular = ");
    id_Pase = inputEntero();

    nodoPase = cliente->headPase->sig;
    while (nodoPase != NULL) {
        if (nodoPase->datosPase != NULL && nodoPase->datosPase->idPase == id_Pase) {
            nodoPase->datosPase->estado = 3;
            printf("Pase anulado con exito!\n");
            return;
        }
        nodoPase = nodoPase->sig;
    }
    printf("Error: No se pudo encontrar el pase con el ID %d.\n", id_Pase);
}

void verCantidad(struct NodoCliente *abb) {
    int idCliente, contador = 0;
    struct Cliente *cliente;
    struct NodoPase *actual;

    printf(" ============ MENU VER CANTIDAD DE PASES ============\n");
    printf("\n");
    printf("Ingrese el id del cliente = ");
    idCliente = inputEntero();
    cliente = buscarClientePorID(abb, idCliente);
    if (cliente == NULL || cliente->headPase->sig == NULL) {
        printf("Error: El cliente no tiene pases o no fue encontrado.\n");
        return;
    }

    actual = cliente->headPase->sig;
    while (actual != NULL) {
        contador++;
        actual = actual->sig;
    }
    printf("\n =====> El numero de pases es: %d <=====\n", contador);
}

void eliminarPases(struct NodoCliente *abb) {
    int idCliente, id_pase;
    struct Cliente *cliente;
    struct NodoPase *actual, *nodoEliminar;

    printf(" ============ MENU ELIMINAR PASE ============\n");
    printf("\n");
    printf("Ingrese el id del cliente = ");
    idCliente = inputEntero();
    cliente = buscarClientePorID(abb, idCliente);
    if (cliente == NULL || cliente->headPase->sig == NULL) {
        printf("Error: El cliente no tiene pases o no fue encontrado.\n");
        return;
    }

    printf("Ingrese el id del pase a eliminar = ");
    id_pase = inputEntero();

    actual = cliente->headPase;
    while (actual->sig != NULL && actual->sig->datosPase->idPase != id_pase) {
        actual = actual->sig;
    }

    if (actual->sig != NULL) {
        nodoEliminar = actual->sig;
        actual->sig = nodoEliminar->sig;
        free(nodoEliminar->datosPase);
        free(nodoEliminar);
        printf("Pase eliminado con exito!\n");
    } else {
        printf("Error: No se pudo encontrar el pase con el ID %d.\n", id_pase);
    }
}

void contarPases(struct NodoCliente *abb, int contarTipoPase[]) {
    struct NodoPase *actual;
    int tipo;

    if (abb == NULL) {
        return;
    }

    if (abb->datosCliente != NULL && abb->datosCliente->headPase != NULL) {
        actual = abb->datosCliente->headPase->sig;
        while (actual != NULL) {
            if (actual->datosPase != NULL) {
                tipo = actual->datosPase->tipoPase;
                if (tipo >= 1 && tipo <= 4) {
                    contarTipoPase[tipo - 1]++;
                }
            }
            actual = actual->sig;
        }
    }
    contarPases(abb->izq, contarTipoPase);
    contarPases(abb->der, contarTipoPase);
}

void verMasVendidos(struct NodoCliente *abb) {
    int contadorTipoPase[4] = {0, 0, 0, 0}, i, mayor = 0;
    contarPases(abb, contadorTipoPase);
    contadorTipoPase[2] = contadorTipoPase[2]/ 4;

    printf(" ============ MENU ESTADISTICAS DE PASES ============\n");
    printf("\n");
    printf("General: %d\nInfantil: %d\nFamiliar: %d\nPrioritario: %d\n",
           contadorTipoPase[0], contadorTipoPase[1], contadorTipoPase[2], contadorTipoPase[3]);

    for (i = 0; i < 4; i++) {
        if (contadorTipoPase[i] > mayor) {
            mayor = contadorTipoPase[i];
        }
    }
    printf("\n =====> Los pases mas vendidos tienen %d ventas <=====\n", mayor);
}

/*funcion que recibe el abb por lo tanto sirve para el menu*/
void listarPases(struct NodoCliente *abb) {
    int idClienteListar;
    struct Cliente *cliente;
    struct NodoPase *actual;

    printf(" ============ MENU LISTAR PASES DEL CLIENTE ============\n");
    printf("\n");
    printf("Ingrese el id del cliente = ");
    idClienteListar = inputEntero();
    cliente = buscarClientePorID(abb, idClienteListar);
    if (cliente == NULL || cliente->headPase->sig == NULL) {
        printf("Error: El cliente no tiene pases o no fue encontrado.\n");
        return;
    }

    printf("\n");
    actual = cliente->headPase->sig;
    while (actual != NULL) {
        if (actual->datosPase != NULL) {
            printf("   [%d] Valor: $%d | Estado: %d | Tipo: %d\n",
                   actual->datosPase->idPase, actual->datosPase->valor,
                   actual->datosPase->estado, actual->datosPase->tipoPase);
        }
        actual = actual->sig;
    }
}

void poblarPrecios(int arregloPrecios[]) {
    arregloPrecios[0] = 5000;
    arregloPrecios[1] = 3000;
    arregloPrecios[2] = 17000;
    arregloPrecios[3] = 10000;
}

void cambiarPrecios(int arregloPrecios[]) {
    printf(" ============ MENU CAMBIAR PRECIOS ============\n");
    printf("\n");
    printf("Ingrese el nuevo precio General = ");
    arregloPrecios[0] = inputEntero();
    printf("Ingrese el nuevo precio Infantil = ");
    arregloPrecios[1] = inputEntero();
    printf("Ingrese el nuevo precio Familiar = ");
    arregloPrecios[2] = inputEntero();
    printf("Ingrese el nuevo precio Prioritario = ");
    arregloPrecios[3] = inputEntero();
    printf("Precios actualizados con exito!\n");
}

void modificarDatosPase(struct Tienda *tienda) {
    int idClienteModificar, idPaseModificar, opcionModificar, i;
    int nuevoValor, nuevoEstado, nuevaSala,nuevaFecha[5],esIgual=1;
    struct Cliente *cliente;
    struct Pase *pase;
    struct Sala *nueva_sala;

    printf(" ============ MENU MODIFICAR PASE ============\n");
    printf("\n");

    printf("Ingrese el id del cliente = ");
    idClienteModificar = inputEntero();
    printf("\n");

    cliente = buscarClientePorID(tienda->raiz, idClienteModificar);

    if (cliente == NULL) {
        printf("Error: No se pudo encontrar al cliente con el ID %d.\n", idClienteModificar);
        return;
    }

    printf("Ingrese el id del pase = ");
    idPaseModificar = inputEntero();
    printf("\n");

    pase = buscarPasePorId(cliente->headPase, idPaseModificar);
    if (pase == NULL) {
        printf("Error: No se pudo encontrar el pase con el ID %d.\n", idPaseModificar);
        return;
    }
    do {
        printf("   Que desea modificar : \n");
        printf("\n");
        printf("      Opcion [1]: Valor del pase \n");
        printf("      Opcion [2]: Estado del pase \n");
        printf("      Opcion [3]: Fecha de uso \n");
        printf("      Opcion [4]: Sala asociada \n");
        printf("\n");
        printf("   Por favor, ingrese una de las opciones disponibles : \n");

        opcionModificar = inputEntero();
        printf("\n");
    } while (opcionModificar < 1 || opcionModificar > 4);

    switch (opcionModificar) {
        case 1:
            printf("\n");
            printf("Ingrese el nuevo valor del pase = ");
            nuevoValor = inputEntero();
            if (nuevoValor < 0) {
                printf("Error al ingresar nuevo valor, debe ser positivo\n");
                break;
            }
            if (nuevoValor == pase->valor) {
                printf("Error, el nuevo valor es igual al anterior\n");
                break;
            }
            printf("\n");
            pase->valor = nuevoValor;
            printf("Valor modificado con exito!\n");
            break;

        case 2:
            printf("\n");
            do {
                printf("   Seleccione el nuevo estado : \n");
                printf("      Opcion [1]: Activo \n");
                printf("      Opcion [2]: Utilizado \n");
                printf("      Opcion [3]: Anulado \n");
                printf("      Opcion [4]: Vencido \n");
                nuevoEstado = inputEntero();
            } while (nuevoEstado < 1 || nuevoEstado > 4);
            if (pase->estado == nuevoEstado) {
                printf("Error: El pase ya se encuentra en ese estado.\n");
            } else {
                pase->estado = nuevoEstado;
                printf("Estado modificado con exito!\n");
            }
            break;

        case 3:
            printf("\n");
            crearFechaPase(nuevaFecha,tienda);
            for (i = 0; i < 5 ; i++) {
                if (pase->arregloFecha[i] != nuevaFecha[i]) {
                    esIgual = 0;
                    break;
                }
            }
            if (esIgual==1) {
                printf("Error: La fecha nueva es igual a la anterior.\n");
            }else {
                for (i = 0; i < 5; i++) {
                    pase->arregloFecha[i] = nuevaFecha[i];
                }
                printf("Fecha modificada con exito!\n");
            }
            break;

        case 4:
            printf("\n");
            do {

                printf("   Ingrese la nueva sala : \n");
                for (i = 0; i < tienda->cantidadSalas;i++) {
                    printf("      Opcion [%d]: %s \n", i + 1, tienda->salas[i]->nombreSala);
                }
                printf("      Opcion [0]: Salir \n");
                nuevaSala = inputEntero();
                if (nuevaSala == 0) {
                    return;
                }

            } while (nuevaSala < 1 || nuevaSala > tienda->cantidadSalas);

            nueva_sala = tienda->salas[nuevaSala - 1];
            if (nueva_sala == NULL) {
                printf("Error: No existe una sala con esa tematica.\n");
            } else if (nueva_sala == pase->datosSala) {
                printf("Error: La sala nueva es igual a la anterior.\n");
            } else {
                pase->datosSala = nueva_sala;
                printf("Sala modificada con exito!\n");
            }
            break;

        default:
            break;
    }

    printf("\n");
    printf("Saliendo del menu modificar pase...\n");
}

void menuPases(struct Tienda *ibcomics, int preciosPases[4]) {
    int opcion;

    do {
        printf("\n");
        printf("  =========================================================\n");
        printf("  ||                                                     ||\n");
        printf("  ||         B I E N V E N I D O   A L   M E N U         ||\n");
        printf("  ||                                                     ||\n");
        printf("  ||                    P A S E S                        ||\n");
        printf("  ||                                                     ||\n");
        printf("  =========================================================\n");
        printf("\n");

        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        printf("\n");
        printf("      Opcion [1]: Ver datos de mi Pase \n");
        printf("      Opcion [2]: Comprar Pase(s) \n");
        printf("      Opcion [3]: Anular mi Pase \n");
        printf("      Opcion [4]: Cantidad de Pases comprados por cliente\n");
        printf("      Opcion [5]: Eliminar Pase(s) \n");
        printf("      Opcion [6]: Cambiar datos del Pase \n");
        printf("      Opcion [7]: Ver Pases mas vendidos \n");
        printf("      Opcion [8]: Listar Pases del Cliente \n");
        printf("\n");
        printf("      Opcion [9]: Salir de Menu \n");
        printf("\n");

        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        opcion = inputEntero();

        switch (opcion) {
            case 1:
                printf("Ha seleccionado la opcion 1: Ver datos de mi pase \n");
                verDatosPases(ibcomics->raiz);
                break;
            case 2:
                printf("Ha seleccionado la opcion 2: Comprar pase(s) \n");
                comprarPases(ibcomics, preciosPases);
                break;
            case 3:
                printf("Ha seleccionado la opcion 3: Anular mi pase \n");
                anularPases(ibcomics->raiz);
                break;
            case 4:
                printf("Ha seleccionado la opcion 4: Cantidad de Pases comprados por cliente \n");
                verCantidad(ibcomics->raiz);
                break;
            case 5:
                printf("Ha seleccionado la opcion 5: Eliminar pase(s) \n");
                eliminarPases(ibcomics->raiz);
                break;
            case 6:
                printf("Ha seleccionado la opcion 6: Cambiar datos del pase \n");
                modificarDatosPase(ibcomics);
                break;
            case 7:
                printf("Ha seleccionado la opcion 7: Ver pases mas vendidos \n");
                verMasVendidos(ibcomics->raiz);
                break;
            case 8:
                printf("Ha seleccionado la opcion 8: Listar pases del cliente \n");
                listarPases(ibcomics->raiz);
                break;
            case 9:
                printf("Saliendo del menu...\n");
                break;
            default:
                printf("\n  ERROR! Opcion no valida. Intente nuevamente :) \n");
                break;
        }
    } while (opcion != 9);
}
/*-------------------Modularizar menu ingresarSacarClientes a las salas--------------------*/

/*retorna 1 si tanto el dia mes o anio corresponden a los de la fecha de apertura del dia*/
int verificarCumpleFechaPase(struct Pase *pase, struct Tienda *ibcomics) {
    int i;

    if (ibcomics == NULL || pase == NULL) {
        return 0;
    }
    for (i = 0; i < 3; i++) { /*recorre el arreglo fecha de pase y tienda y compara dia mes anio*/
        if (pase->arregloFecha[i] != ibcomics->arregloFechaAct[i]) {
            return 0;
        }
    }
    return 1;
}

/*retorna 1 si el horario del pase coincide con el de la sala*/
int verificarHorarioPase(struct Pase *pase) {
    if (pase != NULL && (strcmp(pase->horarioUso, pase->datosSala->horario) == 0)) {
        return 1;
    }
    return 0;
}

int verificarPaseActivo(struct Pase *pase) {
    if (pase != NULL && pase->estado == 1) {
        return 1;
    }
    return 0;
}
int verificarAforoMax(struct Sala *salaEncontrada) {
    if (salaEncontrada != NULL && salaEncontrada->aforoActual < salaEncontrada->aforoMax) {
        return 1;
    }
    return 0;
}
/* rtorna el motivo por el que no se puede ingresasr */
char *motivoRechazoIngreso(struct Tienda *ibcomics, struct Cliente *cliente, struct Pase *pase) {
    if (cliente->datosSala != NULL) {
        return "El cliente ya se encuentra dentro de una sala";
    }
    if (pase->datosSala == NULL) {
        return "La sala del pase fue eliminada.";
    }
    if (verificarPaseActivo(pase) == 0) {
        return "El pase no esta activo";
    }
    if (verificarCumpleFechaPase(pase, ibcomics) == 0) {
        return "La fecha del pase no es la del dia abierto";
    }
    if (verificarHorarioPase(pase) == 0) {
        return "El horario del pase no coincide con el de la sala";
    }
    if (verificarAforoMax(pase->datosSala) == 0) {
        return "La sala esta llena";
    }
    return NULL;
}

int ingresarClienteSala(struct Tienda *ibcomics, struct Cliente *clienteIngresado, struct Pase *pase) {
    if (motivoRechazoIngreso(ibcomics, clienteIngresado, pase) != NULL) {
        return 0;
    }

    clienteIngresado->datosSala = pase->datosSala;
    pase->datosSala->aforoActual++;
    ibcomics->totalClientes++;
    pase->estado = 2; /*utilizado*/
    return 1;
}
void menusIngresarClienteSala(struct Tienda *ibcomics) {
    struct Cliente *clienteEncontrado;
    struct Pase *paseEncontrado;
    char idRutCliente[20];
    int idPase;
    char *motivo;

    if (ibcomics == NULL) {
        return;
    }
    printf(" ============ MENU INGRESAR CLIENTE A SALA ============\n");
    printf("\n");

    if (ibcomics->diaAbierto == 1) {
        /*Si el dia esta abierto*/
        printf("Ingrese ID o RUT del cliente (Ej. 12.345.678-9) = \n");
        fgets(idRutCliente, sizeof(idRutCliente), stdin);
        idRutCliente[strcspn(idRutCliente, "\n")] = '\0';

        clienteEncontrado = verificarPedirCliente(ibcomics, idRutCliente);
        if (clienteEncontrado == NULL) {
            printf("Error: No se encontro al cliente ingresado.\n");
            return;
        }
        printf("\nPases del Cliente = \n");
        listarPasesDeCliente(clienteEncontrado);

        printf("Ingrese el ID del pase = \n");
        idPase = inputEntero();
        paseEncontrado = buscarPasePorId(clienteEncontrado->headPase, idPase);
        if (paseEncontrado == NULL) {
            printf("Error: No se encontro el pase con el ID %d.\n", idPase);
            return;
        }

        motivo = motivoRechazoIngreso(ibcomics, clienteEncontrado, paseEncontrado);
        if (motivo != NULL) {
            printf("Error: %s\n", motivo);
            return;
        }
        ingresarClienteSala(ibcomics, clienteEncontrado, paseEncontrado);
        printf("\nIngreso exitoso!\n");
        printf("El cliente %s ha ingresado a la sala %s\n", clienteEncontrado->nombreCliente, clienteEncontrado->datosSala->nombreSala);
        printf("Aforo actual = %d  |  Aforo maximo = %d\n", clienteEncontrado->datosSala->aforoActual, clienteEncontrado->datosSala->aforoMax);
    }
}
/*hora en la que termina el horario de la sala*/

void sacarClienteDeSala(struct Tienda *ibcomics, struct Cliente *cliente) {
    cliente->datosSala->aforoActual--;
    cliente->datosSala = NULL;
    ibcomics->totalClientes--;
}


/*menu manual para quitar al cliente de la sala (por si quiere salir antes del cierre)*/
void menuSacarClienteSala(struct Tienda *ibcomics) {
    struct Cliente *clienteEncontrado;
    char idRutCliente[50];

    printf(" ============ MENU SACAR CLIENTE DE SALA ============\n\n");
    printf("Ingrese ID o RUT del cliente (Ej. 12.345.678-9) = \n");
    fgets(idRutCliente, sizeof(idRutCliente), stdin);
    idRutCliente[strcspn(idRutCliente, "\n")] = '\0';

    clienteEncontrado = verificarPedirCliente(ibcomics, idRutCliente);
    if (clienteEncontrado == NULL) {
        printf("Error: No se encontro al cliente ingresado\n");
        return;
    }
    if (clienteEncontrado->datosSala == NULL) {
        printf("Error: El cliente no se encuentra en ninguna sala\n");
        return;
    }
    printf("El cliente %s salio de la sala %s\n", clienteEncontrado->nombreCliente, clienteEncontrado->datosSala->nombreSala);
    sacarClienteDeSala(ibcomics,clienteEncontrado);

}
void sacarTodosClientesSala(struct Tienda *ibcomics, struct NodoCliente *raiz, struct Sala *salaQuitada) {
    if (raiz == NULL) {
        return ;
    }
    if (raiz->datosCliente->datosSala == salaQuitada) {
        sacarClienteDeSala(ibcomics, raiz->datosCliente);
    }
    sacarTodosClientesSala(ibcomics,raiz->izq, salaQuitada);
    sacarTodosClientesSala(ibcomics,raiz->der, salaQuitada);
}

void menuSacarTodosClientesSala(struct Tienda *ibcomics) {
    int idSala;
    struct Sala *salaEncontrada;

    printf("============ MENU SACAR TODOS LOS CLIENTES DE UNA SALA ============\n\n");
    listarSalas(ibcomics);
    printf("Ingrese ID de la sala que desea acceder\n");

    idSala = inputEntero();
    salaEncontrada = buscarSalaPorId(ibcomics, idSala);

    if (salaEncontrada == NULL) {
        printf("No se pudo encontrar la sala con ese ID\n");
        return;
    }

    if (salaEncontrada->aforoActual <= 0) {
        printf("La sala no tiene clientes dentro\n");
        return;
    }
    sacarTodosClientesSala(ibcomics,ibcomics->raiz, salaEncontrada);
    printf("\nSe sacaron a todos los clientes de la sala\n");
}


/* logica para abrir el dia y actualizar la fecha */
int logicaAbrirDia(struct Tienda *tienda, int dia, int mes, int anio) {
    if (tienda == NULL) return 0;
    if (tienda->diaAbierto == 1) return -1;

    tienda->arregloFechaAct[0] = dia;
    tienda->arregloFechaAct[1] = mes;
    tienda->arregloFechaAct[2] = anio;
    tienda->diaAbierto = 1;
    return 1;
}

/* logica para cerrar el dia actual en la tienda */
int logicaCerrarDia(struct Tienda *ibcomics) {
    int i;
    if (ibcomics == NULL) return 0;
    if (ibcomics->diaAbierto == 2 || ibcomics->diaAbierto == 0) return -1;
    /*quitar a todos los clientes de todas las salas*/
    for (i = 0; i < ibcomics->cantidadSalas; i++) {
        sacarTodosClientesSala(ibcomics,ibcomics->raiz, ibcomics->salas[i]);
    }

    ibcomics->diaAbierto = 2;
    return 1;
}

/* interfaz de usuario para pedir fecha y abrir el dia */
void abrirDia(struct Tienda *ibcomics) {
    int dia, mes, anio, resultado;

    if (ibcomics == NULL) return;

    if (ibcomics->diaAbierto == 1) {
        printf("\nError: El dia ya se encuentra abierto\n");
        return;
    }

    printf("\n ============ ABRIR DIA =============\n");
    printf("Ingrese el dia (1-31): ");
    dia = inputEntero();
    while (dia < 1 || dia > 31) {
        printf("Error: Ingrese nuevamente (1-31): ");
        dia = inputEntero();
    }

    printf("Ingrese el mes (1-12): ");
    mes = inputEntero();
    while (mes < 1 || mes > 12) {
        printf("Error: Ingrese nuevamente (1-12): ");
        mes = inputEntero();
    }

    printf("Ingrese el anio: ");
    anio = inputEntero();
    while (anio < 2026 || anio > 2100) {
        printf("Error: Ingrese nuevamente: ");
        anio = inputEntero();
    }

    resultado = logicaAbrirDia(ibcomics, dia, mes, anio);

    if (resultado == 1) {
        printf("\nEl dia %02d/%02d/%d ha sido abierto con exito\n",
               ibcomics->arregloFechaAct[0],
               ibcomics->arregloFechaAct[1],
               ibcomics->arregloFechaAct[2]);
    } else {
        printf("\nError al intentar abrir el dia\n");
    }
}

/* interfaz de usuario para cerrar el dia */
void cerrarDia(struct Tienda *ibcomics) {
    int resultado;

    if (ibcomics == NULL) return;
    if (ibcomics->diaAbierto == 2) {
        printf("\nError: El dia ya se encuentra cerrado\n");
        return;
    }

    printf("\n ============ CERRAR DIA =============\n");
    printf("Cerrando operaciones del dia: %02d/%02d/%d\n",
           ibcomics->arregloFechaAct[0],
           ibcomics->arregloFechaAct[1],
           ibcomics->arregloFechaAct[2]);

    resultado = logicaCerrarDia(ibcomics);

    if (resultado == 1) {
        printf("\nEl dia ha sido cerrado correctamente\n");
    } else {
        printf("\nError: El dia no pudo cerrarse\n");
    }
}
void mostrarMenuPrincipal(struct Tienda *ibcomics, int preciosPases[4], int tamSalas) {
    int opcionSeleccionada;

    do {
        printf("\n==========================================================\n");
        printf("         B I E N V E N I D O   A L   M E N U\n");
        printf("                 I B C - OMICS INC.\n");
        printf("============================================================\n");
        printf("      Opcion [1]: Ingresar al Menu Cliente \n");
        printf("      Opcion [2]: Ingresar al Menu Salas \n");
        printf("      Opcion [3]: Ingresar al Menu Titulos \n");
        printf("      Opcion [4]: Ingresar al Menu Lista de espera \n");
        printf("      Opcion [5]: Ingresar al Menu Pases \n");
        printf("      Opcion [6]: Cambiar precios de los pases \n");
        printf("      Opcion [7]: Salir al menu principal (Jornada)\n");
        printf("   Ingrese una opcion valida: ");
        opcionSeleccionada = inputEntero();

        switch (opcionSeleccionada) {
            case 1:
                mostrarMenuClientes(ibcomics);
                break;
            case 2:
                mostrarMenuSalas(ibcomics, tamSalas);
                break;
            case 3:
                mostrarMenuTitulos(ibcomics);
                break;
            case 4:
                imprimirMenuListaEspera(ibcomics);
                break;
            case 5:
                menuPases(ibcomics, preciosPases);
                break;
            case 6:
                cambiarPrecios(preciosPases);
                break;
            case 7:
                printf("\nSaliendo del menu de la tienda...\n");
                break;
            default:
                printf("\nOpcion no valida. Intente nuevamente :)\n");
                break;
        }
    } while (opcionSeleccionada != 7);
}

int main(void) {
    int opcionPrincipal;
    int preciosPases[4];
    int tamSalas = 12;
    struct Tienda *ibcomics;

    srand(time(NULL));
    poblarPrecios(preciosPases);

    ibcomics = (struct Tienda *)malloc(sizeof(struct Tienda));
    if (ibcomics != NULL) {
        ibcomics->nombreTienda = "IBC - OMICS INC.";
        ibcomics->raiz = NULL;
        ibcomics->salas = crearArregloSalas(tamSalas);
        ibcomics->cantidadSalas = 0;
        ibcomics->diaAbierto = 0; // 0: No iniciado, 1: Abierto, 2: Cerrado
        ibcomics->totalClientes = 0;
    }

    do {
        printf("\n==========================================================\n");
        printf("       SISTEMA PRINCIPAL - IBC - OMICS INC.\n");
        printf("==========================================================\n");
        printf("   >>> Clientes dentro de la tienda: %d <<<\n\n", ibcomics->totalClientes);
        printf("   [1] Abrir el dia\n");
        printf("   [2] Ingresar cliente a sala\n");
        printf("   [3] Quitar cliente de sala\n");
        printf("   [4] Quitar a TODOS los clientes de sala\n");
        printf("   [5] Abrir Menu Principal de la Tienda\n");
        printf("   [6] Cerrar el dia\n");
        printf("   [7] Salir del sistema\n");
        printf("   Ingrese una opcion: ");
        opcionPrincipal = inputEntero();

        switch (opcionPrincipal) {
            case 1:
                abrirDia(ibcomics);
                break;
            case 2:
                if (ibcomics->diaAbierto == 1) {
                    menusIngresarClienteSala(ibcomics);
                } else {
                    printf("\nError: El dia debe estar ABIERTO para ingresar clientes a las salas.\n");
                }
                break;
            case 3:
                menuSacarClienteSala(ibcomics);
                break;
            case 4:
                menuSacarTodosClientesSala(ibcomics);
                break;
            case 5:
                if (ibcomics->diaAbierto == 1) {
                    mostrarMenuPrincipal(ibcomics, preciosPases, tamSalas);
                } else {
                    printf("\nError: Debe abrir el dia primero para acceder al menu principal.\n");
                }

            case 6:
                cerrarDia(ibcomics);
                break;

            case 7:
                printf("\nSaliendo del sistema. Que tenga un excelente dia :)\n");
                break;

            default:
                printf("\nOpcion no valida. Intente nuevamente :)\n");
                break;
        }
    } while (opcionPrincipal != 7);

    free(ibcomics->salas);
    free(ibcomics);
    return 0;
}
