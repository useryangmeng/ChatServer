# ChatServer:集群聊天服务器
该集群聊天服务器可以实现个人聊天与群组聊天，并维护个人账号的信息，缓存离线消息。<br>项目使用nginx实现负载均衡环境，并且基于muduo网络库实现网络通信，使用redis的发布订阅功能实现集群服务器之间的通信，mysql存储对应的数据。

### 环境
<br>Ubuntu22.04.5 <br>gcc version 13.1.0

此项目需要nginx配置代理，其配置文件/usr/local/nginx/conf中增加配置：
```cpp
stream{
upstream MyServer{
                server 127.0.0.1:6000 weight=1 max_fails=3 fail_timeout=30s;//配置的服务，ip+端口，weight为权重，max_fails为心跳检测失败上线，fail_timeout为连接超时时间
                server 127.0.0.1:6002 weight=1 max_fails=3 fail_timeout=30s;
        }
        server{
                listen 8000;//nginx监听端口为8000
                proxy_connect_timeout 1s;//连接超时时间
                proxy_pass MyServer;//服务，代指上面MyServer
                tcp_nodelay on;
        }
}
```
