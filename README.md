# Publish-virtual-machine-events-using-MQTT
本项目在linux环境下使用C语言开发，应对云计算场景中，计算节点和管理节点分离，当我们直接对计算节点进行操作时，管理节点没有任务感知。在这样的场景下，我们需要将对虚拟机的操作，以事件的形式发布给到管理节点，以便于上层进行记录，便于后续排障。

## libvirt简介
libvirt是目前使用最为广泛的对KVM虚拟机继续管理的工具和应用程序接口，而且一些常用的虚拟机管理工具（如virsh、virt-manager等）和云计算框架平台（如：OpenStack、ZStack等）都在底层使用libvirt的应用程序接口。
libvirt是为了更方便地管理平台虚拟化技术而设计的开放源代码的应用程序接口(libvirt API)、守护进程(libvirtd)和管理工具(virsh)，它不仅提供了对虚拟化客户机的管理，也提供了对虚拟化网络和存储的管理。
libvirt支持事件机制，在使用该机制注册之后，可以在发生特定的事件（如：domain的启动、暂停、恢复和停止等）时得到自己定义的一些通知。该功能由以virStream开头的一系列函数实现。

## MQTT简介
实现MQTT协议需要客户端和服务器端通讯完成，在通讯过程中，MQTT协议中有三种身份：发布者（ publisher ）、代理（broker）、订阅者（subscriber）。
其中，消息的发布者和订阅者都是客户端，消息代理是服务器，消息发布者可以同时是订阅者。
MQTT传输的消息分为：主题（Topic）和负载（payload）两部分：
（1）Topic：订阅的主题，即channel，频道；
（2）payload：消息的内容，发布者向订阅者发布的具体消息。

##TLS协议简介
本文将会以HTTPS为例，来介绍TLS协议原理。
### 2.1 HTTP为什么不安全？
在介绍TLS协议之前，先介绍一下HTTP协议存在的缺陷，以便于后续大家更好的了解TLS协议的设计思路。
1. 窃听风险
HTTP协议数据包在网络中以明文的形式传递，攻击者通过抓包软件捕获网络中的数据包，即可读到HTTP中的信息。
 

2. 篡改
攻击者（中间人）可以窃取server的信息，对报文内容进行篡改，然后发送给client。
 
 
3. 冒充
client以为在跟合法的网站在通信，实际上可能再跟一个钓鱼网站在通信。
 
HTTP协议之所以会存在上述问题，主要是因为HTTP协议没有对应用数据进行加密，没有完整性校验和身份验证功能。

### 2.2 网络安全的四个原则
机密性：网络中传递的数据，经过加密算法进行加密；就算中间人窃听，他也无法获取其中的内容；
完整性：指数据传输过程中，没有被篡改，就算被篡改，也能检测出来。
身份认证：能够确认对方的身份的合法性。
不可否认：对自己操作过的网络行为，不能否认。

### 2.3 HTTPS通信原理
以下例子为HTTP+TLSv1.2的握手和通信过程。
浏览器向服务器的443端口发起请求，发送报文为client hello报文，报文中请求携带了浏览器支持的加密算法和哈希算法。 
服务器收到client hello报文，选择浏览器和本身都支持的加密算法和哈希算法，回应server hello报文。同时，服务器会将自己的证书信息发送给浏览器，这里的数字证书可以是向某个可靠机构（CA）申请的，也可以是自制的（自签名证书），使用wireshark抓包软件，可以捕捉到certification、exchangeKey和hellodone报文。其中certification携带证书信息，hellodone表示第一阶段的握手完成。
注意，服务器证书中有服务器的签名信息，签名信息怎么来的呢？服务器使用事前协商好的哈希算法对证书进行哈希运行，得到哈希值A，然后使用密钥信息对哈希值A进行加密，得到密文A，这个密文A就是签名。
浏览器收到服务器的证书，于是进入到认证环节。 首先浏览器会从内置的证书列表中索引，找到服务器证书的颁发机构，如果没有找到，此时就会提示用户该证书是不是由权威机构颁发，是不可信任的。浏览器上就会显示证书不安全，由用户来选择是否信任。
如果查到了对应的机构，是可信的，则取出该机构颁发的公钥。用机构的证书公钥解密得到证书的内容和证书签名，内容包括网站的网址、服务器的公钥、证书的有效期等。
浏览器验证证书记录的网址是否和当前网址是一致的，不一致会提示用户。如果网址一致会检查证书有效期，证书过期了也会提示用户。这些都通过认证时，浏览器就可以安全使用证书中的网站（server）公钥了。
浏览器验证证书签名的合法性，使用服务器的公钥对签名进行解密，得到哈希值A。然后，浏览器使用事先协商好的哈希算法对证书进行哈希运行得到哈希B。对比哈希A和哈希B是否一致。如果一致，说明证书没有被篡改，是合法的。
 浏览器生成一个随机数R，并使用服务器的公钥对R进行加密，得到密文RC。浏览器将密文RC传送给服务器。服务器收到密文RC后，用自己的私钥解密得到R。
 后续交互应用层数据时，服务器会议R为密钥，使用事先协商好的对称加密算法，加密网页内容并传输给浏览器。浏览器以R为密钥，使用之前约定好的对称秘钥，解密网页，获取网页内容。
