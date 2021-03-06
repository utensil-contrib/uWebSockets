/* this is just a test example */

#include <iostream>
#include <string>
#include <thread>
using namespace std;

#include "uWS.h"
using namespace uWS;

int connections = 0;
Server *worker, *server;

int main()
{
    try {
        // our listening server
        Server server(3000);
        server.onUpgrade([](FD fd, const char *secKey) {
            // transfer connection to one of our worker servers
            ::worker->upgrade(fd, secKey);
        });

        // our working server, does not listen
        Server worker(0);
        ::worker = &worker;
        ::server = &server;
        worker.onConnection([](Socket socket) {
            cout << "[Connection] clients: " << ++connections << endl;

            //socket.close();
            //socket.close(false, 1011, "abcd", 4);

            // test shutting down the server when two clients are connected
            // this should disconnect both clients and exit libuv loop
            if (connections == 2) {
                ::worker->broadcast("I'm shutting you down now", 25, TEXT);
                cout << "Shutting down server now" << endl;
                ::worker->close();
                ::server->close();
            }
        });

        worker.onMessage([](Socket socket, const char *message, size_t length, OpCode opCode) {
            socket.send((char *) message, length, opCode);
        });

        worker.onDisconnection([](Socket socket) {
            cout << "[Disconnection] clients: " << --connections << endl;

            static int numDisconnections = 0;
            numDisconnections++;
            cout << numDisconnections << endl;
            if (numDisconnections == 302) {
                cout << "Closing after Autobahn test" << endl;
                ::worker->close();
                ::server->close();
            }
        });

        // work on other thread
        thread *t = new thread([]() {
            ::worker->run();
        });

        // listen on main thread
        server.run();
        t->join();
        delete t;
    } catch (...) {
        cout << "ERR_LISTEN" << endl;
    }

    return 0;
}
