# 基于TCP的服务器和客户端程序
1.	服务器能够接受多个客户端的连接，并且能够同时与多个客户端通信    
2.	客户端可以上传某个指定文件名的文本文件到服务器，也可以下载某个指定文件名的文本文件   
3.	客户端能够根据用户输入下载或上传不同文件，如   
   down a.txt    
   up b.txt   
   用户输入”exit”时，客户端退出，服务器端与之通信的线程也退出；   
4.	如果客户端下载时指定的文件不存在，服务器应发信告知客户端。 
# Bug
1.  如果服务器的两个send函数几乎同时发送数据到socket中时,客户端recv函数会阻塞不返回,只有当服务器主动断开socket连接时才会返回,程序往下走.    


# Bug解决方法
1.  在两个send函数中间用sleep函数等待一段时间,不让send同时发送数据.    
2.  可以用互斥机制,在两个send之间,加入recv函数,只有当服务器接受到客户端确认接受信息时,才会执行下一个send函数.    