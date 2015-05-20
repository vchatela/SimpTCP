# SimpTCP
Développer en langage C un protocole de la couche transport de l'architecture TCP/IP appelé simpTCP réplique simplifiée du protocole TCP (Transmission Control Protocol).

Les caractéristiques générales du service SimpTCP sont :

    service avec connexion
    unidirectionnel
    orienté message
    avec garantie d'ordre et de fiabilité

L'accès au service SimpTCP se fait via  API socket, avec les structures de données et appels systèmes classiques déjà utilisés en spécifiant le protocole SimpTCP à la place de UDP et TCP.
Le protocole SimpTCP à développer :

    repose sur le service UDP et non pas IP pour échanger ses PDU (voir figure)
    gère l'établissement et la libération de la connexion tq définis dans TCP (voir RFC 793)
    gère une certaine forme de  multiplexage/démultiplexage
    repose sur le mécanisme de contrôle d'erreurs de paquet « stop-and-wait"
     ne cherche pas à estimer précisément la valeur du RTT qui est supposée alimenter le timer de retransmission
    ne prend pas en compte des problèmes de bouclage de la numérotation de séquence et d'acquittement
    n'intègre pas les mécanismes de contrôle de flux et de contrôle de congestion

