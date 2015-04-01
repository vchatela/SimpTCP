/*! \file simptcp_entity.c
 * \brief 
 *
 */
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>          /* for printf() */
#include <stdint.h>         /* for UINT16_MAX */
#include <string.h>         /* for memset() */
#include <unistd.h>         /* for sleep() */
#include <errno.h>          /* for error numbers */
#include <fcntl.h>              /* for fcntl(), O_NONBLOCK */
#include <arpa/inet.h>
#include <sys/time.h>           /* for gettimeofday,..*/


#include <simptcp_entity.h>
#include <simptcp_lib.h>
#include <simptcp_packet.h>
#include <libc_socket.h>

#include <term_colors.h>
#define __PREFIX__	    "[" COLOR("SIMPTCP_ENTITY", BRIGHT_CYAN) "] "
#include <term_io.h>

#ifndef __DEBUG__
#define __DEBUG__               1
#endif

extern simptcp_socket_states_funcs simptcp_socket_states;


/*!
 * \fn int set_non_blocking(int fd)
 * \brief Place socket udp utilisé par SimpTCP dans un mode non bloquant (en émission/réception)
 * \param fd  descriptor du socket UDP  
 * \return -1 en cas d'echec avec la variable errno positionnee
*/
int set_non_blocking(int fd)
{
    int flags;

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

/* If they have O_NONBLOCK, use the Posix way to do it */
#if defined(O_NONBLOCK)
    flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    /* Otherwise, use the old way of doing it */
    flags = 1;
    return ioctl(fd, FIOBIO, &flags);
#endif
}

/*!
 * \fn int demultiplex_packet(char * buffer,struct sockaddr_in * udp_remote)
 * \brief implemente la fonction de demultiplexage de SimpTCP declenchee a l'arrivee d'un PDU SimpTCP. 
 *A partir d'un PDU simpTCP recu, permet de determiner le socket SimpTCP destinataire
 * deux cas de figure a considerer : Cas1) PDU destine a un "listening simpTCP socket" (cote serveur)
 * suppose recevoir les PDU SimpTCP-SYN de demande d'etablissement d'une nouvelle connexion
 * Cas 2) PDU destine a un "non listening socket" (socket cote client ou cote serveur cree suite
 * a l'acceptation d'une demande de connexion 
 * \param buffer qui pointe sur le PDU SimpTCP (charge utile du paquet UDP recu)
 * \param udp_remote qui pointe sur l'adresse du socket UDP emetteur du PDU SimpTCP 
 * \return le descripteur du socket SimpTCP ou -1 s'il n'est destine a socket SimpTCP
 */
int demultiplex_packet(char * buffer,struct sockaddr_in * udp_remote)
{
  struct simptcp_socket *sock = NULL;
  struct simptcp_socket *new_sock = NULL; 
  struct sockaddr_in simptcp_remote;
  u_int16_t dport;
  int fd; /* simtcp socket descriptor */
  int slen=sizeof(struct sockaddr_in);

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif 

 /* set the sockaddr of the remote simptcp socket 
    from which the packet originates*/
 /* rely on the sockaddr of the remote udp socket */ 
  memcpy(&simptcp_remote, udp_remote, slen);
  simptcp_remote.sin_port = htons(simptcp_get_sport(buffer));
  dport = htons(simptcp_get_dport(buffer));
 
  /* check if the packet is destined for a non-listening socket */
	/* this is an inefficient way to fetch for open sockets 
	   could be imporved using the open_sockets_list 
	   which points to the head of the list of open sockets
	*/
  for (fd=0;fd< MAX_OPEN_SOCK;fd++)
    {
      if ((sock=simptcp_entity.simptcp_socket_descriptors[fd]) != NULL)
	{ /* this is an open socket ..*/
	  if (sock->local_simptcp.sin_port == dport
	      && sock->remote_simptcp.sin_addr.s_addr == simptcp_remote.sin_addr.s_addr
	     && sock->remote_simptcp.sin_port == simptcp_remote.sin_port)
	    { /* this is the fetched socket */
#if __DEBUG__
	      printf("Delivering packet to socket fd %u at state %s\n",
		     fd, simptcp_socket_state_get_str(sock->socket_state));
#endif
	      return fd;
	    } 
	}
    }
   /* now, check if the packet is destined for a listening sock */
  for (fd=0;fd< MAX_OPEN_SOCK;fd++)
    {
      if ((sock=simptcp_entity.simptcp_socket_descriptors[fd]) != NULL)
	{ 
	  if ((sock->local_simptcp.sin_port == dport)
	      && (sock->socket_type == listening_server))
	    { /* this is the fetched listening socket */
#if __DEBUG__
	      printf("Delivering packet to socket fd %u at state %s\n",
		     fd, simptcp_socket_state_get_str(sock->socket_state));
#endif
	      /* for a listening socket an additionnal work is needed :
		 save the remote udp/simpTCP addresses; they will be used
		 when processing the received pdu
	       */	      
	      lock_simptcp_socket(sock);	      
	      
	      memcpy(&(sock->remote_simptcp),&simptcp_remote,slen);
	      memcpy(&(sock->remote_udp),udp_remote,slen);
	      unlock_simptcp_socket(sock);    

	      return fd;
	    }
	}
    }
  /* No match found */
#if __DEBUG__
  printf("No Match found \n");
#endif
 return -1; 
}


/*!
 * \fn void * simptcp_entity_handler()
 * \brief handler lance au demarrage de SimpTCP (au lancement de l'application utilisant 
 * le service SimpTCP) et qui s'execute en continu en // au programme qui l'a lance. 
 * En charge de detecter deux types d'evenments et de lancer les fonctions correspondant 
 * aux traitements associes : 
 * 1) a l'arrivee arrivee d'Un PDU SimpTCP -> determine le socket Simptcp
 * Concerne puis traite le paquet 2) detection de timeout sur les timers utilises 
 * par les socket SimpTCP et lancer les traitements appropries
 */

void * simptcp_entity_handler()
{
  /* simptcp receive buffer */
  char* buffer =  simptcp_entity.in_buffer; 
  /* udp remotre SAP from which the packet originates */
  struct sockaddr_in udp_remote; 
  unsigned int slen = sizeof(struct sockaddr_in);
  int fd; /* simptcp socket file descriptor */
  struct timeval t0;

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

  while (1) {

    usleep(10);
    /* check for a new arriving packet */
    simptcp_entity.in_len = libc_recvfrom(simptcp_entity.udp_fd,buffer, 
				     MAX_SIMPTCP_BUFFER_SIZE,0, 
				      (struct sockaddr*) &udp_remote, &slen);

    if (simptcp_entity.in_len != -1) {
#if __DEBUG__
      printf("************************************************************\n"
	     "Received packet of size %d on %s:%hu\n",
	     simptcp_entity.in_len, inet_ntoa(udp_remote.sin_addr),
	     simptcp_get_dport(buffer));
#endif
      /* check if corrupted */
      if (!simptcp_check_checksum(buffer,simptcp_entity.in_len)) {
#if __DEBUG__
	printf("Dropping corrupted packet\n");
#endif
	/* TODO : on pourrait prévoir un memset */
	continue ;
      }
      else { /* clean simptcp packet */
#if __DEBUG__
	simptcp_print_packet(buffer);
#endif
	/* Demultiplex packet */
	  
	if ((fd=demultiplex_packet(buffer,&udp_remote)) >=0)
	  /* the packets is destined to an open simptcp socket */
	  simptcp_entity.simptcp_socket_descriptors[fd]->socket_state->process_simptcp_pdu(simptcp_entity.simptcp_socket_descriptors[fd],buffer,simptcp_entity.in_len);
      }
    }
    //   else if ((simptcp_entity.in_len ==-1) && (errno != EAGAIN))
    //return -1;

    /* check for timeouts */

    for (fd=0;fd< MAX_OPEN_SOCK;fd++)
      {
	gettimeofday(&t0,NULL);
	if (((simptcp_entity.simptcp_socket_descriptors[fd]) != NULL) &&
	    (has_active_timer(simptcp_entity.simptcp_socket_descriptors[fd])) &&
	    (is_timeout(simptcp_entity.simptcp_socket_descriptors[fd])))
	  {/* timeout detected on the open socket */
	    simptcp_entity.simptcp_socket_descriptors[fd]->socket_state->handle_timeout(simptcp_entity.simptcp_socket_descriptors[fd]);
	  }
      } 

  } /* while(1) */
}


/*!
 * \fn int start_simptcp(int local_udp)
 * \brief initialise simptcp control block et lance 
 * le handler #simptcp_entity_handle
 * \param local_udp numero de port udp utilise par simpTCP.
 * Valeur fixee par #DEFAULT_LOCAL_UDP_PORT
 * \return -1 si echec (avec errno positionne), 0 sinon. 
 */
int start_simptcp(int local_udp)
{    
  int res = -1;
  
#if __DEBUG__
  printf("function %s called\n", __func__);
#endif
	simptcp_entity.local_udp.sin_family = AF_INET;
	simptcp_entity.local_udp.sin_addr.s_addr = htonl(INADDR_ANY);
	simptcp_entity.local_udp.sin_port = htons(local_udp);
	
	/* creation of the underlying UDP socket */
	res = libc_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (res < 0) {
	  perror("Creation of UDP socket for simptcp failed");
	  return res;
	}    
	simptcp_entity.udp_fd=res;
	/* Set socket options: non blockin sys calls */
	set_non_blocking(simptcp_entity.udp_fd); 
	
	/* initialiser le numéro de port du socket */
	res= libc_bind(simptcp_entity.udp_fd,(struct sockaddr *) &simptcp_entity.local_udp,sizeof(simptcp_entity.local_udp) );
	if (res < 0) {
      perror("bind UDP socket for simptcp failed");
      return res;
	}    
	simptcp_entity.simptcp_socket_list=NULL;
	simptcp_entity.simptcp_socket_states=&(simptcp_socket_states);
	simptcp_entity.open_simptcp_connections=0;
	simptcp_entity.open_simptcp_sockets=0;
	memset(simptcp_entity.in_buffer, 0,MAX_SIMPTCP_BUFFER_SIZE);   
    
    
	/* launch a separate process that will execute simptcp_handler in parallel
	 * to the main program (client/server)
	 */
	res = pthread_create(&(simptcp_entity.simptcp_handler), NULL, 
			     simptcp_entity_handler, NULL);
	if (res < 0) {
	  perror("Unable to create core handler");
	  return -1;
	} 
    
	return res;
}





/* vim: set expandtab ts=4 sw=4 tw=80: */
