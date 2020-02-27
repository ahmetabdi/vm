#include "hlapi/hlapi.h"
#include "utils/Logger.h"
#include "utils/minitrace.h"

#include "m0dular/utils/threading.h"
#include "m0dular/utils/pattern_scan.h"

#include <unistd.h> //getpid
#include <thread>
#include <atomic>
#include <csignal>
#include <numeric>
#include <thread>
#include <chrono>
#include <iostream>

#include <unistd.h>   //close
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>

//#include "PacketStructure.h"
#include "Signatures.h"
#include "server_shared.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

FILE* dfile;

static thread_t mainThread;
inline WinProcess *process;
inline WinProcess *destProcess;
inline uintptr_t unityPlayerBase;
inline bool running = true;

#if (LMODE() == MODE_EXTERNAL())

int main() {
    while (running) {
        char c = (char) getchar();

        if (c == 'Q')
            break;
    }

    return 0;
}
#endif

typedef std::chrono::high_resolution_clock Clock;

static bool sigscanFailed = false;

static WinContext *ctx;

static bool IsClosed( int sock ){
    char x;
    interrupted:
    ssize_t r = ::recv(sock, &x, 1, MSG_DONTWAIT|MSG_PEEK);
    if (r < 0) {
        switch (errno) {
            case EINTR:       goto interrupted;
            case EWOULDBLOCK: break; /* empty rx queue */
            case ETIMEDOUT:   break; /* recv timeout */
            case ENOTCONN:    break; /* not connected yet */
            case ECONNRESET:  break; /* connection reset by peer. (closed program) */
            default:          Logger::Log("Connection Closed. Code(%d)\n", errno);
        }
    }
    return r == 0;
}

static void* NewConnection( void *raw_NewSock ){
    Logger::Log("NEW_CONNECTION\n");
    ssize_t r;
    int sock = *(int*)raw_NewSock;
    free( raw_NewSock );

    Packet completion_packet{ };
    Packet response = Packet();
    
    uint8_t buffer[2555];

    Logger::Log("New Connection (%d). Waiting for message.\n", sock);
    
    while( !IsClosed(sock) && running ){
        r = recv( sock, (char*)&completion_packet, sizeof(Packet), 0 );
        
        if( r < 0 ){
            Logger::Log("Error receiving on open sock!(%d)\n", sock); // can happen when client alt-f4's
            break;
        } else {
            //Logger::Log("type: %i\n", completion_packet.header.type);
            switch(completion_packet.header.type){
                case PacketType::packet_copy_memory:
                    //Logger::Log("packet_copy_memory\n");
                    //if( !process ){
                    //    Logger::Log("You need to Open Process before you can read/write!\n");
                    //    break;
                    //}

                    //Logger::Log("src: 0x%X dst: 0x%X size: 0x%X\n", completion_packet.data.copy_memory.src_address, completion_packet.data.copy_memory.dest_address, completion_packet.data.copy_memory.size);
                    process->Read(completion_packet.data.copy_memory.src_address, &buffer, completion_packet.data.copy_memory.size);

                    //Logger::Log("src 0x%X dst 0x%X size 0x%X buffer 0x%X\n", completion_packet.data.copy_memory.src_address, completion_packet.data.copy_memory.dest_address, completion_packet.data.copy_memory.size, buffer);
                    destProcess->Write(completion_packet.data.copy_memory.dest_address, buffer, completion_packet.data.copy_memory.size);

                    response.header.type = PacketType::packet_completed;
                    response.data.completed.result = 0x1;

                    send( sock, &response, sizeof(Packet), 0);
                    break;
                case PacketType::packet_get_base_address:
                    //Logger::Log("packet_get_base_address\n");
                    break;
                case PacketType::packet_get_peb:
                    //Logger::Log("packet_get_peb\n");
                    ctx->processList.Refresh();

                    for (auto& i : ctx->processList) {
                        //printf("%i\t%s\n", i.proc.pid, i.proc.name);

                        if (!strcasecmp("nvidia.exe", i.proc.name)) {
                            printf("Found %i\t%s\n", i.proc.pid, i.proc.name);
                            destProcess = &i;
                        }

                        if (!strcasecmp("EscapeFromTark", i.proc.name) || !strcasecmp("FortniteClient", i.proc.name)) {
                            for (auto& o : i.modules) {
                                //printf("Module: %s\n", o.info.name);
                                if (!strcmp("UnityPlayer.dll", o.info.name) || !strcmp("FortniteClient-Win64-Shipping.exe", o.info.name)) {
                                    printf("Found %i\t%s\n", i.proc.pid, i.proc.name);
                                    process = &i;
                                    printf("Found name %s with base 0x%X and entry 0x%X and size 0x%X\n", o.info.name, o.info.baseAddress, o.info.entryPoint, o.info.sizeOfModule);
                                    unityPlayerBase = o.info.baseAddress;

                                    //PatternScan::FindPattern("48 8b 0d ? ? ? ? 48 85 c9 74 30 e8 ? ? ? ? 48 8b f8", o.info.baseAddress, o.info.baseAddress + o.info.sizeOfModule);
                                }
                            }
                        }
                    }

                    response.header.type = PacketType::packet_completed;
                    response.data.completed.result = unityPlayerBase;

                    send(sock, &response, sizeof(Packet), 0);
                    break;
                default:
                    Logger::Log("%i Unknown Request Type.\n", completion_packet.header.type);
                    break;
            }
        }
        //sleep(0.5);
    }
    Logger::Log("Connection Closed (%d)\n", sock);
    close(sock);


    return nullptr;
}

static void *MainThread(void *) {
    pid_t pid = getpid();
    int opt = true;
    int sock;
    int sockFlags;
    struct addrinfo hints = {};
    struct addrinfo *res = 0, *ai = 0, *ai_ipv4 = 0;
    int *newSock;

    Threading::InitThreads();


FILE* out = stdout;

#if (LMODE() == MODE_EXTERNAL())
    FILE *pipe = popen("pidof qemu-system-x86_64", "r");
    fscanf(pipe, "%d", &pid);
    pclose(pipe);
#else
    out = fopen("/tmp/testr.txt", "w");
    pid = getpid();
#endif

fprintf(out, "Using LMODE: %s\n", TOSTRING(LMODE));
fprintf(out, "Using KMOD_MEMMAP: %s\n", TOSTRING(KMOD_MEMMAP));

dfile = out;

#ifdef MTR_ENABLED
    Logger::Log("Initialize performance tracing...\n");
    mtr_init("/tmp/ReapProcessMemory.json");
    MTR_META_PROCESS_NAME("Reap");
#endif

    try {
        ctx = new WinContext( pid );
    } catch( VMException ex ){
        Logger::Log("VmRead Context Init failed(%d). Stopping.\n", ex.value);
        return nullptr;
    }

    Logger::Log("Main Loaded.\n");

    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo( "0.0.0.0", "27925", &hints, &res);

    for( ai = res; ai; ai = ai->ai_next ){
        if (ai->ai_family == PF_INET6) break;
        else if( ai->ai_family == PF_INET) ai_ipv4 = ai;
    }
    ai = ai ? ai : ai_ipv4;

    if( (sock = socket(ai->ai_family, SOCK_STREAM, 0)) == 0 ){
        Logger::Log("Failed to create socket\n");
        goto quit;
    }
    if( setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) ){
        Logger::Log("Failed to configure socket for multiple connections\n");
        goto quit;
    }
    if( (sockFlags = fcntl( sock, F_GETFL ) ) == -1 ){
        Logger::Log("Couldn't get socket flags!\n");
        goto quit;
    }
    // Set socket to non-blocking. This will prevent accept() from blocking
    // however read()/write() can now fail with error EWOULDBLOCK/EAGAIN(same value).
    if( fcntl( sock, F_SETFL, sockFlags | O_NONBLOCK ) == -1 ){
        Logger::Log("Couldn't set socket flags!\n");
        goto quit;
    }
    if( bind(sock, ai->ai_addr, ai->ai_addrlen) ){
        Logger::Log("Failed to bind socket\n");
        goto quit;
    }
    // Listen for up to 256 pending connections
    if( listen(sock, 256) < 0 ){
        Logger::Log("Failed to listen for connections\n");
        goto quit;
    }

    // Ignore broken pipe signal from send(). Default behavior is to end the program.
    signal(SIGPIPE, SIG_IGN);

    Logger::Log("Main Loop Started. Waiting For Connections...\n");

    newSock = (int*)malloc( sizeof(int) );

    while (running) {
        *newSock = accept(sock, 0, 0);

        if( *newSock == -1 ){
            if( errno == EWOULDBLOCK ){
                // No Pending Connections. Sleep for 5ms or so.

                //usleep( 1000 * 5 );
                continue;
            } else {
                Logger::Log("Error accepting Connection!\n");
                goto quit;
            }
        } else {
            Threading::StartThread(NewConnection, newSock, false); // might want to detach here
            newSock = (int*)malloc( sizeof(int) );
        }
    }


    quit:
    Logger::Log("Main Loop Ended.\n");
    delete ctx;
    running = false;

    Threading::FinishQueue(true);
    Threading::EndThreads();

#ifdef MTR_ENABLED
    mtr_flush();
    mtr_shutdown();
#endif

    Logger::Log("Main Ended.\n");

    return nullptr;
}

static void __attribute__((constructor)) Startup() {
    mainThread = Threading::StartThread(MainThread, nullptr, false);
}

static void __attribute__((destructor)) Shutdown() {
    Logger::Log("Unloading...");

    running = false;

    Threading::JoinThread(mainThread, nullptr);

    Logger::Log("Done\n");
}
