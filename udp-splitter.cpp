#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <limits.h>

#include "argtable3/argtable3.h"
#include "platform.h"
#include "utilstring.h"
#include "daemonize.h"

#include "errlist.h"

const std::string progname = "udp-splitter";
#define DEF_CONFIG_FILE_NAME ".udp-splitter"
#define DEF_TIME_FORMAT      "%F %T"

class UDPSocket {
  public:
    int socket;
    struct addrinfo addr;
    struct sockaddr addrStorage;
    UDPSocket() 
      : socket(0)
    {
      memset(&addr, 0, sizeof(struct addrinfo));
      memset(&addrStorage, 0, sizeof(struct sockaddr));
    }

    UDPSocket(const UDPSocket &value) 
      : socket(value.socket)
    {
      memmove(&addrStorage, value.addr.ai_addr, sizeof(struct sockaddr));
      memmove(&addr, &value.addr, sizeof(struct addrinfo));
      addr.ai_addr = &addrStorage;
    }
    std::string toString() {
      std::stringstream ss;
      ss << socket << " " << inet_ntoa(((struct sockaddr_in *) addr.ai_addr)->sin_addr) 
        << ":" << ntohs(((struct sockaddr_in *) addr.ai_addr)->sin_port);
      return ss.str();
    }
};
  
class UDPSplitter {
  public:
    std::vector<UDPSocket> sockets;
    bool stopped;
    bool daemonize;
    int verbosity;
    std::string logfilename;
    std::ostream *logstream;

    UDPSplitter() :
      stopped(false), daemonize(false), verbosity(0),
      logfilename(""), logstream(&std::cerr)
    {
    }

    ~UDPSplitter() {
      if (logstream) {
        if (!logfilename.empty())
          delete logstream;
        logstream = NULL;
      }
    }

    std::string toString() {
      std::stringstream ss;
      for (std::vector<UDPSocket>::iterator it(sockets.begin()); it != sockets.end(); it++) {
        ss << it->toString() << std::endl;
      }
      return ss.str();
    }

    void closeDevices() {
      for (std::vector<UDPSocket>::iterator it(sockets.begin()); it != sockets.end(); it++) {
        close(it->socket);
      }
    }

    int openSocket(
      UDPSocket &retval,
      const char* address,
      const char* port
    ) {
      struct addrinfo hints;
      memset(&hints, 0, sizeof(hints));
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_DGRAM;
      hints.ai_protocol = IPPROTO_UDP;
      struct addrinfo *addr;
      
      int r = getaddrinfo(address, port, &hints, &addr);
      if (r != 0 || addr == NULL) {
          std::cerr << "get address info error " << errno << ": " << strerror(errno) << std::endl;
          return -1;
      }
      retval.socket = socket(addr->ai_family, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
      memmove(&retval.addr, addr, sizeof(struct addrinfo));
      if (logstream && verbosity > 2) {
        struct sockaddr_in *s = (struct sockaddr_in *) retval.addr.ai_addr;
        *logstream << inet_ntoa(s->sin_addr) << ":" << ntohs(s->sin_port) << std::endl;
      }

      freeaddrinfo(addr);
      
      if (retval.socket == -1) {
          std::cerr << "open socket error " << errno << ": " << strerror(errno) << std::endl;
          return -1;
      }
      return retval.socket;
    }

    int listenSocket(
      UDPSocket &retval,
      const char* address,
      const char* port
    ) {
      int sock = openSocket(retval, address, port);
      if (sock >= 0) {
        int r = bind(sock, retval.addr.ai_addr, retval.addr.ai_addrlen);
        if (r < 0) {
          std::cerr << "bind error " << errno << ": "<< strerror(errno) << std::endl;
          close(sock);
          return r;
        }
      }
      return sock;
    }

    bool addAddress(const std::string &address) {
      size_t pos = address.find(":");
      if (pos == std::string::npos)
        return false;
      int sock;
      UDPSocket s;

      std::string h(address.substr(0, pos));
      std::string p(address.substr(pos + 1));

      if (!sockets.size())
        sock = listenSocket(s, h.c_str(), p.c_str());
      else
        sock = openSocket(s, h.c_str(), p.c_str());
      if (sock >= 0)
        sockets.push_back(s);
      return (sock >= 0);
    }

    int receiveMain(
      struct sockaddr_in *remotePeerAddr,
      char *msg,
      size_t max_size,
      int max_wait_s
    ) {
      if (sockets.size() < 2) {
        return ERR_CODE_COMMAND_LINE;
      }
      fd_set s;
      FD_ZERO(&s);
      FD_SET(sockets[0].socket, &s);
      struct timeval timeout;
      timeout.tv_sec = max_wait_s;
      timeout.tv_usec = 0;
      int retval = select(sockets[0].socket + 1, &s, &s, &s, &timeout);
      if (retval == -1) {
          // select() set errno accordingly
          return -1;
      }
      if (retval > 0) {
          // our socket has data
          socklen_t addrlen = sizeof(struct sockaddr_in); 
          return recvfrom(sockets[0].socket, msg, max_size, 0, (struct sockaddr *) remotePeerAddr, &addrlen);
      }

      // our socket has no data
      errno = EAGAIN;
      return -1;
    }

    bool isPeerAddr(
      struct sockaddr_in *remotePeerAddr
    ) {
      // skip first socket
      std::vector<UDPSocket>::const_iterator it(sockets.begin());
      it++;
      for (; it != sockets.end(); it++) {
        struct sockaddr_in *s = (struct sockaddr_in *) it->addr.ai_addr;
        if (s->sin_family == remotePeerAddr->sin_family && 
          memcmp(&s->sin_addr, &remotePeerAddr->sin_addr, sizeof(struct in_addr)) == 0 && 
          s->sin_port == remotePeerAddr->sin_port
        ) {
          return true;
        }
      }
      return false;
    }

    int sendDown(
      const char *msg,
      size_t size
    ) {
      if (logstream  && verbosity > 2) {
        *logstream << "sendDown" << std::endl;
      }
      // skip first socket
      std::vector<UDPSocket>::const_iterator it(sockets.begin());
      it++;
      for (; it != sockets.end(); it++) {
        if (logstream && verbosity > 2) {
          struct sockaddr_in *s = (struct sockaddr_in *) it->addr.ai_addr;
          *logstream << "send down " << hexString(msg, size) 
            << " to " << inet_ntoa(s->sin_addr) << ":" << ntohs(s->sin_port) << std::endl;
        }

        size_t r = sendto(it->socket, msg, size, 0, it->addr.ai_addr, it->addr.ai_addrlen);
        if (r < 0) {
          return r;
        }
      }
      return 0;
    }

    int sendUp(
      const char *msg,
      size_t size
    ) {
      // skip first socket
      // size_t r = sendto(sockets[0].socket, msg, size, 0, addr.ai_addr, addr.ai_addrlen);
      if (logstream && verbosity > 2) {
        *logstream << "sendUp() not implemented yet " << std::endl;
      }
      size_t r = -1;
      return r;
    }

};

static UDPSplitter splitter;

static void done()
{
 // destroy and free all
  splitter.closeDevices();
  if (splitter.verbosity > 1)
    std::cerr << "UDP splitter closed gracefully" << std::endl;
  exit(0);
}

static void stop()
{
  splitter.stopped = true;
}

void signalHandler(int signal)
{
	switch(signal)
	{
	case SIGINT:
		std::cerr << MSG_INTERRUPTED << std::endl;
		stop();
    done();
		break;
	default:
		break;
	}
}

#ifdef _MSC_VER
// TODO
void setSignalHandler()
{
}
#else
void setSignalHandler()
{
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = &signalHandler;
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGHUP, &action, NULL);
}
#endif

time_t time_ms(int &ms) {
  struct timeval tp;
  gettimeofday(&tp, NULL);
  ms = tp.tv_usec / 1000;
  return tp.tv_sec;
}

/**
 * Parse command line
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int parseCmd
(
  UDPSplitter &splitter,
	int argc,
	char* argv[]
)
{
  // device path
  struct arg_str *a_address = arg_strn(NULL, NULL, "<host:port>", 2, 100, "source dest1 [dest2]..");
  struct arg_str *a_logfilename = arg_str0("l", "logfile", "<file>", "log file");
  struct arg_lit *a_daemonize = arg_lit0("d", "daemonize", "run daemon");
  struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
	struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_address, a_logfilename, a_daemonize, a_verbosity, a_help, a_end 
	};

	int nerrors;

	// verify the argtable[] entries were allocated successfully
	if (arg_nullcheck(argtable) != 0)
	{
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}
	// Parse the command line as defined by argtable[]
	nerrors = arg_parse(argc, argv, argtable);

  splitter.daemonize = a_daemonize->count > 0;
  splitter.verbosity = a_verbosity->count;

  if (a_logfilename->count) {
      splitter.logfilename = *a_logfilename->sval;
      splitter.logstream = new std::fstream(*a_logfilename->sval, std::ostream::out);
      if (!splitter.logstream || splitter.logstream->bad()) {
        std::cerr << ERR_INVALID_PAR_LOG_FILE  << std::endl;
        nerrors++;
        if (splitter.logstream) {
          delete splitter.logstream;
          splitter.logstream = NULL;
        }
      }
  } else {
    splitter.logstream = &std::cerr;
    splitter.logfilename = "";
  }

  if (!nerrors) {
    for (int i = 0; i < a_address->count; i++) {
      if (!splitter.addAddress(a_address->sval[i])) {
        nerrors++;
        break;
      }
      std::cerr << "{" <<std::endl << splitter.toString() << "}" << std::endl;
    }
  }
  if (splitter.sockets.size() < 2) {
    nerrors++;
  }

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors) {
		if (nerrors)
			arg_print_errors(stderr, a_end, progname.c_str());
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "UDP splitter" << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}

static void run()
{
	std::stringstream buf;
	int c = 0;
  char msg[1024];  // 51, 115, 222

  if (splitter.logstream && splitter.verbosity > 2) {
    for (std::vector<UDPSocket>::const_iterator it(splitter.sockets.begin()); it != splitter.sockets.end(); it++) {
      struct sockaddr_in *s = (struct sockaddr_in *) it->addr.ai_addr;
      *splitter.logstream << "socket: " <<  it->socket << ", address " << inet_ntoa(s->sin_addr) << ":" << ntohs(s->sin_port) << std::endl;
    }
  }

	while (!splitter.stopped) {
    struct sockaddr_in remotePeerAddress;
    int r = splitter.receiveMain(&remotePeerAddress, msg, sizeof(msg), 1);
    switch (r)
    {
    case ERR_CODE_COMMAND_LINE:
      std::cerr << "Error " << r << ": " << strerror_udpsplitter(r) << std::endl;
      break;
    case -1:  // timeout
      break;
    case 0:   // ?!!
      break;
    default:
      if (splitter.logstream) {
        *splitter.logstream << hexString(msg, r) << std::endl;
      }
      if (splitter.isPeerAddr(&remotePeerAddress)) {
        splitter.sendUp(msg, r);
      } else {
        splitter.sendDown(msg, r);
      }
      break;
    }
  }
}


int main(
  int argc,
	char* argv[]
) {
  if (parseCmd(splitter, argc, argv) != 0) {
    exit(ERR_CODE_COMMAND_LINE);  
  }
#ifdef _MSC_VER
#else  
  setSignalHandler();
#endif

	if (splitter.daemonize) {
		char wd[PATH_MAX];
		std::string progpath = getcwd(wd, PATH_MAX);	
		if (splitter.verbosity > 1)
			std::cerr << MSG_DAEMON_STARTED << progpath << "/" << progname << MSG_DAEMON_STARTED_1 << std::endl;
		OPENSYSLOG() 
 		Daemonize daemonize(progname, progpath, run, stop, done);
		// CLOSESYSLOG()
	} else {
		setSignalHandler();
		run();
		done();
	}
  return 0;
}
