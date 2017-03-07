# QtWebApp
 QtWebApp is a HTTP server library in C++, based on Qt Framework.
 Fork from Stefan QtWebApp: 
 http://stefanfrings.de/qtwebapp/index-en.html

The main idea of this fork is to add Websockets support.  
Qt framework doesn't support direct  upgrade from QTcpSocket to QWebSocket sockets in the current moment. Therefore, I decided to copy QtWebsocket library in QtWebApp and to made necessary changes in library in order to support  websocket upgrade.
Example Demo1 has been updated to demonstrate WebSocket communication.
By default server is started in https secured mode on https://localhost:8080. 

You can go in browser on https://localhost:8080 and there is located a link to simple Websockets demo. 
