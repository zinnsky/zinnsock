#pragma once

#include "pch.h"
#include "Server.h"
#include "ServerService.h"
#include "Session.h"

DWORD ProcessIoCompletion(LPVOID param);
void DoAccept(Session* session, DWORD dwBytes);
void DoReceive(Session* session, DWORD dwBytes);
void DoSend(Session* session, DWORD dwBytes);


int main()
{
	Server server;

	server.Init();
	server.RunWorkers((_beginthreadex_proc_type)ProcessIoCompletion);

	std::cout << server.ToString();

	server.Join();
	server.Close();

	return 0;
}

DWORD ProcessIoCompletion(LPVOID param)
{
	Server* server = static_cast<Server*>(param);

	//함수 Lookup 배열
	void (*task[])(Session*, DWORD) = {
			DoReceive,
			DoSend,
			DoAccept,
	};

	while (server->Running())
	{
		Sleep(1);

		HANDLE				iocpHandle = server->GetHandle();
		DWORD				dwBytes = 0;
		ULONG_PTR			key = 0;
		SessionContext* ctx = nullptr;

		if (GetQueuedCompletionStatus((HANDLE)iocpHandle, &dwBytes, (PULONG_PTR)&key, (LPOVERLAPPED*)&ctx, INFINITE))
		{
			Session* session = ctx->GetOwner();
			uint8 ctxType = static_cast<uint8>(ctx->GetType());
			auto handleEvent = task[ctxType];
			handleEvent(session, dwBytes);
		}
		else
		{
			DWORD error = WSAGetLastError();
			std::cout << "ERROR: " << error << endl;

			if (error == 64)
			{
				cout << "Session Closed" << endl;
				break;
			}
		}

	}

	return 0;
}


void DoAccept(Session* session, DWORD dwBytes)
{
	// 연결이 생성되면, 해당 소켓을 ACCEPT 상태로 갱신.
	SOCKET listener = session->GetListenSocket();
	::setsockopt(session->GetSocket(), SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&listener, sizeof(SOCKET));

	std::cout << "[ACCEPT]" << dwBytes << " bytes received from " << session->ToString() << std::endl;

	session->RegisterRecv();
}


void DoReceive(Session* session, DWORD dwBytes)
{
	ServerService* service = session->GetOwner();
	BYTE* recvBuff = session->GetRecvBuffer();
	packet data{ 0 };
	::memcpy(&data.length, recvBuff, 4);
	::memcpy(&data.id, recvBuff + 4, 4);
	::memcpy(&data.data, recvBuff + 8, 4);

	std::cout << dwBytes << " bytes received: " << std::endl;

	if (dwBytes == 0) {
		std::cout << "Closing session id: " << session->GetId() << std::endl;
		session->Close();

		service->Accept(session);
		return;
	}

	session->RegisterRecv();
}

void DoSend(Session* session, DWORD dwBytes)
{
	// TODO: define packet data and register Send I/O 
	// session->RegisterSend(data);
	// cout << dwBytes << " bytes sent to id: " << session->GetId() << endl;
}
