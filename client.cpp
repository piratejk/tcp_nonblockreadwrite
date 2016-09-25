#include <iostream>
#include <string>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <arpa/inet.h>

using namespace std;
int SetNonBlcok(int fd)
{
    int flags = fcntl(fd,F_GETFL,0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    return 0;
}
int ConnectWithTimeout(const int iFd, const string strIp, const uint16_t usPort, const uint32_t uiTimeWait)
{
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10000);
    addr.sin_addr.s_addr = inet_addr(strIp.c_str());
    int iRet = connect(iFd, (struct sockaddr *)&addr, sizeof(addr));
    if(0 == iRet)
    {
        cout << "connect success at first try" << endl;
        return 0;
    }
    else if((iRet < 0) && (errno == EINPROGRESS)) // 正在建立连接
    {
        fd_set set;
        FD_ZERO(&set);
        FD_SET(iFd, &set);  //相反的是FD_CLR(_sock_fd,&set)
        // 设置超时
        struct timeval timeo;
        timeo.tv_sec = uiTimeWait / 1000;
        timeo.tv_usec = (uiTimeWait % 1000) * 1000;
        iRet = select(iFd + 1, NULL, &set, NULL, &timeo);
        if(iRet < 0)
        {
            cout << "connect select fail" << endl;
            return -1;
        }
        else if(0 == iRet) // 超时
        {
            cout << "connect timeout" << endl;
            return -2;
        }
        if(FD_ISSET(iFd, &set))
        {
            // 检查是否连接成功
            int error = 0;
            uint32_t socklen = sizeof(error);
            iRet = getsockopt(iFd, SOL_SOCKET, SO_ERROR, (void *)&error, &socklen);
            if(iRet < 0)
            {
                cout << "connect fail" <<endl;
                return -3;
            }
            if(0 != error)
            {
                cout << "connect fail. error: " << error << endl;
                return -4;
            }
            cout << "wait connect success." << endl;
            return 0;
        }
        else
        {
            cout << "connect fail." << endl;
            return -5;
        }
    }
    else
    {
        cout << "connect fail. iRet:" << iRet << endl;
        return -6;
    }
    return 0;
}
int SendWithTimeout(const int iFd, char* pSendData, int iSendLen, const uint32_t uiTimeWait)
{
    if(NULL == pSendData)
    {
        return -1;
    }
    fd_set set;
    FD_ZERO(&set);
    FD_SET(iFd, &set);  //相反的是FD_CLR(_sock_fd,&set)
    // 设置超时
    struct timeval timeo;
    timeo.tv_sec = uiTimeWait / 1000;
    timeo.tv_usec = (uiTimeWait % 1000) * 1000;
    int iRet = select(iFd + 1, NULL, &set, NULL, &timeo);
    if(iRet < 0)
    {
        cout << "send select fail" << endl;
        return -1;
    }
    else if(0 == iRet) // 超时
    {
        cout << "send timeout" << endl;
        return -2;
    }
    if(FD_ISSET(iFd, &set))
    {
        ssize_t sendedLen = 0;
        while(sendedLen < iSendLen)
        {
            ssize_t ret = send(iFd, (const void*)(pSendData + sendedLen), iSendLen - sendedLen, 0);
            if(ret < 0)
            {
                cout << "send error" << endl;
                return -3;
            }
            sendedLen += ret;
        }
        return 0;
    }
    else
    {
        cout << "send fail." << endl;
        return -5;
    }

    return 0;
}
int RecvWithTimeout(const int iFd, char* pRecvData, int& iRecvLen, const uint32_t uiTimeWait)
{
    if(NULL == pRecvData)
    {
        return -1;
    }
    fd_set set;
    FD_ZERO(&set);
    FD_SET(iFd, &set);  //相反的是FD_CLR(_sock_fd,&set)
    // 设置超时
    struct timeval timeo;
    timeo.tv_sec = uiTimeWait / 1000;
    timeo.tv_usec = (uiTimeWait % 1000) * 1000;
    int iRet = select(iFd + 1, &set, NULL, NULL, &timeo);
    if(iRet < 0)
    {
        cout << "recv select fail" << endl;
        return -1;
    }
    else if(0 == iRet) // 超时
    {
        cout << "recv timeout" << endl;
        return -2;
    }
    if(FD_ISSET(iFd, &set))
    {
        ssize_t recvedLen = 0;
        while(recvedLen < iRecvLen)
        {
    cout << "aa" <<endl;
            // 一直读取，直到无数据可读
            errno = 0;
            ssize_t ret = recv(iFd, (void*)(pRecvData + recvedLen), iRecvLen - recvedLen, 0);
    cout << "bb" <<endl;
            if(ret < 0)
            {
                if(EAGAIN == errno) // 无数据可读
                {
                    iRecvLen = recvedLen;
                    return 0;
                }
                else
                {
                    cout << "recv fail" << endl;
                    return -5;
                }
            }
            else if(0 == ret)
            {
                cout << "sever close" << endl;
                return -6;
            }
            recvedLen += ret;
        }
    }
    else
    {
        cout << "recv fail." << endl;
        return -6;
    }

    return 0;
}

int main(int argc,char **argv)
{
    // 1、创建socket
    int iFd = socket(AF_INET, SOCK_STREAM, 0);
    if(iFd < 0)
    {
        cout << "create socket fail." << endl;
        return -1;
    }
    cout << "create socket success. fd: " << iFd << endl;
    // 2、设置非阻塞socket
    if(0 != SetNonBlcok(iFd))
    {
        cout << "set non block fail." << endl;
        return -2;
    }
    uint16_t usPort = 10000;
    string strIp = "127.0.0.1";
    uint32_t uiTimeWait = 3000;
    int iRet = ConnectWithTimeout(iFd, strIp, usPort, uiTimeWait);
    if(0 != iRet)
    {
        cout << "ConnectWithTimeout fail. iRet:" << iRet << endl;
        close(iFd);
        return -3;
    }
    // 3、发送数据
    string strSendData = "hello server, this is client";
    char sendData[1024];
    memset(sendData, 0 , sizeof(sendData));
    memcpy(sendData, strSendData.data(), strSendData.length());
    iRet = SendWithTimeout(iFd,sendData, strSendData.length(), uiTimeWait);
    if(0 != iRet)
    {
        cout << "SendWithTimeout fail. iRet:" << iRet << endl;
        close(iFd);
        return -4;
    }
    else
    {
        cout << "SendWithTimeout success" << endl;
    }
    // 4、接收数据
    char recvData[2024];
    memset(recvData, 0 , sizeof(recvData));
    int iRecvLen = sizeof(recvData) - 1;
    iRet = RecvWithTimeout(iFd, recvData, iRecvLen, uiTimeWait);
    if(iRet < 0)
    {
        cout << "RecvWithTimeout fail" << endl;
        close(iFd);
        return -5;
    }
    cout << "recv len:" << iRecvLen << endl;
    cout << "recv msg:" << recvData << endl;
    return 0;
}
