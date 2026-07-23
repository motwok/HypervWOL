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
// Run
// Opens one or more UDP sockets on configured interface:port entries and processes incoming WOL packets.
// Maintains a MAC->VM cache (TTL: VM_CACHE_TTL_MS) and a per-MAC cooldown.
// Blocks until stopEvent is signalled.
// ---------------------------------------------------------------------------
void HypervWOLWorker::Run(HANDLE stopEvent, const Config& config)
{
    if (!stopEvent) return;
    WSADATA wsaData{};
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    { std::wcerr << L"WSAStartup failed." << std::endl; return; }

    const ListenerAddressList& endpoints = config.endpoints;

    std::vector<ListenerSocketContext> sockets;
    sockets.reserve(endpoints.size());

    m_vmCatalog.Refresh();

    const size_t maxSockets = MAXIMUM_WAIT_OBJECTS - 1;
    bool warnedAboutLimit = false;

    for (const auto& endpoint : endpoints)
    {
        if (sockets.size() >= maxSockets)
        {
            if (!warnedAboutLimit)
            {
                std::wcerr << L"Too many listener endpoints for WaitForMultipleObjects. Maximum supported sockets: "
                           << maxSockets << std::endl;
                warnedAboutLimit = true;
            }
            break;
        }

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

        WSAEVENT eventHandle = WSACreateEvent();
        if (eventHandle == WSA_INVALID_EVENT)
        {
            std::wcerr << L"WSACreateEvent() failed for " << endpoint.ip << L":" << endpoint.port
                       << L"  WSA error: " << WSAGetLastError() << std::endl;
            closesocket(sock);
            continue;
        }

        if (WSAEventSelect(sock, eventHandle, FD_READ | FD_CLOSE) == SOCKET_ERROR)
        {
            std::wcerr << L"WSAEventSelect() failed for " << endpoint.ip << L":" << endpoint.port
                       << L"  WSA error: " << WSAGetLastError() << std::endl;
            WSACloseEvent(eventHandle);
            closesocket(sock);
            continue;
        }

        std::wcout << L"WOL listener active on " << endpoint.ip << L":" << endpoint.port << std::endl;
        sockets.push_back({ sock, eventHandle, endpoint });
    }

    if (sockets.empty())
    {
        std::wcerr << L"No WOL listener sockets could be created." << std::endl;
        WSACleanup();
        return;
    }

    std::vector<HANDLE> waitHandles;
    waitHandles.reserve(sockets.size() + 1);
    waitHandles.push_back(stopEvent);
    for (const auto& socketCtx : sockets)
        waitHandles.push_back(socketCtx.eventHandle);

    std::array<unsigned char, 1024> buf{};
    while (true)
    {
        DWORD waitResult = WaitForMultipleObjects(static_cast<DWORD>(waitHandles.size()), waitHandles.data(), FALSE, 1000);
        if (waitResult == WAIT_OBJECT_0)
            break;

        if (waitResult == WAIT_TIMEOUT)
        {
            if (GetTickCount64() - m_vmCatalog.GetLastRefreshTick() >= VM_CACHE_TTL_MS)
                m_vmCatalog.Refresh();
            continue;
        }

        if (waitResult == WAIT_FAILED)
        {
            std::wcerr << L"WaitForMultipleObjects failed. Error: " << GetLastError() << std::endl;
            break;
        }

        const DWORD handleIndex = waitResult - WAIT_OBJECT_0;
        if (handleIndex == 0 || handleIndex > sockets.size())
            continue;

        const auto& socketCtx = sockets[handleIndex - 1];
        WSANETWORKEVENTS networkEvents{};
        if (WSAEnumNetworkEvents(socketCtx.socket, socketCtx.eventHandle, &networkEvents) == SOCKET_ERROR)
        {
            std::wcerr << L"WSAEnumNetworkEvents failed on " << socketCtx.endpoint.ip << L":" << socketCtx.endpoint.port
                       << L"  WSA error: " << WSAGetLastError() << std::endl;
            continue;
        }

        if ((networkEvents.lNetworkEvents & FD_READ) == 0)
            continue;

        while (true)
        {
            sockaddr_in sender{};
            int senderLen = sizeof(sender);
            int received = recvfrom(socketCtx.socket, reinterpret_cast<char*>(buf.data()), static_cast<int>(buf.size()), 0,
                reinterpret_cast<sockaddr*>(&sender), &senderLen);
            if (received <= 0)
            {
                const int err = WSAGetLastError();
                if (err == WSAEWOULDBLOCK)
                    break;
                if (err == WSAECONNRESET)
                    continue;
                break;
            }

            MacAddress mac;
            if (!WolPacket::TryExtractMac(buf, received, mac))
                continue;

            wchar_t senderIp[INET_ADDRSTRLEN];
            InetNtopW(AF_INET, &sender.sin_addr, senderIp, INET_ADDRSTRLEN);

            if (!m_vmCatalog.contains(mac))
            {
                std::wcout << L"WOL from " << senderIp << L"  MAC " << mac.ToString()
                           << L"  (no VM in cache, ignored)" << std::endl;
                continue;
            }

            const VmInfo& vmInfo = m_vmCatalog[mac];

            const ULONGLONG now = GetTickCount64();
            bool inCooldown = false;
            {
                std::lock_guard<std::mutex> lock(m_lastTriggerMutex);
                auto cdIt = m_lastTrigger.find(mac);
                if (cdIt != m_lastTrigger.end() && (now - cdIt->second) < WOL_COOLDOWN_MS)
                {
                    inCooldown = true;
                }
                else
                {
                    m_lastTrigger[mac] = now;
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
    }

    for (auto& socketCtx : sockets)
    {
        WSACloseEvent(socketCtx.eventHandle);
        closesocket(socketCtx.socket);
    }

    WSACleanup();
}
