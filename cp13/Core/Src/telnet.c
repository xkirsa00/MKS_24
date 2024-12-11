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


#include "lwip/opt.h"

#if LWIP_NETCONN

#include "lwip/sys.h"
#include "lwip/api.h"
#include "string.h"

#define TELNET_THREAD_PRIO  ( tskIDLE_PRIORITY + 4 )
#define CMD_BUFFER_LEN 256


/*-----------------------------------------------------------------------------------*/

static void http_client(char *s, uint16_t size);




static void telnet_process_command(char *cmd, struct netconn* conn)
{
	char buf[1024];
	char *token = strtok(cmd, " ");



	if (strcasecmp(token, "HELLO") == 0)
	{
		sprintf(buf, "Communication OK\n");
		netconn_write(conn, buf, strlen(buf), NETCONN_COPY);
	}

	else if (strcasecmp(token, "LD1") == 0)
	{
		token = strtok(NULL, " ");
		if (strcasecmp(token, "ON") == 0) HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_SET);
		else if (strcasecmp(token, "OFF") == 0) HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, GPIO_PIN_RESET);
		sprintf(buf, "OK\n");
		netconn_write(conn, buf, strlen(buf), NETCONN_COPY);

	}
	else if (strcasecmp(token, "LD2") == 0)
	{
		token = strtok(NULL, " ");
		if (strcasecmp(token, "ON") == 0) HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
		else if (strcasecmp(token, "OFF") == 0) HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		sprintf(buf, "OK\n");
		netconn_write(conn, buf, strlen(buf), NETCONN_COPY);

	}


	else if (strcasecmp(token, "LD3") == 0)
		{
			token = strtok(NULL, " ");
			if (strcasecmp(token, "ON") == 0) HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
			else if (strcasecmp(token, "OFF") == 0) HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
			sprintf(buf, "OK\n");
			netconn_write(conn, buf, strlen(buf), NETCONN_COPY);

		}


	else if (strcasecmp(token, "CLIENT") == 0)
		{
		char buf_2[1024];

		http_client(buf_2, 1024);
		netconn_write(conn, buf_2, strlen(buf_2), NETCONN_COPY);

		}


	else if (strcasecmp(token, "STATUS") == 0)
	{
		uint8_t status1 = HAL_GPIO_ReadPin(LD1_GPIO_Port, LD1_Pin);
		uint8_t status2 = HAL_GPIO_ReadPin(LD2_GPIO_Port, LD2_Pin);
		uint8_t status3 = HAL_GPIO_ReadPin(LD3_GPIO_Port, LD3_Pin);

		/*if (status1)
					{
					    status1 = "ON";
					} else {
					    status1 = "OFF";
					}

					if (status2)
					{
					    status2 = "ON";
					} else {
					    status2 = "OFF";
					}

					sprintf(buf, "LD1: %3s ", (status1));
					sprintf(buf, "LD2: %3s ", (status2));*/

		sprintf(buf, "LD1: %3s	LD2: %3s	  LD3: %3s\n", (status1 ? "ON" : "OFF"), ((status2 ? "ON" : "OFF")), ((status3 ? "ON" : "OFF")));
		netconn_write(conn, buf, strlen(buf), NETCONN_COPY);

	}



	else
	{
		sprintf(buf, "Nelze\n");
		netconn_write(conn, buf, strlen(buf), NETCONN_COPY);

	}

}

static void telnet_byte_available(uint8_t c, struct netconn* conn)
{
	static uint16_t cnt;
	static char data[CMD_BUFFER_LEN];
	if (cnt < CMD_BUFFER_LEN && c >= 32 && c <= 127) data[cnt++] = c;
	if (c == '\n' || c == '\r') {
		data[cnt] = '\0';
		telnet_process_command(data, conn);
		cnt = 0;
	}
}




static void telnet_thread(void *arg)
{
	struct netconn *conn, *newconn;
	err_t err, accept_err;
	struct netbuf *buf;
	void *data;
	u16_t len;

	LWIP_UNUSED_ARG(arg);

	/* Create a new connection identifier. */
	conn = netconn_new(NETCONN_TCP);

	if (conn!=NULL)
	{
		/* Bind connection to well known port number 23. */
		err = netconn_bind(conn, NULL, 23);

		if (err == ERR_OK)
		{
			/* Tell connection to go into listening mode. */
			netconn_listen(conn);

			while (1)
			{
				/* Grab new connection. */
				accept_err = netconn_accept(conn, &newconn);

				/* Process the new connection. */
				if (accept_err == ERR_OK)
				{

					while (netconn_recv(newconn, &buf) == ERR_OK)
					{
						do
						{
							// netbuf_data(buf, &data, &len);
							// netconn_write(newconn, data, len, NETCONN_COPY);

							netbuf_data(buf, (void**)&data, &len);
							while (len--) telnet_byte_available(*((uint8_t*)(data++)), newconn);

						}
						while (netbuf_next(buf) >= 0);

						netbuf_delete(buf);
					}

					/* Close connection and discard connection identifier. */
					netconn_close(newconn);
					netconn_delete(newconn);
				}
			}
		}
		else
		{
			netconn_delete(newconn);
		}
	}
}
/*-----------------------------------------------------------------------------------*/

static void http_client(char *s, uint16_t size)
{
	struct netconn *client;
	struct netbuf *buf;
	ip_addr_t ip;
	uint16_t len = 0;
	IP_ADDR4(&ip, 147,229,144,124);
	const char *request = "GET /ip.php HTTP/1.1\r\n"
			"Host: www.urel.feec.vutbr.cz\r\n"
			"Connection: close\r\n"
			"\r\n\r\n";
	client = netconn_new(NETCONN_TCP);
	if (netconn_connect(client, &ip, 80) == ERR_OK) {
		netconn_write(client, request, strlen(request), NETCONN_COPY);
		// Receive the HTTP response
		s[0] = 0;
		while (len < size && netconn_recv(client, &buf) == ERR_OK) {
			len += netbuf_copy(buf, &s[len], size-len);
			s[len] = 0;
			netbuf_delete(buf);
		}
	} else {
		sprintf(s, "Chyba pripojeni\n");
	}
	netconn_delete(client);
}



void telnet_init(void)
{
	sys_thread_new("telnet_thread", telnet_thread, NULL, DEFAULT_THREAD_STACKSIZE, TELNET_THREAD_PRIO);
}
/*-----------------------------------------------------------------------------------*/

#endif /* LWIP_NETCONN */
