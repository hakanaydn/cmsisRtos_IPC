#ifndef _QUEUECONTROL_H_
#define _QUEUECONTROL_H_

#include "main.h"
#include "cmsis_os.h"

#define QUEUE_NUMBER 				(3)  //TASK ID SAYISI KADAR
#define QUEUE_SIZE  				(50)

typedef struct 
{
	uint32_t event;
	uint32_t length;
	uint8_t  *pcMessage;
}queueMesssage_t;

typedef void (*qCb)(queueMesssage_t * p);

typedef struct
{
	osPoolId qMemPoolId;
	osMessageQId queueHandle;
	osEvent qRxEvent;
	qCb cb;
}tsQmsgIts;


void queueCreate(uint8_t taskId,qCb cb);
int  queueMsgGet(uint8_t src,uint32_t timeOut);
int  queueMsgSend(uint8_t dst, int msg_event, uint8_t * data, int data_len);
void queueMsgDelete(uint8_t src,queueMesssage_t * rxMsg);

#endif /*_QUEUECONTROL_H_*/