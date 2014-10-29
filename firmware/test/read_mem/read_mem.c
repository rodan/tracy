
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define STR_LEN 256

volatile sig_atomic_t keep_going = 1;

void catch_alarm (int sig)
{
    keep_going = 0;
    exit(EXIT_SUCCESS);
    //signal (sig, catch_alarm);
}

int main()
{
    int fd_dev;
    char buff;
    char *ttydevice;
    char str_temp[STR_LEN];

    signal (SIGALRM, catch_alarm);

    ttydevice = getenv("dev");
    if (ttydevice == NULL) {
        ttydevice = "/dev/ttyACM0";
    }

    /*
       stty -a -F /dev/ttyUSB0 # should look like

       speed 57600 baud; rows 0; columns 0; line = 0;
       intr = ^C; quit = ^\; erase = ^?; kill = ^U; eof = ^D; eol = <undef>;
       eol2 = <undef>; swtch = <undef>; start = ^Q; stop = ^S; susp = ^Z; rprnt = ^R;
       werase = ^W; lnext = ^V; flush = ^O; min = 1; time = 5;
       -parenb -parodd cs8 hupcl -cstopb cread clocal -crtscts
       ignbrk -brkint -ignpar -parmrk -inpck -istrip -inlcr -igncr -icrnl -ixon -ixoff
       -iuclc -ixany -imaxbel -iutf8
       -opost -olcuc -ocrnl -onlcr -onocr -onlret -ofill -ofdel nl0 cr0 tab0 bs0 vt0 ff0
       -isig -icanon -iexten -echo -echoe -echok -echonl -noflsh -xcase -tostop -echoprt
       -echoctl -echoke
     */

    snprintf(str_temp, STR_LEN, "stty -F %s 9600 cs8 raw ignbrk -onlcr -iexten -echo -echoe -echok -echoctl -echoke time 5", ttydevice);

    if (system(str_temp) == -1) {
        fprintf(stderr, "error: stty cannot be executed\n");
        return EXIT_FAILURE;
    }

    fd_dev = open(ttydevice, O_RDWR);
    if (fd_dev < 0) {
        fprintf(stderr, "error: cannot open %s\n", ttydevice);
        return EXIT_FAILURE;
    }

    // if pipe is used, I need more than an empty output
    setvbuf(stdout, NULL, _IONBF, 0);

    write(fd_dev, "r\r\n", 3);
    alarm(1);

    while (keep_going) {
        if (read(fd_dev, &buff, 1) == 1) {
            printf("%c", buff);
            alarm(1);
        }
    }

    return EXIT_SUCCESS;
}
