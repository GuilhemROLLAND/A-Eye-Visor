#include "tcp.h"
#include "ip4_addr.h"
#include "ip_addr.h"
#include "tcpbase.h"

#include "tcp_server.h"

err_t err;
static struct tcp_pcb *tcp_pcb;
struct tcp_pcb *pcb;
const ip_addr_t *localIP = IP4_ADDR_ANY;
u16_t port = 1028;




// init
void initTCP(void)
{
    lwip_init();
    sys_check_timeouts();
    tcp_tmr();

    tcp_pcb = tcp_new_ip_type(IPADDR_TYPE_V4);

    if (tcp_pcb != NULL)
    {
        err_t err;

        err = tcp_bind(tcp_pcb, IP4_ADDR_ANY, 7);
        if (err == ERR_OK)
        {
            tcp_pcb = tcp_listen(tcp_pcb);
            tcp_accept(tcp_pcb, tcp_pcb);
        }
    }
}


static err_t sentTCP(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    struct tcp_state *es;

    LWIP_UNUSED_ARG(len);

    es = (struct tcp_state *)arg;
    es->retries = 0;

    if (es->p != NULL)
    {
        /* still got pbufs to send */
        tcp_sent(tpcb, sentTCP);
        sendTCP(tpcb, es);
    }
    else
    {
        /* no more pbufs to send */
        if (es->state == ES_CLOSING)
        {
            closeTCP(tpcb, es);
        }
    }
    return ERR_OK;
}

static void sendTCP(struct tcp_pcb *tpcb, struct tcp_state *es)
{
    struct pbuf *ptr;
    err_t wr_err = ERR_OK;

    while ((wr_err == ERR_OK) &&
           (es->p != NULL) &&
           (es->p->len <= tcp_sndbuf(tpcb)))
    {
        ptr = es->p;

        /* enqueue data for transmission */
        wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);
        if (wr_err == ERR_OK)
        {
            u16_t plen;

            plen = ptr->len;
            /* continue with next pbuf in chain (if any) */
            es->p = ptr->next;
            if (es->p != NULL)
            {
                /* new reference! */
                pbuf_ref(es->p);
            }
            /* chop first pbuf from chain */
            pbuf_free(ptr);
            /* we can read more data now */
            tcp_recved(tpcb, plen);
        }
        else if (wr_err == ERR_MEM)
        {
            /* we are low on memory, try later / harder, defer to poll */
            es->p = ptr;
        }
        else
        {
            /* other problem ?? */
        }
    }
}

static void closeTCP(struct tcp_pcb *tpcb, struct tcp_state *es)
{
  tcp_arg(tpcb, NULL);
  tcp_sent(tpcb, NULL);
  tcp_recv(tpcb, NULL);
  tcp_err(tpcb, NULL);
  tcp_poll(tpcb, NULL, 0);

  freeTCP(es);

  tcp_close(tpcb);
}

static void errorTCP(void *arg, err_t err)
{
  struct tcp_state *es;

  LWIP_UNUSED_ARG(err);

  es = (struct tcp_state *)arg;

  freeTCP(es);
}

static err_t recvTCP(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
  struct tcp_state *es;
  err_t ret_err;

  LWIP_ASSERT("arg != NULL",arg != NULL);
  es = (struct tcp_state *)arg;
  if (p == NULL) {
    /* remote host closed connection */
    es->state = ES_CLOSING;
    if(es->p == NULL) {
      /* we're done sending, close it */
     closeTCP(tpcb, es);
    } else {
      /* we're not done yet */
      sendTCP(tpcb, es);
    }
    ret_err = ERR_OK;
  } else if(err != ERR_OK) {
    /* cleanup, for unknown reason */
    LWIP_ASSERT("no pbuf expected here", p == NULL);
    ret_err = err;
  }
  else if(es->state == ES_ACCEPTED) {
    /* first data chunk in p->payload */
    es->state = ES_RECEIVED;
    /* store reference to incoming pbuf (chain) */
    es->p = p;
    sendTCP(tpcb, es);
    ret_err = ERR_OK;
  } else if (es->state == ES_RECEIVED) {
    /* read some more data */
    if(es->p == NULL) {
      es->p = p;
      sendTCP(tpcb, es);
    } else {
      struct pbuf *ptr;

      /* chain pbufs to the end of what we recv'ed previously  */
      ptr = es->p;
      pbuf_cat(ptr,p);
    }
    ret_err = ERR_OK;
  } else {
    /* unknown es->state, trash data  */
    tcp_recved(tpcb, p->tot_len);
    pbuf_free(p);
    ret_err = ERR_OK;
  }
  return ret_err;
}

static err_t pollTCP(void *arg, struct tcp_pcb *tpcb)
{
  err_t ret_err;
  struct tcp_state *es;

  es = (struct tcp_state *)arg;
  if (es != NULL) {
    if (es->p != NULL) {
      /* there is a remaining pbuf (chain)  */
      sendTCP(tpcb, es);
    } else {
      /* no remaining pbuf (chain)  */
      if(es->state == ES_CLOSING) {
        closeTCP(tpcb, es);
      }
    }
    ret_err = ERR_OK;
  } else {
    /* nothing to be done */
    tcp_abort(tpcb);
    ret_err = ERR_ABRT;
  }
  return ret_err;
}


static err_t acceptTCP(void *arg, struct tcp_pcb *newpcb, err_t err)
{
  err_t ret_err;
  struct tcp_state *es;

  LWIP_UNUSED_ARG(arg);
  if ((err != ERR_OK) || (newpcb == NULL)) {
    return ERR_VAL;
  }

  /* Unless this pcb should have NORMAL priority, set its priority now.
     When running out of pcbs, low priority pcbs can be aborted to create
     new pcbs of higher priority. */
  tcp_setprio(newpcb, TCP_PRIO_MIN);

  es = (struct tcp_state *)mem_malloc(sizeof(struct tcp_state));
  if (es != NULL) {
    es->state = ES_ACCEPTED;
    es->pcb = newpcb;
    es->retries = 0;
    es->p = NULL;
    /* pass newly allocated es to our callbacks */
    tcp_arg(newpcb, es);
    tcp_recv(newpcb, recvTCP);
    tcp_err(newpcb, errorTCP);
    tcp_poll(newpcb, pollTCP, 0);
    tcp_sent(newpcb, sentTCP);
    ret_err = ERR_OK;
  } else {
    ret_err = ERR_MEM;
  }
  return ret_err;
}


