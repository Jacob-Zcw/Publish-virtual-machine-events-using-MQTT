# Publish-virtual-machine-events-using-MQTT
本项目在linux环境下使用C语言开发，应对云计算场景中，计算节点和管理节点分离，当我们在计算节点上，对虚拟机进行操作时，管理节点是没有任何感知。这样的话，就不便于管理人员通过前端的任务记录，对出现的问题进行分析。为了让管理节点能够感知到计算节点发生的变化，当我们对虚拟机进行操作时，使用libvirt中的virEvent机制，监听事件信息。然后，使用MQTT协议，将监听到的event信息发布给管理节点。管理节点收到event信息，会根据event信息，生成任务记录，便于后续的问题排查。

## 1. 前言
网络技术不断发展，网络中存在的攻击行为也层出不穷，恶意攻击者作为中间人窃取网络信息，篡改网络信息，或者冒充合法服务器对合法用户实施攻击。因此，我们设计网络协议，和部署网络通信服务器时，必须重点考虑网络安全问题。在这样的背景下TLS协议运行而生，TLS协议工作在传输层和应用层之间，能够为上层应用提供数据加密，数据完整性验证和用户验证等功能，因此广泛的应用于网络通信中。

MQTT（Message Queuing Telemetry Transport，消息队列遥测传输协议），是一种基于发布/订阅（publish/subscribe）模式的轻量级协议，该协议构建于TCP/IP协议之上，MQTT最大优点在于，可以以极少的代码和有限的带宽，为连接远程设备提供实时可靠的消息服务。

该协议默认情况下，以明文的形式进行消息的发布。因此，在本章节中，将通过TLS协议对MQTT发布的信息进行加密，确保消息的安全传输，保证数据的保密性和数据保证性。

本项目最终目的是，当在计算节点对虚拟机进行操作时，即虚拟机的状态发生变化时，发布者(即计算节点)会以事件的形式，通知到订阅者(即，管理节点)，发布的事件信息通过TLS协议进行加密。

## 2. libvirt简介
libvirt是目前使用最为广泛的对KVM虚拟机继续管理的工具和应用程序接口，而且一些常用的虚拟机管理工具（如virsh、virt-manager等）和云计算框架平台（如：OpenStack、ZStack等）都在底层使用libvirt的应用程序接口。
libvirt是为了更方便地管理平台虚拟化技术而设计的开放源代码的应用程序接口(libvirt API)、守护进程(libvirtd)和管理工具(virsh)，它不仅提供了对虚拟化客户机的管理，也提供了对虚拟化网络和存储的管理。
libvirt支持事件机制，在使用该机制注册之后，可以在发生特定的事件（如：domain的启动、暂停、恢复和停止等）时得到自己定义的一些通知。该功能由以virStream开头的一系列函数实现。

## 3. MQTT简介
实现MQTT协议需要客户端和服务器端通讯完成，在通讯过程中，MQTT协议中有三种身份：发布者（ publisher ）、代理（broker）、订阅者（subscriber）。
其中，消息的发布者和订阅者都是客户端，消息代理是服务器，消息发布者可以同时是订阅者。
MQTT传输的消息分为：主题（Topic）和负载（payload）两部分：

Topic：订阅的主题，即channel，频道；

payload：消息的内容，发布者向订阅者发布的具体消息。

## 4. 实验环境
libvirt version: 4.5.0

mosquitto version 1.6.10

Linux version 4.19.90

gcc version 7.3.0

## 5. 运行
cd ./src

make

发布者服务器上：./libvirt_event_publish <代理服务器的主机名> <端口号>

订阅者服务器上：./libvirt_event_subscribe <代理服务器的主机名> <端口号>

在发布者上对虚拟机进行操作，例如：destroy open0610，start open0610。

订阅者会接收到发布者发布的虚拟机的event信息，例如：Receive a message topic1: 2022/7/11 15:57:54 event(5) occurred in the domain = < open0610 >。

## 6. 学习路线
a. 学习libvirt的作用，提供的API接口，主要学习virEvent和virConnectDomainEvent相关的API；

b. 学习MQTT的协议原理；

c. 学习如何建立长连接和如何使用TLS协议加密MQTT发布的消息；

## 7. 参考文献
我在CSDN上发表了三篇技术博客，分别介绍了libvirt的event机制和代码实现、mqtt协议原理和代码实现，以及基于TLS协议加密的虚拟机事件发布。

libvirt的event机制和代码实现：https://blog.csdn.net/Jacobsea/article/details/125616913

mqtt协议原理和代码实现：https://blog.csdn.net/Jacobsea/article/details/125613457

基于TLS协议加密的虚拟机事件发布：https://blog.csdn.net/Jacobsea/article/details/125681905

## 8. 其他
该项目还有待进一步的优化和升级，欢迎各位小伙伴提出建议~

