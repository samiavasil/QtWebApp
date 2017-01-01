# QtWebApp
 QtWebApp is a HTTP server library in C++, based on Qt Framework.
 Fork from Stefan QtWebApp: 
 http://stefanfrings.de/qtwebapp/index-en.html

The main idea of this fork is to add Websockets support. Because Qt framework  websockets implementation in current moment  doesn't allow direct  upgrade from TCP socket to websocket i decided to copy QWebsocket library in QtWebApp library and to made needed modifications.
