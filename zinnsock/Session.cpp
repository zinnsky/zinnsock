#include "pch.h"
#include "Session.h"

Session::Session(HANDLE iocpHandle, SOCKET listener) : _listener(listener)
{
	_socket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);
	assert(_socket != INVALID_SOCKET);
	assert(iocpHandle != INVALID_HANDLE_VALUE);
	assert(CreateIoCompletionPort((HANDLE)_socket, iocpHandle, 0, 0) != NULL);

	_recvContext->_owner = this;
	_sendContext->_owner = this;
	_acceptContext->_owner = this;
}

Session::~Session()
{
	delete _sendContext;
	delete _recvContext;
	delete _acceptContext;
}

void Session::Init()
{
	ZeroMemory(_recvBuffer, sizeof(_recvBuffer));
	ZeroMemory(_sendBuffer, sizeof(_sendBuffer));
	_recvContext->Init();
	_sendContext->Init();
	_acceptContext->Init();

	SetId(reinterpret_cast<int>(this));
}

void Session::RegisterInitialSend()
{
	DWORD flags = 0;
	WSABUF sendbuf;
	_sendContext->Init();

	::memcpy_s(_sendBuffer, sizeof(_sendBuffer), &_id, sizeof(_id));
	sendbuf.len = sizeof(_sendBuffer);
	sendbuf.buf = (char*)_sendBuffer;

	WSASend(_socket, &sendbuf, 1, NULL, NULL, (LPOVERLAPPED)_sendContext, NULL);
}

void Session::RegisterSend(packet* data)
{
	DWORD flags = 0;
	WSABUF sendbuf;

	_sendContext->Init();

	::memcpy_s(_sendBuffer, sizeof(_sendBuffer), data, 24);

	sendbuf.len = sizeof(_sendBuffer);
	sendbuf.buf = (char*)_sendBuffer;
	WSASend(_socket, &sendbuf, 1, NULL, NULL, (LPOVERLAPPED)_sendContext, NULL);
}

void Session::RegisterRecv()
{
	DWORD flags = 0;
	WSABUF wsaBuf;
	_recvContext->Init();

	wsaBuf.buf = (char*)_recvBuffer;
	wsaBuf.len = sizeof(_recvBuffer);
	WSARecv(_socket, (LPWSABUF)&wsaBuf, 1, 0, &flags, _recvContext, NULL);
}

void Session::Close()
{
	closesocket(_socket);
}

void Session::SetOwner(ServerService* owner)
{
	_owner = owner;
}

ServerService* Session::GetOwner()
{
	return _owner;
}

SessionContext* Session::GetContext(EventType type)
{
	switch (type) {
	case EventType::Recv:
		return _recvContext;
	case EventType::Send:
		return _sendContext;
	case EventType::Accept:
		return _acceptContext;
	default:
		return nullptr;
	}
}

sockaddr_in* Session::GetRemoteAddr()
{
	size_t cbInitialRecvLen = 0;
	size_t cbLocalAddr = sizeof(sockaddr_in) + 16; // 32바이트

	return reinterpret_cast<sockaddr_in*>(_acceptBuffer + cbInitialRecvLen + cbLocalAddr);
}

sockaddr_in* Session::GetLocalAddr()
{
	size_t cbInitialRecvLen = 0;

	return reinterpret_cast<sockaddr_in*>(_acceptBuffer + cbInitialRecvLen);
}

void Session::SetId(uint32 id)
{
	_id = id;
}

uint32 Session::GetId()
{
	return _id;
}

string Session::ToString()
{
	sockaddr_in* pRemoteAddr = GetRemoteAddr();

	char clientIP[INET_ADDRSTRLEN] = { 0 };
	inet_ntop(AF_INET, &(pRemoteAddr->sin_addr), clientIP, INET_ADDRSTRLEN);
	int clientPort = ntohs(pRemoteAddr->sin_port);

	return std::format("{}:{}", clientIP, clientPort);
}

