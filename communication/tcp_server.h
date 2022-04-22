/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include <stdint.h>
#include "tcp.h"
#include "ip4_addr.h"
#include "ip_addr.h"
#include "tcpbase.h"


#ifndef LWIP_TCPECHO_H
#define LWIP_TCPECHO_H

#if !defined(LWIP_HAVE_INT64) && defined(UINT64_MAX)
#define LWIP_HAVE_INT64 1
#endif
typedef uint8_t   u8_t;
typedef int8_t    s8_t;
typedef uint16_t  u16_t;
typedef int16_t   s16_t;
typedef uint32_t  u32_t;
typedef int32_t   s32_t;
#if LWIP_HAVE_INT64
typedef uint64_t  u64_t;
typedef int64_t   s64_t;
#endif
typedef uintptr_t mem_ptr_t;
#endif


void initTCP(void);

enum tcp_states
{
    ES_NONE = 0,
    ES_ACCEPTED,
    ES_RECEIVED,
    ES_CLOSING
};

struct tcp_state
{
    u8_t state;
    u8_t retries;
    struct tcp_pcb *pcb;
    /* pbuf (chain) to recycle */
    struct pbuf *p;
};

/**
 * @brief specify a callback function for acknowledegments 
 * 
 * @param arg application link to the pcb
 * @param tpcb pcb of the connection
 * @param len len of data
 * @return err_t 
 */
static err_t sentTCP(void *arg, struct tcp_pcb *tpcb, u16_t len);

/**
 * @brief 
 * 
 * @param tpcb 
 * @param es 
 */
static void sendTCP(struct tcp_pcb *tpcb, struct tcp_state *es);

static void closeTCP(struct tcp_pcb *tpcb, struct tcp_state *es)

static void errorTCP(void *arg, err_t err)

static err_t recvTCP(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)

static err_t pollTCP(void *arg, struct tcp_pcb *tpcb)

static err_t acceptTCP(void *arg, struct tcp_pcb *newpcb, err_t err)



#endif /* LWIP_TCPECHO_H */