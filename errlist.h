#define OK                 				0
#define ERR_CODE_COMMAND_LINE		    -500
#define ERR_CODE_OPEN_DEVICE		    -501
#define ERR_CODE_CLOSE_DEVICE		    -502
#define ERR_CODE_BAD_STATUS		        -503
#define ERR_CODE_INVALID_PAR_LOG_FILE   -504

#define ERR_COMMAND_LINE        		"Wrong parameter(s)"
#define ERR_OPEN_DEVICE         		"Error open device "
#define ERR_CLOSE_DEVICE        		"Error open device "
#define ERR_BAD_STATUS          		"Bad status"
#define ERR_INVALID_PAR_LOG_FILE		"Invalid log file "

#define MSG_INTERRUPTED 				"Interrupted "
#define MSG_PG_CONNECTED        		"Connected"
#define MSG_PG_CONNECTING       		"Connecting..."
#define MSG_DAEMON_STARTED      		"Start daemon "
#define MSG_DAEMON_STARTED_1    		". Check syslog."
#define MSG_WS_TIMEOUT					"Web service time out"

const char *strerror_humandetector(int errcode);
