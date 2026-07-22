/*
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * Copyright (C) 2026 Emmo "mo2000" Emminghaus mo2000 at mo2000 dot de
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include <winsock2.h>
#include <ws2tcpip.h>
#include "HypervWOLWorker.h"
#include "MacAddress.h"
#include "VmStarter.h"
#include "WolPacket.h"

#include <array>
#include <iostream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

static constexpr ULONGLONG WOL_COOLDOWN_MS = 60000;
static constexpr ULONGLONG VM_CACHE_TTL_MS = 300000;

// ---------------------------------------------------------------------------
// ListenerThreadProc
// Receives packets on a single socket until stopEvent is signalled or the socket is closed.
// ---------------------------------------------------------------------------
DWORD WINAPI HypervWOLWorker::ListenerThreadProc(LPVOID lpParam)
{
    auto* ctx = reinterpret_cast<ListenerThreadContext*>(lpParam);
    if (!ctx || !ctx->worker) return 0;

    std::array<unsigned char, 1024> buf{};
    while (WaitForSingleObject(ctx->stopEvent, 0) != WAIT_OBJECT_0)
    {
        sockaddr_in sender{};
        int senderLen = sizeof(sender);
        int received = recvfrom(ctx->socket, reinterpret_cast<char*>(buf.data()), static_cast<int>(buf.size()), 0,
            reinterpret_cast<sockaddr*>(&sender), &senderLen);
        if (received <= 0)
        {
            if (WaitForSingleObject(ctx->stopEvent, 0) == WAIT_OBJECT_0)
                break;
            const int err = WSAGetLastError();
            if (err == WSAENOTSOCK || err == WSAEINTR || err == WSAECONNRESET)
                break;
            continue;
        }

        MacAddress mac;
        if (!WolPacket::TryExtractMac(buf, received, mac))
            continue;

        wchar_t senderIp[INET_ADDRSTRLEN];
        InetNtopW(AF_INET, &sender.sin_addr, senderIp, INET_ADDRSTRLEN);

        if (!ctx->worker->m_vmCatalog.contains(mac))
        {
            std::wcout << L"WOL from " << senderIp << L"  MAC " << mac.ToString()
                       << L"  (no VM in cache, ignored)" << std::endl;
            continue;
        }

        const VmInfo& vmInfo = ctx->worker->m_vmCatalog[mac];

        const ULONGLONG now = GetTickCount64();
        bool inCooldown = false;
        {
            std::lock_guard<std::mutex> lock(ctx->worker->m_lastTriggerMutex);
            auto cdIt = ctx->worker->m_lastTrigger.find(mac);
            if (cdIt != ctx->worker->m_lastTrigger.end() && (now - cdIt->second) < WOL_COOLDOWN_MS)
            {
                inCooldown = true;
            }
            else
            {
                ctx->worker->m_lastTrigger[mac] = now;
            }
        }

        if (inCooldown)
        {
            std::wcout << L"WOL from " << senderIp << L"  MAC " << mac.ToString()
                       << L"  VM '" << vmInfo.name << L"'  (cooldown, ignored)" << std::endl;
            continue;
        }

        std::wcout << L"WOL from " << senderIp << L"  MAC " << mac.ToString()
                   << L"  -> VM '" << vmInfo.name << L"'" << std::endl;
        VmStarter::StartVm(vmInfo);
    }

    return 0;
}

// ---------------------------------------------------------------------------
// Run
// Opens one or more UDP sockets on configured interface:port entries and processes incoming WOL packets.
// Maintains a MAC->VM cache (TTL: VM_CACHE_TTL_MS) and a per-MAC cooldown.
// Blocks until stopEvent is signalled.
// ---------------------------------------------------------------------------
void HypervWOLWorker::Run(HANDLE stopEvent, const std::wstring& listenSpec)
{
    if (!stopEvent) return;
    WSADATA wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    { std::wcerr << L"WSAStartup failed." << std::endl; return; }

    ListenerAddressList endpoints(listenSpec);

    std::vector<ListenerThreadContext> contexts;
    contexts.reserve(endpoints.size());
    std::vector<HANDLE> threads;
    threads.reserve(endpoints.size());

    m_vmCatalog.Refresh();

    for (const auto& endpoint : endpoints)
    {
        SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sock == INVALID_SOCKET)
        {
            std::wcerr << L"socket() failed for " << endpoint.ip << L":" << endpoint.port
                       << L"  WSA error: " << WSAGetLastError() << std::endl;
            continue;
        }

        BOOL reuse = TRUE;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&reuse), sizeof(reuse));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(endpoint.port);
        if (InetPtonW(AF_INET, endpoint.ip.c_str(), &addr.sin_addr) != 1)
        {
            std::wcerr << L"Invalid listen IP: " << endpoint.ip << std::endl;
            closesocket(sock);
            continue;
        }

        if (bind(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
        {
            std::wcerr << L"bind() failed on " << endpoint.ip << L":" << endpoint.port
                       << L"  WSA error: " << WSAGetLastError() << std::endl;
            closesocket(sock);
            continue;
        }

        std::wcout << L"WOL listener active on " << endpoint.ip << L":" << endpoint.port << std::endl;
        contexts.push_back({ this, sock, endpoint, stopEvent });
        HANDLE thread = CreateThread(nullptr, 0, ListenerThreadProc, &contexts.back(), 0, nullptr);
        if (!thread)
        {
            std::wcerr << L"Failed to create listener thread for " << endpoint.ip << L":" << endpoint.port << std::endl;
            closesocket(sock);
            contexts.pop_back();
            continue;
        }
        threads.push_back(thread);
    }

    if (threads.empty())
    {
        std::wcerr << L"No WOL listener sockets could be created." << std::endl;
        WSACleanup();
        return;
    }

    while (WaitForSingleObject(stopEvent, 1000) != WAIT_OBJECT_0)
    {
        if (GetTickCount64() - m_vmCatalog.GetLastRefreshTick() >= VM_CACHE_TTL_MS)
            m_vmCatalog.Refresh();
    }

    for (auto& ctx : contexts)
        closesocket(ctx.socket);

    for (HANDLE thread : threads)
    {
        WaitForSingleObject(thread, INFINITE);
        CloseHandle(thread);
    }

    WSACleanup();
}
