#include "pch.h"
#include "Server.h"

Server::Server()
{

}

Server::~Server()
{
	delete _service;
}

void Server::Init()
{
	WSADATA wsaData;
	assert(WSAStartup(MAKEWORD(2, 2), &wsaData) == 0);

	_service->Init();
	_service->AcceptAllSession();
}

void Server::RunWorkers(_beginthreadex_proc_type proc) {
	for (int i = 0; i < NUM_THREADS; i++)
	{
		DWORD tid = 0;
		HANDLE hThread = NULL;
		hThread = (HANDLE)_beginthreadex(
			NULL,
			0,
			proc,
			(LPVOID)this,  // thread 함수의 인자로 server 포인터를 넘겨줍니다.
			NULL,
			(unsigned int*)&tid
		);

		assert(hThread != NULL);

		AddWorker(hThread);
	}
}

// TODO: define data type
void Server::Broadcast(packet* data)
{
	_service->Broadcast(data);
}

HANDLE Server::GetHandle()
{
	return _service->GetHandle();
}

bool Server::Running()
{
	if (_nThreads == 0)
		_bRunning = false;

	return _bRunning;
}

void Server::AddWorker(HANDLE hthread)
{
	_service->AddWorker(hthread);
}

vector<HANDLE>& Server::GetWorkers()
{
	return _service->GetWorkers();
}



void Server::Join()
{
	vector<HANDLE> threads = GetWorkers();
	for (int i = 0; i < NUM_THREADS; i++)
		WaitForMultipleObjects(NUM_THREADS, threads.data(), true, INFINITE);
}

void Server::Close()
{
	_bRunning = false;
	_service->Close();
}
