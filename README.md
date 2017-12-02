# QtWebApp
 QtWebApp is a HTTP server library in C++, based on Qt Framework.
 Fork from Stefan QtWebApp: 
 http://stefanfrings.de/qtwebapp/index-en.html

The main idea of this fork is to add Websockets support.  
Qt framework doesn't support direct  upgrade from QTcpSocket to QWebSocket sockets in the current moment. Therefore, I decided to copy QtWebsocket library in QtWebApp and to made necessary changes in library in order to support  websocket upgrade.
Example Demo1 has been updated to demonstrate WebSocket communication.
By default server is started in https secured mode on https://localhost:8080. 
In order to work correctly DEMO1 you need to copy directory /etc (located in Demo1 folder) in location where 
Demo1 executable will be located after the build.

You can go in browser on https://localhost:8080 and there is located a link to simple Websockets demo. 
P.S.  Your browser can give you a point where your conection isn't secured (becouse used certificate aren't signed by CA) and you need to skip this and continue. 
