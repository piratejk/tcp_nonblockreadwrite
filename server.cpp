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
//    // 2、设置非阻塞socket
//    if(0 != SetNonBlcok(iFd))
//    {
//        cout << "set non block fail." << endl;
//        return -2;
//    }
    // 绑定端口
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(10000);
    addr.sin_addr.s_addr = INADDR_ANY;
    int iRet = bind(iFd, (sockaddr*)&addr, sizeof(addr));
    if(0 != iRet)
    {
        cout << "bind fail. errno: " << errno << ", error_msg: " << strerror(errno)<< endl;
        return -3;
    }
    iRet = listen(iFd, 10);
    if(iRet < 0)
    {
        cout << "listen fail. errno: " << errno << ", error_msg: " << strerror(errno)<< endl;
        return -4;
    }
    //sleep(10);

    while(true)
    {
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);
        int fd = accept(iFd, (sockaddr*)&clientAddr, &addrLen);
        if(fd < 0)
        {
            cout << "accept fail. errno: " << errno << ", error_msg: " << strerror(errno) << endl;
            return -4;
        }
        // 读数据
        char recvBuf[1024];
        memset(recvBuf, 0 , sizeof(recvBuf));
        int iRecvLen = recv(fd, recvBuf, sizeof(recvBuf), 0);
        if(iRecvLen <= 0)
        {
            cout << "recv error" << endl;
            close(fd);
            continue;
        }
        cout << "recv info: " << recvBuf << endl;
        // 写数据
        sleep(1);
        string strSendInfo = "hello, client";
        int iSendSize = send(fd, (void*)strSendInfo.data(), strSendInfo.length(), 0);
        if(iSendSize <= 0)
        {
            cout << "send fail" << endl;
            close(fd);
            continue;
        }
        cout << "send size: " << iSendSize << endl;
        close(fd);
    }

    return 0;
}
