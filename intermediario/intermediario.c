#include "comun.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MAX 65
#define EVENTO_OK 0
#define EVENTO_ERROR -1

typedef struct entrada_tema{
	int id_topic;
	char tema[64];
} EntradaTema;

typedef struct entrada_sub{
	int id_sub;
	char addr[32];
	int port;
} EntradaSub;

typedef struct num_subs_topic{
	int id_topic;
	int n_subs;
} EntradaNsubsTopic;

typedef struct subs_topic{
	int id_sub;
	int id_topic;
} EntradaSuscripcion;


/* --------------- Variables ---------------- */
int service_port;
int n_subs; // numero de suscriptores totales
int n_topics; // numero de temas totales
int n_suscripciones; // numero de entradas en la lista de suscriptores-tema
EntradaNsubsTopic * n_sub_topic;
EntradaSuscripcion * suscriptores_temas;
EntradaTema * temas;
EntradaSub * suscriptores;

/* --------------- Funciones ---------------- */
/* PRE: El topic existe */
/* Devuelve el numero de suscriptores suscritos al tema */
int getNSubsTopic(int id_topic){
	int numero_subs = -1;
	int i;
	bool found = false;
	for(i=0;i<n_topics && !found;i++){
		if(n_sub_topic[i].id_topic ==  id_topic){
			numero_subs = n_sub_topic[i].n_subs;
			found = true;
		}
	}
	return numero_subs;
}

void printNsubsTopic(){
	int i;
	printf("+-------------------------------+---------------+\n");
	printf("| TopicId \t|  N_subs \t|\n");
	for(i=0; i<n_topics; i++){
		printf("| %d \t\t| %d \t\t|\n", n_sub_topic[i].id_topic, n_sub_topic[i].n_subs);
	}
}

void printSuscripciones(){
	int i;
	printf("+-------------------------------+\n");
	printf("| IdSub \t| IdTopic \t|\n");
	for (i=0; i < n_suscripciones; i++){
		printf("| %d \t\t| %d \t\t|\n", suscriptores_temas[i].id_sub, suscriptores_temas[i].id_topic);
	}
}

void printSubs(){
	int i;
	printf("+-------------------------------+--------------+\n");
	printf("| IdSub \t| AddrSub \t| Port \t\t| \n");
	for(i=0; i<n_subs; i++){
		printf("| %d \t\t| %s \t| %d \t|\n",
		 suscriptores[i].id_sub, suscriptores[i].addr, suscriptores[i].port);
	}
}
void printTemas(){
	int i;
	printf("+-------------------------------+---------------+\n");
	printf("| TopicId \t| Topic \t| N_subs \t|\n");
	for(i=0; i<n_topics; i++){
		printf("| %d \t\t| %s \t| %d \t\t|\n", temas[i].id_topic, temas[i].tema, getNSubsTopic(temas[i].id_topic));
	}
}

bool addNSubTopic(int id_topic){
	int i;
	bool found = false;
	for(i=0;i<n_topics && !found;i++){
		if(n_sub_topic[i].id_topic == id_topic){
			n_sub_topic[i].n_subs = n_sub_topic[i].n_subs+1;
			found = true;
		}
	}
	return found;
}

int delNSubTopic(int id_topic){
	int i;
	bool found = false;
	for(i=0;i<n_topics && !found;i++){
		if(n_sub_topic[i].id_topic == id_topic){
			n_sub_topic[i].n_subs = n_sub_topic[i].n_subs-1;
			found = true;
		}
	}
	return found;
}

/* Devuelve true si el suscriptor id_sub esta suscrito al tema id_topic */
bool isSubbed(int id_sub, int id_topic){
	int i = 0;
	bool suscrito = false;
	for(i=0;i<n_suscripciones && !suscrito;i++){
		if(suscriptores_temas[i].id_sub == id_sub){
			if(suscriptores_temas[i].id_topic == id_topic){
				suscrito = true;
			}
		}
	}
	return suscrito;
}

/* Devuelve el identificador de tema, o -1 si no esta en la lista de temas */
int getTopicId(char *topic){
	bool found = false;
	int topic_id = -1;
	int i;
	for(i=0; i<n_topics && !found; i++){
		if(strcmp(temas[i].tema,topic) == 0){
			found = true;
			topic_id = temas[i].id_topic;
		}
	}
	return topic_id;
}

/* Añade un tema a la lista de temas y devuelve su identificador o -1 si error */
int addTopic(char *topic){
	if((getTopicId(topic)) != -1){
		return -1;
	}
	int id_topic = n_topics;

	EntradaTema new_entrada;
	new_entrada.id_topic = id_topic;
	strcpy(new_entrada.tema,topic);

	temas[n_topics] = new_entrada;
	n_topics++;

	temas = realloc(temas,(n_topics+1)*sizeof(struct entrada_tema));
	if(temas == NULL){
		fprintf(stderr, "No se pudo ubicar la memoria dinamica necesaria\n");
		return -1;
	}

	n_sub_topic = realloc(n_sub_topic,(n_topics+1)*sizeof(struct num_subs_topic));
	if(n_sub_topic == NULL){
		fprintf(stderr, "No se pudo ubicar la memoria dinamica necesaria\n");
		return -1;
	}
	EntradaNsubsTopic new_entrada_nsubs;
	new_entrada_nsubs.id_topic = id_topic;
	new_entrada_nsubs.n_subs = 0;

	n_sub_topic[id_topic]= new_entrada_nsubs;
	return id_topic;
}

/* Devuelve el identificador de suscriptor, o -1 si no esta en la lista de suscriptores */
int getSubId(char * subscriber, int port){
	int i;
	bool found = false;
	int sub_id = -1;
	for(i=0; i<n_subs && !found; i++){
		if(strcmp(suscriptores[i].addr, subscriber)==0){
			if(suscriptores[i].port == port){
				found = true;
				sub_id = suscriptores[i].id_sub;
			}
		}
	}
	return sub_id;
}

int getSubPort(int id_sub){
	int i;
	bool found = false;
	int port=-1;
	for(i=0; i<n_subs && !found; i++){
		if(suscriptores[i].id_sub == id_sub){
			found = true;
			port = suscriptores[i].port;
		}
	}
	return port;
}

char * getSubAddr(int id_sub){
	int i;
	bool found = false;
	char* sub_addr = malloc(sizeof(char*));
	if(!sub_addr){
		return NULL;
	}
	for(i=0; i<n_subs && !found; i++){
		if(suscriptores[i].id_sub == id_sub){
			found = true;
			strcpy(sub_addr,suscriptores[i].addr);
		}
	}
	return sub_addr;
}

/* Añade un suscriptor a la lista de suscriptores y devuelve su identificador o -1 si error */
int addSub(char *subscriber, int port){
	/* Comprobar si suscriptor esta en la bbdd */
	int id_sub = n_subs;
	EntradaSub new_entrada;
	/* Si no esta, añadirlo */
	suscriptores = realloc(suscriptores,(n_subs+1)*sizeof(struct entrada_sub));
	if(suscriptores == NULL){
		return -1;
	}
	new_entrada.id_sub = id_sub;
	strcpy(new_entrada.addr,subscriber);
	new_entrada.port = port;
	suscriptores[id_sub] = new_entrada;
	
	n_subs++;
	return id_sub;
}

/* Da de alta a un suscriptor al tema especificado */
int altaSubTopic(char *subscriber, int port, char *topic){
	int id_topic;
	int id_sub;
	EntradaSuscripcion new_entrada;

	/* Comprobar si existe el topic */
	id_topic=getTopicId(topic);
	if(id_topic == -1){
		return -1;
	}
	/* Comprobar subscriber */
	id_sub=getSubId(subscriber, port);
	if(id_sub == -1){
		if((addSub(subscriber,port)) == -1){
			return -1;
		}
	}
	else{
	/* Comprobar que no esta dado de alta */
		if(isSubbed(id_sub,id_topic)){
			return -1;
		}
	}

	// dar de alta
	suscriptores_temas = realloc(suscriptores_temas,(n_suscripciones+1)*sizeof(struct subs_topic));
	if(suscriptores_temas == NULL){
		fprintf(stderr, "No se pudo ubicar la memoria dinamica necesaria\n");
		return -1;
	}
	if(id_sub == -1){
		id_sub = getSubId(subscriber, port);
	}
	new_entrada.id_topic = id_topic;
	new_entrada.id_sub = id_sub;
	suscriptores_temas[n_suscripciones] = new_entrada;
	
	addNSubTopic(new_entrada.id_topic);
	n_suscripciones++;
	return 0;
}
/* Da de alta a un suscriptor al tema especificado */
int bajaSubTopic(char *subscriber, int port, char *topic){
	int id_topic;
	int id_sub;
	int id_sub_last;
	int id_topic_last;

	/* Comprobar si existe el topic */
	id_topic=getTopicId(topic);
	if(id_topic == -1){
		return -1;
	}
	/* Comprobar subscriber */
	id_sub=getSubId(subscriber,port);
	if(id_sub == -1){
		addSub(subscriber,port);
		return -1;
	}
	else{
	/* Comprobar que esta dado de alta */
		if(!isSubbed(id_sub,id_topic)){
			return -1;
		}
	}

	if(n_suscripciones==0){
		return -1;
	}
	else if(n_suscripciones==1){
		if(suscriptores_temas[0].id_sub == id_sub && suscriptores_temas[0].id_topic == id_topic){
			free(suscriptores_temas);
			suscriptores_temas = malloc(sizeof(struct subs_topic));
			if(suscriptores_temas == NULL){
				return -1;
			}
			delNSubTopic(id_topic);
		}
	}
	else{
	// dar de baja
		bool deleted;
		int i;
		for(i=0; i<n_suscripciones && !deleted; i++){
			if(suscriptores_temas[i].id_sub == id_sub && suscriptores_temas[i].id_topic == id_topic){
				if(i != n_suscripciones-1){ // si la entrada NO es la última
					// coger la ultima entrada
					id_topic_last = suscriptores_temas[n_suscripciones-1].id_topic;
					id_sub_last = suscriptores_temas[n_suscripciones-1].id_sub;

					suscriptores_temas[i].id_sub = id_sub_last;
					suscriptores_temas[i].id_topic = id_topic_last;
				}
				suscriptores_temas = realloc(suscriptores_temas,(n_suscripciones-1)*sizeof(struct subs_topic));
				if(suscriptores_temas == NULL){
					fprintf(stderr, "No se pudo ubicar la memoria dinamica necesaria\n");
					return -1;
				}
				delNSubTopic(id_topic);
				deleted = true;
			}
		}
		if(!deleted){
			return -1;
		}
	}
	n_suscripciones--;
	return 0;
}
/* PRE: El tema existe */
/* Envia un evento a los suscriptores correspondientes */
int push_notification(char *topic, char *value){
	int i;
	int id_topic;
	int id_sub;
	int port;
	int tcp_sr;
	struct sockaddr_in tcp_addr_client;
	msg nuevo_evento;

	escribir_msg(EVENTO,0,topic,value,&nuevo_evento);

	id_topic = getTopicId(topic);

		for(i=0;i<n_suscripciones;i++){
			if(suscriptores_temas[i].id_topic == id_topic){
				id_sub = suscriptores_temas[i].id_sub;
			
				tcp_sr = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
				if(tcp_sr < 0){
					return -1;
				}
				bzero((char *) &tcp_addr_client, sizeof(tcp_addr_client));  // Inicializar estructura

				/* Establecer parametros de la direccion TCP del intermediario */
				tcp_addr_client.sin_family = AF_INET;

				if((port=getSubPort(id_sub)) == -1){ // si no es posible obtener el puerto del suscriptor
					continue;
				}
				tcp_addr_client.sin_port = htons(port);

				if(!getSubAddr(id_sub)){ //si no es posible obtener la direccion del suscriptor
					continue;
				}
				tcp_addr_client.sin_addr.s_addr = inet_addr(getSubAddr(id_sub)); // tcp_addr_interm.sin_addr.s_addr = intermediario;
				if(connect(tcp_sr,(struct sockaddr*) &tcp_addr_client,sizeof(struct sockaddr_in))<0)
				{
					continue;
				}
				/* Mandar evento a suscriptor i*/
				escribir_msg(EVENTO,0,topic,value,&nuevo_evento);
				/* Abrir conexion con nuevo socket tcp y puerto de notificaciones */
				send(tcp_sr,&nuevo_evento,sizeof(struct mensaje),0);
			}
		}
	close(tcp_sr);
	return 0;
}

int main(int argc, char *argv[]) {

	/* Temas y suscriptores */
	FILE * fichero_temas;
	char linea[64];

	n_subs = 0;
	n_topics = 0;
	n_suscripciones = 0;

	n_sub_topic = malloc(sizeof(struct num_subs_topic));
	suscriptores_temas = malloc(sizeof(struct subs_topic));
	temas = malloc(sizeof(struct entrada_tema));
	suscriptores = malloc(sizeof(struct entrada_sub));

	/* Conexion TCP */
	int accept_sd;
	int tcp_sd;
	int size;
	struct sockaddr_in tcp_addr_interm, tcp_addr_client;
	msg peticion;
	int respuesta;

	/* Parsear argumentos */
	if (argc!=3) {
		fprintf(stderr,"Uso: %s puerto fichero_temas\n", argv[0]);
		return 1;
	}

	service_port = atoi(argv[1]);
	if ((fichero_temas = fopen(argv[2], "r")) == NULL ){
		fprintf(stderr,"Fichero de temas no disponible\n");
		return -1;
	}
	
	/* Leer fichero de temas y crear estructura de temas-subscriptores */
	while (fgets(linea,MAX,fichero_temas)!= NULL){
		char *c = strchr(linea, '\n');
		if (c)
			*c = 0;
		addTopic(linea);
	}
	printTemas();
	fclose(fichero_temas);

	/* ------------- Preparar recepcion de mensajes ---------------- */

	/* Creacion del socket TCP de servicio */
	tcp_sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(tcp_sd < 0){
		return -1;
	}

  	/* Asignacion de la direccion local (del servidor) Puerto TCP*/    
	bzero((char *) &tcp_addr_interm, sizeof(tcp_addr_interm));

	tcp_addr_interm.sin_family = AF_INET;
	tcp_addr_interm.sin_addr.s_addr = INADDR_ANY;
	tcp_addr_interm.sin_port = htons(service_port);

	if(bind(tcp_sd, (struct sockaddr *) &tcp_addr_interm, sizeof(tcp_addr_interm)) < 0){
		close(tcp_sd);
		return -1;
	}

  	/* Aceptamos conexiones por el socket */
	if(listen(tcp_sd,5)<0){
		return -1;
	}

	/* Recibir mensajes de alta, baja o evento */
	// printf("Numero de topics: %d \t| Numero de suscriptores: %d \t| Numero de entradas: %d |\n", n_topics, n_subs, n_suscripciones);
	while(1){
		/* Esperamos la llegada de una conexion */
		bzero((char *) &tcp_addr_client, sizeof(tcp_addr_client));
		size = sizeof(tcp_addr_client);
		if((accept_sd=accept(tcp_sd, (struct sockaddr *) &tcp_addr_client, (socklen_t *)  &size)) < 0){
			continue;
		}
		else{ // conexion correcta
			/* Recibir peticion */
			recv(accept_sd,(msg *)&peticion,sizeof(struct mensaje),0);
			/* Analizar peticion */
			if(ntohs(peticion.cod_op)==ALTA){
				respuesta=altaSubTopic(inet_ntoa(tcp_addr_client.sin_addr), ntohs(peticion.port), peticion.tema);

				/* Enviar respuesta */
				send(accept_sd,&respuesta,sizeof(int),0);
				close(accept_sd);
				// printf("Numero de topics: %d \t| Numero de suscriptores: %d \t| Numero de entradas: %d |\n", n_topics, n_subs, n_suscripciones);
				// printTemas();
				// printSubs();
				// printSuscripciones();
			}
			else if(ntohs(peticion.cod_op)==BAJA){
				respuesta=bajaSubTopic(inet_ntoa(tcp_addr_client.sin_addr), ntohs(peticion.port), peticion.tema);

				/* Enviar respuesta */
				send(accept_sd,&respuesta,sizeof(int),0);
				close(accept_sd);
				// printf("Numero de topics: %d \t| Numero de suscriptores: %d \t| Numero de entradas: %d \t|\n", n_topics, n_subs, n_suscripciones);
				// printTemas();
				// printSubs();
				// printSuscripciones();
			}
			else if(ntohs(peticion.cod_op)==EVENTO){

				/* Comprobar si tema existe */
				if((getTopicId(peticion.tema)) != -1){
					respuesta = EVENTO_OK;
				}
				else{
					respuesta = EVENTO_ERROR;
				}
				printf("Resultado del evento generado: %d\n", respuesta);
				send(accept_sd,&respuesta,sizeof(int),0);
				close(accept_sd);

				// Enviar notificacion a suscritos en el tema 
				push_notification(peticion.tema, peticion.valor);
			}
			else{
				close(accept_sd);
			}
		}
	}
	close(tcp_sd);
	free(n_sub_topic);
	free(temas);
	free(suscriptores);
	free(suscriptores_temas);
	return 0;
}
