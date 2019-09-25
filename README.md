# EventServer

#### 介绍
{**以下是码云平台说明，您可以替换此简介**
基于libevent,支持HTTP、HTTPS、TCP、WEBSOCKET协议的lua服务器框架,可以应用于web服务，游戏服务。}

#### 软件架构


依赖libevent(修改http.c）
依赖openssl
依赖luajit



#### 安装教程(linux)

1. cd build
2. cmake ../
3. make


#### 使用说明

cd 目标目录
vi server.cfg

ip="127.0.0.1"
port=i0
lua="main.lua"
https=1
httpip="192.168.2.217"
httpport=i443
timer=5


1. cp gameserver 目标目录
2. ./gameserver&


#### 参与贡献

1. Fork 本仓库
2. 新建 Feat_xxx 分支
3. 提交代码
4. 新建 Pull Request

