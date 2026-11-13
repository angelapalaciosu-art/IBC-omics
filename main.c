#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_SALAS 6

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

/* =========================================================================
   ESTRUCTURAS GLOBALES DEL PROYECTO
   ========================================================================= */
struct Pase {
    int idPase;
    int tipoPase; /* 1:General / 2:Infantil / 3:Familiar / 4:Prioritario  */
    int valor;
    int estado; /* 1:Activo / 2:Utilizado / 3:Anulado / 4:Vencido */
    char *fechaUso;
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
    char *horario;
    int aforoMax;
    int cantTrabajadores;
    struct NodoTitulo *headNodoTitulo;
};

struct Tienda {
    char *nombreTienda;
    int cantidadSalas;
    struct Sala **salas;
    struct NodoCliente *raiz;
};

/* =========================================================================
   UTILIDADES
   ========================================================================= */
int inputEntero(void) {
    int numero, caracterLeido;
    while (scanf("%d", &numero) != 1) {
        printf("ERROR: Ingrese un numero valido: ");
        while ((caracterLeido = getchar()) != '\n' && caracterLeido != EOF);
    }
    while ((caracterLeido = getchar()) != '\n' && caracterLeido != EOF);
    return numero;
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

/* =========================================================================
   MODULO DE CLIENTES Y ARBOL (Escarlet)
   ========================================================================= */

/* Busca un cliente por ID recorriendo el ABB. */
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

/* Busca un cliente por RUT. */
struct Cliente *buscarClientePorRut(struct NodoCliente *raiz, char *rut) {
    struct Cliente *clienteBuscar;
    if (raiz == NULL) {
        return NULL;
    }
    if (strcmp(raiz->datosCliente->rutCliente, rut) == 0) {
        return raiz->datosCliente;
    }
    /* El arbol esta ordenado por ID, no por RUT: hay que revisar ambos lados */
    clienteBuscar = buscarClientePorRut(raiz->izq, rut);
    if (clienteBuscar != NULL) {
        return clienteBuscar;
    }
    return buscarClientePorRut(raiz->der, rut);
}

/* Genera un ID aleatorio que no este usado por ningun cliente del arbol. */
int crearIdNuevoCliente(struct NodoCliente *raiz) {
    int idNuevo = 0, esUnico = 0;
    while (esUnico != 1) {
        idNuevo = (rand() % 10000) + (rand() % 10000) + 1;
        if (buscarClientePorID(raiz, idNuevo) == NULL) {
            esUnico = 1;
        }
    }
    return idNuevo;
}

/* Pide por consola los datos de un cliente nuevo y lo construye (no lo inserta en el arbol).*/
struct Cliente *crearCliente(struct NodoCliente *raiz) {
    int idNuevo;
    struct Cliente *clienteNuevo;

    idNuevo = crearIdNuevoCliente(raiz);
    clienteNuevo = malloc(sizeof(struct Cliente));
    if (clienteNuevo == NULL) {
        return NULL;
    }
    clienteNuevo->idCliente = idNuevo;
    clienteNuevo->nombreCliente = malloc(sizeof(char) * 50);
    clienteNuevo->correo = malloc(sizeof(char) * 50);

    printf(" ============ MENU REGISTRAR CLIENTE ============\n");
    printf("\n");

    printf("Ingrese el nombre completo del cliente = ");
    fgets(clienteNuevo->nombreCliente, 50, stdin);
    clienteNuevo->nombreCliente[strcspn(clienteNuevo->nombreCliente, "\n")] = '\0'; /* quita el \n de fgets */

    printf("Ingrese el rut del cliente (Ej: 12.345.678-9 ) = ");
    fgets(clienteNuevo->rutCliente, 13, stdin);
    clienteNuevo->rutCliente[strcspn(clienteNuevo->rutCliente, "\n")] = '\0';

    /* RUT repetido: se libera lo reservado y se cancela la creacion */
    if (buscarClientePorRut(raiz, clienteNuevo->rutCliente) != NULL) {
        printf("Error: Ya existe un cliente con ese RUT.\n");
        free(clienteNuevo->nombreCliente);
        free(clienteNuevo->correo);
        free(clienteNuevo);
        return NULL;
    }

    printf("Ingrese la edad del cliente = ");
    clienteNuevo->edad = inputEntero();
    while (clienteNuevo->edad < 1 || clienteNuevo->edad > 120) {
        printf("Error: Ingrese una edad valida (1-120) = ");
        clienteNuevo->edad = inputEntero();
    }

    printf("Ingrese el correo del cliente = ");
    fgets(clienteNuevo->correo, 50, stdin);
    clienteNuevo->correo[strcspn(clienteNuevo->correo, "\n")] = '\0';

    /* Sin sala asignada y con lista de pases vacia (solo nodo cabecera) */
    clienteNuevo->datosSala = NULL;
    clienteNuevo->headPase = malloc(sizeof(struct NodoPase));
    clienteNuevo->headPase->datosPase = NULL;
    clienteNuevo->headPase->sig = NULL;

    return clienteNuevo;
}

/* Busca el nodo que sera el PADRE de un cliente nuevo dentro del ABB.*/
struct NodoCliente *buscarNodoParaClienteNuevo(struct NodoCliente *raiz, int idClienteNuevo) {
    struct NodoCliente *rec;
    if (raiz != NULL) {
        rec = raiz;
        while (rec != NULL) {
            if (rec->datosCliente->idCliente < idClienteNuevo) {
                if (rec->der == NULL) {
                    return rec;
                }
                rec = rec->der;
            } else {
                if (rec->izq == NULL) {
                    return rec;
                }
                rec = rec->izq;
            }
        }
    }
    return NULL;
}

/* Crea un cliente nuevo y lo inserta en el ABB. */
void crearYAgregarClienteAArbol(struct NodoCliente **raiz) {
    struct NodoCliente *nodoNuevo, *nodoAnterior;
    struct Cliente *clienteNuevo;

    clienteNuevo = crearCliente(*raiz);
    if (clienteNuevo == NULL) {
        return;
    }

    nodoNuevo = malloc(sizeof(struct NodoCliente));
    nodoNuevo->datosCliente = clienteNuevo;
    nodoNuevo->izq = NULL;
    nodoNuevo->der = NULL;

    /* Se engancha al padre (derecha si su ID es mayor, izquierda si no) */
    nodoAnterior = buscarNodoParaClienteNuevo(*raiz, clienteNuevo->idCliente);
    if (nodoAnterior == NULL) {
        *raiz = nodoNuevo;
    } else if (nodoAnterior->datosCliente->idCliente < clienteNuevo->idCliente) {
        nodoAnterior->der = nodoNuevo;
    } else {
        nodoAnterior->izq = nodoNuevo;
    }
    printf("\nSe ha anadido con exito al cliente! \n");
    printf("Nombre del cliente: %s\n", clienteNuevo->nombreCliente);
    printf("Rut del cliente: %s\n", clienteNuevo->rutCliente);
    printf("ID asignado automaticamente: %d\n\n", clienteNuevo->idCliente);
}

/* Libera toda la memoria de un cliente: pases, nombre, correo y la estructura misma. */
void liberarCliente(struct Cliente *cliente) {
    struct NodoPase *rec = cliente->headPase->sig, *temp; /* empieza despues head */
    while (rec != NULL) {
        temp = rec;
        rec = rec->sig;
        if (temp->datosPase != NULL) {
            free(temp->datosPase->fechaUso);
            free(temp->datosPase);
        }
        free(temp);
    }
    free(cliente->headPase);
    free(cliente->nombreCliente);
    free(cliente->correo);
    free(cliente);
}

/* Quita al cliente de las listas de espera de todos los titulos de todas las salas. */
int quitarClienteDeListasEspera(struct Tienda *tienda, int idCliente) {
    int i, quitados = 0;
    struct NodoTitulo *rec;

    if (tienda == NULL || tienda->salas == NULL) {
        return 0;
    }
    for (i = 0; i < tienda->cantidadSalas; i++) {
        if (tienda->salas[i] != NULL && tienda->salas[i]->headNodoTitulo != NULL) {
            rec = tienda->salas[i]->headNodoTitulo->sig; /* salta la cabecera */
            while (rec != NULL) {
                if (rec->datosTitulo != NULL &&
                    eliminarClienteDeListaEspera(&(rec->datosTitulo->headListaEspera), idCliente) == 1) {
                    quitados++;
                }
                rec = rec->sig;
            }
        }
    }
    return quitados;
}

/* Cuenta los pases de un cliente (los libera liberarCliente al eliminarlo).*/
int contarPasesDeCliente(struct Cliente *cliente) {
    int cantidad = 0;
    struct NodoPase *rec;

    if (cliente == NULL || cliente->headPase == NULL) {
        return 0;
    }
    rec = cliente->headPase->sig;
    while (rec != NULL) {
        if (rec->datosPase != NULL) {
            cantidad++;
        }
        rec = rec->sig;
    }
    return cantidad;
}

/* Elimina al cliente del sistema completo: lo quita de las listas de espera de todos los titulos,
   libera sus pases (liberarCliente) y finalmente lo saca del arbol.*/
void eliminarClienteDeArbol(struct Tienda *tienda, int idEliminar) {
    struct NodoCliente **raiz;
    struct NodoCliente *nodoEliminar, *nodoAnterior = NULL, *nodoHijo;
    struct NodoCliente *nodoSucesor, *anteriorSucesor;
    int listasQuitadas, pasesEliminados;

    printf(" ============ MENU ELIMINAR CLIENTE ============\n");
    printf("\n");
    if (tienda == NULL || tienda->raiz == NULL) {
        printf("Error: No existe ningun cliente en el sistema.\n");
        return;
    }
    raiz = &(tienda->raiz);

    /* Busca el nodo a eliminar y guarda a su padre en nodoAnterior */
    nodoEliminar = *raiz;
    while (nodoEliminar != NULL && nodoEliminar->datosCliente->idCliente != idEliminar) {
        nodoAnterior = nodoEliminar;
        if (idEliminar < nodoEliminar->datosCliente->idCliente) {
            nodoEliminar = nodoEliminar->izq;
        } else {
            nodoEliminar = nodoEliminar->der;
        }
    }
    if (nodoEliminar == NULL) {
        printf("Error: No se pudo encontrar al cliente con el ID %d.\n", idEliminar);
        return;
    }

    /* Limpieza en el resto del sistema ANTES de liberar al cliente */
    listasQuitadas = quitarClienteDeListasEspera(tienda, idEliminar);
    pasesEliminados = contarPasesDeCliente(nodoEliminar->datosCliente); /* liberarCliente libera los pases */

    if (nodoEliminar->izq != NULL && nodoEliminar->der != NULL) {
        /* Dos hijos: se reemplaza por el sucesor (el menor del subarbol derecho) */
        anteriorSucesor = nodoEliminar;
        nodoSucesor = nodoEliminar->der;
        while (nodoSucesor->izq != NULL) {
            anteriorSucesor = nodoSucesor;
            nodoSucesor = nodoSucesor->izq;
        }
        liberarCliente(nodoEliminar->datosCliente);
        nodoEliminar->datosCliente = nodoSucesor->datosCliente;
        /* Se desconecta el sucesor, conservando su hijo derecho */
        if (anteriorSucesor == nodoEliminar) {
            anteriorSucesor->der = nodoSucesor->der;
        } else {
            anteriorSucesor->izq = nodoSucesor->der;
        }
        free(nodoSucesor);
    } else {
        if (nodoEliminar->izq != NULL) {
            nodoHijo = nodoEliminar->izq;
        } else {
            nodoHijo = nodoEliminar->der;
        }
        if (nodoAnterior == NULL) {
            *raiz = nodoHijo; /* se elimino la raiz */
        } else if (nodoAnterior->izq == nodoEliminar) {
            nodoAnterior->izq = nodoHijo;
        } else {
            nodoAnterior->der = nodoHijo;
        }
        liberarCliente(nodoEliminar->datosCliente);
        free(nodoEliminar);
    }
    printf("Cliente eliminado del sistema con exito! \n");
    printf("Pases eliminados: %d | Listas de espera de las que fue removido: %d\n", pasesEliminados, listasQuitadas);
}
/*Libera todo el arbol y los clientes que contiene (primero hijos, luego el nodo)*/
void liberarArbol(struct NodoCliente *raiz) {
    if (raiz == NULL) {
        return;
    }
    liberarArbol(raiz->izq);
    liberarArbol(raiz->der);
    liberarCliente(raiz->datosCliente);
    free(raiz);
}

/* Muestra por pantalla todos los datos de un cliente. */
void mostrarDatosCliente(struct Cliente *cliente) {
    printf("\n Cliente encontrado con exito!\n");
    printf(" ============ DATOS DEL CLIENTE ============\n");
    printf("\n");
    printf("ID: %d \n", cliente->idCliente);
    printf("Nombre: %s \n", cliente->nombreCliente);
    printf("RUT: %s \n", cliente->rutCliente);
    printf("Edad: %d anios \n", cliente->edad);
    printf("Correo: %s \n\n", cliente->correo);
}

/* Lista todos los clientes ordenados por ID (recorrido inorden). */
void listarClientes(struct NodoCliente *raiz) {
    if (raiz == NULL) {
        return;
    }
    listarClientes(raiz->izq);
    printf("   [%d] %s | RUT: %s | %d anios \n", raiz->datosCliente->idCliente,
           raiz->datosCliente->nombreCliente, raiz->datosCliente->rutCliente, raiz->datosCliente->edad);
    listarClientes(raiz->der);
}

/* Submenu para modificar el nombre, la edad o el correo de un cliente.*/
void menuModificarCliente(struct Cliente *clienteModificar) {
    int opcionModificar;
    if (clienteModificar == NULL) {
        printf("\nError: No existe el cliente que se quiere modificar.\n");
        return;
    }

    do {
        printf("\n");
        printf(" ============ MENU MODIFICAR CLIENTE ============\n");
        printf("\n");
        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        printf("\n");
        printf("      Opcion [1]: Modificar nombre \n");
        printf("      Opcion [2]: Modificar edad \n");
        printf("      Opcion [3]: Modificar correo \n");
        printf("\n");
        printf("      Opcion [0]: Salir de Menu \n");
        printf("\n");
        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        opcionModificar = inputEntero();
        printf("\n");
        switch (opcionModificar) {
            case 1:
                printf("Ha seleccionado la opcion 1: Modificar nombre \n");
                printf("Nombre registrado: %s \n", clienteModificar->nombreCliente);
                printf("Ingrese el nuevo nombre = ");
                fgets(clienteModificar->nombreCliente, 50, stdin);
                clienteModificar->nombreCliente[strcspn(clienteModificar->nombreCliente, "\n")] = '\0';
                printf("Nombre modificado con exito! Nuevo nombre: %s \n\n", clienteModificar->nombreCliente);
                break;
            case 2:
                printf("Ha seleccionado la opcion 2: Modificar edad \n");
                printf("Edad registrada: %d \n", clienteModificar->edad);
                printf("Ingrese la nueva edad = ");
                clienteModificar->edad = inputEntero();
                while (clienteModificar->edad < 1 || clienteModificar->edad > 120) {
                    printf("Error: Ingrese una edad valida (1-120) = ");
                    clienteModificar->edad = inputEntero();
                }
                printf("Edad modificada con exito! Nueva edad: %d \n\n", clienteModificar->edad);
                break;
            case 3:
                printf("Ha seleccionado la opcion 3: Modificar correo \n");
                printf("Correo registrado: %s \n", clienteModificar->correo);
                printf("Ingrese el nuevo correo = ");
                fgets(clienteModificar->correo, 50, stdin);
                clienteModificar->correo[strcspn(clienteModificar->correo, "\n")] = '\0';
                printf("Correo modificado con exito! Nuevo correo: %s \n\n", clienteModificar->correo);
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

/* Menu principal de clientes: registrar, modificar, eliminar, buscar (por ID o RUT) y listar.*/
void mostrarMenuClientes(struct Tienda *tienda) {
    int opcionMenu, opcionValida, idOperar;
    char rut[13];
    struct Cliente *clienteOperar = NULL;
    struct NodoCliente **raiz = &(tienda->raiz); /* doble puntero: la raiz puede cambiar al insertar/eliminar */

    do {
        opcionValida = 1; /* pasa a 0 solo si la opcion ingresada no existe */
        printf("\n");
        printf("  =========================================================\n");
        printf("  ||                                                     ||\n");
        printf("  ||         B I E N V E N I D O   A L   M E N U         ||\n");
        printf("  ||                                                     ||\n");
        printf("  ||                   C L I E N T E S                   ||\n");
        printf("  ||                                                     ||\n");
        printf("  =========================================================\n");
        printf("\n");

        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        printf("\n");
        printf("      Opcion [1]: Registrar nuevo Cliente en el sistema \n");
        printf("      Opcion [2]: Modificar datos de un Cliente \n");
        printf("      Opcion [3]: Eliminar Cliente del sistema \n");
        printf("      Opcion [4]: Buscar Cliente por ID \n");
        printf("      Opcion [5]: Buscar Cliente por RUT \n");
        printf("      Opcion [6]: Listar todos los Clientes \n");
        printf("\n");
        printf("      Opcion [0]: Salir de Menu \n");
        printf("\n");

        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        opcionMenu = inputEntero();
        printf("\n");
        switch (opcionMenu) {
            case 1:
                printf("Ha seleccionado la opcion 1: Registrar nuevo cliente en el sistema \n");
                crearYAgregarClienteAArbol(raiz);
                break;
            case 2:
                printf("Ha seleccionado la opcion 2: Modificar datos de un cliente \n");
                printf(" ============ MENU BUSCAR CLIENTE POR ID ============\n");
                printf("Ingrese el id del cliente = ");
                idOperar = inputEntero();
                clienteOperar = buscarClientePorID(*raiz, idOperar);
                if (clienteOperar != NULL) {
                    printf("Cliente encontrado con exito!\n");
                    printf("Nombre: %s | RUT: %s\n", clienteOperar->nombreCliente, clienteOperar->rutCliente);
                    menuModificarCliente(clienteOperar);
                } else {
                    printf("Error: No se pudo encontrar al cliente con el ID %d.\n\n", idOperar);
                }
                break;
            case 3:
                printf("Ha seleccionado la opcion 3: Eliminar cliente del sistema \n");
                printf("Ingrese el id del cliente = ");
                idOperar = inputEntero();
                eliminarClienteDeArbol(tienda, idOperar); /* recibe tienda para limpiar listas de espera */
                break;
            case 4:
                printf("Ha seleccionado la opcion 4: Buscar cliente por ID \n");
                printf(" ============ MENU BUSCAR CLIENTE POR ID ============\n");
                printf("Ingrese el id del cliente = ");
                idOperar = inputEntero();
                clienteOperar = buscarClientePorID(*raiz, idOperar);
                if (clienteOperar != NULL) {
                    mostrarDatosCliente(clienteOperar);
                } else {
                    printf("Error: No se pudo encontrar al cliente con el ID %d.\n\n", idOperar);
                }
                break;
            case 5:
                printf("Ha seleccionado la opcion 5: Buscar cliente por RUT \n");
                printf(" ============ MENU BUSCAR CLIENTE POR RUT ============\n");
                printf("Ingrese el rut del cliente (Ej: 12.345.678-9 ) = ");
                fgets(rut, 13, stdin);
                rut[strcspn(rut, "\n")] = '\0';
                clienteOperar = buscarClientePorRut(*raiz, rut);
                if (clienteOperar != NULL) {
                    mostrarDatosCliente(clienteOperar);
                } else {
                    printf("Error: No se encontro al cliente con el RUT ingresado.\n\n");
                }
                break;
            case 6:
                printf("Ha seleccionado la opcion 6: Listar todos los clientes \n");
                printf(" ============ MENU LISTAR CLIENTES ============\n");
                printf("\n");
                if (*raiz == NULL) {
                    printf("Error: No hay clientes registrados en el sistema.\n\n");
                } else {
                    listarClientes(*raiz);
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
   MODULO DE SALAS Y TITULOS (Alejandro)
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
struct Sala *agregarSala(struct Tienda *tienda, struct Sala *sala) {
    if (tienda == NULL || sala == NULL || tienda->salas == NULL) {
        return NULL;
    }
    if (tienda->cantidadSalas >= MAX_SALAS) {
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
    strcpy(nuevaSala->nombreSala, nombreSala);

    nuevaSala->horario = (char *) malloc(strlen(horario) + 1);
    strcpy(nuevaSala->horario, horario);

    nuevaSala->headNodoTitulo = crearNodoTitulo(NULL);
    nuevaSala->idSala = idSala;
    nuevaSala->tematica = tematica;
    nuevaSala->aforoMax = aforoMax;
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
                printf("Clasificacion modificada con exito! Nueva clasificacion: %d \n\n", tituloModificar->clasificacionEdad);
                break;
            case 4:
                printf("Ha seleccionado la opcion 4: Modificar precio \n");
                printf("Precio registrado: %d \n", tituloModificar->precio);
                printf("Ingrese el nuevo precio = ");
                tituloModificar->precio = inputEntero();
                printf("Precio modificado con exito! Nuevo precio: %d \n\n", tituloModificar->precio);
                break;;
            case 5:
                printf("Ha seleccionado la opcion 5: Modificar cantidad de ejemplares \n");
                printf("Ejemplares registrados: %d \n", tituloModificar->cantidadEjemplares);
                printf("Ingrese la nueva cantidad = ");
                tituloModificar->cantidadEjemplares = inputEntero();
                printf("Cantidad modificada con exito! Nueva cantidad de ejemplares: %d \n\n", tituloModificar->cantidadEjemplares);
                break;;
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
                printf("Aforo modificado con exito! Nuevo aforo maximo: %d \n\n", salaModificar->aforoMax);
                break;
            case 4:
                printf("Ha seleccionado la opcion 4: Modificar cantidad de trabajadores \n");
                printf("Trabajadores registrados: %d \n", salaModificar->cantTrabajadores);
                printf("Ingrese la nueva cantidad de trabajadores = ");
                salaModificar->cantTrabajadores = inputEntero();
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
    struct NodoListaEspera *recEspera, *siguienteEspera;

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

    desvincularPasesDeSala(tienda->raiz, salaEliminar);

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
            if (rec->datosTitulo->headListaEspera != NULL) {
                recEspera = rec->datosTitulo->headListaEspera->sig;
                while (recEspera != rec->datosTitulo->headListaEspera) {
                    siguienteEspera = recEspera->sig;
                    free(recEspera);
                    recEspera = siguienteEspera;
                }
                free(rec->datosTitulo->headListaEspera);
            }
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
int contarPersonasEnSala(struct NodoCliente *raiz, struct Sala *sala) {
    int cantidad = 0, tienePase = 0;
    struct NodoPase *rec;

    if (raiz != NULL && sala != NULL) {
        if (raiz->datosCliente != NULL) {
            rec = raiz->datosCliente->headPase;
            while (rec != NULL && tienePase == 0) {
                if (rec->datosPase != NULL && rec->datosPase->datosSala == sala && rec->datosPase->estado == 1) {
                    tienePase = 1;
                }
                rec = rec->sig;
            }
            cantidad = tienePase;
        }
        cantidad += contarPersonasEnSala(raiz->izq, sala);
        cantidad += contarPersonasEnSala(raiz->der, sala);
    }
    return cantidad;
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
            if (contarPersonasEnSala(tienda->raiz, tienda->salas[j]) < contarPersonasEnSala(tienda->raiz, tienda->salas[j + 1])) {
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

struct Sala *crearYAgregarSala(struct Tienda *tienda) {
    int idSala, tematica, aforoMax, cantTrabajadores;
    char nombreSala[100], horario[100];
    struct Sala *nuevaSala;

    if (tienda == NULL || tienda->salas == NULL) {
        return NULL;
    }
    if (tienda->cantidadSalas >= MAX_SALAS) {
        printf("Error: No se pueden registrar mas salas (maximo %d).\n\n", MAX_SALAS);
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
    return agregarSala(tienda, nuevaSala);
}

void mostrarMenuSalas(struct Tienda *tienda) {
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
                salaOperar = crearYAgregarSala(tienda);
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
                if (tituloEliminar->headListaEspera != NULL) {
                    recEspera = tituloEliminar->headListaEspera->sig;
                    while (recEspera != tituloEliminar->headListaEspera) {
                        siguienteEspera = recEspera->sig;
                        free(recEspera);
                        recEspera = siguienteEspera;
                    }
                    free(tituloEliminar->headListaEspera);
                }
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
struct Cliente *buscarClientePorRutOIdArbol(struct NodoCliente *raiz, char *rutId) {
    int i;

    if (raiz == NULL || rutId == NULL || rutId[0] == '\0') {
        return NULL;
    }
    /*recorre todo el char ingresado y si tiene guion o punto se asume que se ingresa un rut*/
    for (i = 0; rutId[i] != '\0'; i++) {
        if (rutId[i] == '-' || rutId[i] == '.') {
            return buscarClientePorRut(raiz, rutId);  /*si es rut me busca al cliente por rut en el arbol*/
        }
    }
    return buscarClientePorID(raiz, atoi(rutId));
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

    if (head == NULL || *head == NULL || idBuscado < 0) {
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
        printf("La lista se encuentra vacia");
        return;
    }

    rec = head;

    do {
        lugarCliente++;
        printf("[%d] | ID: %d | RUT: %s | NOMBRE: %s | EDAD: %d ", lugarCliente, rec->datosCliente->idCliente, rec->datosCliente->rutCliente, rec->datosCliente->nombreCliente, rec->datosCliente->edad);
        rec = rec -> sig;
    }while (rec != head);
}


/*AGREGAR CLIENTE A LA LISTA DE ESPERA*/
void agregarClienteListaEsperaMenu(struct Tienda *ibcomics) {
    char idRutCliente[15];
    int idTituloBuscado;
    struct Cliente *clienteEnTienda = NULL;
    struct Cliente *clienteEnFila = NULL;
    struct Titulo *tituloEncontrado = NULL;
    struct NodoListaEspera *nuevoNodo = NULL;

    if (ibcomics != NULL) {
        printf(" ============ MENU AGREGAR CLIENTE LISTA DE ESPERA ============\n");
        printf("\n");

        while (tituloEncontrado == NULL) {  /*tituloEncontrado empieza en null, despues si se encuentra no vuelve a este while y sigue (se utiliza para preguntar hasta encontrar)*/
            printf("Bienvenido/da, por favor ingrese el id del titulo que desea esperar : \n");

            idTituloBuscado = inputEntero();

            if (idTituloBuscado == 0) {
                printf("Error: Operacion cancelada . . .");
                return;
            }

            tituloEncontrado = buscarTituloEnSalasPorId(ibcomics, idTituloBuscado);

            if (tituloEncontrado == NULL) {
                printf("Error: No se pudo encontrar el titulo con el ID %d. Intente nuevamente.\n\n", idTituloBuscado);
            }
        }

        printf("Por favor ingrese ID del cliente o RUT (Ej: 12.345.678-9 ) = \n");
        scanf("%14s", idRutCliente);
        while (getchar() != '\n');

        clienteEnTienda = buscarClientePorRutOIdArbol(ibcomics->raiz, idRutCliente);  /*busca al cliente en el arbol*/

        if (clienteEnTienda != NULL) {
            clienteEnFila = buscarClientePorRutOIdListaEspera(tituloEncontrado->headListaEspera, idRutCliente); /*busca al cliente en la lista de espera*/
            /*si retorna null significa que no está en la lista de espera por lo que se puede ingresar*/

            if (clienteEnFila == NULL) {  /*para que un mismo cliente no ingrese dos veces*/

                if (tituloEncontrado->estadoListaEspera == 1){  /*Si la lista de espera está activa*/
                    nuevoNodo = crearNodoListasEspera(clienteEnTienda);

                    if (nuevoNodo != NULL) {
                        enlazarNodoListaEspera(&(tituloEncontrado->headListaEspera), nuevoNodo);

                        printf("Se ha anadido con exito al cliente! \n");
                        printf("Nombre del cliente: %s\n", clienteEnTienda->nombreCliente);
                        printf("Rut del cliente: %s\n", clienteEnTienda->rutCliente);
                        printf("Titulo en la lista de espera : %s\n", tituloEncontrado->nombreTitulo);
                    }
                    else {
                        printf("Error al agregar al cliente.\n");
                    }
                }
                else {
                    printf("Error: La lista de espera esta suspendida. \n");
                }
            }
            else {
                printf("Error: El cliente ya se encuentra dentro de la lista de espera\n");
            }
        } else {
            printf("Error: El cliente no se encuentra registrado en el sistema general de la tienda.\n");
        }
    }
}

void quitarClienteListaEsperaMenu(struct Tienda *ibcomics) {
    int idTituloBuscado;
    char idRutCliente[15];
    struct Titulo *tituloEncontrado = NULL;
    struct Cliente *clienteEncontrado = NULL;
    int resultado;

    if (ibcomics != NULL) {
        printf(" ============ MENU QUITAR CLIENTE LISTA DE ESPERA ============\n");

        while (tituloEncontrado == NULL) {  /*empieza como null, despues en la segunda vuelta ya tiene a un titulo por lo que sale del bucle*/
            printf("Ingrese el id del titulo que desea buscar = ");

            if (scanf("%d", &idTituloBuscado) != 1) {  /*para que solo reconozca int*/
                while (getchar() != '\n');
                printf("Error: Ingrese un ID numerico valido.\n\n");
                continue;
            }

            tituloEncontrado = buscarTituloEnSalasPorId(ibcomics, idTituloBuscado);

            if (tituloEncontrado == NULL) {
                printf("Error: No se pudo encontrar el titulo con el ID %d. Intente nuevamente.\n\n", idTituloBuscado);
            }
        }
        /*si se sale del bucle significa q lo encontro*/

        printf("Titulo encontrado con exito! \n");
        printf("Nombre del titulo: %s\n\n", tituloEncontrado->nombreTitulo);

        printf("Ingrese el id o rut del cliente que desea quitar (Ej : 12.345.678-9 ) = ");
        scanf(" %14s", idRutCliente);

        clienteEncontrado = buscarClientePorRutOIdListaEspera(tituloEncontrado->headListaEspera, idRutCliente);

        if (clienteEncontrado != NULL) {  /*si la funcion encuentra al cliente retorna sus datos*/
            printf("Cliente encontrado con exito!\n");
            printf("Nombre: %s | RUT: %s\n\n", clienteEncontrado->nombreCliente, clienteEncontrado->rutCliente);

            resultado = eliminarClienteDeListaEspera(&(tituloEncontrado->headListaEspera), clienteEncontrado->idCliente);

            if (resultado == 1) {
                printf("Cliente eliminado con exito! \n");
            } else {
                printf("Error: El cliente no pudo ser eliminado.\n");
            }
        } else {
            printf("Error: No se encontro al cliente con los datos ingresados en la lista de espera.\n");
        }
    }
}

void MenuCalcularTiempoEstimadoEsperaDias(struct Tienda *ibcomics) {
    struct Titulo *tituloEncontrado = NULL;
    int idTituloBuscado;
    int cantidadDias;

    if (ibcomics == NULL) {
        return;
    }

    printf(" ============ MENU CALCULAR TIEMPO ESTIMADO DE ESPERA POR TITULO ============\n");
    /*solicita id del titulo hasta encontrar uno existente*/
    while (tituloEncontrado == NULL) {
        printf("Ingrese el ID del titulo que desea buscar = ");

        if (scanf("%d", &idTituloBuscado) != 1) {
            while (getchar() != '\n');
            printf("Error: Ingrese un ID numerico valido.\n\n");
            continue;
        }

        tituloEncontrado = buscarTituloEnSalasPorId(ibcomics, idTituloBuscado);

        if (tituloEncontrado == NULL) {
            printf("Error: No se pudo encontrar el titulo con el ID %d. Intente nuevamente.\n\n", idTituloBuscado);
        }
    }

    /*si se sale del bucle es pq encontro el titulo*/

    printf("Titulo encontrado con exito!\n");
    printf("Nombre del titulo buscado: %s\n", tituloEncontrado->nombreTitulo);
    printf("Cantidad de ejemplares por despacho: %d\n", tituloEncontrado->ejemplaresPorDespacho);
    printf("Dias de demora por despacho: %d\n\n", tituloEncontrado->diasDemoraDespacho);
    printf("\n");

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

    if (ibcomics != NULL) {
        printf(" ============ MENU CONTAR CANTIDAD DE CLIENTES EN LISTA DE ESPERA ============\n");

        while (tituloEncontrado == NULL) {   /*pregunta hasta encontrar un titulo, en la primera vuelta siempre es null*/
            printf("Ingrese el id del titulo que desea buscar: \n");

            if (scanf("%d", &idTituloBuscado) != 1) {
                while (getchar() != '\n');
                printf("Error: Ingrese un ID numerico valido.\n\n");
                continue;
            }
            tituloEncontrado = buscarTituloEnSalasPorId(ibcomics, idTituloBuscado);

            if (tituloEncontrado == NULL) {
                printf("Error: No se pudo encontrar el titulo con el ID %d. Intente nuevamente.\n\n", idTituloBuscado);
            }
        }

        printf("Nombre del titulo buscado: %s\n", tituloEncontrado->nombreTitulo);
        cantidadClientes = contarCantidadClientesEnListaEspera(tituloEncontrado->headListaEspera);

        if (cantidadClientes == 1) {
            printf("\n");
            printf("Cantidad de personas en la lista de espera de %s es de : %d persona\n", tituloEncontrado->nombreTitulo, cantidadClientes);
        }
        else {
            printf("\n");
            printf("Cantidad de personas en la lista de espera de %s es de : %d personas\n", tituloEncontrado->nombreTitulo, cantidadClientes);
        }
    }
}

void mostrarTituloMayorListaEsperaEnSalaMenu(struct Tienda *ibcomics) {
    struct Titulo *tituloEncontrado = NULL;
    int opcionSeleccionada = 0;
    int i;
    int indiceSala;

    if (ibcomics != NULL) {
        printf(" ============ MENU MOSTRAR TITULO CON MAYOR LISTA DE ESPERA POR SALA ============\n");
        printf("\n");

        while (opcionSeleccionada < 1 || opcionSeleccionada > 6) {
            printf(" Ingrese la sala que desea buscar: \n");
            printf(" Opciones disponibles : \n");
            printf(" === [1] SuperHeroe / [2] Manga y Anime / [3] Novela Grafica  ===\n");
            printf(" === [4] Comic Independiente / [5] Infantil y Humor Grafico / [6] Coleccionista ===\n");

            if (scanf(" %d", &opcionSeleccionada) != 1) { /*para que solo tome int*/
                while (getchar() != '\n');
                printf("Error: Ingrese una opcion numerica valida.\n\n");
                continue;
            }

            if (opcionSeleccionada < 1 || opcionSeleccionada > 6) {
                printf("Error: Opcion fuera de rango (1-6). Intente nuevamente.\n\n");
            }
        }

        indiceSala = -1;
        for (i = 0; i < ibcomics->cantidadSalas; i++) {
            if (ibcomics->salas[i] != NULL && ibcomics->salas[i]->tematica == opcionSeleccionada) {
                indiceSala = i;
                break;
            }
        }

        if (indiceSala != -1) {
            tituloEncontrado = obtenerTituloMayorListaEspera(ibcomics->salas[indiceSala]->headNodoTitulo);  /*ira a la sala correspondinte y hace el calculo*/

            if (tituloEncontrado != NULL) {
                printf("\n =====> El titulo con mayor lista de espera es: '%s' <=====\n", tituloEncontrado->nombreTitulo);
            } else {
                printf("\n No hay titulos con lista de espera en esta sala.\n");
            }
        } else {
            printf("\n Error: No se encontro ninguna sala registrada con la tematica seleccionada.\n");
        }
    }
}


void menuListarClientesListaEspera(struct Tienda *ibcomics) {
    struct Titulo *tituloEncontrado;
    int idTitulo;

    if (ibcomics != NULL) {
        printf(" ============ MENU LISTAR LISTA DE ESPERA ============\n");
        printf("\n");

        while (tituloEncontrado == NULL) {
            printf("Ingrese el ID del titulo (escriba 0 para cancelar): \n");
            idTitulo = inputEntero();

            if (idTitulo == 0) {
                printf("La operacion se ha cancelado . . .. \n");
                return;
            }

            tituloEncontrado = buscarTituloEnSalasPorId(ibcomics, idTitulo);

            if (tituloEncontrado == NULL) {
                printf("No existe titulo con ese ID, Intente nuevamente \n");
            }
        }

        printf("El titulo ha sido encontrado! \n");
        printf("Nombre: %s", tituloEncontrado->nombreTitulo);
        if (tituloEncontrado->estadoListaEspera == 1) {
            printf("Estado de la lista de espera: Activa \n");
        }
        else {
            printf("Estado de la lista de espera: Suspendida \n");
        }
        printf("\n");
        listarClientesListaEspera(tituloEncontrado->headListaEspera);
    }
}

void vaciarListaEsperaMenu(struct Tienda *ibcomics) {
    struct Titulo *tituloEncontrado = NULL;
    int idTituloBuscado;
    int resultado;

    if (ibcomics != NULL) {
        printf(" ============ MENU VACIAR LISTA DE ESPERA ============\n");
        printf("\n");

        /*bucle pedir el id del titulo hasta encontrar uno existente*/
        while (tituloEncontrado == NULL) {
            printf("Ingrese ID del titulo que desea buscar: \n");

            if (scanf("%d", &idTituloBuscado) != 1) {
                while (getchar() != '\n');
                printf("Error: Ingrese un ID numerico valido.\n\n");
                continue;
            }

            tituloEncontrado = buscarTituloEnSalasPorId(ibcomics, idTituloBuscado);

            if (tituloEncontrado == NULL) {
                printf("Error: No se pudo encontrar el titulo con el ID %d. Intente nuevamente.\n\n", idTituloBuscado);
            }
        }

        printf("Nombre del titulo encontrado : %s \n", tituloEncontrado->nombreTitulo);

        if (tituloEncontrado->headListaEspera != NULL) {
            resultado = vaciarListaEspera(&(tituloEncontrado->headListaEspera));

            if (resultado == 1) {
                printf("La Lista de espera del titulo %s ha sido vaciada con exito !! \n", tituloEncontrado->nombreTitulo);
            } else {
                printf("Error: La Lista de espera no pudo vaciarse \n");
            }
        } else {
            printf("La lista de espera para este titulo ya se encuentra vacia.\n");
        }
    }
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

        if (scanf("%d", &opcionSeleccionada) != 1) {
            while (getchar() != '\n');

            opcionSeleccionada = -1;
        }

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
    pase->fechaUso = NULL;
    pase->datosSala = NULL;
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
struct Pase *crearPaseCompleto(struct Tienda *tienda, int tipoPase, int valorPase, const char *fecha, struct Sala *sala) {
    struct Pase *nuevo = crearPase();
    if (nuevo == NULL) {
        return NULL;
    }
    nuevo->idPase = crearIdPase(tienda->raiz);
    nuevo->valor = valorPase;
    nuevo->estado = 1;
    nuevo->tipoPase = tipoPase;
    nuevo->fechaUso = copiarTexto(fecha);  /*para reservar memoria*/
    nuevo->datosSala = sala;
    return nuevo;
}
/*resolver que el puntero datoSala no queden apuntando a una sala ya eliminada*/
void desvincularPasesDeSala(struct NodoCliente *raiz, struct Sala *sala) {
    struct NodoPase *rec;

    if (raiz == NULL) {
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

void comprarPases(struct Tienda *tienda, int precios[]) {
    int tipoPase, valorPase = 0, idClienteCompra, salaSeleccionada, i, j, esIdValido,indice,idPaseFamiliar;
    char fechaUsoPase[20];
    int idClienteFamiliar[4];
    struct Cliente *clienteComprador;
    struct Cliente *clienteFamiliar;
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
        tipoPase = inputEntero();
    } while (tipoPase < 1 || tipoPase > 4);

    switch (tipoPase) {
        case 1:
            valorPase = precios[0];
            break;
        case 2:
            valorPase = precios[1];
            if (clienteComprador->edad > 10 || clienteComprador->edad < 1) {
                printf("Error: El cliente no cumple con las condiciones de edad para este pase.\n");
                return;
            }
            break;
        case 3:
            valorPase = precios[2];
            break;
        case 4:
            valorPase = precios[3];
            break;
        default:
            break;
    }

    do {
        printf("\n");
        printf("   Ingrese la sala que desea visitar : \n");
        printf("\n");
        for (indice = 0 ; indice < tienda->cantidadSalas; indice++) {
            printf("      Opcion [%d]: %s \n", indice + 1, tienda->salas[indice]->nombreSala);
        }
        printf("\n");
        printf("   Por favor, ingrese una de las opciones disponibles : \n");
        salaSeleccionada = inputEntero();
    } while (salaSeleccionada < 1 || salaSeleccionada > tienda->cantidadSalas);


    salaElegida = tienda->salas[salaSeleccionada - 1];

    if (salaElegida == NULL) {
        printf("Error: No se encontro la sala seleccionada.\n");
        return;
    }
    printf("Sala seleccionada con exito!\n");

    printf("Ingrese la fecha de uso del pase = ");
    scanf(" %19s",fechaUsoPase);

    printf("Valor del pase: %d\n", valorPase);


    if (tipoPase != 3) {
        nuevo = crearPaseCompleto(tienda, tipoPase, valorPase, fechaUsoPase, salaElegida);
        if (nuevo == NULL || agregarPaseACliente(clienteComprador, nuevo) == 0) {
            printf("Error: No se pudo crear el pase.\n");
            return;
        }
        printf("Pase creado con exito!\n");
        printf("ID del pase: %d\n", nuevo->idPase);
    } else {
        do {
            esIdValido = 1;
            printf("\n");
            printf("Para salir ingrese 0\n");
            for (i = 0; i < 4; i++) {
                printf("Ingrese el id del cliente integrante %d = ", i + 1);
                idClienteCompra = inputEntero();
                if (idClienteCompra == 0) {
                    return;
                }

                if (buscarClientePorID(tienda->raiz, idClienteCompra) == NULL) {
                    printf("Error: No se pudo encontrar al cliente con el ID %d.\n", idClienteCompra);
                    printf("Por favor, registrelo primero desde el Menu de Clientes.\n");
                    esIdValido = 0;
                    break;
                }
                idClienteFamiliar[i] = idClienteCompra;

                for (j = 0; j < i; j++) {
                    if (idClienteFamiliar[i] == idClienteFamiliar[j]) {
                        printf("Error: El cliente ya fue ingresado, registre un nuevo cliente.\n");
                        esIdValido = 0;
                        break;
                    }
                }
                if (!esIdValido) {
                    break;
                }
            }
        } while (esIdValido == 0);

        idPaseFamiliar = crearIdPase(tienda->raiz);
        /* Cada integrante recibe su propio pase (evita double free al liberar clientes) */
        for (i = 0; i < 4; i++) {
            clienteFamiliar = buscarClientePorID(tienda->raiz, idClienteFamiliar[i]);

            if (clienteFamiliar == NULL) {
                printf("Error: No se pudo encontrar al cliente.\n");
                return;
            }
            nuevo = crearPaseCompleto(tienda, tipoPase, valorPase, fechaUsoPase, salaElegida);
            if (nuevo == NULL) {
                printf("Error: No se pudo crear el pase familiar.\n");
                return;
            }
            nuevo->idPase = idPaseFamiliar;

            if (agregarPaseACliente(clienteFamiliar, nuevo) == 0) {
                free(nuevo->fechaUso);
                free(nuevo);

                printf("Error: No se pudo crear el pase familiar.\n");
                return;
            }
            printf("Pase familiar creado con exito!\n");
            printf("ID del pase: %d\n", nuevo->idPase);
        }
    }
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
        free(nodoEliminar->datosPase->fechaUso);
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
    int nuevoValor, nuevoEstado, nuevaSala;
    char fecha[20];
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
            printf("Ingrese la nueva fecha (ejemplo: 29/08/2026)=");
            scanf(" %19s", fecha);
            if (pase->fechaUso != NULL && strcmp(pase->fechaUso, fecha) == 0) {
                printf("Error: La fecha nueva es igual a la anterior.\n");
            } else {
                /* Se reasigna memoria para que el nuevo texto siempre quepa */
                free(pase->fechaUso);
                pase->fechaUso = copiarTexto(fecha); /*hay que reservar memoria con la funcion, porque la nueva fecha puede tener largo distinto*/
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
        printf("      Opcion [4]: Ver cantidad de Pases comprados \n");
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
                printf("Ha seleccionado la opcion 4: Ver cantidad de pases comprados \n");
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
/* =========================================================================
   FUNCION PRINCIPAL (MAIN)
   ========================================================================= */
int main(void) {
    int opcionSeleccionada;
    int preciosPases[4];
    struct Tienda *ibcomics;

    srand(time(NULL));
    poblarPrecios(preciosPases);

    ibcomics = (struct Tienda *)malloc(sizeof(struct Tienda));
    if (ibcomics != NULL) {
        ibcomics->nombreTienda = "IBC - OMICS INC.";
        ibcomics->raiz = NULL;
        ibcomics->salas = crearArregloSalas(MAX_SALAS);
        ibcomics->cantidadSalas = 0;
    }

    do {
        printf("\n==========================================================\n");
        printf("         B I E N V E N I D O   A L   M E N U\n");
        printf("                  I B C - OMICS INC.\n");
        printf("============================================================\n");
        printf("      Opcion [1]: Ingresar al Menu Cliente \n");
        printf("      Opcion [2]: Ingresar al Menu Salas \n");
        printf("      Opcion [3]: Ingresar al Menu Titulos \n");
        printf("      Opcion [4]: Ingresar al Menu Lista de espera \n");
        printf("      Opcion [5]: Ingresar al Menu Pases \n");
        printf("      Opcion [6]: Cambiar precios de los pases \n");
        printf("      Opcion [7]: Salir del sistema \n");
        printf("   Ingrese una opcion valida: ");
        opcionSeleccionada = inputEntero();

        switch (opcionSeleccionada) {
            case 1:
                mostrarMenuClientes(ibcomics);
                break;
            case 2:
                mostrarMenuSalas(ibcomics);
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
                printf("\nSaliendo del sistema. Que tenga un excelente dia :)\n");
                break;
            default:
                printf("\nOpcion no valida. Intente nuevamente :)\n");
                break;
        }
    } while (opcionSeleccionada != 7);

    liberarArbol(ibcomics->raiz);
    free(ibcomics->salas);
    free(ibcomics);
    return 0;
}
