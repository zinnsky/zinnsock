#pragma once
#include "pch.h"

#include "ServerService.h";

class Server
{
public:
	Server();
	~Server();

	void Init();
	void RunWorkers(_beginthreadex_proc_type proc);
	void Broadcast(packet* data);
	void Join();
	void Close();

	HANDLE GetHandle();
	bool   Running();


	void AddWorker(HANDLE hthread);
	vector<HANDLE>& GetWorkers();

public:
	ServerService* _service = new ServerService();
	bool			_bRunning = true;
	int				_nThreads = NUM_THREADS;
};