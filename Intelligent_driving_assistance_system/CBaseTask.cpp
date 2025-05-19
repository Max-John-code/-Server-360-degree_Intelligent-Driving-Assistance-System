#include "CBaseTask.h"

CBaseTask::CBaseTask(char* data)
{
	this->ipc = NULL;
	bzero(this->data, sizeof(this->data));
	memcpy(this->data, data, sizeof(this->data));
}

CBaseTask::~CBaseTask()
{
	delete this->data;
}

void CBaseTask::setWorkfd(int val)
{
	this->workfd = val;
}

void CBaseTask::setIPC(IPC* ipc)
{
	this->ipc = ipc;
}
