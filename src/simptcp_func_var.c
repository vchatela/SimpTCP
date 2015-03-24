#include <simptcp_lib.h>


/* The following declarations and definitions define all the
 * functions for each of the simptc_socket_state_func structs
 * Many of them respond by returning an error
 * For example it is illegal to call send or recv on an initial
 * socket
 */

/* closed state functions */
static simptcp_socket_state_active_open closed_simptcp_socket_state_active_open;
static simptcp_socket_state_passive_open closed_simptcp_socket_state_passive_open;
static simptcp_socket_state_accept closed_simptcp_socket_state_accept;
static simptcp_socket_state_send closed_simptcp_socket_state_send;
static simptcp_socket_state_recv closed_simptcp_socket_state_recv;
static simptcp_socket_state_close closed_simptcp_socket_state_close;
static  simptcp_socket_state_shutdown closed_simptcp_socket_state_shutdown;
static simptcp_socket_state_process_pdu closed_simptcp_socket_state_process_simptcp_pdu;
static simptcp_socket_state_handle_timeout closed_simptcp_socket_state_handle_timeout;


/* listen state functions */
static simptcp_socket_state_active_open listen_simptcp_socket_state_active_open;
static simptcp_socket_state_passive_open listen_simptcp_socket_state_passive_open;
static simptcp_socket_state_accept listen_simptcp_socket_state_accept;
static simptcp_socket_state_send listen_simptcp_socket_state_send;
static simptcp_socket_state_recv listen_simptcp_socket_state_recv;
static simptcp_socket_state_close listen_simptcp_socket_state_close;
static  simptcp_socket_state_shutdown listen_simptcp_socket_state_shutdown;
static simptcp_socket_state_process_pdu listen_simptcp_socket_state_process_simptcp_pdu;
static simptcp_socket_state_handle_timeout listen_simptcp_socket_state_handle_timeout;

/* synsent state functions */
static simptcp_socket_state_active_open synsent_simptcp_socket_state_active_open;
static simptcp_socket_state_passive_open synsent_simptcp_socket_state_passive_open;
static simptcp_socket_state_accept synsent_simptcp_socket_state_accept;
static simptcp_socket_state_send synsent_simptcp_socket_state_send;
static simptcp_socket_state_recv synsent_simptcp_socket_state_recv;
static simptcp_socket_state_close synsent_simptcp_socket_state_close;
static  simptcp_socket_state_shutdown synsent_simptcp_socket_state_shutdown;
static simptcp_socket_state_process_pdu synsent_simptcp_socket_state_process_simptcp_pdu;
static simptcp_socket_state_handle_timeout synsent_simptcp_socket_state_handle_timeout;

/* synrcvd state functions */
static simptcp_socket_state_active_open synrcvd_simptcp_socket_state_active_open;
static simptcp_socket_state_passive_open synrcvd_simptcp_socket_state_passive_open;
static simptcp_socket_state_accept synrcvd_simptcp_socket_state_accept;
static simptcp_socket_state_send synrcvd_simptcp_socket_state_send;
static simptcp_socket_state_recv synrcvd_simptcp_socket_state_recv;
static simptcp_socket_state_close synrcvd_simptcp_socket_state_close;
static  simptcp_socket_state_shutdown synrcvd_simptcp_socket_state_shutdown;
static simptcp_socket_state_process_pdu synrcvd_simptcp_socket_state_process_simptcp_pdu;
static simptcp_socket_state_handle_timeout synrcvd_simptcp_socket_state_handle_timeout;

/* established state functions */
static simptcp_socket_state_active_open established_simptcp_socket_state_active_open;
static simptcp_socket_state_passive_open established_simptcp_socket_state_passive_open;
static simptcp_socket_state_accept established_simptcp_socket_state_accept;
static simptcp_socket_state_send established_simptcp_socket_state_send;
static simptcp_socket_state_recv established_simptcp_socket_state_recv;
static simptcp_socket_state_close established_simptcp_socket_state_close;
static  simptcp_socket_state_shutdown established_simptcp_socket_state_shutdown;
static simptcp_socket_state_process_pdu established_simptcp_socket_state_process_simptcp_pdu;
static simptcp_socket_state_handle_timeout established_simptcp_socket_state_handle_timeout;

/* closewait state functions */
static simptcp_socket_state_active_open closewait_simptcp_socket_state_active_open;
static simptcp_socket_state_passive_open closewait_simptcp_socket_state_passive_open;
static simptcp_socket_state_accept closewait_simptcp_socket_state_accept;
static simptcp_socket_state_send closewait_simptcp_socket_state_send;
static simptcp_socket_state_recv closewait_simptcp_socket_state_recv;
static simptcp_socket_state_close closewait_simptcp_socket_state_close;
static  simptcp_socket_state_shutdown closewait_simptcp_socket_state_shutdown;
static simptcp_socket_state_process_pdu closewait_simptcp_socket_state_process_simptcp_pdu;
static simptcp_socket_state_handle_timeout closewait_simptcp_socket_state_handle_timeout;

/* finwait1 state functions */
static simptcp_socket_state_active_open finwait1_simptcp_socket_state_active_open;
static simptcp_socket_state_passive_open finwait1_simptcp_socket_state_passive_open;
static simptcp_socket_state_accept finwait1_simptcp_socket_state_accept;
static simptcp_socket_state_send finwait1_simptcp_socket_state_send;
static simptcp_socket_state_recv finwait1_simptcp_socket_state_recv;
static simptcp_socket_state_close finwait1_simptcp_socket_state_close;
static  simptcp_socket_state_shutdown finwait1_simptcp_socket_state_shutdown;
static simptcp_socket_state_process_pdu finwait1_simptcp_socket_state_process_simptcp_pdu;
static simptcp_socket_state_handle_timeout finwait1_simptcp_socket_state_handle_timeout;

/* finwait2 state functions */
static simptcp_socket_state_active_open finwait2_simptcp_socket_state_active_open;
static simptcp_socket_state_passive_open finwait2_simptcp_socket_state_passive_open;
static simptcp_socket_state_accept finwait2_simptcp_socket_state_accept;
static simptcp_socket_state_send finwait2_simptcp_socket_state_send;
static simptcp_socket_state_recv finwait2_simptcp_socket_state_recv;
static simptcp_socket_state_close finwait2_simptcp_socket_state_close;
static  simptcp_socket_state_shutdown finwait2_simptcp_socket_state_shutdown;
static simptcp_socket_state_process_pdu finwait2_simptcp_socket_state_process_simptcp_pdu;
static simptcp_socket_state_handle_timeout finwait2_simptcp_socket_state_handle_timeout;

/* closing state functions */
static simptcp_socket_state_active_open closing_simptcp_socket_state_active_open;
static simptcp_socket_state_passive_open closing_simptcp_socket_state_passive_open;
static simptcp_socket_state_accept closing_simptcp_socket_state_accept;
static simptcp_socket_state_send closing_simptcp_socket_state_send;
static simptcp_socket_state_recv closing_simptcp_socket_state_recv;
static simptcp_socket_state_close closing_simptcp_socket_state_close;
static  simptcp_socket_state_shutdown closing_simptcp_socket_state_shutdown;
static simptcp_socket_state_process_pdu closing_simptcp_socket_state_process_simptcp_pdu;
static simptcp_socket_state_handle_timeout closing_simptcp_socket_state_handle_timeout;

/* lastack state functions */
static simptcp_socket_state_active_open lastack_simptcp_socket_state_active_open;
static simptcp_socket_state_passive_open lastack_simptcp_socket_state_passive_open;
static simptcp_socket_state_accept lastack_simptcp_socket_state_accept;
static simptcp_socket_state_send lastack_simptcp_socket_state_send;
static simptcp_socket_state_recv lastack_simptcp_socket_state_recv;
static simptcp_socket_state_close lastack_simptcp_socket_state_close;
static  simptcp_socket_state_shutdown lastack_simptcp_socket_state_shutdown;
static simptcp_socket_state_process_pdu lastack_simptcp_socket_state_process_simptcp_pdu;
static simptcp_socket_state_handle_timeout lastack_simptcp_socket_state_handle_timeout;

/* timewait state functions */
static simptcp_socket_state_active_open timewait_simptcp_socket_state_active_open;
static simptcp_socket_state_passive_open timewait_simptcp_socket_state_passive_open;
static simptcp_socket_state_accept timewait_simptcp_socket_state_accept;
static simptcp_socket_state_send timewait_simptcp_socket_state_send;
static simptcp_socket_state_recv timewait_simptcp_socket_state_recv;
static simptcp_socket_state_close timewait_simptcp_socket_state_close;
static  simptcp_socket_state_shutdown timewait_simptcp_socket_state_shutdown;
static simptcp_socket_state_process_pdu timewait_simptcp_socket_state_process_simptcp_pdu;
static simptcp_socket_state_handle_timeout timewait_simptcp_socket_state_handle_timeout;

simptcp_socket_states_funcs simptcp_socket_states =
{
    {
        &closed_simptcp_socket_state_active_open,
        &closed_simptcp_socket_state_passive_open,
        &closed_simptcp_socket_state_accept,
        &closed_simptcp_socket_state_send,
        &closed_simptcp_socket_state_recv,
        &closed_simptcp_socket_state_close,
        &closed_simptcp_socket_state_shutdown,
        &closed_simptcp_socket_state_process_simptcp_pdu,
        &closed_simptcp_socket_state_handle_timeout
    },
    {
        &listen_simptcp_socket_state_active_open,
        &listen_simptcp_socket_state_passive_open,
        &listen_simptcp_socket_state_accept,
        &listen_simptcp_socket_state_send,
        &listen_simptcp_socket_state_recv,
        &listen_simptcp_socket_state_close,
        &listen_simptcp_socket_state_shutdown,
        &listen_simptcp_socket_state_process_simptcp_pdu,
        &listen_simptcp_socket_state_handle_timeout
    },
    {
        &synsent_simptcp_socket_state_active_open,
        &synsent_simptcp_socket_state_passive_open,
        &synsent_simptcp_socket_state_accept,
        &synsent_simptcp_socket_state_send,
        &synsent_simptcp_socket_state_recv,
        &synsent_simptcp_socket_state_close,
        &synsent_simptcp_socket_state_shutdown,
        &synsent_simptcp_socket_state_process_simptcp_pdu,
        &synsent_simptcp_socket_state_handle_timeout
    },
    {
        &synrcvd_simptcp_socket_state_active_open,
        &synrcvd_simptcp_socket_state_passive_open,
        &synrcvd_simptcp_socket_state_accept,
        &synrcvd_simptcp_socket_state_send,
        &synrcvd_simptcp_socket_state_recv,
        &synrcvd_simptcp_socket_state_close,
        &synrcvd_simptcp_socket_state_shutdown,
        &synrcvd_simptcp_socket_state_process_simptcp_pdu,
        &synrcvd_simptcp_socket_state_handle_timeout
    },
    {
        &established_simptcp_socket_state_active_open,
        &established_simptcp_socket_state_passive_open,
        &established_simptcp_socket_state_accept,
        &established_simptcp_socket_state_send,
        &established_simptcp_socket_state_recv,
        &established_simptcp_socket_state_close,
        &established_simptcp_socket_state_shutdown,
        &established_simptcp_socket_state_process_simptcp_pdu,
        &established_simptcp_socket_state_handle_timeout
    },
    {
        &closewait_simptcp_socket_state_active_open,
        &closewait_simptcp_socket_state_passive_open,
        &closewait_simptcp_socket_state_accept,
        &closewait_simptcp_socket_state_send,
        &closewait_simptcp_socket_state_recv,
        &closewait_simptcp_socket_state_close,
        &closewait_simptcp_socket_state_shutdown,
        &closewait_simptcp_socket_state_process_simptcp_pdu,
        &closewait_simptcp_socket_state_handle_timeout
    },
    {
        &finwait1_simptcp_socket_state_active_open,
        &finwait1_simptcp_socket_state_passive_open,
        &finwait1_simptcp_socket_state_accept,
        &finwait1_simptcp_socket_state_send,
        &finwait1_simptcp_socket_state_recv,
        &finwait1_simptcp_socket_state_close,
        &finwait1_simptcp_socket_state_shutdown,
        &finwait1_simptcp_socket_state_process_simptcp_pdu,
        &finwait1_simptcp_socket_state_handle_timeout
    },
    {
        &finwait2_simptcp_socket_state_active_open,
        &finwait2_simptcp_socket_state_passive_open,
        &finwait2_simptcp_socket_state_accept,
        &finwait2_simptcp_socket_state_send,
        &finwait2_simptcp_socket_state_recv,
        &finwait2_simptcp_socket_state_close,
        &finwait2_simptcp_socket_state_shutdown,
        &finwait2_simptcp_socket_state_process_simptcp_pdu,
        &finwait2_simptcp_socket_state_handle_timeout
    },
    {
        &closing_simptcp_socket_state_active_open,
        &closing_simptcp_socket_state_passive_open,
        &closing_simptcp_socket_state_accept,
        &closing_simptcp_socket_state_send,
        &closing_simptcp_socket_state_recv,
        &closing_simptcp_socket_state_close,
        &closing_simptcp_socket_state_shutdown,
        &closing_simptcp_socket_state_process_simptcp_pdu,
        &closing_simptcp_socket_state_handle_timeout
    },
    {
        &lastack_simptcp_socket_state_active_open,
        &lastack_simptcp_socket_state_passive_open,
        &lastack_simptcp_socket_state_accept,
        &lastack_simptcp_socket_state_send,
        &lastack_simptcp_socket_state_recv,
        &lastack_simptcp_socket_state_close,
        &lastack_simptcp_socket_state_shutdown,
        &lastack_simptcp_socket_state_process_simptcp_pdu,
        &lastack_simptcp_socket_state_handle_timeout
    },
    {
        &timewait_simptcp_socket_state_active_open,
        &timewait_simptcp_socket_state_passive_open,
        &timewait_simptcp_socket_state_accept,
        &timewait_simptcp_socket_state_send,
        &timewait_simptcp_socket_state_recv,
        &timewait_simptcp_socket_state_close,
        &timewait_simptcp_socket_state_shutdown,
        &timewait_simptcp_socket_state_process_simptcp_pdu,
        &timewait_simptcp_socket_state_handle_timeout
    }
};
