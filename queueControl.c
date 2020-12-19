#include "queueControl.h"
#include "main.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

static tsQmsgIts QmsgIts[QUEUE_NUMBER];

//static func 
static int queueSend(queueMesssage_t * txMsg, uint8_t dst);

//queue create
void queueCreate(uint8_t taskId,qCb cb)
{	
	osPoolDef(qMemPoolId_t,QUEUE_SIZE,queueMesssage_t);
	osMessageQDef(vQueue_t, QUEUE_SIZE, sizeof(queueMesssage_t));
	
	QmsgIts[taskId].queueHandle = osMessageCreate(osMessageQ(vQueue_t), NULL);
	QmsgIts[taskId].qMemPoolId = osPoolCreate(osPool(qMemPoolId_t));
	QmsgIts[taskId].cb = cb;
}

//queue msg get
int queueMsgGet(uint8_t src,uint32_t timeOut)
{
	queueMesssage_t * p;
	
	QmsgIts[src].qRxEvent = osMessageGet(QmsgIts[src].queueHandle,timeOut);
	
	if( QmsgIts[src].qRxEvent.status != osEventMessage)
	{
		 return -1;
	}
	else
	{
		p  = (queueMesssage_t*)(QmsgIts[src].qRxEvent.value.p);
		
		QmsgIts[src].cb(p);
		
		queueMsgDelete(src,p);
	}

	return 0;
}

static int queueSend(queueMesssage_t * txMsg, uint8_t dst)
{
	int iRet =0;
	
	queueMesssage_t *pMsg = NULL;
	
	if(txMsg == NULL)
	{
		iRet = 1;
		goto done;
	}
	
	pMsg = (queueMesssage_t*)osPoolCAlloc(QmsgIts[dst].qMemPoolId);
	if(pMsg == NULL)
	{
		iRet = 1;
		goto done;
	}
	
	pMsg->event = txMsg->event;
	pMsg->length = txMsg->length;
	pMsg->pcMessage = NULL;
	
	if((txMsg->pcMessage) && (txMsg->length))
	{
		pMsg->pcMessage = (void*)malloc(txMsg->length);
		if(pMsg->pcMessage != NULL)
		{
			memcpy(pMsg->pcMessage,txMsg->pcMessage,txMsg->length);
		}
		else
		{
			iRet = 1;
			goto done;		
		}
	}
	
	if(osMessagePut(QmsgIts[dst].queueHandle,(uint32_t)pMsg,osWaitForever) != osOK)
	{
		iRet = 1;
		goto done;	
	}
			
	return iRet;

	done:
		if(iRet)
		{
			if(pMsg)
			{
				if(pMsg->pcMessage)
				{
					free(pMsg->pcMessage);
				}
				osPoolFree(QmsgIts[dst].qMemPoolId,pMsg);
			}
		}
		
	return iRet;
}

//queue Send
int queueMsgSend(uint8_t dst, int msg_event, uint8_t * data, int data_len)
{
	queueMesssage_t qTxMsg = {0};

	qTxMsg.event  			= msg_event;
	qTxMsg.length  		  = data_len;
	qTxMsg.pcMessage  	= data;

	return (queueSend(&qTxMsg,dst));
}

//queue Msg Delete
void queueMsgDelete(uint8_t src,queueMesssage_t * rxMsg)
{
	if(rxMsg->pcMessage != NULL)
		free(rxMsg->pcMessage);
	
	osPoolFree(QmsgIts[src].qMemPoolId,rxMsg);
}
