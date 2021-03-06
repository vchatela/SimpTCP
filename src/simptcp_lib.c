/*! \file simptcp_lib.c
*  \brief{Defines the functions that gather the actions performed by a simptcp protocol entity in reaction to events (system calls, simptcp packet arrivals, timeouts) given its state at a point in time  (closed, ..established,..).}
*  \author{DGEI-INSAT 2010-2011}
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>              /* for errno macros */
#include <sys/socket.h>
#include <netinet/in.h>         /* for htons,.. */
#include <arpa/inet.h>
#include <unistd.h>             /* for usleep() */
#include <sys/time.h>           /* for gettimeofday,..*/

#include <libc_socket.h>
#include <simptcp_packet.h>
#include <simptcp_entity.h>
#include "simptcp_func_var.c"    /* for socket related functions' prototypes */
#include <term_colors.h>        /* for color macros */
#define __PREFIX__              "[" COLOR("SIMPTCP_LIB", BRIGHT_YELLOW) " ] "
#include <term_io.h>

#ifndef __DEBUG__
#define __DEBUG__               1
#endif


/*! \fn char *  simptcp_socket_state_get_str(simptcp_socket_state_funcs * state)
* \brief renvoie une chaine correspondant a l'etat dans lequel se trouve un socket simpTCP. Utilisee a des fins d'affichage
* \param state correspond typiquement au champ socket_state de la structure #simptcp_socket qui indirectement identifie l'etat dans lequel le socket se trouve et les fonctions qu'il peut appeler depuis cet etat
* \return chaine de carateres correspondant a l'etat dans lequel se trouve le socket simpTCP
*/
char *  simptcp_socket_state_get_str(simptcp_socket_state_funcs * state)
{
    if (state == &  simptcp_socket_states.closed)
        return "CLOSED";
    else if (state == & simptcp_socket_states.listen)
        return "LISTEN";
    else if (state == & simptcp_socket_states.synsent)
        return "SYNSENT";
    else if (state == & simptcp_socket_states.synrcvd)
        return "SYNRCVD";
    else if (state == & simptcp_socket_states.established)
        return "ESTABLISHED";
    else if (state == & simptcp_socket_states.closewait)
        return "CLOSEWAIT";
    else if (state == & simptcp_socket_states.finwait1)
        return "FINWAIT1";
    else if (state == & simptcp_socket_states.finwait2)
        return "FINWAIT2";
    else if (state == & simptcp_socket_states.closing)
        return "CLOSING";
    else if (state == & simptcp_socket_states.lastack)
        return "LASTACK";
    else if (state == & simptcp_socket_states.timewait)
        return "TIMEWAIT";
    else
        assert(0);
}

/**
 * \brief called at socket creation
 * \return the first sequence number to be used by the socket
 * \TODO: randomize the choice of the sequence number to fit TCP behaviour..
 */
unsigned int get_initial_seq_num()
{
    unsigned int init_seq_num=15;
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    return init_seq_num;
}


/*!
* \brief Initialise les champs de la structure #simptcp_socket
* \param sock pointeur sur la structure simptcp_socket associee a un socket simpTCP
* \param lport numero de port associe au socket simptcp local
*/
void init_simptcp_socket(struct simptcp_socket *sock, unsigned int lport)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    assert(sock != NULL);
    pthread_mutex_init(&(sock->mutex_socket), NULL);

    lock_simptcp_socket(sock);

    /* Initialization code */

    sock->socket_type = unknown;
    sock->new_conn_req=NULL;
    sock->pending_conn_req=0;

    /* set simpctp local socket address */
    memset(&(sock->local_simptcp), 0, sizeof (struct sockaddr));
    sock->local_simptcp.sin_family = AF_INET;
    sock->local_simptcp.sin_addr.s_addr = htonl(INADDR_ANY);
    sock->local_simptcp.sin_port = htons(lport);

    memset(&(sock->remote_simptcp), 0, sizeof (struct sockaddr));


    sock->socket_state = &(simptcp_entity.simptcp_socket_states->closed);

    /* protocol entity sending side */
    sock->socket_state_sender=-1;
    sock->next_seq_num=get_initial_seq_num();
    memset(sock->out_buffer, 0, SIMPTCP_SOCKET_MAX_BUFFER_SIZE);
    sock->out_len=0;
    sock->nbr_retransmit=0;
    sock->timer_duration=1500;
    /* protocol entity receiving side */
    sock->socket_state_receiver=-1;
    sock->next_ack_num=0;
    memset(sock->in_buffer, 0, SIMPTCP_SOCKET_MAX_BUFFER_SIZE);
    sock->in_len=0;

    /* timeut initialization */
    sock->timeout.tv_sec=0;
    sock->timeout.tv_usec=0;
    /* MIB statistics initialisation  */
    sock->simptcp_send_count=0;
    sock->simptcp_receive_count=0;
    sock->simptcp_in_errors_count=0;
    sock->simptcp_retransmit_count=0;


    /* Add Optional field initialisations */
    unlock_simptcp_socket(sock);

}



/*! \fn int create_simptcp_socket()
* \brief cree un nouveau socket SimpTCP et l'initialise.
* parcourt la table de  descripteur a la recheche d'une entree libre. S'il en trouve, cree
* une nouvelle instance de la structure simpTCP, la rattache a la table de descrpteurs et l'initialise.
* \return descripteur du socket simpTCP cree ou une erreur en cas d'echec
*/
int create_simptcp_socket()
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    int fd;
    struct simptcp_socket*  new_sock;


    /* get a free simptcp socket descriptor */
    for (fd=0; fd< MAX_OPEN_SOCK; fd++)
    {
        if ((simptcp_entity.simptcp_socket_descriptors[fd]) == NULL)
        {
            /* this is a free descriptor */
            /* Allocating memory for the new simptcp_socket */
            new_sock =
                (struct simptcp_socket *) malloc(sizeof(struct simptcp_socket));
            if (!new_sock)
            {
                return -ENOMEM;
            }
            /* initialize the simptcp socket control block with
             local port number set to 15000+fd */
            init_simptcp_socket(new_sock,15000+fd);
            simptcp_entity.open_simptcp_sockets++;

            simptcp_entity.simptcp_socket_descriptors[fd]=new_sock;
            /* return the socket descriptor */
            return fd;
        }
    } /* for */
    /* The maximum number of open simptcp
     socket reached  */
    return -ENFILE;
}

/*! \fn void print_simptcp_socket(struct simptcp_socket *sock)
* \brief affiche sur la sortie standard les variables d'etat associees a un socket simpTCP
* Les valeurs des principaux champs de la structure simptcp_socket d'un socket est affichee a l'ecran
* \param sock pointeur sur les variables d'etat (#simptcp_socket) d'un socket simpTCP
*/
void print_simptcp_socket(struct simptcp_socket *sock)
{
    printf("----------------------------------------\n");
    printf("local simptcp address: %s:%hu \n",inet_ntoa(sock->local_simptcp.sin_addr),ntohs(sock->local_simptcp.sin_port));
    printf("remote simptcp address: %s:%hu \n",inet_ntoa(sock->remote_simptcp.sin_addr),ntohs(sock->remote_simptcp.sin_port));
    printf("remote udp address: %s:%hu \n",inet_ntoa(sock->remote_udp.sin_addr),ntohs(sock->remote_udp.sin_port));
    printf("socket type      : %d\n", sock->socket_type);
    printf("socket state: %s\n",simptcp_socket_state_get_str(sock->socket_state) );
    if (sock->socket_type == listening_server)
        printf("pending connections : %d\n", sock->pending_conn_req);
    printf("sending side \n");
    printf("sender state       : %d\n", sock->socket_state_sender);
    printf("transmit  buffer occupation : %d\n", sock->out_len);
    printf("next sequence number : %u\n", sock->next_seq_num);
    printf("retransmit number : %u\n", sock->nbr_retransmit);

    printf("Receiving side \n");
    printf("receiver state       : %d\n", sock->socket_state_receiver);
    printf("Receive  buffer occupation : %d\n", sock->in_len);
    printf("next ack number : %u\n", sock->next_ack_num);

    printf("send count       : %lu\n", sock->simptcp_send_count);
    printf("receive count       : %lu\n", sock->simptcp_receive_count);
    printf("receive error count       : %lu\n", sock->simptcp_in_errors_count);
    printf("retransmit count       : %lu\n", sock->simptcp_retransmit_count);
    printf("----------------------------------------\n");
}


/*! \fn inline int lock_simptcp_socket(struct simptcp_socket *sock)
* \brief permet l'acces en exclusion mutuelle a la structure #simptcp_socket d'un socket
* Les variables d'etat (#simptcp_socket) d'un socket simpTCP peuvent etre modifiees par
* l'application (client ou serveur via les appels systeme) ou l'entite protocolaire (#simptcp_entity_handler).
* Cette fonction repose sur l'utilisation de semaphores binaires (un semaphore par socket simpTCP).
* Avant tout  acces en ecriture a ces variables, l'appel a cette fonction permet
* 1- si le semaphore est disponible (unlocked) de placer le semaphore dans une etat indisponible
* 2- si le semaphore est indisponible, d'attendre jusqu'a ce qu'il devienne disponible avant de le "locker"
* \param sock pointeur sur les variables d'etat (#simptcp_socket) d'un socket simpTCP
*/
int lock_simptcp_socket(struct simptcp_socket *sock)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    if (!sock)
        return -1;

    return pthread_mutex_lock(&(sock->mutex_socket));
}

/*! \fn inline int unlock_simptcp_socket(struct simptcp_socket *sock)
* \brief permet l'acces en exclusion mutuelle a la structure #simptcp_socket d'un socket
* Les variables d'etat (#simptcp_socket) d'un socket simpTCP peuvent etre modifiees par
* l'application (client ou serveur via les appels systeme) ou l'entite protocolaire (#simptcp_entity_handler).
* Cette fonction repose sur l'utilisation de semaphores binaires (un semaphore par socket simpTCP).
* Après un acces "protege" en ecriture a ces variables, l'appel a cette fonction permet de liberer le semaphore
* \param sock pointeur sur les variables d'etat (#simptcp_socket) d'un socket simpTCP
*/
int unlock_simptcp_socket(struct simptcp_socket *sock)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    if (!sock)
        return -1;

    return pthread_mutex_unlock(&(sock->mutex_socket));
}

/*! \fn void start_timer(struct simptcp_socket * sock, int duration)
 * \brief lance le timer associe au socket en fixant l'instant ou la duree a mesurer "duration" sera ecoulee (champ "timeout" de #simptcp_socket)
 * \param sock  pointeur sur les variables d'etat (#simptcp_socket) d'un socket simpTCP
 * \param duration duree a mesurer en ms
*/
void start_timer(struct simptcp_socket * sock, int duration)
{
    struct timeval t0;
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    assert(sock!=NULL);

    gettimeofday(&t0,NULL);

    sock->timeout.tv_sec=t0.tv_sec + (duration/1000);
    sock->timeout.tv_usec=t0.tv_usec + (duration %1000)*1000;
}

/*! \fn void stop_timer(struct simptcp_socket * sock)
 * \brief stoppe le timer en reinitialisant le champ "timeout" de #simptcp_socket
 * \param sock  pointeur sur les variables d'etat (#simptcp_socket) d'un socket simpTCP
 */
void stop_timer(struct simptcp_socket * sock)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    assert(sock!=NULL);
    sock->timeout.tv_sec=0;
    sock->timeout.tv_usec=0;
}

int resendBuffer(struct simptcp_socket *sock) {
                                                                                                                                                                                                                                                                                                                                                                                    return 0;
    stop_timer(sock);
    int res = libc_sendto(simptcp_entity.udp_fd,
                          sock->out_buffer,
                          simptcp_get_total_len(sock->out_buffer),
                          0,
                          (struct sockaddr *) &(sock->remote_udp),
                          sizeof(struct sockaddr_in));
    if (res == -1) {
        printf("ERROR: Sending FIN failed.\n");
        return res;
    }
    start_timer(sock, getTimeoutDuration(sock));

    return 0;
}
/*! \fn int has_active_timer(struct simptcp_socket * sock)
 * \brief Indique si le timer associe a un socket simpTCP est actif ou pas
 * \param sock  pointeur sur les variables d'etat (#simptcp_socket) d'un socket simpTCP
 * \return 1 si timer actif, 0 sinon
 */
int has_active_timer(struct simptcp_socket * sock)
{
    return (sock->timeout.tv_sec!=0) || (sock->timeout.tv_usec!=0);
}

/*! \fn int is_timeout(struct simptcp_socket * sock)
 * \brief Indique si la duree mesuree par le timer associe a un socket simpTCP est actifs'est ecoulee ou pas
 * \param sock  pointeur sur les variables d'etat (#simptcp_socket) d'un socket simpTCP
 * \return 1 si duree ecoulee, 0 sinon
 */
int is_timeout(struct simptcp_socket * sock)
{
    struct timeval t0;

    assert(sock!=NULL);
    /* make sure that the timer is launched */
    assert(has_active_timer(sock));

    gettimeofday(&t0, NULL);
    return ((sock->timeout.tv_sec < t0.tv_sec) ||
            ( (sock->timeout.tv_sec == t0.tv_sec) && (sock->timeout.tv_usec < t0.tv_usec)));
}

/*
 * Obtient la durée du timeout à mettre dans start timer.
 */
int getTimeoutDuration(struct simptcp_socket * sock)
{
    return sock->rtt_estimate * 4 * 1000;
}

int checkSequenceNumber(struct simptcp_socket *sock, void *buf) {
    int seq = simptcp_get_seq_num(buf);
    int expected = sock->next_ack_num;
    if (seq == expected) {
        printf("Good sequence number : expected %d, got %d\n", expected, seq);
        return 1;
    }
    else {
        printf("Bad sequence number : expected %d, got %d\n", expected, seq);
        return 0;
    }
}



/*** socket state dependent functions ***/


/*********************************************************
 * closed_state functions *
 *********************************************************/

/*! \fn int closed_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "connect" alors que le socket simpTCP est dans l'etat "closed"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param addr adresse de niveau transport du socket simpTCP destination
 * \param len taille en octets de l'adresse de niveau transport du socket destination
 * \return  0 si succes, -1 si erreur
 */
int closed_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    // A faire :
    // Démarrage du timer
    // Envoi du PDU Syn
    // Etat suivant : syn_sent
    if(len != sizeof(struct sockaddr_in))
        printf("bad address size\n");

    sock->remote_simptcp = *((struct sockaddr_in *)addr);
    sock->remote_udp = *((struct sockaddr_in *)addr);
    sock->socket_type = client;
    // Numéro de séquence du premier pdu
    // Next seq num devra être incrémenté à la réception
    // du pdu ack. 
    sock->next_seq_num = get_initial_seq_num();

    printf("***** SYN; SEQ=%d, ACK=%d\n", sock->next_seq_num, sock->next_ack_num);

    char* pdu = simptcp_make_pdu(&sock->local_simptcp,
                             &sock->remote_simptcp,
                             NULL, // payload
                             0, // len
                             sock->next_seq_num, // seq
                             0, // ack
                             SYN);

    // Copie le pdu dans le out buffer.
    memcpy(sock->out_buffer, pdu, simptcp_get_total_len(pdu));
		sock->next_seq_num = 0;

    // printf("DEBUG: udp fd = %d\n", simptcp_entity.udp_fd);
    //simptcp_print_packet(pdu);
		
    free(pdu);

    int res = libc_sendto(simptcp_entity.udp_fd,
                     sock->out_buffer,
                     simptcp_get_total_len(sock->out_buffer),
                     0,
                     (struct sockaddr*) &(sock->remote_udp),
                     sizeof(struct sockaddr_in));
    
    

    if(res == -1)
    {
        printf("sendto: failed\n");
    }
    else
    {
        printf("sendto: success\n");
        start_timer(sock, getTimeoutDuration(sock));
        sock->socket_state = &(simptcp_entity.simptcp_socket_states->synsent);
    }
    return 0;
}


/*! \fn int closed_simptcp_socket_state_passive_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "listen" alors que le socket simpTCP est dans l'etat "closed"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param n  nbre max de demandes de connexion en attente (taille de la file des demandes de connexion)
 * \return  0 si succes, -1 si erreur
 */
int closed_simptcp_socket_state_passive_open (struct simptcp_socket* sock, int n)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
		// On passe à l'état listen.
		sock->socket_state = &(simptcp_entity.simptcp_socket_states->listen);
        sock->socket_type = listening_server;
        sock->pending_conn_req = 0;
        sock->max_conn_req_backlog = n;

        // socket_state_sender : soit wait ack soit message.
        // prend sa valeur une fois le socket à l'état established.
        
        if(sock->new_conn_req != NULL)
            free(sock->new_conn_req);

		sock->new_conn_req = (struct simptcp_socket **)malloc(n * sizeof(struct simptcp_socket*));
		// TODO : penser à free cette superbe magnifique, vraiment très jolie, variable. 
		// 			  merci, cordialement.
    return 0;
}


/*! \fn int closed_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
 * \brief lancee lorsque l'application lance l'appel "accept" alors que le socket simpTCP est dans l'etat "closed"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] addr pointeur sur l'adresse du socket distant de la connexion qui vient d'etre acceptee
 * len taille en octet de l'adresse du socket distant
 * \return 0 si succes, -1 si erreur/echec
 */
int closed_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    return 0;
}


/*! \fn ssize_t closed_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "send" alors que le socket simpTCP est dans l'etat "closed"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf  pointeur sur le message a transmettre
 * \param len taille en octet du message à transmettre
 * \param flags options
 * \return taille en octet du message envoye ; -1 sinon
 */
ssize_t closed_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;
}


/*! \fn ssize_t closed_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "recv" alors que le socket simpTCP est dans l'etat "closed"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] buf  pointeur sur le message recu
 * \param [out] len taille en octet du message recu
 * \param flags options
 * \return  taille en octet du message recu, -1 si echec
 */
ssize_t closed_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;
}

/**
 * called when application calls close
 */

/*! \fn  int closed_simptcp_socket_state_close (struct simptcp_socket* sock)
 * \brief lancee lorsque l'application lance l'appel "close" alors que le socket simpTCP est dans l'etat "closed"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \return  0 si succes, -1 si erreur
 */
int closed_simptcp_socket_state_close (struct simptcp_socket* sock)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    return 0;
}

/*! \fn  int closed_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
 * \brief lancee lorsque l'application lance l'appel "shutdown" alors que le socket simpTCP est dans l'etat "closed"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param how sens de fermeture de le connexion (en emisison, reception, emission et reception)
 * \return  0 si succes, -1 si erreur
 */
int closed_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/*!
 * \fn void closed_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
 * \brief lancee lorsque l'entite protocolaire demultiplexe un PDU simpTCP pour le socket simpTCP alors qu'il est dans l'etat "closed"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf pointeur sur le PDU simpTCP recu
 * \param len taille en octets du PDU recu
 */
void closed_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

}

/**
 * called after a timeout has detected
 */
/*!
 * \fn void closed_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
 * \brief lancee lors d'un timeout du timer du socket simpTCP alors qu'il est dans l'etat "closed"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 */
void closed_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    resendBuffer(sock);
}


/*********************************************************
 * listen_state functions *
 *********************************************************/

/*! \fn int listen_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "connect" alors que le socket simpTCP est dans l'etat "listen"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param addr adresse de niveau transport du socket simpTCP destination
 * \param len taille en octets de l'adresse de niveau transport du socket destination
 * \return  0 si succes, -1 si erreur
 */
int listen_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;
}

/**
 * called when application calls listen
 */
/*! \fn int listen_simptcp_socket_state_passive_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "listen" alors que le socket simpTCP est dans l'etat "listen"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param n  nbre max de demandes de connexion en attente (taille de la file des demandes de connexion)
 * \return  0 si succes, -1 si erreur
 */
int listen_simptcp_socket_state_passive_open (struct simptcp_socket* sock, int n)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    return 0;
}

/**
 * called when application calls accept
 */
/*! \fn int listen_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
 * \brief lancee lorsque l'application lance l'appel "accept" alors que le socket simpTCP est dans l'etat "listen"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] addr pointeur sur l'adresse du socket distant de la connexion qui vient d'etre acceptee
 * len taille en octet de l'adresse du socket distant
 * \return le nouveau fd si succes, -1 si erreur/echec
 */
int listen_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    // On attend tant qu'aucune connexion entrante n'est détectée.
    while(sock->pending_conn_req == 0) 
        usleep(100);
    // On récupère le socket à traiter.
    // #OnEstPasMalins : pending_conn_req - 1 !!!! sinon segfault.
    struct simptcp_socket * conn_req = sock->new_conn_req[sock->pending_conn_req - 1];
    memcpy(addr, &conn_req->remote_simptcp, sizeof(struct sockaddr_in));
    
    sock->pending_conn_req--;
    

    // ANCHOR_ACCEPT
    sock->next_seq_num = get_initial_seq_num();
    printf("****** SEND SYN/ACK : SEQ=%d, ACK=%d\n", sock->next_seq_num, sock->next_ack_num);

    // On a reçu un syn => on renvoie un syn ack
    char* pdu = simptcp_make_pdu(&sock->local_simptcp,
                                         &sock->remote_simptcp,
                                         NULL, // payload
                                         0, // len
                                         sock->next_seq_num, // seq
                                         sock->next_ack_num, // ack
                                         ACK | SYN);
    
	// Copie le pdu dans le out buffer.
    memcpy(sock->out_buffer, pdu, simptcp_get_total_len(pdu));

	// Envoie le pdu
	int res = sendto(simptcp_entity.udp_fd, sock->out_buffer,
				simptcp_get_total_len(sock->out_buffer), 0,
				(struct sockaddr*) &(sock->remote_udp),
				sizeof(struct sockaddr_in));

	free(pdu);
    
	// Si échec de l'envoi, on renvoie -1.
	if (res == -1)
    {
        printf("Echec de l'envoi !!!!\n");
		return -1;
    }

	// Lance le timer.
    start_timer(sock, getTimeoutDuration(sock));
	
    
    // Recherche du file descriptor
    int newfd = -1;
    int fd;
    for (fd=0; fd< MAX_OPEN_SOCK; fd++)
    {
        if (conn_req==simptcp_entity.simptcp_socket_descriptors[fd])
            newfd = fd;
    }

    // On update le numéro d'ack du socket fils. // ANCHOR_B
    struct simptcp_socket *newsock = simptcp_entity.simptcp_socket_descriptors[newfd];
    newsock->next_ack_num = sock->next_ack_num;
    printf("MON FILS TU AS SEQ=%d, ACK=%d\n", newsock->next_seq_num, newsock->next_ack_num);

    //print_simptcp_socket(simptcp_entity.simptcp_socket_descriptors[newfd]);
    conn_req->socket_state = &(simptcp_entity.simptcp_socket_states->synrcvd);
    // On attend la réception du ack, qui fera tout passer à established.
    while(conn_req->socket_state != &(simptcp_entity.simptcp_socket_states->established))
    {
        usleep(100);
    }

    return newfd;
}

/**
 * called when application calls send
 */
/*! \fn ssize_t listen_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "send" alors que le socket simpTCP est dans l'etat "listen"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf  pointeur sur le message a transmettre
 * \param len taille en octet du message à transmettre
 * \param flags options
 * \return taille en octet du message envoye ; -1 sinon
 */
ssize_t listen_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;
}

/**
 * called when application calls recv
 */
/*! \fn ssize_t listen_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "recv" alors que le socket simpTCP est dans l'etat "listen"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] buf  pointeur sur le message recu
 * \param [out] len taille en octet du message recu
 * \param flags options
 * \return  taille en octet du message recu, -1 si echec
 */
ssize_t listen_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    // Générer erreur
    return 0;

}

/**
 * called when application calls close
 */
/*! \fn  int listen_simptcp_socket_state_close (struct simptcp_socket* sock)
 * \brief lancee lorsque l'application lance l'appel "close" alors que le socket simpTCP est dans l'etat "listen"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \return  0 si succes, -1 si erreur
 */
int listen_simptcp_socket_state_close (struct simptcp_socket* sock)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls shutdown
 */
/*! \fn  int listen_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
 * \brief lancee lorsque l'application lance l'appel "shutdown" alors que le socket simpTCP est dans l'etat "listen"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param how sens de fermeture de le connexion (en emisison, reception, emission et reception)
 * \return  0 si succes, -1 si erreur
 */
int listen_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when library demultiplexed a packet to this particular socket
 */
/*!
 * \fn void listen_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
 * \brief lancee lorsque l'entite protocolaire demultiplexe un PDU simpTCP pour le socket simpTCP alors qu'il est dans l'etat "listen"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf pointeur sur le PDU simpTCP recu
 * \param len taille en octets du PDU recu
 */
void listen_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
{
    // ANCHOR LISTEN
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    // On check le checksum du message.
    if(!simptcp_check_checksum(buf, len))
    {
#if __DEBUG
        printf("corrupted packet\n");
        return -1;
#endif
    }
   
    // On vérifie que le message est un syn
    unsigned char flags = simptcp_get_flags(buf);

    // TODO : vérifier les seq numbers.

    if((flags & SYN) == SYN)
    {
        // Réception (ICI premier SYN)
        // On crée un nouveau socket dont on obtient le fd.
        int fd = create_simptcp_socket();
        struct simptcp_socket* newsock = simptcp_entity.simptcp_socket_descriptors[fd]; 
        newsock->socket_type = nonlistening_server; 
        newsock->remote_simptcp = sock->remote_simptcp;
        newsock->local_simptcp = sock->local_simptcp;
        newsock->remote_udp = sock->remote_udp;
        // Ajout le nouveau socket à la file des connexions et on incrémente
        // le nombre de connexions en cours.
        sock->new_conn_req[sock->pending_conn_req] = newsock;
        sock->pending_conn_req++;
        // On met àjour les numéros de séquence du nouveau socket.
        newsock->next_seq_num = sock->next_seq_num + 1;
        sock->next_ack_num = simptcp_get_seq_num(buf) + 1;

    }
    else if((flags & ACK) == ACK)
    {
        // On récupère le fils
        struct simptcp_socket* child = sock->new_conn_req[sock->pending_conn_req];
        child->socket_state = &(simptcp_entity.simptcp_socket_states->established);
        child->next_ack_num++;
        memset(&sock->remote_simptcp, 0, sizeof(struct sockaddr_in));

        sock->next_seq_num = get_initial_seq_num();
        sock->next_ack_num = 0;
        stop_timer(sock);
    }

    
}

/**
 * called after a timeout has detected
 */
/*!
 * \fn void listen_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
 * \brief lancee lors d'un timeout du timer du socket simpTCP alors qu'il est dans l'etat "listen"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 */
void listen_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    resendBuffer(sock);
}


/*********************************************************
 * synsent_state functions *
 *********************************************************/

/**
 * called when application calls connect
 */

/*! \fn int synsent_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "connect" alors que le socket simpTCP est dans l'etat "synsent"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param addr adresse de niveau transport du socket simpTCP destination
 * \param len taille en octets de l'adresse de niveau transport du socket destination
 * \return  0 si succes, -1 si erreur
 */
int synsent_simptcp_socket_state_active_open (struct  simptcp_socket* sock,struct sockaddr* addr, socklen_t len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls listen
 */
/*! \fn int synsent_simptcp_socket_state_passive_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "listen" alors que le socket simpTCP est dans l'etat "synsent"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param n  nbre max de demandes de connexion en attente (taille de la file des demandes de connexion)
 * \return  0 si succes, -1 si erreur
 */
int synsent_simptcp_socket_state_passive_open (struct simptcp_socket* sock, int n)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls accept
 */
/*! \fn int synsent_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
 * \brief lancee lorsque l'application lance l'appel "accept" alors que le socket simpTCP est dans l'etat "synsent"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] addr pointeur sur l'adresse du socket distant de la connexion qui vient d'etre acceptee
 * len taille en octet de l'adresse du socket distant
 * \return 0 si succes, -1 si erreur/echec
 */
int synsent_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls send
 */
/*! \fn ssize_t synsent_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "send" alors que le socket simpTCP est dans l'etat "synsent"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf  pointeur sur le message a transmettre
 * \param len taille en octet du message à transmettre
 * \param flags options
 * \return taille en octet du message envoye ; -1 sinon
 */
ssize_t synsent_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls recv
 */
/*! \fn ssize_t synsent_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "recv" alors que le socket simpTCP est dans l'etat "synsent"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] buf  pointeur sur le message recu
 * \param [out] len taille en octet du message recu
 * \param flagz options
 * \return  taille en octet du message recu, -1 si echec
 */
ssize_t synsent_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flagz)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;
}

/**
 * called when application calls close
 */
/*! \fn  int synsent_simptcp_socket_state_close (struct simptcp_socket* sock)
 * \brief lancee lorsque l'application lance l'appel "close" alors que le socket simpTCP est dans l'etat "synsent"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \return  0 si succes, -1 si erreur
 */
int synsent_simptcp_socket_state_close (struct simptcp_socket* sock)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls shutdown
 */
/*! \fn  int synsent_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
 * \brief lancee lorsque l'application lance l'appel "shutdown" alors que le socket simpTCP est dans l'etat "synsent"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param how sens de fermeture de le connexion (en emisison, reception, emission et reception)
 * \return  0 si succes, -1 si erreur
 */
int synsent_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when library demultiplexed a packet to this particular socket
 */
/*!
 * \fn void synsent_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
 * \brief lancee lorsque l'entite protocolaire demultiplexe un PDU simpTCP pour le socket simpTCP alors qu'il est dans l'etat "synsent"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf pointeur sur le PDU simpTCP recu
 * \param len taille en octets du PDU recu
 */
void synsent_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
{

    // ANCHOR SYNSENT
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    if(!simptcp_check_checksum(buf, len))
    { 
#if __DEBUG
        printf("corrupted packet");
        return -1;
#endif
    }

    // On copie le packet dans le in_buffer.
    memcpy(sock->in_buffer, buf, len);
    unsigned char flags = simptcp_get_flags(buf);
		
    if((flags & SYN) == SYN)
    {
		// Spécifie les bons numéros d'ack etc...
        sock->next_ack_num = simptcp_get_seq_num(buf) + 1;
        sock->next_seq_num = simptcp_get_ack_num(buf);

        printf("***** ACK: ACK=%d, SEQ=%d\n", sock->next_ack_num, sock->next_seq_num);
		// On a reçu un syn, => on renvoie un ack
        char* pdu = simptcp_make_pdu(&sock->local_simptcp,
                                     &sock->remote_simptcp,
                                     NULL, // payload
                                     0, // len
                                     sock->next_seq_num, // seq
                                     sock->next_ack_num, // ack
                                     ACK);

        // Copie le pdu dans le out buffer.
        memcpy(sock->out_buffer, pdu, simptcp_get_total_len(pdu));

        // Envoie le pdu [TODO : au fils !!]
        int res = sendto(simptcp_entity.udp_fd,
                         sock->out_buffer,
                         simptcp_get_total_len(sock->out_buffer),
                         0,
                         (struct sockaddr*) &(sock->remote_udp),
                         sizeof(struct sockaddr_in));

        free(pdu);

        // Si échec de l'envoi, on renvoie -1.
        if(res == -1)
            return;

        // Lance le timer.
        start_timer(sock, getTimeoutDuration(sock));

        if((flags & ACK) == ACK)
        {
            // On a reçu un syn ack
            sock->socket_state = &(simptcp_entity.simptcp_socket_states->established);
            stop_timer(sock);
            printf("Syn/Ack reçu => passage à established !\n");
        }
        else
        {
            // On a reçu seulement un ack.
            sock->socket_state = &(simptcp_entity.simptcp_socket_states->synrcvd);
        }
    }
}

/**
 * called after a timeout has detected
 */
/*!
 * \fn void synsent_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
 * \brief lancee lors d'un timeout du timer du socket simpTCP alors qu'il est dans l'etat "synsent"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 */
void synsent_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    resendBuffer(sock);

}


/*********************************************************
 * synrcvd_state functions *
 *********************************************************/

/**
 * called when application calls connect
 */

/*! \fn int synrcvd_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "connect" alors que le socket simpTCP est dans l'etat "synrcvd"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param addr adresse de niveau transport du socket simpTCP destination
 * \param len taille en octets de l'adresse de niveau transport du socket destination
 * \return  0 si succes, -1 si erreur
 */
int synrcvd_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls listen
 */
/*! \fn int synrcvd_simptcp_socket_state_passive_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "listen" alors que le socket simpTCP est dans l'etat "synrcvd"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param n  nbre max de demandes de connexion en attente (taille de la file des demandes de connexion)
 * \return  0 si succes, -1 si erreur
 */
int synrcvd_simptcp_socket_state_passive_open (struct simptcp_socket* sock, int n)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls accept
 */
/*! \fn int synrcvd_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
 * \brief lancee lorsque l'application lance l'appel "accept" alors que le socket simpTCP est dans l'etat "synrcvd"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] addr pointeur sur l'adresse du socket distant de la connexion qui vient d'etre acceptee
 * len taille en octet de l'adresse du socket distant
 * \return 0 si succes, -1 si erreur/echec
 */
int synrcvd_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls send
 */
/*! \fn ssize_t synrcvd_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "send" alors que le socket simpTCP est dans l'etat "synrcvd"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf  pointeur sur le message a transmettre
 * \param len taille en octet du message à transmettre
 * \param flags options
 * \return taille en octet du message envoye ; -1 sinon
 */
ssize_t synrcvd_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls recv
 */
/*! \fn ssize_t synrcvd_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "recv" alors que le socket simpTCP est dans l'etat "synrcvd"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] buf  pointeur sur le message recu
 * \param [out] len taille en octet du message recu
 * \param flags options
 * \return  taille en octet du message recu, -1 si echec
 */
ssize_t synrcvd_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flagz)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    if(!simptcp_check_checksum(buf, n))
    {
#if __DEBUG
        printf("corrupted packet\n");
        return -1;
#endif
    }

    /*// On copie le packet dans le in_buffer.
    memcpy(sock->in_buffer, buf, n);
    unsigned char flags = simptcp_get_flags(buf);
		
    if((flags & ACK) == ACK)
    {
				int ack_num = simptcp_get_seq_num(buf);	
				if(ack_num != sock->next_seq_num)
				{
					printf("Attendu ack=%d, obtenu ack=%d", sock->next_seq_num, 
									ack_num);
					return -1;
				}
				// Spécifie les bons numéros d'ack etc...
        sock->next_ack_num = ack_num + 1;

        // On a reçu un syn ack
        sock->socket_state = &(simptcp_entity.simptcp_socket_states->established);
    }*/
    printf("NNNNNNNNNNNNNNNNNNOOOOOOOOOOOOOO\n");
    return n;
}

/**
 * called when application calls close
 */
/*! \fn  int synrcvd_simptcp_socket_state_close (struct simptcp_socket* sock)
 * \brief lancee lorsque l'application lance l'appel "close" alors que le socket simpTCP est dans l'etat "synrcvd"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \return  0 si succes, -1 si erreur
 */
int synrcvd_simptcp_socket_state_close (struct simptcp_socket* sock)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls shutdown
 */
/*! \fn  int synrcvd_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
 * \brief lancee lorsque l'application lance l'appel "shutdown" alors que le socket simpTCP est dans l'etat "synrcvd"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param how sens de fermeture de le connexion (en emisison, reception, emission et reception)
 * \return  0 si succes, -1 si erreur
 */
int synrcvd_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when library demultiplexed a packet to this particular socket
 */
/*!
 * \fn void synrcvd_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
 * \brief lancee lorsque l'entite protocolaire demultiplexe un PDU simpTCP pour le socket simpTCP alors qu'il est dans l'etat "synrcvd"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf pointeur sur le PDU simpTCP recu
 * \param len taille en octets du PDU recu
 */
void synrcvd_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    // ANCHOR NOW
    if(!simptcp_check_checksum(buf, len))
    {
#if __DEBUG
        printf("corrupted packet\n");
        return -1;
#endif
    }


    // ANCHOR SYNRCVD
    // On copie le packet dans le in_buffer.
    memcpy(sock->in_buffer, buf, len);
    unsigned char flags = simptcp_get_flags(buf);
		
    if((flags & ACK) == ACK)
    {
        int ack_num = simptcp_get_seq_num(buf);
        printf("Attendu ack=%d, obtenu ack=%d\n", sock->next_seq_num,
               ack_num);

        if(ack_num != sock->next_seq_num)
        {
            return; 
        }

        // Spécifie les bons numéros d'ack etc...
        sock->next_ack_num = ack_num + 1;
        stop_timer(sock);
        // On a reçu un syn ack
        sock->socket_state = &(simptcp_entity.simptcp_socket_states->established);
    }

    return;

}

/**
 * called after a timeout has detected
 */
/*!
 * \fn void synrcvd_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
 * \brief lancee lors d'un timeout du timer du socket simpTCP alors qu'il est dans l'etat "synrcvd"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 */
void synrcvd_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    resendBuffer(sock);
}


/*********************************************************
 * established_state functions *
 *********************************************************/

/**
 * called when application calls connect
 */

/*! \fn int established_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "connect" alors que le socket simpTCP est dans l'etat "established"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param addr adresse de niveau transport du socket simpTCP destination
 * \param len taille en octets de l'adresse de niveau transport du socket destination
 * \return  0 si succes, -1 si erreur
 */
int established_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls listen
 */
/*! \fn int established_simptcp_socket_state_passive_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "listen" alors que le socket simpTCP est dans l'etat "established"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param n  nbre max de demandes de connexion en attente (taille de la file des demandes de connexion)
 * \return  0 si succes, -1 si erreur
 */
int established_simptcp_socket_state_passive_open (struct simptcp_socket* sock, int n)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls accept
 */
/*! \fn int established_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
 * \brief lancee lorsque l'application lance l'appel "accept" alors que le socket simpTCP est dans l'etat "established"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] addr pointeur sur l'adresse du socket distant de la connexion qui vient d'etre acceptee
 * len taille en octet de l'adresse du socket distant
 * \return 0 si succes, -1 si erreur/echec
 */
int established_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls send
 */
/*! \fn ssize_t established_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "send" alors que le socket simpTCP est dans l'etat "established"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf  pointeur sur le message a transmettre
 * \param len taille en octet du message à transmettre
 * \param flags options
 * \return taille en octet du message envoye ; -1 sinon
 */
ssize_t established_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    // ANCHOR SEND
    // Envoi depuis un client

    // Numéro de séquence du premier pdu
    sock->next_seq_num++;

    char *pdu = simptcp_make_pdu(&sock->local_simptcp,
                                 &sock->remote_simptcp,
                                 buf, // payload
                                 n, // len
                                 sock->next_seq_num, // seq
                                 sock->next_ack_num, // ack
                                 0);

    // Copie le pdu dans le out buffer.
    memcpy(sock->out_buffer, pdu, simptcp_get_total_len(pdu));

    free(pdu);

    int oldAckNum = sock->next_ack_num;
    int res = libc_sendto(simptcp_entity.udp_fd,
                          sock->out_buffer,
                          simptcp_get_total_len(sock->out_buffer),
                          0,
                          (struct sockaddr *) &(sock->remote_udp),
                          sizeof(struct sockaddr_in));

    printf("***** SEND: SEQ=%d, ACK=%d\n", sock->next_seq_num, sock->next_ack_num);

    // Si le client fait plusieurs send rapidement, on attend le ack avant de lancer le prochain
    // send.
    while (sock->next_ack_num == oldAckNum) {
        usleep(100);
    }

    printf("***** OUT OF SEND. \n");
    return 0;
}
/**
 * called when application calls recv
 */
/*! \fn ssize_t established_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "recv" alors que le socket simpTCP est dans l'etat "established"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] buf  pointeur sur le message recu
 * \param [out] len taille en octet du message recu
 * \param flags options
 * \return  taille en octet du message recu, -1 si echec
 */
ssize_t established_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    // Attente de réception du paquet.
    while (sock->in_len == 0) {
        usleep(100);
    }

    // Quand on a un paquet => on le donne à l'user.
    int hlen = simptcp_get_head_len(sock->in_buffer);
    int length = sock->in_len - hlen;
    length = length <= n ? length : n;

    memcpy(buf, (sock->in_buffer + hlen), length);

    sock->in_len = 0;

    return length;
}

/**
 * called when application calls close
 */
/*! \fn  int established_simptcp_socket_state_close (struct simptcp_socket* sock)
 * \brief lancee lorsque l'application lance l'appel "close" alors que le socket simpTCP est dans l'etat "established"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \return  0 si succes, -1 si erreur
 */
int established_simptcp_socket_state_close (struct simptcp_socket* sock)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    return 0;

}

/**
 * called when application calls shutdown
 */
/*! \fn  int established_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
 * \brief lancee lorsque l'application lance l'appel "shutdown" alors que le socket simpTCP est dans l'etat "established"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param how sens de fermeture de le connexion (en emisison, reception, emission et reception)
 * \return  0 si succes, -1 si erreur
 */
int established_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    // ANCHOR CLOSE

    // On suppose que c'est le client qui ferme la connexion.
    if (sock->socket_type == nonlistening_server) {
        printf("***** WAITING FOR FIN FROM CLIENT. \n");
        while (sock->socket_state != &(simptcp_entity.simptcp_socket_states->closewait)) {
            usleep(500);
        }
        printf("***** CLOSE CALL DONE GO YO LAST ACK. \n");
        sock->socket_state = &(simptcp_entity.simptcp_socket_states->closewait);
        closewait_simptcp_socket_state_shutdown(sock, how);

        // On attend le dernier ack
        while (sock->socket_state != &(simptcp_entity.simptcp_socket_states->closed)) {
            usleep(500);
        }
        return 0;
    }
    else if (sock->socket_type == client) {
        // Incrémentation du seq number
        sock->next_seq_num++;
        // On a reçu un syn, => on renvoie un ack
        char *pdu = simptcp_make_pdu(&sock->local_simptcp,
                                     &sock->remote_simptcp,
                                     NULL, // payload
                                     0, // len
                                     sock->next_seq_num, // seq
                                     sock->next_ack_num, // ack
                                     FIN);


        // Copie le pdu dans le out buffer.
        memcpy(sock->out_buffer, pdu, simptcp_get_total_len(pdu));

        // Envoie le pdu
        int res = sendto(simptcp_entity.udp_fd,
                         sock->out_buffer,
                         simptcp_get_total_len(sock->out_buffer),
                         0,
                         (struct sockaddr *) &(sock->remote_udp),
                         sizeof(struct sockaddr_in));

        // Gestion de l'erreur.
        if (res == -1)
            return -1;

        free(pdu);


        sock->socket_state = &(simptcp_entity.simptcp_socket_states->finwait1);
        printf("***** FIN SENT | WAITING FOR END OF PROTOCOL TO EXIT FUNCTION. \n");
        while (sock->socket_state != &(simptcp_entity.simptcp_socket_states->closed)) {
            usleep(500);
        }
        printf("***** SOCKET CLOSED PROPERLY !!\n");
    }
    return 0;

}



/**
 * called when library demultiplexed a packet to this particular socket
 */
/*!
 * \fn void established_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
 * \brief lancee lorsque l'entite protocolaire demultiplexe un PDU simpTCP pour le socket simpTCP alors qu'il est dans l'etat "established"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf pointeur sur le PDU simpTCP recu
 * \param len taille en octets du PDU recu
 */
void established_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    // ANCHOR PROCESS

    int seq = simptcp_get_seq_num(buf);
    int expected = sock->next_ack_num;

    if (sock->socket_type == nonlistening_server) {
        printf("***** PKT RECU: SEQ=%d, ACK=%d\n", simptcp_get_seq_num(buf), simptcp_get_ack_num(buf));

        if (seq == expected) {
            sock->next_ack_num++;
            printf("Good sequence number : expected %d, got %d\n", expected, seq);
            // Cas où on reçoit un paquet
            // 1. On stocke le paquet dans le in buffer.
            int length = len < SIMPTCP_SOCKET_MAX_BUFFER_SIZE ? len : MAX_SIMPTCP_BUFFER_SIZE;
            memcpy(sock->in_buffer, buf, length);
            sock->in_len = len;

            // 2. on renvoie un ack.
            char *pdu = simptcp_make_pdu(&sock->local_simptcp,
                                         &sock->remote_simptcp,
                                         NULL, // payload
                                         0, // len
                                         sock->next_seq_num, // seq
                                         sock->next_ack_num, // ack
                                         ACK);

            // Copie le pdu dans le out buffer.
            memcpy(sock->out_buffer, pdu, simptcp_get_total_len(pdu));

            free(pdu);

            int res = libc_sendto(simptcp_entity.udp_fd,
                                  sock->out_buffer,
                                  simptcp_get_total_len(sock->out_buffer),
                                  0,
                                  (struct sockaddr *) &(sock->remote_udp),
                                  sizeof(struct sockaddr_in));

            printf("***** ACK SENT: SEQ=%d, ACK=%d (res = %d)\n", sock->next_seq_num, sock->next_ack_num, res);

            // On incrémente le prochain seq number.
            sock->next_seq_num++;

            // Si le paquet est un FIN => on passe dans l'état closewait.
            if ((simptcp_get_flags(buf) & FIN) == FIN) {
                sock->socket_state = &(simptcp_entity.simptcp_socket_states->closewait);
            }
        }
        else {
            printf("Bad sequence number : expected %d, got %d\n", expected, seq);
        }



    }
    else if (sock->socket_type == client) // client
    {
        printf("Sequence number : expected %d, got %d\n", expected, seq);
        // Réception du ack.
        if (seq == expected && ((simptcp_get_flags(buf) & ACK) == ACK)) {
            sock->next_ack_num++;
            stop_timer(sock);
        }
        else {
            printf("BAD ACK\n");
        }
    }
    else {
        printf("WTF ????????????????\n");
    }


}

/**
 * called after a timeout has detected
 */
/*!
 * \fn void established_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
 * \brief lancee lors d'un timeout du timer du socket simpTCP alors qu'il est dans l'etat "established"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 */
void established_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif


    resendBuffer(sock);
}


/*********************************************************
 * closewait_state functions *
 *********************************************************/

/**
 * called when application calls connect
 */

/*! \fn int closewait_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "connect" alors que le socket simpTCP est dans l'etat "closewait"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param addr adresse de niveau transport du socket simpTCP destination
 * \param len taille en octets de l'adresse de niveau transport du socket destination
 * \return  0 si succes, -1 si erreur
 */
int closewait_simptcp_socket_state_active_open (struct  simptcp_socket* sock,  struct sockaddr* addr, socklen_t len)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls listen
 */
/*! \fn int closewait_simptcp_socket_state_passive_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "listen" alors que le socket simpTCP est dans l'etat "closewait"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param n  nbre max de demandes de connexion en attente (taille de la file des demandes de connexion)
 * \return  0 si succes, -1 si erreur
 */
int closewait_simptcp_socket_state_passive_open (struct simptcp_socket* sock, int n)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls accept
 */
/*! \fn int closewait_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
 * \brief lancee lorsque l'application lance l'appel "accept" alors que le socket simpTCP est dans l'etat "closewait"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] addr pointeur sur l'adresse du socket distant de la connexion qui vient d'etre acceptee
 * len taille en octet de l'adresse du socket distant
 * \return 0 si succes, -1 si erreur/echec
 */
int closewait_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls send
 */
/*! \fn ssize_t closewait_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "send" alors que le socket simpTCP est dans l'etat "closewait"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf  pointeur sur le message a transmettre
 * \param len taille en octet du message à transmettre
 * \param flags options
 * \return taille en octet du message envoye ; -1 sinon
 */
ssize_t closewait_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls recv
 */
/*! \fn ssize_t closewait_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "recv" alors que le socket simpTCP est dans l'etat "closewait"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] buf  pointeur sur le message recu
 * \param [out] len taille en octet du message recu
 * \param flags options
 * \return  taille en octet du message recu, -1 si echec
 */
ssize_t closewait_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls close
 */
/*! \fn  int closewait_simptcp_socket_state_close (struct simptcp_socket* sock)
 * \brief lancee lorsque l'application lance l'appel "close" alors que le socket simpTCP est dans l'etat "closewait"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \return  0 si succes, -1 si erreur
 */
int closewait_simptcp_socket_state_close (struct simptcp_socket* sock)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    return 0;
}

/**
 * called when application calls shutdown
 */
/*! \fn  int closewait_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
 * \brief lancee lorsque l'application lance l'appel "shutdown" alors que le socket simpTCP est dans l'etat "closewait"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param how sens de fermeture de le connexion (en emisison, reception, emission et reception)
 * \return  0 si succes, -1 si erreur
 */
int closewait_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    // ANCHOR CLOSEWAIT
    char *pdu = simptcp_make_pdu(&sock->local_simptcp,
                                 &sock->remote_simptcp,
                                 NULL, // payload
                                 0, // len
                                 sock->next_seq_num, // seq
                                 sock->next_ack_num, // ack
                                 FIN);

    // Copie le pdu dans le out buffer.
    memcpy(sock->out_buffer, pdu, simptcp_get_total_len(pdu));
    sock->next_seq_num++;


    free(pdu);

    int res = libc_sendto(simptcp_entity.udp_fd,
                          sock->out_buffer,
                          simptcp_get_total_len(sock->out_buffer),
                          0,
                          (struct sockaddr *) &(sock->remote_udp),
                          sizeof(struct sockaddr_in));

    // Gestion de l'erreur.
    if (res == -1)
        return res;

    start_timer(sock, getTimeoutDuration(sock));
    sock->socket_state = &(simptcp_entity.simptcp_socket_states->lastack);
    printf("***** CLOSE CALL RECEIVED. GO TO LAST ACK.\n");
    return 0;

}

/**
 * called when library demultiplexed a packet to this particular socket
 */
/*!
 * \fn void closewait_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
 * \brief lancee lorsque l'entite protocolaire demultiplexe un PDU simpTCP pour le socket simpTCP alors qu'il est dans l'etat "closewait"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf pointeur sur le PDU simpTCP recu
 * \param len taille en octets du PDU recu
 */
void closewait_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

}

/**
 * called after a timeout has detected
 */
/*!
 * \fn void closewait_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
 * \brief lancee lors d'un timeout du timer du socket simpTCP alors qu'il est dans l'etat "closewait"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 */
void closewait_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    resendBuffer(sock);
}


/*********************************************************
 * finwait1_state functions *
 *********************************************************/

/**
 * called when application calls connect
 */

/*! \fn int finwait1_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "connect" alors que le socket simpTCP est dans l'etat "finwait1"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param addr adresse de niveau transport du socket simpTCP destination
 * \param len taille en octets de l'adresse de niveau transport du socket destination
 * \return  0 si succes, -1 si erreur
 */
int finwait1_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls listen
 */
/*! \fn int finwait1_simptcp_socket_state_passive_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "listen" alors que le socket simpTCP est dans l'etat "finwait1"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param n  nbre max de demandes de connexion en attente (taille de la file des demandes de connexion)
 * \return  0 si succes, -1 si erreur
 */
int finwait1_simptcp_socket_state_passive_open (struct simptcp_socket* sock, int n)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;
}

/**
 * called when application calls accept
 */
/*! \fn int finwait1_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
 * \brief lancee lorsque l'application lance l'appel "accept" alors que le socket simpTCP est dans l'etat "finwait1"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] addr pointeur sur l'adresse du socket distant de la connexion qui vient d'etre acceptee
 * len taille en octet de l'adresse du socket distant
 * \return 0 si succes, -1 si erreur/echec
 */
int finwait1_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls send
 */
/*! \fn ssize_t finwait1_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "send" alors que le socket simpTCP est dans l'etat "finwait1"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf  pointeur sur le message a transmettre
 * \param len taille en octet du message à transmettre
 * \param flags options
 * \return taille en octet du message envoye ; -1 sinon
 */
ssize_t finwait1_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls recv
 */
/*! \fn ssize_t finwait1_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "recv" alors que le socket simpTCP est dans l'etat "finwait1"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] buf  pointeur sur le message recu
 * \param [out] len taille en octet du message recu
 * \param flags options
 * \return  taille en octet du message recu, -1 si echec
 */
ssize_t finwait1_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls close
 */
/*! \fn  int finwait1_simptcp_socket_state_close (struct simptcp_socket* sock)
 * \brief lancee lorsque l'application lance l'appel "close" alors que le socket simpTCP est dans l'etat "finwait1"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \return  0 si succes, -1 si erreur
 */
int finwait1_simptcp_socket_state_close (struct simptcp_socket* sock)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls shutdown
 */
/*! \fn  int finwait1_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
 * \brief lancee lorsque l'application lance l'appel "shutdown" alors que le socket simpTCP est dans l'etat "finwait1"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param how sens de fermeture de le connexion (en emisison, reception, emission et reception)
 * \return  0 si succes, -1 si erreur
 */
int finwait1_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when library demultiplexed a packet to this particular socket
 */
/*!
 * \fn void finwait1_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
 * \brief lancee lorsque l'entite protocolaire demultiplexe un PDU simpTCP pour le socket simpTCP alors qu'il est dans l'etat "finwait1"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf pointeur sur le PDU simpTCP recu
 * \param len taille en octets du PDU recu
 */
void finwait1_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    // ANCHOR FINWAIT1
    unsigned char flags = simptcp_get_flags(buf);
    int expected = sock->next_ack_num;
    if (simptcp_get_seq_num(buf) == expected) {
        if ((flags & ACK) == ACK) {

            // Spécifie les bons numéros d'ack etc...
            sock->next_ack_num = simptcp_get_seq_num(buf) + 1;
            // On a reçu un ack of fin
            sock->socket_state = &(simptcp_entity.simptcp_socket_states->finwait2);
            stop_timer(sock);
            printf("***** ACK OF FIN RECEIVED\n");
        }
        else {
            printf("***** UNEXPECTED PACKET IN FINWAIT1\n");
        }
    }
    else {
        printf("BAD SEQ : expected %d, got %d\n", expected, simptcp_get_seq_num(buf));
    }
}

/**
 * called after a timeout has detected
 */
/*!
 * \fn void finwait1_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
 * \brief lancee lors d'un timeout du timer du socket simpTCP alors qu'il est dans l'etat "finwait1"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 */
void finwait1_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    resendBuffer(sock);
}


/*********************************************************
 * finwait2_state functions *
 *********************************************************/

/**
 * called when application calls connect
 */

/*! \fn int finwait2_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "connect" alors que le socket simpTCP est dans l'etat "fainwait2"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param addr adresse de niveau transport du socket simpTCP destination
 * \param len taille en octets de l'adresse de niveau transport du socket destination
 * \return  0 si succes, -1 si erreur
 */
int finwait2_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls listen
 */
/*! \fn int finwait2_simptcp_socket_state_passive_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "listen" alors que le socket simpTCP est dans l'etat "finwait2"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param n  nbre max de demandes de connexion en attente (taille de la file des demandes de connexion)
 * \return  0 si succes, -1 si erreur
 */
int finwait2_simptcp_socket_state_passive_open (struct simptcp_socket* sock, int n)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls accept
 */
/*! \fn int finwait2_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
 * \brief lancee lorsque l'application lance l'appel "accept" alors que le socket simpTCP est dans l'etat "finwait2"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] addr pointeur sur l'adresse du socket distant de la connexion qui vient d'etre acceptee
 * len taille en octet de l'adresse du socket distant
 * \return 0 si succes, -1 si erreur/echec
 */
int finwait2_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls send
 */
/*! \fn ssize_t finwait2_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "send" alors que le socket simpTCP est dans l'etat "finwait2"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf  pointeur sur le message a transmettre
 * \param len taille en octet du message à transmettre
 * \param flags options
 * \return taille en octet du message envoye ; -1 sinon
 */
ssize_t finwait2_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls recv
 */
/*! \fn ssize_t finwait2_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "recv" alors que le socket simpTCP est dans l'etat "finwait2"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] buf  pointeur sur le message recu
 * \param [out] len taille en octet du message recu
 * \param flags options
 * \return  taille en octet du message recu, -1 si echec
 */
ssize_t finwait2_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls close
 */
/*! \fn  int finwait2_simptcp_socket_state_close (struct simptcp_socket* sock)
 * \brief lancee lorsque l'application lance l'appel "close" alors que le socket simpTCP est dans l'etat "finwait2"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \return  0 si succes, -1 si erreur
 */
int finwait2_simptcp_socket_state_close (struct simptcp_socket* sock)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls shutdown
 */
/*! \fn  int finwait2_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
 * \brief lancee lorsque l'application lance l'appel "shutdown" alors que le socket simpTCP est dans l'etat "finwait2"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param how sens de fermeture de le connexion (en emisison, reception, emission et reception)
 * \return  0 si succes, -1 si erreur
 */
int finwait2_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when library demultiplexed a packet to this particular socket
 */
/*!
 * \fn void finwait2_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
 * \brief lancee lorsque l'entite protocolaire demultiplexe un PDU simpTCP pour le socket simpTCP alors qu'il est dans l'etat "finwait2"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf pointeur sur le PDU simpTCP recu
 * \param len taille en octets du PDU recu
 */
void finwait2_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    // ANCHOR FINWAIT2
    unsigned char flags = simptcp_get_flags(buf);
    int expected = sock->next_ack_num;
    if (simptcp_get_seq_num(buf) == expected) {
        if ((flags & FIN) == FIN) {

            // On envoie le ACK
            sock->next_seq_num++;
            char *pdu = simptcp_make_pdu(&sock->local_simptcp,
                                         &sock->remote_simptcp,
                                         NULL, // payload
                                         0, // len
                                         sock->next_seq_num, // seq
                                         sock->next_ack_num, // ack
                                         ACK);
            memcpy(sock->out_buffer, pdu, simptcp_get_total_len(pdu));
            free(pdu);
            int res = libc_sendto(simptcp_entity.udp_fd,
                                  sock->out_buffer,
                                  simptcp_get_total_len(sock->out_buffer),
                                  0,
                                  (struct sockaddr *) &(sock->remote_udp),
                                  sizeof(struct sockaddr_in));

            if (res == -1) {
                printf("ERROR: Sending FIN failed.\n");
                return;
            }

            // Spécifie les bons numéros d'ack etc...
            sock->next_ack_num = simptcp_get_seq_num(buf) + 1;
            // On a reçu un ack of fin
            sock->socket_state = &(simptcp_entity.simptcp_socket_states->timewait);
            printf("***** FIN RECEIVED | ACK OF FIN SENT\n");

            start_timer(sock, getTimeoutDuration(sock));
        }
    }
    else {
        printf("BAD SEQ : expected %d, got %d\n", expected, simptcp_get_seq_num(buf));
    }
}

/**
 * called after a timeout has detected
 */
/*!
 * \fn void finwait2_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
 * \brief lancee lors d'un timeout du timer du socket simpTCP alors qu'il est dans l'etat "finwait2"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 */
void finwait2_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    sock->socket_state = &(simptcp_entity.simptcp_socket_states->closed);

    // DELETE TCB.
    free(sock->in_buffer);
    free(sock->out_buffer);
    stop_timer(sock);
}


/*********************************************************
 * closing_state functions *
 *********************************************************/

/**
 * called when application calls connect
 */

/*! \fn int closing_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "connect" alors que le socket simpTCP est dans l'etat "closing"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param addr adresse de niveau transport du socket simpTCP destination
 * \param len taille en octets de l'adresse de niveau transport du socket destination
 * \return  0 si succes, -1 si erreur
 */
int closing_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls listen
 */
/*! \fn int closing_simptcp_socket_state_passive_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "listen" alors que le socket simpTCP est dans l'etat "closing"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param n  nbre max de demandes de connexion en attente (taille de la file des demandes de connexion)
 * \return  0 si succes, -1 si erreur
 */
int closing_simptcp_socket_state_passive_open (struct simptcp_socket* sock, int n)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls accept
 */
/*! \fn int closing_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
 * \brief lancee lorsque l'application lance l'appel "accept" alors que le socket simpTCP est dans l'etat "closing"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] addr pointeur sur l'adresse du socket distant de la connexion qui vient d'etre acceptee
 * len taille en octet de l'adresse du socket distant
 * \return 0 si succes, -1 si erreur/echec
 */
int closing_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls send
 */
/*! \fn ssize_t closing_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "send" alors que le socket simpTCP est dans l'etat "closing"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf  pointeur sur le message a transmettre
 * \param len taille en octet du message à transmettre
 * \param flags options
 * \return taille en octet du message envoye ; -1 sinon
 */
ssize_t closing_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls recv
 */
/*! \fn ssize_t closing_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "recv" alors que le socket simpTCP est dans l'etat "closing"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] buf  pointeur sur le message recu
 * \param [out] len taille en octet du message recu
 * \param flags options
 * \return  taille en octet du message recu, -1 si echec
 */
ssize_t closing_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls close
 */
/*! \fn  int closing_simptcp_socket_state_close (struct simptcp_socket* sock)
 * \brief lancee lorsque l'application lance l'appel "close" alors que le socket simpTCP est dans l'etat "closing"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \return  0 si succes, -1 si erreur
 */
int closing_simptcp_socket_state_close (struct simptcp_socket* sock)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls shutdown
 */
/*! \fn  int closing_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
 * \brief lancee lorsque l'application lance l'appel "shutdown" alors que le socket simpTCP est dans l'etat "closing"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param how sens de fermeture de le connexion (en emisison, reception, emission et reception)
 * \return  0 si succes, -1 si erreur
 */
int closing_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when library demultiplexed a packet to this particular socket
 */
/*!
 * \fn void closing_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
 * \brief lancee lorsque l'entite protocolaire demultiplexe un PDU simpTCP pour le socket simpTCP alors qu'il est dans l'etat "closing"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf pointeur sur le PDU simpTCP recu
 * \param len taille en octets du PDU recu
 */
void closing_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
}

/**
 * called after a timeout has detected
 */
/*!
 * \fn void closing_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
 * \brief lancee lors d'un timeout du timer du socket simpTCP alors qu'il est dans l'etat "closing"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 */
void closing_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    resendBuffer(sock);
}


/*********************************************************
 * lastack_state functions *
 *********************************************************/

/**
 * called when application calls connect
 */

/*! \fn int lastack_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "connect" alors que le socket simpTCP est dans l'etat "lastack"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param addr adresse de niveau transport du socket simpTCP destination
 * \param len taille en octets de l'adresse de niveau transport du socket destination
 * \return  0 si succes, -1 si erreur
 */
int lastack_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls listen
 */
/*! \fn int lastack_simptcp_socket_state_passive_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "listen" alors que le socket simpTCP est dans l'etat "lastack"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param n  nbre max de demandes de connexion en attente (taille de la file des demandes de connexion)
 * \return  0 si succes, -1 si erreur
 */
int lastack_simptcp_socket_state_passive_open (struct simptcp_socket* sock, int n)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls accept
 */
/*! \fn int lastack_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
 * \brief lancee lorsque l'application lance l'appel "accept" alors que le socket simpTCP est dans l'etat "lastack"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] addr pointeur sur l'adresse du socket distant de la connexion qui vient d'etre acceptee
 * len taille en octet de l'adresse du socket distant
 * \return 0 si succes, -1 si erreur/echec
 */
int lastack_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls send
 */
/*! \fn ssize_t lastack_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "send" alors que le socket simpTCP est dans l'etat "lastack"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf  pointeur sur le message a transmettre
 * \param len taille en octet du message à transmettre
 * \param flags options
 * \return taille en octet du message envoye ; -1 sinon
 */
ssize_t lastack_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls recv
 */
/*! \fn ssize_t lastack_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "recv" alors que le socket simpTCP est dans l'etat "lastack"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] buf  pointeur sur le message recu
 * \param [out] len taille en octet du message recu
 * \param flags options
 * \return  taille en octet du message recu, -1 si echec
 */
ssize_t lastack_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls close
 */
/*! \fn  int lastack_simptcp_socket_state_close (struct simptcp_socket* sock)
 * \brief lancee lorsque l'application lance l'appel "close" alors que le socket simpTCP est dans l'etat "lastack"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \return  0 si succes, -1 si erreur
 */
int lastack_simptcp_socket_state_close (struct simptcp_socket* sock)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls shutdown
 */
/*! \fn  int lastack_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
 * \brief lancee lorsque l'application lance l'appel "shutdown" alors que le socket simpTCP est dans l'etat "lastack"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param how sens de fermeture de le connexion (en emisison, reception, emission et reception)
 * \return  0 si succes, -1 si erreur
 */
int lastack_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when library demultiplexed a packet to this particular socket
 */
/*!
 * \fn void lastack_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
 * \brief lancee lorsque l'entite protocolaire demultiplexe un PDU simpTCP pour le socket simpTCP alors qu'il est dans l'etat "lastack"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf pointeur sur le PDU simpTCP recu
 * \param len taille en octets du PDU recu
 */
void lastack_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    // ANCHOR LASTACK
    unsigned char flags = simptcp_get_flags(buf);
    if (checkSequenceNumber(sock, buf) && ((flags & ACK) == ACK)) {
        sock->socket_state = &(simptcp_entity.simptcp_socket_states->closed);
        stop_timer(sock);
        printf("****** SOCKET CLOSED PROPERLY.\n");
    }

}

/**
 * called after a timeout has detected
 */
/*!
 * \fn void lastack_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
 * \brief lancee lors d'un timeout du timer du socket simpTCP alors qu'il est dans l'etat "lastack"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 */
void lastack_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    resendBuffer(sock);
}


/*********************************************************
 * timewait_state functions *
 *********************************************************/

/**
 * called when application calls connect
 */


/*! \fn int timewait_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "connect" alors que le socket simpTCP est dans l'etat "timewait"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param addr adresse de niveau transport du socket simpTCP destination
 * \param len taille en octets de l'adresse de niveau transport du socket destination
 * \return  0 si succes, -1 si erreur
 */
int timewait_simptcp_socket_state_active_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls listen
 */
/*! \fn int timewait_simptcp_socket_state_passive_open (struct  simptcp_socket* sock, struct sockaddr* addr, socklen_t len)
 * \brief lancee lorsque l'application lance l'appel "listen" alors que le socket simpTCP est dans l'etat "timewait"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param n  nbre max de demandes de connexion en attente (taille de la file des demandes de connexion)
 * \return  0 si succes, -1 si erreur
 */
int timewait_simptcp_socket_state_passive_open (struct simptcp_socket* sock, int n)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls accept
 */
/*! \fn int timewait_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
 * \brief lancee lorsque l'application lance l'appel "accept" alors que le socket simpTCP est dans l'etat "timewait"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] addr pointeur sur l'adresse du socket distant de la connexion qui vient d'etre acceptee
 * len taille en octet de l'adresse du socket distant
 * \return 0 si succes, -1 si erreur/echec
 */
int timewait_simptcp_socket_state_accept (struct simptcp_socket* sock, struct sockaddr* addr, socklen_t* len)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls send
 */
/*! \fn ssize_t timewait_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "send" alors que le socket simpTCP est dans l'etat "timewait"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf  pointeur sur le message a transmettre
 * \param len taille en octet du message à transmettre
 * \param flags options
 * \return taille en octet du message envoye ; -1 sinon
 */
ssize_t timewait_simptcp_socket_state_send (struct simptcp_socket* sock, const void *buf, size_t n, int flags)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls recv
 */
/*! \fn ssize_t timewait_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
 * \brief lancee lorsque l'application lance l'appel "recv" alors que le socket simpTCP est dans l'etat "timewait"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param [out] buf  pointeur sur le message recu
 * \param [out] len taille en octet du message recu
 * \param flags options
 * \return  taille en octet du message recu, -1 si echec
 */
ssize_t timewait_simptcp_socket_state_recv (struct simptcp_socket* sock, void *buf, size_t n, int flags)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls close
 */
/*! \fn  int timewait_simptcp_socket_state_close (struct simptcp_socket* sock)
 * \brief lancee lorsque l'application lance l'appel "close" alors que le socket simpTCP est dans l'etat "timewait"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \return  0 si succes, -1 si erreur
 */
int timewait_simptcp_socket_state_close (struct simptcp_socket* sock)
{

#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when application calls shutdown
 */
/*! \fn  int timewait_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
 * \brief lancee lorsque l'application lance l'appel "shutdown" alors que le socket simpTCP est dans l'etat "timewait"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param how sens de fermeture de le connexion (en emisison, reception, emission et reception)
 * \return  0 si succes, -1 si erreur
 */
int timewait_simptcp_socket_state_shutdown (struct simptcp_socket* sock, int how)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
    return 0;

}

/**
 * called when library demultiplexed a packet to this particular socket
 */
/*!
 * \fn void timewait_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
 * \brief lancee lorsque l'entite protocolaire demultiplexe un PDU simpTCP pour le socket simpTCP alors qu'il est dans l'etat "timewait"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 * \param buf pointeur sur le PDU simpTCP recu
 * \param len taille en octets du PDU recu
 */
void timewait_simptcp_socket_state_process_simptcp_pdu (struct simptcp_socket* sock, void* buf, int len)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif
}

/**
 * called after a timeout has detected
 */
/*!
 * \fn void timewait_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
 * \brief lancee lors d'un timeout du timer du socket simpTCP alors qu'il est dans l'etat "timewait"
 * \param sock pointeur sur les variables d'etat (#simptcp_socket) du socket simpTCP
 */
void timewait_simptcp_socket_state_handle_timeout (struct simptcp_socket* sock)
{
#if __DEBUG__
    printf("function %s called\n", __func__);
#endif

    // ANCHOR TIMEWAIT TIMEOUT
    sock->socket_state = &(simptcp_entity.simptcp_socket_states->closed);
    stop_timer(sock);
}


