
/*! \file Simptcp_entity.h
*  \brief{Defines non socket dependent state variables (meaningful to all simpTCP sockets).These are wrapped up in a global variable called "simptcp_entity"}
* \author{DGEI-INSAT 2010-2011}
*/
#ifndef _SIMPTCP_ENTITY_H_
#define _SIMPTCP_ENTITY_H_

#include <stdint.h>             /* for INT32_MAX */
#include <pthread.h>            /* for pthread_mutex_t, pthread_cond_t */
#include <sys/socket.h>
#include <netinet/in.h>
#include <simptcp_lib.h>

#define MAX_OPEN_SOCK 5 /* the maximum number of open sockets */
#define ETH_MTU 1500 /* Ethernet Max transmit Unit */
#define  MAX_SIMPTCP_BUFFER_SIZE (ETH_MTU-20-8) /* to avoid IP fragmentation */

/*!
*  \struct simptcp 
* \brief structure regroupant toutes les donnees non specifiques a un socket simpTCP
*  necessaires au fonctionnement d'une entite protocolaire simpTCP et notamment:   
* - Table des descripteurs de sockets simpTCP. Le "descripteur d'un socket simpTCP"
*   joue le role d'indice de cette table. Un element de la table est un pointeur sur
*   la structure de donnes simptcp_socket qui regrouppe les donnees relatives a un socket
*   simpTCP.
* - nombre et liste des socket simTCP crees a la charge de l'entite simpTCP
* - nombre de connexions simpTCP creees a la charge de l'entite simpTCP
* - descripteur  et l'adresse de niveau transport du socket UDP utilise par l'entite simpTCP pour acceder au service UDP
* - occupation et pointeur sur le buffer qui memorise un PDU simpTCP recu avant l'etape de demultiplexage permettant d'identifier le socket simpTCP cible
* - Table de pointeur vers les fonctions qu'execute une entite simpTCP, se trouvant dans un etat donne, en reaction a un evennement (timout, reception PDU,..)  
*/
struct simptcp { 
	struct simptcp_socket *simptcp_socket_descriptors[MAX_OPEN_SOCK];/*!< SimpTCP socket descriptor table */
	struct simptcp_socket * simptcp_socket_list; /*!< Open simpTCP socket list */
	unsigned int open_simptcp_sockets; /*!< open simpTCP sockets number */
	unsigned int open_simptcp_connections; 	/*!< number of open simpTCP connections */
	
	int udp_fd; /*!< udp socket descriptor */
	struct sockaddr_in local_udp;  /*!< local UDP socket SAP address */
	
	char in_buffer[MAX_SIMPTCP_BUFFER_SIZE]; /*!< SimpTCP socket Receive buffer used ;	
											  Provisionned for one single MAXSIZE PDU */
	unsigned int in_len; /*!< instantaneous in_buffer occupation */
	
	
	
	simptcp_socket_states_funcs * simptcp_socket_states; /*!< List of pointers to the functions that SimpTCP
														   entity run in reaction to a timeout, packet arrival, tx requets */


	pthread_t simptcp_handler; /*!< handler in charge of detecting simptcp
								packet arrivals and timeouts : #simptcp_entity_handler */
};


/*! \var struct simptcp simptcp_entity; 
 * \brief Variable globale instance de la structure simpTCP permettant a l'entite protocolaire 
 * simpTCP (materialisee par le handler #simptcp_entity_handler et les fonctions #simptcp_socket_states)
 * d'acceder aux donnees qui lui sont necessaires
 *
*/
struct simptcp simptcp_entity; 



/* create a simptcp_core handler */
int start_simptcp (int local_udp);

#endif /* _SIMPTCP_ENTITY_H_ */

/* vim: set expandtab ts=4 sw=4 tw=80: */
